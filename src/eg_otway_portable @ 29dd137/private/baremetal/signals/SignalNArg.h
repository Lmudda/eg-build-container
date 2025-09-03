/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "SignalEvent.h"
#include "utilities/CriticalSection.h"
#include <type_traits>
#include <tuple>


namespace eg {



// Simple compile time test used to ensure that member functions used 
// for Signal callbacks/connections satisfy certain constraints:
// - Must return void
// - Either: All arguments must be passed by value (or const value)
// - Or:     All arguments must be passed by const reference
// Note that the member may optionally be marked as a const function. 
// It would be nice to relax this a little so that the rules apply to 
// each argument individually, but that's a little more template magic.
template <typename Callback, typename Class, typename... Args>
consteval bool is_valid_member_callback()
{
    // All arguments passed by value (or const value).
    // Value arguments are used in Bugaboo but are deprecated.
#ifdef OTWAY_ALLOW_SIGNAL_VALUE_ARGS
    using ValArgs  = void (Class::*)(Args...); 
    using ValArgsC = void (Class::*)(Args...) const; 
    auto vals  = std::is_same_v<ValArgs, Callback>;
    auto vals2 = std::is_same_v<ValArgsC, Callback>;
#else
    auto vals  = false;
    auto vals2 = false;
#endif

    // All arguments passed by const reference. 
    using RefArgs  = void (Class::*)(const Args&...); 
    using RefArgsC = void (Class::*)(const Args&...) const; 
    auto refs  = std::is_same_v<RefArgs, Callback>;
    auto refs2 = std::is_same_v<RefArgsC, Callback>;

    return vals || vals2 || refs || refs2;
}


// Simple compile time test used to ensure that free functions and static member 
// functions used for Signal callbacks/connections satisfy certain constraints:
// - Must return void
// - Either: All arguments must be passed by value (or const value)
// - Or:     All arguments must be passed by const reference
// It would be nice to relax this a little so that the rules apply to 
// each argument individually, but that's a little more template magic.
template <typename Callback, typename... Args>
consteval bool is_valid_function_callback()
{
    // All arguments passed by value (or const value).
    // Value arguments are used in Bugaboo but are deprecated.
#ifdef OTWAY_ALLOW_SIGNAL_VALUE_ARGS
    using ValArgs = void (*)(Args...); 
    auto vals = std::is_same_v<ValArgs, Callback>;
#else 
    auto vals = false;
#endif

    // All arguments passed by const reference. 
    using RefArgs = void (*)(const Args&...); 
    auto refs = std::is_same_v<RefArgs, Callback>;

    return vals || refs;
}


// This class implements a form of the Observer pattern and is somewhat similar in usage to a C# delegate
// (https://learn.microsoft.com/en-US/dotnet/csharp/programming-guide/delegates/using-delegates). The class
// holds a collection of connected callbacks which are invoked by calling either the call() or emit() method.
// - call(Args...) is a synchronous method which immediately invokes all the connected callback.
// - emit(Args...) is an asynchronous method which collaborates with one or more event loops to defer to the
//   invocation of the connected callbacks. This allows events to be easily marshalled between handler mode (ISRs)
//   and thread mode, and between threads.
template <typename... Args>
class Signal : public SignalBase
{
private:
    struct SignalLink
    {
        // This is the loop in which emit() posts events for this connection.
        IEventLoop* loop;

        // Pointer to an object when the slot is a member of some class, else NULL.
        void* obj;

        // Pointer to the intermediate function which calls the callback. This signature
        // makes it possible to call any of free functions, static member functions and
        // non-static member functions.
        using SlotFunc = void (*)(void* obj, const Args&...);
        SlotFunc slot_func = nullptr;

        void call(const Args&... args)
        {
            slot_func(obj, args...);
        }

        // We keep these objects in a linked list so that a signal may service any
        // number of connections.
        SignalLink* next_head;
        SignalLink* next_link;
    };

    // We rely on the fact that DummyLink and SignalLink<Args...> all have exactly the same
    // layout. This means we can use pool of DummyLink objects as an allocator. It also means that,
    // with some judicious use of dynamic_cast, we can move a lot of the linked list management for
    // connections into SignalBase, which avoids a lot of duplication in template instantiations.
    static_assert(sizeof(SignalLink) == sizeof(DummyLink), "Link object has unexpected size");
    static_assert((sizeof(Args) + ... + 0) <= Event::kMaxEventData, "Arguments too big for event");
    static_assert(sizeof...(Args) <= 2, "Signal supports only up to 2 callback arguments");
    static_assert(std::is_trivially_copyable_v<Event>, "Event must be copyable with memcpy (because FreeRTOS)");

