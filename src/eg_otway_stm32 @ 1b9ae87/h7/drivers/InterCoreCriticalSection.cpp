/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/InterCoreCriticalSection.h"
#include "utilities/CriticalSection.h"
#include "logging/Assert.h"
#include "stm32h7xx_hal.h"


// We have some dependencies on settings in stm32h7xx_hal_conf.h
#ifndef HAL_HSEM_MODULE_ENABLED
#error HAL_HSEM_MODULE_ENABLED must be defined to use this driver
#endif


namespace eg {


InterCoreCriticalSection::InterCoreCriticalSection(const Config& conf)
: m_conf{conf}
, m_timer{1000u, eg::Timer::Type::OneShot}
, m_sem_mask{1u << conf.sem_id}
, m_state{State::Unlocked}
{
    HSEMHelpers::assert_id(m_conf.sem_id);
    HSEMHelpers::configure();

    // Set up interrupt handler for notification when semaphore is released,
    // but we don't initially want to receive interrupts, so disable the interrupt
    // for this specific semaphore ID.
    HSEMHelpers::irq_handler().connect<&InterCoreCriticalSection::on_released_interrupt>(this);
    HSEMHelpers::irq_enable(m_conf.core, m_conf.priority);
    HAL_HSEM_DeactivateNotification(m_sem_mask);

    // Configure timer callback.
    m_timer.on_update().connect<&InterCoreCriticalSection::on_timer>(this);
}


void InterCoreCriticalSection::lock(uint32_t timeout_ms)
{
    bool can_lock = false;
    {
        CriticalSection cs;
        can_lock = (m_state == State::Unlocked);
    }

    if (can_lock)
    {
        if (try_lock())
        {
            // Lock has been acquired so we're done.
            {
                CriticalSection cs;
                m_state = State::Locked;
            }
            m_on_locked.emit();
        }
        else
        {
            // Waiting for lock to be acquired, so start the timer
            // with the appropriate timeout.
            {
                CriticalSection cs;
                m_state = State::Pending;
            }
            m_timer.set_period(timeout_ms);
            m_timer.start();
        }
    }
    else
    {
        EG_ASSERT_FAIL("Recursive locking not supported");
    }
}


void InterCoreCriticalSection::unlock()
{
    CriticalSection cs;
    if (m_state == State::Locked)
    {
        HAL_HSEM_Release(m_conf.sem_id, 0u);
        m_state = State::Unlocked;
    }
    else
    {
        EG_ASSERT_FAIL("Attempting to unlock semaphore that hasn't been locked");
    }
}


SignalProxy<> InterCoreCriticalSection::on_locked()
{
    return eg::SignalProxy<>(m_on_locked);
}


SignalProxy<> InterCoreCriticalSection::on_lock_timeout()
{
    return eg::SignalProxy<>(m_on_lock_timeout);
}


void InterCoreCriticalSection::on_released_interrupt(const uint32_t &mask)
{
    // Does the interrupt relate to this semaphore?
    if ((mask & m_sem_mask) != 0u)
    {
        // This follows the procedure recommended in the section 11.3.7 of
        // the STM32H747 reference manual (RM0399 Rev 4).
        
        // Try to get the lock again. If this fails, it will setup for
        // notification when the semaphore is next released.
        if (try_lock())
        {
            // Lock has been acquired so we're done.
            m_timer.stop();
            {
                CriticalSection cs;
                m_state = State::Locked;
            }
            m_on_locked.emit();
        }
    }
}


void InterCoreCriticalSection::on_timer()
{
    // Arriving here means that the lock hasn't been acquired within
    // the timeout period. Timer is OneShot, so no need to stop it.
    // Need to be careful in case the software timer was firing while 
    // the lock was acquired under interrupt context.
    bool notify = false;
    {
        CriticalSection cs;
        if (m_state == State::Pending)
        {
            // Timeout condition, so disable notification interrupt
            // for this semaphore.
            notify = true;
            m_state = State::Unlocked;
            HAL_HSEM_DeactivateNotification(m_sem_mask);
        }
    }

    if (notify)
    {    
        m_on_lock_timeout.emit();
    }
}


bool InterCoreCriticalSection::try_lock()
{
    // This follows the procedure recommended in the section 11.3.7 of
    // the STM32H747 reference manual (RM0399 Rev 4).

    // Try to acquire the lock immediately
    bool locked = (HAL_HSEM_FastTake(m_conf.sem_id) == HAL_OK);
    if (!locked)
    {
        // Failed to lock, so start setting up to get notification
        // for when the semaphore is freed.
        // Clear pending semaphore interrupt status for the interrupt line.
        __HAL_HSEM_CLEAR_FLAG(m_sem_mask);

        // Try to get the lock again. If this succeeds then it means that
        // the semaphore was freed since the initial lock attempt.
        locked = (HAL_HSEM_FastTake(m_conf.sem_id) == HAL_OK);
        if (!locked)
        {
            // Enable the interrupt for this semaphore ID.
            HAL_HSEM_ActivateNotification(m_sem_mask);
        }
    }

    return locked;
}


} // namespace eg {
