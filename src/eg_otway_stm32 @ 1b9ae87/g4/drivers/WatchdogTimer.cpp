/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/WatchdogTimer.h"
#include "timers/Timer.h"
#include "logging/Assert.h"
#include "stm32g4xx_hal.h"
#include <cstdint>


namespace eg {


namespace {


// This information comes from the reference manual.
constexpr uint32_t kLSIFrequencyKHz  = 32;
constexpr uint32_t kMaxReload        = 4'095; // 0x0FFF
constexpr uint32_t kMaxFactor        = 256;    

// The watchdog is operated with special key values for the 
// KR register which act as commands.
constexpr uint32_t kEnableWatchdog   = 0x0000'CCCC;
constexpr uint32_t kReloadCounter    = 0x0000'AAAA;
constexpr uint32_t kEnableWriteRegs  = 0x0000'5555;
//constexpr uint32_t kDisableWriteRegs = 0x0000'0000;


// There can be only one instance so make the timers file static. Could implement
// IndependentWatchdog as a lazily-initialised singleton but there seemed no benefit 
// over writing a simple start() method to get the ball rolling. 
Timer g_timer{1'000, eg::Timer::Type::Repeating};


void on_iwdg_timer()
{
    // This is called in thread context via an event generated in a timer interrupt.
    // The software timers are typicaly driven by SysTick.
    IWDG->KR = kReloadCounter;
}


} // namespace {}


void IndependentWatchdog::start(uint16_t period_ms)
{
    uint16_t factor = 4; 
    uint32_t reload = kLSIFrequencyKHz * period_ms / factor;
    while ((reload > kMaxReload) && (factor < kMaxFactor))
    {
        factor = factor << 1;
        reload = kLSIFrequencyKHz * period_ms / factor;
    }

    EG_ASSERT(period_ms >= 2, "IWDG period too short");
    EG_ASSERT(factor <  kMaxFactor, "IWDG prescaler too large");
    EG_ASSERT(reload <= kMaxReload, "IWDG reload value too large");

    // Lookup table straight from the reference manual. Could instead 
    // increment a counter in the loop since the mapping is simple:
    // factor = 1 << (prescaler + 2). 
    uint16_t prescaler = 0b000;
    switch (factor)
    {
        case 4:   prescaler = 0b000; 
        case 8:   prescaler = 0b001; 
        case 16:  prescaler = 0b010; 
        case 32:  prescaler = 0b011; 
        case 64:  prescaler = 0b100; 
        case 128: prescaler = 0b101; 
        case 256: prescaler = 0b110; 
    }

    // Enable and configure the watchdog.
    IWDG->KR   = kEnableWatchdog;
    IWDG->KR   = kEnableWriteRegs;
    IWDG->PR   = prescaler;
    IWDG->RLR  = reload;

    // Wait for the registers to be updated and then start.
    while (IWDG->SR != 0);
    IWDG->KR = kReloadCounter;

    // The software counter will reload the watchdog in plenty of time. 
    g_timer.on_update().connect<&on_iwdg_timer>();
    g_timer.set_period(period_ms / 2);
    g_timer.start();
}


} // namespace eg {


