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
#if defined(OTWAY_EVENT_LOOP_WATER_MARK)
    // Retrieves the high water mark of the event loop.
    virtual uint16_t get_high_water_mark() const = 0;
#endif
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


struct DummyLink;


// Base class for all signals. This interface is used by Event to dispatch signals to slots.
class SignalBase : private NonCopyable
{
public:
    virtual ~SignalBase();
	virtual void dispatch(const Event& event) const = 0;
	bool disconnect(void* conn); 

    // Just added for a little link pool monitoring.
    static uint16_t pool_size();
    static uint16_t pool_free();
    
protected:    
    static void* alloc_link();
    static void  free_link(void* link);
	void* connect(void* conn);

protected:
    DummyLink* m_head{};
};


// Every signal declares a structure like this one. Its size should always be the same. 
// These objects form a doubly linked list of connections to signal handler functions.
// The size of the structure is 20-bytes. We create a pool of these objects from which 
// to allocate connections. The size of the pool can be tailored to the project to save 
// linker allocated but otherwise unused RAM. See MAX_SIGNAL_LINKS.
struct DummyLink
{
    // This is the loop in which to post events for this connection. 
    IEventLoop* loop;
    
    // Pointer to an object when the slot is a member of some class, else NULL.
    void* obj;
    union
    {
        // Pointer to the slot function when it is static or a non-member.
        void (*static_slot)(const uint32_t&);
        // Pointer to an intermediate trampoline function which encapsulates
        // the pointer-to-member stuff - instance of a static function template.
        void (*member_slot)(void* obj, const uint32_t&);
    };
    
    // We keep these objects in a linked list so that a signal may service any
    // number of connections.
    DummyLink* next_head;
    DummyLink* next_link;
};


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
// Speculative 


// // This is a simple handle class which represents a connection made to a Signal.
// // It is returned by Signal::connect(). Handle::disconnect() can be called to 
// // disconnect from the signal without having to explicitly remember the Signal 
// // instance and calling Signal::disconnect(). It is safe to copy and the return 
// // value of Signal::connect() can be safely ignored. 
// class Handle
// {
// public:
//     Handle() = default;    
//     Handle(SignalBase* signal, void* conn)
//     : m_signal{signal}
//     , m_conn{conn}
//     {        
//     }

//     void disconnect()
//     {
//         if (m_signal)
//             m_signal->disconnect(m_conn);
//     }
// private:
//     SignalBase* m_signal{};
//     void* m_conn{};
// };


// // This is a wrapper for a Handle which uses RAII to automatically disconnect 
// // from the Signal when the owner of the ScopedHandle goes out of scope. You can 
// // use this to store the return value from Signal::connect() in a member object. 
// // When the member is destroyed, Signal::disconnect() will be called. It is not 
// // possible to copy this object as that would result in double disconnection (though
// // the implementation makes that harmless).
// // 
// // Note that this idiom only really makes sense in an environment in which short-lived
// // objects make connections to Signals. I've never needed that for any embedded system.
// // It *might* make more sense for a Linux application, in which case it is probably 
// // worth dealing with relative lifetimes more fully using std::shared_ptr and std::weak_ptr.
// // It is possible, for example, that the Signal goes out of scope before the ScopedHandle.
// class ScopedHandle : private NonCopyable
// {
// public:
//     ScopedHandle() = default;
//     ScopedHandle(const Handle& handle)
//     : m_handle{handle}
//     {
//     }
//     ScopedHandle& operator=(const Handle& handle)
//     {        
//         m_handle.disconnect();
//         m_handle = handle;
//         return *this;
//     }
//     ~ScopedHandle()
//     {
//         m_handle.disconnect();
//     }

// private:
//     Handle m_handle{};
// };


} // namespace eg {