    // This is intended to prevent qualified types being used as arguments for the Signal template.
    // For example: Signal<const T>, Signal<T*>, and Signal<const volatile T&&> are all blocked.
    // The arguments should all be unqualified structs or built-in types.
    static_assert((... && std::is_same_v<Args, std::remove_cvref_t<Args>>), "const, volatile and reference types are not allowed.");
    // Pointer arguments are used in Bugaboo but are deprecated.
#ifndef OTWAY_ALLOW_SIGNAL_POINTER_ARGS
    static_assert((... && !std::is_pointer_v<Args>), "Pointer arguments are not allowed.");
#endif

public:
    // Connect a non-static member function of a specific object.
    template <auto SlotFunc, typename Class>
    void* connect(Class* obj, IEventLoop& loop_ref = default_event_loop())
    {
        // Enforce the expected signature of the callback with a more helpful error message.
        //using ValidFunc = void (Class::*)(const Args&...);
        //static_assert(std::is_same_v<decltype(SlotFunc), ValidFunc>, "Callback function must have const& argument(s).");
        static_assert(is_valid_member_callback<decltype(SlotFunc), Class, Args...>(), "Callback function must have const& argument(s).");       

        IEventLoop* loop = &loop_ref;
        // this_event_loop() needs to be called before disabling interrupts. Why? FreeRTOS TLS?
        // This test seems redundant given that we are passing in a reference.
        if (!loop)
            loop = &this_event_loop();

        // Better to set up all the connections from a single thread during initialisation.
        CriticalSection cs;

        SignalLink* link  = static_cast<SignalLink*>(alloc_link());
        link->loop        = loop;
        link->obj         = obj;
        // TODO_AC Not using lambda here makes the stack trace a bit shorter and clearer. See impl below.
        //link->slot_func   = &Signal::invoke<SlotFunc, Class>;
        link->slot_func   = [](void* data, const Args&... args)
        {
            Class* obj = static_cast<Class*>(data);
            (obj->*SlotFunc)(args...);
        };
        return SignalBase::connect(link);
    }

    // Connect a non-member function or a static member function.
    template <void (*SlotFunc)(const Args&...)>
    void* connect(IEventLoop& loop_ref = default_event_loop())
    {
        // Enforce the expected signature of the callback with a more helpful error message.
        //using ValidFunc = void (*)(const Args&...);
        //static_assert(std::is_same_v<decltype(SlotFunc), ValidFunc>, "Callback function must have const& argument(s).");
        static_assert(is_valid_function_callback<decltype(SlotFunc), Args...>(), "Callback function must have const& argument(s).");       

        IEventLoop* loop = &loop_ref;
        // this_event_loop() needs to be called before disabling interrupts. Why? FreeRTOS TLS?
        // This test seems redundant given that we are passing in a reference.
        if (!loop)
            loop = &this_event_loop();

        // Better to set up all the connections from a single thread during initialisation.
        CriticalSection cs;

        SignalLink* link  = static_cast<SignalLink*>(alloc_link());
        link->loop        = loop;
        link->obj         = nullptr;
        link->slot_func   = []([[maybe_unused]] void* data, const Args&... args) 
        {
            SlotFunc(args...);
        };
        return SignalBase::connect(link);
    }

    // Make a synchronous (direct) call to the connected functions, if any.
    void call(const Args&... args) const
    {
        // Call all the connected methods - regardless of thread context.
        SignalLink* head = reinterpret_cast<SignalLink*>(m_head);
        while (head)
        {
            SignalLink* link = head;
            while (link)
            {
                link->call(args...);
                link = link->next_link;
            }

            head = head->next_head;
        }
    }

    // Make an asynchronous (delayed) call to the connected functions, if any.
    // Post an event containing the argument data to the scheduler...
    void emit(const Args&... args) const
    {
        DummyLink* link = m_head;
        while (link)
        {
            //Event must be constructed for each link head (i.e. recieving task)
            //so that a copy of the arguments is constructed for each
            //and therefore can be destructed for each dispatch safely
            Event event(*this);
            (event.pack(args), ...);

            link->loop->post(event);
            link = link->next_head;
        }
    }

    // ...unpack the argument data now that the scheduler has dispatched the event,
    // and make the delayed call to the connected functions, if any.
	virtual void dispatch(const Event& event) const override
    {
        // Get emit function for the active thread.
        IEventLoop* loop = &this_event_loop();

        SignalLink* link = reinterpret_cast<SignalLink*>(m_head);
        // Only dispatch to methods connected from the active thread.
        while (link && (link->loop != loop))
        {
            link = link->next_head;
        }

        // We deal with specific number of arguments because this makes the the code
        // a little more straightforward. It is possible to pack the arguments into
        // a std::tuple and then use std::apply() to unpack them, but it is not known how
        // expensive this is in terms of generated code. We deal with up to 2 argsuments
        // though in practice up to one is sufficient.
        if constexpr (sizeof...(Args) == 0)
        {
            // Now call methods connected from the active thread.
            while (link)
            {
                link->call();
                link = link->next_link;
            }
        }
        else if constexpr (sizeof...(Args) == 1)
        {
            using Arg = typename std::tuple_element<0, std::tuple<Args...>>::type;
            Arg arg;
            event.unpack(arg, 0);

            // Now call methods connected from the active thread.
            while (link)
            {
                link->call(arg);
                link = link->next_link;
            }
        }
        else if constexpr (sizeof...(Args) == 2)
        {
            using Arg1 = typename std::tuple_element<0, std::tuple<Args...>>::type;
            using Arg2 = typename std::tuple_element<1, std::tuple<Args...>>::type;
            Arg1 arg1;
            Arg2 arg2;
            event.unpack(arg1, 0);
            event.unpack(arg2, sizeof(Arg1));

            // Now call methods connected from the active thread.
            while (link)
            {
                link->call(arg1, arg2);
                link = link->next_link;
            }
        }
    }

// private:
//     template <auto SlotFunc, typename Class>
//     static void invoke(void* data, const Args&... args)
//     {
//         Class* obj = static_cast<Class*>(data);
//         (obj->*SlotFunc)(args...);
//     }
};


} // namespace eg {
