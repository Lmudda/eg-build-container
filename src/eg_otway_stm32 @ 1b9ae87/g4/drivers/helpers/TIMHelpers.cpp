/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "TIMHelpers.h"
#include "utilities/ErrorHandler.h"
#include "stm32g4xx_hal.h"
#include <cmath>


namespace eg {


// Note that the more advanced timers have multiple vectors.
// TODO_AC Add config macros to remove the items we aren't using?
#if defined(TIM1_BASE)
static InterruptHandler g_tim1_break_irq;
static InterruptHandler g_tim1_update_irq;
static InterruptHandler g_tim1_trigger_irq;
static InterruptHandler g_tim1_common_irq;
static InterruptHandler g_tim1_capture_irq;
#endif
#if defined(TIM2_BASE)
static InterruptHandler g_tim2_irq;
#endif
#if defined(TIM3_BASE)
static InterruptHandler g_tim3_irq;
#endif
#if defined(TIM4_BASE)
static InterruptHandler g_tim4_irq;
#endif
#if defined(TIM6_BASE)
static InterruptHandler g_tim6_irq;
#endif
#if defined(TIM7_BASE)
static InterruptHandler g_tim7_irq;
#endif
#if defined(TIM8_BASE)
static InterruptHandler g_tim8_break_irq;
static InterruptHandler g_tim8_update_irq;
static InterruptHandler g_tim8_trigger_irq;
static InterruptHandler g_tim8_common_irq;
static InterruptHandler g_tim8_capture_irq;
#endif
#if defined(TIM15_BASE)
static InterruptHandler g_tim15_irq;
#endif
#if defined(TIM16_BASE)
static InterruptHandler g_tim16_irq;
#endif
#if defined(TIM17_BASE)
static InterruptHandler g_tim17_irq;
#endif
#if defined(TIM20_BASE)
static InterruptHandler g_tim20_break_irq;
static InterruptHandler g_tim20_update_irq;
static InterruptHandler g_tim20_trigger_irq;
static InterruptHandler g_tim20_common_irq;
static InterruptHandler g_tim20_capture_irq;
#endif


InterruptHandler* TIMHelpers::irq_handler(Tim tim, IRQType type)
{
    switch (tim)
    {
        #if defined(TIM1_BASE)
        case Tim::Tim1:
            switch (type)
            {
                case IRQType::Break:   return &g_tim1_break_irq;
                case IRQType::Update:  return &g_tim1_update_irq;
                case IRQType::Trigger: return &g_tim1_trigger_irq;
                case IRQType::Common:  return &g_tim1_common_irq;
                case IRQType::Capture: return &g_tim1_capture_irq;
                default: Error_Handler();
            }
        #endif
        #if defined(TIM2_BASE)
        case Tim::Tim2: return &g_tim2_irq;
        #endif
        #if defined(TIM3_BASE)
        case Tim::Tim3: return &g_tim3_irq;
        #endif
        #if defined(TIM4_BASE)
        case Tim::Tim4: return &g_tim4_irq;
        #endif
        #if defined(TIM6_BASE)
        case Tim::Tim6: return &g_tim6_irq;
        #endif
        #if defined(TIM7_BASE)
        case Tim::Tim7: return &g_tim7_irq;
        #endif
        #if defined(TIM8_BASE)
        case Tim::Tim8:
            switch (type)
            {
                case IRQType::Break:   return &g_tim8_break_irq;
                case IRQType::Update:  return &g_tim8_update_irq;
                case IRQType::Trigger: return &g_tim8_trigger_irq;
                case IRQType::Common:  return &g_tim8_common_irq;
                case IRQType::Capture: return &g_tim8_capture_irq;
                default: Error_Handler();
            }
        #endif
        #if defined(TIM15_BASE)
        case Tim::Tim15: return &g_tim15_irq;
        #endif
        #if defined(TIM16_BASE)
        case Tim::Tim16: return &g_tim16_irq;
        #endif
        #if defined(TIM17_BASE)
        case Tim::Tim17: return &g_tim17_irq;
        #endif
        #if defined(TIM20_BASE)
        case Tim::Tim20:
            switch (type)
            {
                case IRQType::Break:   return &g_tim20_break_irq;
                case IRQType::Update:  return &g_tim20_update_irq;
                case IRQType::Trigger: return &g_tim20_trigger_irq;
                case IRQType::Common:  return &g_tim20_common_irq;
                case IRQType::Capture: return &g_tim20_capture_irq;
                default: Error_Handler();
            }
        #endif

        default: Error_Handler();
    }

    return nullptr;
}


void TIMHelpers::irq_enable(Tim tim, IRQType type, uint32_t priority)
{
    IRQn_Type irqn;

    switch (tim)
    {
        #if defined(TIM1_BASE)
        case Tim::Tim1:
            switch (type)
            {
                case IRQType::Break:   irqn = TIM1_BRK_TIM15_IRQn; break;
                case IRQType::Update:  irqn = TIM1_UP_TIM16_IRQn; break;
                case IRQType::Trigger: irqn = TIM1_TRG_COM_TIM17_IRQn; break;
                case IRQType::Common:  irqn = TIM1_TRG_COM_TIM17_IRQn; break;
                case IRQType::Capture: irqn = TIM1_CC_IRQn; break;
                default: Error_Handler();
            }
        #endif
        #if defined(TIM2_BASE)
        case Tim::Tim2: irqn = TIM2_IRQn; break;
        #endif
        #if defined(TIM3_BASE)
        case Tim::Tim3: irqn = TIM3_IRQn; break;
        #endif
        #if defined(TIM4_BASE)
        case Tim::Tim4: irqn = TIM4_IRQn; break;
        #endif
        #if defined(TIM6_BASE)
        case Tim::Tim6: irqn = TIM6_DAC_IRQn; break;
        #endif
        #if defined(TIM7_BASE)
        case Tim::Tim7: irqn = TIM7_IRQn; break;
        #endif
        #if defined(TIM8_BASE)
        case Tim::Tim8:
            switch (type)
            {
                case IRQType::Break:   irqn = TIM8_BRK_IRQn; break;
                case IRQType::Update:  irqn = TIM8_UP_IRQn; break;
                case IRQType::Trigger: irqn = TIM8_TRG_COM_IRQn; break;
                case IRQType::Common:  irqn = TIM8_TRG_COM_IRQn; break;
                case IRQType::Capture: irqn = TIM8_CC_IRQn; break;
                default: Error_Handler();
            }
        #endif
        #if defined(TIM15_BASE)
        case Tim::Tim15: irqn = TIM1_BRK_TIM15_IRQn; break;
        #endif
        #if defined(TIM16_BASE)
        case Tim::Tim16: irqn = TIM1_UP_TIM16_IRQn; break;
        #endif
        #if defined(TIM17_BASE)
        case Tim::Tim17: irqn = TIM1_TRG_COM_TIM17_IRQn; break;
        #endif
        #if defined(TIM20_BASE)
        case Tim::Tim20:
            switch (type)
            {
                case IRQType::Break:   irqn = TIM20_BRK_IRQn; break;
                case IRQType::Update:  irqn = TIM20_UP_IRQn; break;
                case IRQType::Trigger: irqn = TIM20_TRG_COM_IRQn; break;
                case IRQType::Common:  irqn = TIM20_TRG_COM_IRQn; break;
                case IRQType::Capture: irqn = TIM20_CC_IRQn; break;
                default: Error_Handler();
            }
        #endif

        default: 
            Error_Handler();
            return;
    }    

    HAL_NVIC_SetPriority(irqn, priority, GlobalsDefs::DefSubPrio);
    HAL_NVIC_EnableIRQ(irqn);    
}


void TIMHelpers::enable_clock(Tim tim, uint32_t clk_src)
{
    // Only LPTIM1 seems to have a clock source selection option.
    switch (tim)
    {
        #if defined(TIM1_BASE)
        case Tim::Tim1:
            __HAL_RCC_TIM1_CLK_ENABLE();
            break;
        #endif
        #if defined(TIM2_BASE)
        case Tim::Tim2:
            __HAL_RCC_TIM2_CLK_ENABLE();
            break;
        #endif
        #if defined(TIM3_BASE)
        case Tim::Tim3:
            __HAL_RCC_TIM3_CLK_ENABLE();
            break;
        #endif
        #if defined(TIM4_BASE)
        case Tim::Tim4:
            __HAL_RCC_TIM4_CLK_ENABLE();
            break;
        #endif
        #if defined(TIM6_BASE)
        case Tim::Tim6:
            __HAL_RCC_TIM6_CLK_ENABLE();
            break;
        #endif
        #if defined(TIM7_BASE)
        case Tim::Tim7:
            __HAL_RCC_TIM7_CLK_ENABLE();
            break;
        #endif
        #if defined(TIM8_BASE)
        case Tim::Tim8:
            __HAL_RCC_TIM8_CLK_ENABLE();
            break;
        #endif
        #if defined(TIM15_BASE)
        case Tim::Tim15:
            __HAL_RCC_TIM15_CLK_ENABLE();
            break;
        #endif
        #if defined(TIM16_BASE)
        case Tim::Tim16:
            __HAL_RCC_TIM16_CLK_ENABLE();
            break;
        #endif
        #if defined(TIM17_BASE)
        case Tim::Tim17:
            __HAL_RCC_TIM17_CLK_ENABLE();
            break;
        #endif
        #if defined(TIM20_BASE)
        case Tim::Tim20:
            __HAL_RCC_TIM20_CLK_ENABLE();
            break;
        #endif

        default: Error_Handler();
    }
}


TIM_TypeDef* TIMHelpers::instance(Tim tim)
{
    return reinterpret_cast<TIM_TypeDef*>(tim);
}


uint32_t TIMHelpers::SimpleTimers::get_input_freq()
{
    // for a simple clock the clock comes from apb1
    // if the apb prescaler is 0, the timer clock freq is the apb domain frequency
    // otherwise, the timer clock frequency is 2 x apb domain frequency

    uint32_t input_freq = HAL_RCC_GetPCLK1Freq();
    if (APBPrescTable[(RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos] != 0)
    {
        input_freq <<= 1;
    }

    return input_freq;
}


TIMHelpers::ClockConfig TIMHelpers::SimpleTimers::calculate_clock_config(uint32_t output_freq)
{
    uint32_t input_freq = get_input_freq();

    if (input_freq < output_freq)
    {
        // Should never happen in practice.
        Error_Handler();
    }

    // this is the desired ratio between the two
    // find the smallest integer prescaler that will work
    // calculate the actual period with this integer prescaler
    // prescaler will never exceed 65535 and so is safe

    float ratio = float(input_freq) / float(output_freq);

    ClockConfig config{};
    config.prescaler = static_cast<uint32_t>(ceilf(ratio / 65536.0f)) - 1;
    config.period    = static_cast<uint32_t>(roundf(ratio / float(config.prescaler + 1))) - 1;
    return config;
}


#if defined(TIM1_BASE) || defined(TIM15_BASE)
extern "C" void TIM1_BRK_TIM15_IRQHandler()
{
    g_tim1_break_irq.call();
    g_tim15_irq.call();
}
#endif


#if defined(TIM1_BASE) || defined(TIM16_BASE)
extern "C" void TIM1_UP_TIM16_IRQHandler()
{
    g_tim1_update_irq.call();
    g_tim16_irq.call();

}
#endif


#if defined(TIM1_BASE) || defined(TIM17_BASE)
extern "C" void TIM1_TRG_COM_TIM17_IRQHandler()
{
    g_tim1_trigger_irq.call();
    g_tim1_common_irq.call();
    g_tim17_irq.call();
}
#endif


#if defined(TIM1_BASE)
extern "C" void TIM1_CC_IRQHandler()
{
    g_tim1_capture_irq.call();
}
#endif


#if defined(TIM2_BASE) 
extern "C" void TIM2_IRQHandler()
{
    g_tim2_irq.call();
}
#endif


#if defined(TIM3_BASE) 
extern "C" void TIM3_IRQHandler()
{
    g_tim3_irq.call();
}
#endif


#if defined(TIM4_BASE) 
extern "C" void TIM4_IRQHandler()
{
    g_tim4_irq.call();
}
#endif


#if defined(TIM6_BASE) 
extern "C" void TIM6_DAC_IRQHandler()
{
    g_tim6_irq.call();
    // Note this also needs to deal with DAC interrupts if we have any.
}
#endif


#if defined(TIM7_BASE) 
extern "C" void TIM7_IRQHandler()
{
    g_tim7_irq.call();
}
#endif


#if defined(TIM8_BASE) 
extern "C" void TIM8_BRK_IRQHandler()
{
    g_tim8_break_irq.call();
}
#endif


#if defined(TIM8_BASE) 
extern "C" void TIM8_UP_IRQHandler()
{
    g_tim8_update_irq.call();
}
#endif


#if defined(TIM8_BASE) 
extern "C" void TIM8_TRG_COM_IRQHandler()
{
    g_tim8_trigger_irq.call();
    g_tim8_common_irq.call();
}
#endif


#if defined(TIM8_BASE) 
extern "C" void TIM8_CC_IRQHandler()
{
    g_tim8_capture_irq.call();
}
#endif


#if defined(TIM20_BASE) 
extern "C" void TIM20_BRK_IRQHandler()
{
    g_tim20_break_irq.call();
}
#endif


#if defined(TIM20_BASE) 
extern "C" void TIM20_UP_IRQHandler()
{
    g_tim20_update_irq.call();
}
#endif


#if defined(TIM20_BASE) 
extern "C" void TIM20_TRG_COM_IRQHandler()
{
    g_tim20_trigger_irq.call();
    g_tim20_common_irq.call();
}
#endif


#if defined(TIM20_BASE) 
extern "C" void TIM20_CC_IRQHandler()
{
    g_tim20_capture_irq.call();
}
#endif


} // namespace eg {
