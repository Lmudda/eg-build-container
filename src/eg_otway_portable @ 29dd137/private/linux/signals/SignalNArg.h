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
#include <type_traits>
#include <functional>
#include <tuple>
#include <map>
#include <list>
#include <mutex>
#include <tuple>


namespace eg {


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
static_assert((sizeof(Args) + ... + 0) <= Event::MAX_EVENT_DATA, "Arguments too big for event");
static_assert(sizeof...(Args) <= 2, "Signal supports only up to 2 callback arguments");

public:
    using Handler = std::function<void(Args...)>;

    void connect(Handler handler, IEventLoop& loop = default_event_loop())
    {
        std::lock_guard lock{m_mutex};
        auto& handlers = m_connections[&loop];
        handlers.push_back(handler);
    }

    // This is present to make the API match the embedded version.
    // Connect a non-static member function.
    template <auto SlotFunc, typename Class>
    void* connect(Class* obj, IEventLoop& loop = default_event_loop())
    {
        Handler handler{[obj](Args... args) 
        {
            (obj->*SlotFunc)(args...);
        }};
        connect(handler, loop);
        return nullptr;
    }

    // This is present to make the API match the embedded version.
    // Connect a non-member function or a static member function.
    template <void (*SlotFunc)(const Args&...)>
    void* connect(IEventLoop& loop = default_event_loop())
    {
        Handler handler{[](Args... args) 
        {
            SlotFunc(args...);
        }};
        connect(handler, loop);
        return nullptr;
    }

    // Make a synchronous (direct) call to the connected functions, if any. All connected functions
    // will be invoked in the current thread.
    void call(const Args& ...args) const
    {
        // This lock should be redundant if no more handlers are connected after initialisation phase.
        //std::lock_guard lock{m_mutex};       
        for (const auto& [emitter, handlers]: m_connections)
        {
            for (const auto& handler: handlers)
            {
                handler(args...);
            }
        }
    }

    // Make an asynchronous (delayed) call to the connected functions, if any.
    // Post an Event containing the argument data for each emitter for which we
    // have handlers (this posts the Event to an EventLoop's queue).
    void emit(const Args& ...args) const
    {
        // This lock should be redundant if no more handlers are connected after initialisation phase.
        //std::lock_guard lock{m_mutex};        
        for (const auto& [loop, handlers]: m_connections)
        {
            Event event(*this);
            (event.pack(args), ... );
            loop->post(event);
        }
    }


    // Unpack the argument data now that the scheduler has dispatched the event,
    // and make the delayed call to the connected functions for the current thread,
    // if any.
	void dispatch(const Event& event) const override
    {
        // This lock should be redundant if no more handlers are connected after initialisation phase.
        //std::lock_guard lock{m_mutex};

        IEventLoop& loop = this_event_loop();
        if (m_connections.find(&loop) == m_connections.end())
        {
            // Error
            return;
        }
        const auto& handlers = m_connections.at(&loop);

        if constexpr (sizeof...(Args) == 0)
        {
            for (const auto& handler: handlers)
            {
                handler();
            }
        }
        else if constexpr (sizeof...(Args) == 1)
        {
            using Arg1 = typename std::tuple_element<0, std::tuple<Args...>>::type;
            Arg1 arg1;
            event.unpack(arg1);

            for (const auto& handler: handlers)
            {
                handler(arg1);
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

            for (const auto& handler: handlers)
            {
                handler(arg1, arg2);
            }
        }
    }

private:
    using Connections = std::map<IEventLoop*, std::list<Handler>>;
    Connections m_connections{};
    mutable std::mutex m_mutex{};
};


} // namespace eg {
