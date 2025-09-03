/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <type_traits>
#include <cstdint>
#include <memory>
#include <new>


namespace eg {


// NOTE: This could/should be a simple wrapper for std::function in Linux version.


// This template is a simple lightweight version of something a bit like std::function, but
//used for callbacks in which the callee could be any callable object. This is always is 
// typically a free function or a member function of some class instance, but lambdas have
// been added for good measure. Lambdas need to be stored so they can be called later, for 
// which there is a limited buffer.

// Primary template. The sole function of this is to allow the partial specialisation 
// below to convert multiple template arguments into a function signature type.
template <typename Signature> class Callback {};


// Specialised for the callback signature. We generally expect the return and argument
// types to be values rather than references, but they could presumably be anything.
template <typename Ret, typename... Args>
class Callback<Ret(Args...)>
{
public:
    // Connect to a non-static member function of a specific object. The call is converted
    // into a no-capture lambda which is essentially a free function whose address we can take.   
    // e.g. Foo::Foo()           { m_timer.on_update().connect<&Foo::on_timer>(this); }
    //      void Foo::on_timer() { handle_event(ON_TIMER); }
    template <auto MemFunc, typename Class>
    void connect(Class* obj) noexcept
    {
        m_obj  = obj;
        m_slot = [](void* data, Args... args) -> Ret
        {
            Class* obj = static_cast<Class*>(data);
            return (obj->*MemFunc)(std::forward<Args>(args)...);
        };
    }

    // Connect to a lambda expression. The expression is copied into a buffer, so the size of 
    // captures is limited. This should not be a practical problem. The typical use case is to 
    // create a simple one liner instead of an explicit member function to do the same work.     
    // e.g. Foo::Foo() { m_timer.on_update().connect([this](){ handle_event(ON_TIMER); }); }
    template <typename Lambda>
    void connect(Lambda lambda) noexcept
    {
        static_assert(sizeof(Lambda) <= sizeof(m_lambda));
        static_assert(std::alignment_of_v<Lambda> <= LAMBDA_ALIGNMENT);

        // Placement new to copy the lambda into our buffer.
        m_obj = new (m_lambda) Lambda(lambda);
        m_slot = [](void* data, Args... args) -> Ret
        { 
            Lambda& lambda = *static_cast<Lambda*>(data);
            return lambda(std::forward<Args>(args)...);
        };
    }

    // Connect to a non-member function or a static member function.
    // e.g. void on_timer() { some_led.toggle(); }
    //      void foo()      { some_timer.on_update().connect<on_timer>(); }
    template <Ret (*FreeFunc)(Args...)>
    void connect() noexcept
    {
        m_obj  = nullptr;
        m_slot = [](void* data, Args... args) -> Ret 
        {
            return FreeFunc(std::forward<Args>(args)...);    
        };
    }

    // Possibly never used.
    void disconnect() noexcept
    {
        m_obj  = nullptr;
        m_slot = nullptr;
    }
    
    // Make a synchronous (direct) call to the connected functions, if any.
    // The result is used by ISRs to determine if an interrupt has been handled.
    Ret call(Args... args) const noexcept // terminate() if callee throws
    {
        return m_slot(m_obj, std::forward<Args>(args)...);
    }

    // Personally I prefer the explicit call() method as it is more searchable.
    Ret operator()(Args... args) const noexcept // terminate() if callee throws
    {
        return m_slot(m_obj, std::forward<Args>(args)...);
    }

private:
    // Potentially move these into template parameters, and then create one or more 
    // specialisations to capture particular values.        
    static constexpr uint16_t LAMBDA_ALIGNMENT = sizeof(uint64_t);  
    static constexpr uint16_t LAMBDA_MAX_SIZE  = 2 * sizeof(uint32_t);

    using SlotFunc = Ret (*)(void*, Args...);

    // Potentially move these members into a struct. Can then support multiple connections
    // with an array or list of structures.    
    void*    m_obj  = nullptr;
    SlotFunc m_slot = [](void*, Args...) { return Ret(); };

    // Buffer reserved for lambda connections. The lambda is copied into this buffer, 
    // and then called via a static template trampoline function. Typical use case is 
    // to capture the address of a long-lived application object such as a driver or 
    // finite state machine.
    alignas(LAMBDA_ALIGNMENT)
    uint8_t m_lambda[LAMBDA_MAX_SIZE];
};


} // namespace eg {
