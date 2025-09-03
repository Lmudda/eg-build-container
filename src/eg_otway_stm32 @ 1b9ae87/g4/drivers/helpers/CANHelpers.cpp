/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "CANHelpers.h"
#include "utilities/ErrorHandler.h"
#include "stm32g4xx_hal.h"


namespace eg {


#if defined(FDCAN1_BASE)
static InterruptHandler g_fdcan1_it0_irq;
static InterruptHandler g_fdcan1_it1_irq;
#endif
#if defined(FDCAN2_BASE)
static InterruptHandler g_fdcan2_it0_irq;
static InterruptHandler g_fdcan2_it1_irq;
#endif


InterruptHandler* CANHelpers::irq_handler(Can can, IrqType type)
{
    switch (can)
    {
        #if defined(FDCAN1_BASE)
        case Can::FdCan1: return (type == IrqType::IT0) ? &g_fdcan1_it0_irq : &g_fdcan1_it1_irq;
        #endif
        #if defined(FDCAN2_BASE)
        case Can::FdCan2: return (type == IrqType::IT0) ? &g_fdcan2_it0_irq : &g_fdcan2_it1_irq;
        #endif

        default: Error_Handler();
    }

    return nullptr;
}


void CANHelpers::irq_enable(Can can, IrqType type, uint32_t priority)
{
    IRQn_Type irqn;

    switch (can)
    {
        #if defined(FDCAN1_BASE)
        case Can::FdCan1: 
            irqn = (type == IrqType::IT0) ? FDCAN1_IT0_IRQn : FDCAN1_IT1_IRQn;
            break;
        #endif
        #if defined(FDCAN2_BASE)
        case Can::FdCan2: 
            irqn = (type == IrqType::IT0) ? FDCAN2_IT0_IRQn : FDCAN2_IT1_IRQn;
            break;
        #endif

        default: 
            Error_Handler();
            return;
    }    

    HAL_NVIC_SetPriority(irqn, priority, GlobalsDefs::DefSubPrio);
    HAL_NVIC_EnableIRQ(irqn);    
}


void CANHelpers::enable_clock(Can can, uint32_t clk_src)
{
    switch (can)
    {
        #if defined(FDCAN1_BASE)
        case Can::FdCan1: 
            __HAL_RCC_FDCAN_CONFIG(clk_src);
            __HAL_RCC_FDCAN_CLK_ENABLE();
            break;
        #endif
        #if defined(FDCAN2_BASE)
        case Can::FdCan2: 
            // The FDCAN peripherals share an RCC clock. The HAL code 
            // counts the enables to manage later disables (we don't bother).
            __HAL_RCC_FDCAN_CONFIG(clk_src);
            __HAL_RCC_FDCAN_CLK_ENABLE();
            break;
        #endif

        default: Error_Handler();
    }
}


#if defined(FDCAN1_BASE)
extern "C" void FDCAN1_IT0_IRQHandler()
{
    g_fdcan1_it0_irq.call();
}


extern "C" void FDCAN1_IT1_IRQHandler()
{
    g_fdcan1_it1_irq.call();
}
#endif


#if defined(FDCAN2_BASE)
extern "C" void FDCAN2_IT0_IRQHandler()
{
    g_fdcan2_it0_irq.call();
}


extern "C" void FDCAN2_IT1_IRQHandler()
{
    g_fdcan2_it1_irq.call();
}
#endif


} // namespace eg {


