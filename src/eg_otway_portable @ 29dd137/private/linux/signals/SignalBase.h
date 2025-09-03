/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once 
#include <cstdint>
#include "utilities/NonCopyable.h"


namespace eg {


// All events posted to event queues are of this type.
class Event;


// Interface for all event loops which are used in conjunction with Signals.
class IEventLoop : private NonCopyable
{
public:
    // Unlikely to be required but good practice.
    virtual ~IEventLoop() {}
    // Post an event (usually to a queue) to be dispatched later.
    virtual void post(const Event& event) = 0;
    // Dispatch any pending events. Typically does not return.
    virtual void run() = 0;
};


// These methods must be implemented by the application to associated an 
// event loop instance with each thread used in the application. The purpose 
// of the indirection is to keep implementatin details of the event loops 
// away from Signals, which makes them more platform independenta and may 
// help with testing.
IEventLoop* this_event_loop_impl();
IEventLoop* default_event_loop_impl();

// These methods are implemented by Otway. They call the corresponding 
// xxx_impl methods and catch errors.
IEventLoop& this_event_loop();
IEventLoop& default_event_loop();


// Base class for all signals. This interface is used by Event to dispatch signals to slots.
class SignalBase : private NonCopyable
{
public:
    virtual ~SignalBase() = default;
	virtual void dispatch(const Event& event) const = 0;
};


} // namespace eg {
