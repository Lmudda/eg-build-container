/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "UARTHelpers.h"
#include "utilities/ErrorHandler.h"
#include "stm32u5xx_hal_rcc.h"
#include "stm32u5xx_hal.h"


namespace eg {


// Note that the more advanced timers have multiple vectors.
#if defined(USART2_BASE)
static InterruptHandler g_usart2_irq;
#endif
#if defined(USART3_BASE)
static InterruptHandler g_usart3_irq;
#endif


USART_TypeDef* UARTHelpers::instance(Uart uart)
{
    return reinterpret_cast<USART_TypeDef*>(uart);
}


InterruptHandler* UARTHelpers::irq_handler(Uart uart)
{
    switch (uart)
    {
        #if defined(USART2_BASE)
        case Uart::Usart2: return &g_usart2_irq;
        #endif
        #if defined(USART3_BASE)
        case Uart::Usart3: return &g_usart3_irq;
        #endif

        default: Error_Handler();
    }

    return nullptr;
}


void UARTHelpers::irq_enable(Uart uart)
{
    IRQn_Type irqn;

    switch (uart)
    {
        #if defined(USART2_BASE)
        case Uart::Usart2: irqn = USART2_IRQn; break;
        #endif
        #if defined(USART3_BASE)
        case Uart::Usart3: irqn = USART3_IRQn; break;
        #endif

    default: 
        Error_Handler();
        return;
    }    

    // For now we hard-code the priority. 
    HAL_NVIC_SetPriority(irqn, 0, 0);
    HAL_NVIC_EnableIRQ(irqn);    
}


void UARTHelpers::enable_clock(Uart uart)
{
    switch (uart)
    {
        // These constants are defined in the CMSIS code rather than HAL.
        #if defined(USART2_BASE)
        case Uart::Usart2: 
            __HAL_RCC_USART2_CLK_ENABLE();
            break;
        #endif
        #if defined(USART3_BASE)
        case Uart::Usart3: 
            __HAL_RCC_USART3_CLK_ENABLE();
            break;
        #endif

        default: Error_Handler();
    }
}


#if defined(USART2_BASE)
extern "C" void USART2_IRQHandler()
{
    g_usart2_irq.call();
}
#endif


#if defined(USART3_BASE)
extern "C" void USART3_IRQHandler()
{
    g_usart3_irq.call();
}
#endif


} // namespace eg {


