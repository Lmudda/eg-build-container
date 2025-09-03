/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "BootSequence.h"
#include "utilities/ErrorHandler.h"
#include "stm32h7xx_hal.h"


namespace eg {


void BootSequence::cm7_init_start()
{
    // Wait until CPU2 boots and enters stop mode or timeout
    int32_t timeout = 0xFFFF;
    while ((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) != RESET) && (timeout-- > 0)) ;
    if (timeout < 0)
    {
        Error_Handler();
    }

    // The following is useful if debugging both cores
    //while (__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) != RESET)
    //{
    //    asm("nop");
    //}
}


void BootSequence::cm7_init_complete(IInterCoreEventSource& source)
{
    // When system initialization is finished, Cortex-M7 will release Cortex-M4
    // by means of HSEM notification
    if (source.signal() == false)
    {
        Error_Handler();
    }

    // Wait until CPU2 wakes up from stop mode
    int32_t timeout = 0xFFFF;
    while ((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) == RESET) && (timeout-- > 0));
    if (timeout < 0)
    {
        Error_Handler();
    }
}


void BootSequence::cm4_await_init(IInterCoreEventSink& /*sink*/)
{
    // NB sink isn't used explicitly, but it has to have been created
    // (and therefore configured). So requiring it as part of this interface
    // is just a trick to force the caller to create it.
    HAL_PWREx_ClearPendingEvent();
    HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFE, PWR_D2_DOMAIN);
}


} // namespace eg {
