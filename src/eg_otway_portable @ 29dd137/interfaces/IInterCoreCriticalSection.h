/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "utilities/NonCopyable.h"
#include "signals/Signal.h"


namespace eg {


// Provides critical section functionality for communicating between cores. Used to lock
// access while writing to or reading from an area of shared memory. Notification functionality
// has been split out (see IInterCoreEventSource and IInterCoreEventSink).
class IInterCoreCriticalSection : private NonCopyable
{
    public:
        virtual ~IInterCoreCriticalSection() = default;
        virtual void lock(uint32_t timeout_ms) = 0;
        virtual void unlock() = 0;
        virtual SignalProxy<> on_locked() = 0;
        virtual SignalProxy<> on_lock_timeout() = 0;
};


} // namespace eg {
