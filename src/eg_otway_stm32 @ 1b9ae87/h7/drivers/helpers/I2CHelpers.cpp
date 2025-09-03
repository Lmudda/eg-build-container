/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "I2CHelpers.h"
#include "utilities/ErrorHandler.h"
#include "stm32h7xx_hal.h"


namespace eg {


#if defined(I2C1_BASE)
static InterruptHandler g_i2c1_ev_irq;
static InterruptHandler g_i2c1_er_irq;
#endif
#if defined(I2C2_BASE)
static InterruptHandler g_i2c2_ev_irq;
static InterruptHandler g_i2c2_er_irq;
#endif
#if defined(I2C3_BASE)
static InterruptHandler g_i2c3_ev_irq;
static InterruptHandler g_i2c3_er_irq;
#endif
#if defined(I2C4_BASE)
static InterruptHandler g_i2c4_ev_irq;
static InterruptHandler g_i2c4_er_irq;
#endif


I2C_TypeDef* I2CHelpers::instance(I2c i2c)
{
    return reinterpret_cast<I2C_TypeDef*>(i2c);
}


InterruptHandler* I2CHelpers::irq_handler(I2c i2c, IrqType type)
{
    switch (i2c)
    {
        #if defined(I2C1_BASE)
        case I2c::I2c1: return (type == IrqType::Event) ? &g_i2c1_ev_irq : &g_i2c1_er_irq;
        #endif
        #if defined(I2C2_BASE)
        case I2c::I2c2: return (type == IrqType::Event) ? &g_i2c2_ev_irq : &g_i2c2_er_irq;
        #endif
        #if defined(I2C3_BASE)
        case I2c::I2c3: return (type == IrqType::Event) ? &g_i2c3_ev_irq : &g_i2c3_er_irq;
        #endif
        #if defined(I2C4_BASE)
        case I2c::I2c4: return (type == IrqType::Event) ? &g_i2c4_ev_irq : &g_i2c4_er_irq;
        #endif

        default: Error_Handler();
    }

    return nullptr;
}


void I2CHelpers::irq_enable(I2c i2c, IrqType type, uint32_t priority)
{
    IRQn_Type irqn;

    switch (i2c)
    {
        #if defined(I2C1_BASE)
        case I2c::I2c1: irqn = (type == IrqType::Event) ? I2C1_EV_IRQn : I2C1_ER_IRQn; break;
        #endif
        #if defined(I2C2_BASE)
        case I2c::I2c2: irqn = (type == IrqType::Event) ? I2C2_EV_IRQn : I2C2_ER_IRQn; break;
        #endif
        #if defined(I2C3_BASE)
        case I2c::I2c3: irqn = (type == IrqType::Event) ? I2C3_EV_IRQn : I2C3_ER_IRQn; break;
        #endif
        #if defined(I2C4_BASE)
        case I2c::I2c4: irqn = (type == IrqType::Event) ? I2C4_EV_IRQn : I2C4_ER_IRQn; break;
        #endif

        default:
	        Error_Handler();
	        return;
    }    

    HAL_NVIC_SetPriority(irqn, priority, GlobalsDefs::DefSubPrio);
    HAL_NVIC_EnableIRQ(irqn);    
}


void I2CHelpers::enable_clock(I2c i2c, uint32_t clk_src)
{
    switch (i2c)
    {
        // These constants are defined in the CMSIS code rather than HAL.
        #if defined(I2C1_BASE)
        case I2c::I2c1: 
            __HAL_RCC_I2C1_CONFIG(clk_src);
            __HAL_RCC_I2C1_CLK_ENABLE();
            break;
        #endif
        #if defined(I2C2_BASE)
        case I2c::I2c2: 
            __HAL_RCC_I2C2_CONFIG(clk_src);
            __HAL_RCC_I2C2_CLK_ENABLE();
            break;
        #endif
        #if defined(I2C3_BASE)
        case I2c::I2c3: 
            __HAL_RCC_I2C3_CONFIG(clk_src);
            __HAL_RCC_I2C3_CLK_ENABLE();
            break;
        #endif
        #if defined(I2C4_BASE)
        case I2c::I2c4: 
            __HAL_RCC_I2C4_CONFIG(clk_src);
            __HAL_RCC_I2C4_CLK_ENABLE();
            break;
        #endif

        default: Error_Handler();
    }
}


void I2CHelpers::disable_clock(I2c i2c)
{
    switch (i2c)
    {
        // These constants are defined in the CMSIS code rather than HAL.
        #if defined(I2C1_BASE)
        case I2c::I2c1: 
            __HAL_RCC_I2C1_CLK_DISABLE();
            break;
        #endif
        #if defined(I2C2_BASE)
        case I2c::I2c2: 
            __HAL_RCC_I2C2_CLK_DISABLE();
            break;
        #endif
        #if defined(I2C3_BASE)
        case I2c::I2c3: 
            __HAL_RCC_I2C3_CLK_DISABLE();
            break;
        #endif
        #if defined(I2C4_BASE)
        case I2c::I2c4: 
            __HAL_RCC_I2C4_CLK_DISABLE();
            break;
        #endif

        default: Error_Handler();
    }
}


#if defined(I2C1_BASE)
extern "C" void I2C1_EV_IRQHandler()
{
    g_i2c1_ev_irq.call();
}


extern "C" void I2C1_ER_IRQHandler()
{
    g_i2c1_er_irq.call();
}
#endif


#if defined(I2C2_BASE)
extern "C" void I2C2_EV_IRQHandler()
{
    g_i2c2_ev_irq.call();
}


extern "C" void I2C2_ER_IRQHandler()
{
    g_i2c2_er_irq.call();
}
#endif


#if defined(I2C3_BASE)
extern "C" void I2C3_EV_IRQHandler()
{
    g_i2c3_ev_irq.call();
}


extern "C" void I2C3_ER_IRQHandler()
{
    g_i2c3_er_irq.call();
}
#endif


#if defined(I2C4_BASE)
extern "C" void I2C4_EV_IRQHandler()
{
    g_i2c4_ev_irq.call();
}


extern "C" void I2C4_ER_IRQHandler()
{
    g_i2c4_er_irq.call();
}
#endif


} // namespace eg {
