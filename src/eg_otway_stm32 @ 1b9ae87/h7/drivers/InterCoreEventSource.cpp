/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/InterCoreEventSource.h"
#include "drivers/helpers/HSEMHelpers.h"
#include "stm32h7xx_hal.h"


// We have some dependencies on settings in stm32h7xx_hal_conf.h
#ifndef HAL_HSEM_MODULE_ENABLED
#error HAL_HSEM_MODULE_ENABLED must be defined to use this driver
#endif


namespace eg {


InterCoreEventSource::InterCoreEventSource(uint8_t sem_id) : m_sem_id{sem_id}
{
    HSEMHelpers::assert_id(m_sem_id);
    HSEMHelpers::configure();
}


bool InterCoreEventSource::signal()
{
    // Generate the signal by locking and releasing the semaphore.
    bool signalled = false;
    if (HAL_HSEM_FastTake(m_sem_id) == HAL_OK)
    {
        HAL_HSEM_Release(m_sem_id, 0u);
        signalled = true;
    }

    return signalled;
}


} // namespace eg {
