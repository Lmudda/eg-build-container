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
    explicit SignalProxy(Signal<Args...>& signal)
    : m_signal{signal}
    {
    }
 
    // Connect a pointer to an object. An IEventLoop& is needed here because a 
    // pointer confuses the type deduction mechanism.
    template <auto SlotFunc, typename Class>
    void* connect(Class* obj, IEventLoop& loop_ref = default_event_loop())
    {
        return m_signal.template connect<SlotFunc, Class>(obj, loop_ref);
    }

    // Connect a non-member function or a static member function.
    template <void (*SlotFunc)(const Args&...)>
    void* connect(IEventLoop& loop_ref = default_event_loop())
    {
        return m_signal.template connect<SlotFunc>(loop_ref);
    }

    bool disconnect(void* data)
    {
        return m_signal.disconnect(data);
    }
    
private:
    Signal<Args...>& m_signal;
};


} // namespace eg {


