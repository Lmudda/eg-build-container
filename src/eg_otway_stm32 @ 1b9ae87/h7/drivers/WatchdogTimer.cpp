/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/WatchdogTimer.h"
#include "logging/Assert.h"
#include "utilities/ErrorHandler.h"
#include <cstdint>


namespace eg {


IndependentWatchdog::IndependentWatchdog(IndWdg ind_wdg, uint16_t period_ms)
    : m_timer{period_ms, eg::Timer::Type::Repeating}
    , m_iwdg_handle{}
{
    // This information comes from the reference manual.
    constexpr uint32_t kLSIFrequencyKHz  = 32;
    constexpr uint32_t kMaxReload        = 4'095; // 0x0FFF
    constexpr uint32_t kMaxFactor        = 256;    

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
        case 4:   prescaler = 0b000; break;
        case 8:   prescaler = 0b001; break;
        case 16:  prescaler = 0b010; break;
        case 32:  prescaler = 0b011; break;
        case 64:  prescaler = 0b100; break;
        case 128: prescaler = 0b101; break;
        case 256: prescaler = 0b110; break;
        default:  Error_Handler();   break;
    }

    // Enable and configure the watchdog.
    m_iwdg_handle.Instance = reinterpret_cast<IWDG_TypeDef*>(ind_wdg);
    m_iwdg_handle.Init.Prescaler = prescaler;
    m_iwdg_handle.Init.Reload = reload;
    m_iwdg_handle.Init.Window = kMaxReload;
    if (HAL_IWDG_Init(&m_iwdg_handle) != HAL_OK)
    {
        Error_Handler();
    }

    // The software counter will reload the watchdog in plenty of time. 
    m_timer.on_update().connect<&IndependentWatchdog::on_iwdg_timer>(this);
    m_timer.set_period(period_ms / 2);
    m_timer.start();
}


void IndependentWatchdog::on_iwdg_timer()
{
    // This is called in thread context via an event generated in a timer interrupt.
    // The software timers are typically driven by SysTick.
    // If SysTick is set to the lowest interrupt priority (0xF) then this should ensure
    // that neither the thread context (which is the main context on bare metal) nor any
    // interrupt contexts have locked up.
    HAL_IWDG_Refresh(&m_iwdg_handle);
}


} // namespace eg {
