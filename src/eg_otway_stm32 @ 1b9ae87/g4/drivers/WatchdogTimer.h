/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "utilities/NonCopyable.h"
#include <cstdint>


namespace eg {


// The IWDG timer has its own LSI timer which runs at a notional 32'000Hz. This can be prescaled to 
// change the granularity of the counter. The reload register can be set to determine the timeout 
// period in terms of prescaled ticks. The timeout in us is 1000000 * prescaler * reload / 32000.
// The shortest possible timeout is 125us; the longest is a bit over 32.7s (32'700'000us).  
//
// The implementation uses a software timer to reload the watchdog.
struct IndependentWatchdog
{
    // The struct is really just to give this function a namespace. 
    // The watchdog cannot be stopped once started. 
    static void start(uint16_t period_ms);
};


} // namespace eg {


