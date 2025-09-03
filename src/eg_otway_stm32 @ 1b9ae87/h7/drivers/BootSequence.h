
/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "interfaces/IInterCoreEventSource.h"
#include "interfaces/IInterCoreEventSink.h"
#include <cstdint>


namespace eg {


// Manages the boot sequence between the Cortex-M7 and the Cortex-M4.
// On boot, the CM4 configures a hardware semaphore and goes to sleep
// by calling cm4_await_init(). Meanwhile, the CM7 waits for the CM4
//  to enter stop mode by calling cm7_ini_start(). The CM7 can then
// configure the system clock and other peripherals as required, and
// can signal the CM4 to wake by calling cm7_init_complete().
//
// Note that the IInterCoreEventSink argument to cm4_await_init()
// isn't used explicitly, but is a trick to force the caller to
// create it and therefore configure the underlying semaphore.
struct BootSequence
{
    static void cm7_init_start();
    static void cm7_init_complete(IInterCoreEventSource& source);
    static void cm4_await_init(IInterCoreEventSink& sink);
};


} // namespace eg {

