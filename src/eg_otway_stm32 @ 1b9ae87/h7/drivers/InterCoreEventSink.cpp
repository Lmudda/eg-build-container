/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/InterCoreEventSink.h"
#include "stm32h7xx_hal.h"


// We have some dependencies on settings in stm32h7xx_hal_conf.h
#ifndef HAL_HSEM_MODULE_ENABLED
#error HAL_HSEM_MODULE_ENABLED must be defined to use this driver
#endif


namespace eg {


InterCoreEventSink::InterCoreEventSink(const Config& conf)
: m_conf{conf}
, m_sem_mask{1u << conf.sem_id}
{
    HSEMHelpers::assert_id(m_conf.sem_id);
    HSEMHelpers::configure();

    // Set up for notification when semaphore is released.
    // NB we will also receive notification when we release the semaphore.
    HSEMHelpers::irq_handler().connect<&InterCoreEventSink::on_interrupt>(this);
    HSEMHelpers::irq_enable(m_conf.core, m_conf.priority);
    HAL_HSEM_ActivateNotification(m_sem_mask);
}


SignalProxy<> InterCoreEventSink::on_signalled()
{
    return eg::SignalProxy<>{m_on_signalled};
}


void InterCoreEventSink::on_interrupt(const uint32_t &mask)
{
    if ((mask & m_sem_mask) != 0u)
    {
        // Enable notifications again
        HAL_HSEM_ActivateNotification(m_sem_mask);
        m_on_signalled.emit();
    }
}


} // namespace eg {
