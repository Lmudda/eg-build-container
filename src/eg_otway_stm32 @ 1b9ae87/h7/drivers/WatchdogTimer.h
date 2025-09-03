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
#include "timers/Timer.h"
#include "stm32h7xx_hal.h"
#include <cstdint>


// We have some dependencies on settings in stm32h7xx_hal_conf.h
#ifndef HAL_IWDG_MODULE_ENABLED
#error HAL_IWDG_MODULE_ENABLED must be defined to use this driver
#endif


namespace eg {


// The STM32H7 has two independent watchdogs, one for each core.
enum class IndWdg
{
    #if defined(IWDG1_BASE)
    IndWdg1 = IWDG1_BASE,
    #endif
    #if defined(IWDG2_BASE)
    IndWdg2 = IWDG2_BASE,
    #endif
};


// The IWDG timer has its own LSI timer which runs at a notional 32'000Hz. This can be prescaled to 
// change the granularity of the counter. The reload register can be set to determine the timeout 
// period in terms of prescaled ticks. The timeout in us is 1000000 * prescaler * reload / 32000.
// The shortest possible timeout is 125us; the longest is a bit over 32.7s (32'700'000us).  
//
// The implementation uses a software timer to reload the watchdog.
class IndependentWatchdog : private NonCopyable
{
public:
    // Constructing the watchdog starts it. It cannot then be stopped. 
    IndependentWatchdog(IndWdg ind_wdg, uint16_t period_ms);

private:
    void on_iwdg_timer();

private:
    eg::Timer m_timer;
    IWDG_HandleTypeDef m_iwdg_handle;
};


} // namespace eg {


