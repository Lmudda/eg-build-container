/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once 
#include "SignalNArg.h"


namespace eg {


// This class is intended as an adapter for a Signal object which exposes only the 
// methods needed by a consumer to connect (and disconnect) callbacks. Though it 
// has never come up, it is possible that a consumer could abuse the Signal API and 
// call emit() directly. We don't really want that.
template <typename... Args>
class SignalProxy
{
public:
    using Handler = typename Signal<Args...>::Handler;

    explicit SignalProxy(Signal<Args...>& signal)
    : m_signal{signal}
    {
    }

    void connect(Handler handler, IEventLoop& loop = default_event_loop())
    {
        m_signal.connect(handler, loop);
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
    
    bool disconnect([[maybe_unused]] void* data)
    {
        // TODO_AC This does not seem to be implemented for Linux.
        //return m_signal.disconnect(data);
        return true;
    }

private:
    Signal<Args...>& m_signal;
};


} // namespace eg {


