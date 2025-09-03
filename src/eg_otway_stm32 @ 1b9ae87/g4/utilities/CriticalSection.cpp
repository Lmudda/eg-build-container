/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "utilities/CriticalSection.h"
#include "stm32g4xx_hal.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"


namespace eg { 


#if defined(OTWAY_TARGET_PLATFORM_FREERTOS)


// Are we currently running in an ISR context. FreeRTOS cares about this and we 
// have to call different APIs to do the same things. Something about the way it
// manages task switches.
//static bool is_irq() { return __get_IPSR() != 0U; }


void CriticalSection::platform_enter_critical()
{
    // if (is_irq())
    // {
    //     // This macro is supposed to be part of the FreeRTOS API but does not seem to exist 
    //     // for the CM4 port.
    //     //taskENTER_CRITICAL_FROM_ISR();
    // }
    // else
    // {
    //     taskENTER_CRITICAL();
    // }

    __disable_irq();
    __ISB();
    __DSB();
}


void CriticalSection::platform_exit_critical()
{
    // if (is_irq())
    // {
    //     // This macro is supposed to be part of the FreeRTOS API but does not seem to exist 
    //     // for the CM4 port.
    //     //taskEXIT_CRITICAL_FROM_ISR(m_status);
    // }
    // else
    // {
    //     taskEXIT_CRITICAL();
    // }

    __enable_irq();
    __ISB();
    __DSB();
}


#endif


#if defined(OTWAY_TARGET_PLATFORM_BAREMETAL) 


void CriticalSection::platform_disable_interrupts()
{
    // TODO_AC platform_disable_interrupts(): Improve this with inline assembly?
    __disable_irq();
    __ISB();
    __DSB();
}


void CriticalSection::platform_enable_interrupts()
{
    __enable_irq();
    __ISB();
    __DSB();
}


#endif


} // namespace eg { 
