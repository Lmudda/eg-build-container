/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/CycleCounter.h"
#include "logging/Assert.h"
#include "stm32h7xx.h"


namespace eg {


CycleCounter::CycleCounter(uint32_t expiry_us)
{
    // Unlock access to the cycle counter register
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
#if (CORE_CM7)
    // Only exist on CM7, not required for CM4
    DWT->LAR = 0xC5ACCE55;
#endif
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    uint64_t expiry_cycles = (expiry_us * static_cast<uint64_t>(SystemCoreClock)) / 1'000'000u;
    // TODO: This is naive because the cycle counter might wrap while doing the check
    EG_ASSERT(expiry_cycles <= 0xFFFF'FFFFu, "expiry period too long");
    m_expiry_cycles = static_cast<uint32_t>(expiry_cycles);
}

uint32_t CycleCounter::start()
{
    m_start_cycle = DWT->CYCCNT;
    m_has_expired = false;
    return m_start_cycle;
}


uint32_t CycleCounter::stop()
{
    return DWT->CYCCNT;
}


bool CycleCounter::has_expired()
{
    if (!m_has_expired)
    {
        m_has_expired = ((DWT->CYCCNT - m_start_cycle) > m_expiry_cycles);
    }
    return m_has_expired;
}


} // namespace eg {


