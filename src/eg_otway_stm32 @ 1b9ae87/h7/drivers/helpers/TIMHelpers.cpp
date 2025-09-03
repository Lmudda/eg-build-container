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
#include "stm32h7xx_hal.h"
#include "logging/Assert.h"
#include <cmath>


namespace eg {


// Note that the more advanced timers have multiple vectors.
// TODO_AC Add config macros to remove the items we aren't using?
#if defined(TIM1_BASE)
static InterruptHandler g_tim1_break_irq;
static InterruptHandler g_tim1_update_irq;
static InterruptHandler g_tim1_trigger_irq;
static InterruptHandler g_tim1_commutation_irq;
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
#if defined(TIM5_BASE)
static InterruptHandler g_tim5_irq;
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
static InterruptHandler g_tim8_commutation_irq;
static InterruptHandler g_tim8_capture_irq;
#endif
#if defined(TIM12_BASE)
static InterruptHandler g_tim12_irq;
#endif
#if defined(TIM13_BASE)
static InterruptHandler g_tim13_irq;
#endif
#if defined(TIM14_BASE)
static InterruptHandler g_tim14_irq;
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



InterruptHandler* TIMHelpers::irq_handler(Tim tim, IRQType type)
{
    switch (tim)
    {
        #if defined(TIM1_BASE)
        case Tim::Tim1:
            switch (type)
            {
                case IRQType::Break:       return &g_tim1_break_irq;
                case IRQType::Update:      return &g_tim1_update_irq;
                case IRQType::Trigger:     return &g_tim1_trigger_irq;
                case IRQType::Commutation: return &g_tim1_commutation_irq;
                case IRQType::Capture:     return &g_tim1_capture_irq;
                default: Error_Handler();
            }
            break;
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
        #if defined(TIM5_BASE)
        case Tim::Tim5: return &g_tim5_irq;
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
                case IRQType::Break:       return &g_tim8_break_irq;
                case IRQType::Update:      return &g_tim8_update_irq;
                case IRQType::Trigger:     return &g_tim8_trigger_irq;
                case IRQType::Commutation: return &g_tim8_commutation_irq;
                case IRQType::Capture:     return &g_tim8_capture_irq;
                default: Error_Handler();
            }
            break;
        #endif
        #if defined(TIM12_BASE)
        case Tim::Tim12: return &g_tim12_irq;
        #endif
        #if defined(TIM13_BASE)
        case Tim::Tim13: return &g_tim13_irq;
        #endif
        #if defined(TIM14_BASE)
        case Tim::Tim14: return &g_tim14_irq;
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
                case IRQType::Break:       irqn = TIM1_BRK_IRQn; break;
                case IRQType::Update:      irqn = TIM1_UP_IRQn; break;
                case IRQType::Trigger:     irqn = TIM1_TRG_COM_IRQn; break;
                case IRQType::Commutation: irqn = TIM1_TRG_COM_IRQn; break;
                case IRQType::Capture:     irqn = TIM1_CC_IRQn; break;
                default:
	                Error_Handler();
	                return;
            }
            break;
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
        #if defined(TIM5_BASE)
        case Tim::Tim5: irqn = TIM5_IRQn; break;
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
                case IRQType::Break:       irqn = TIM8_BRK_TIM12_IRQn; break;
                case IRQType::Update:      irqn = TIM8_UP_TIM13_IRQn; break;
                case IRQType::Trigger:     irqn = TIM8_TRG_COM_TIM14_IRQn; break;
                case IRQType::Commutation: irqn = TIM8_TRG_COM_TIM14_IRQn; break;
                case IRQType::Capture:     irqn = TIM8_CC_IRQn; break;
                default:
	                Error_Handler();
	                return;
            }
            break;
        #endif
        #if defined(TIM12_BASE)
        case Tim::Tim12: irqn = TIM8_BRK_TIM12_IRQn; break;
        #endif
        #if defined(TIM13_BASE)
        case Tim::Tim13: irqn = TIM8_UP_TIM13_IRQn; break;
        #endif
        #if defined(TIM14_BASE)
        case Tim::Tim14: irqn = TIM8_TRG_COM_TIM14_IRQn; break;
        #endif
        #if defined(TIM15_BASE)
        case Tim::Tim15: irqn = TIM15_IRQn; break;
        #endif
        #if defined(TIM16_BASE)
        case Tim::Tim16: irqn = TIM16_IRQn; break;
        #endif
        #if defined(TIM17_BASE)
        case Tim::Tim17: irqn = TIM17_IRQn; break;
        #endif

        default:
	        Error_Handler();
	        return;
    }    

    HAL_NVIC_SetPriority(irqn, priority, GlobalsDefs::DefSubPrio);
    HAL_NVIC_EnableIRQ(irqn);    
}


void TIMHelpers::enable_clock(Tim tim, [[maybe_unused]] uint32_t clk_src)
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
        #if defined(TIM5_BASE)
        case Tim::Tim5:
            __HAL_RCC_TIM5_CLK_ENABLE();
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
        #if defined(TIM12_BASE)
        case Tim::Tim12:
            __HAL_RCC_TIM12_CLK_ENABLE();
            break;
        #endif
        #if defined(TIM13_BASE)
        case Tim::Tim13:
            __HAL_RCC_TIM13_CLK_ENABLE();
            break;
        #endif
        #if defined(TIM14_BASE)
        case Tim::Tim14:
            __HAL_RCC_TIM14_CLK_ENABLE();
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

        default: Error_Handler();
    }
}


TIM_TypeDef* TIMHelpers::instance(Tim tim)
{
    return reinterpret_cast<TIM_TypeDef*>(tim);
}



uint32_t TIMHelpers::SimpleTimers::get_input_freq(Tim tim)
{
    uint32_t input_freq = 0u;

    // The clock domain depends on the timer.
    // if the APB prescaler is 0, the timer clock frequence is the APB domain frequency
    // otherwise, the timer clock frequency is 2 x APB domain frequency
    switch (get_clock_domain(tim))
    {
        case TIMHelpers::ClockDomain::APB1:
            input_freq = HAL_RCC_GetPCLK1Freq();
            if (D1CorePrescTable[(RCC->D2CFGR & RCC_D2CFGR_D2PPRE1) >> RCC_D2CFGR_D2PPRE1_Pos] != 0)
            {
                input_freq <<= 1;
            }
            break;
        case TIMHelpers::ClockDomain::APB2:
            input_freq = HAL_RCC_GetPCLK2Freq();
            if (D1CorePrescTable[(RCC->D2CFGR & RCC_D2CFGR_D2PPRE2) >> RCC_D2CFGR_D2PPRE2_Pos] != 0)
            {
                input_freq <<= 1;
            }
            break;
        default:
            EG_ASSERT_FAIL("Unsupported clock domain");
            break;
    }

    return input_freq;
}


TIMHelpers::ClockConfig TIMHelpers::SimpleTimers::calculate_clock_config(Tim tim, uint32_t output_freq)
{
    uint32_t input_freq = get_input_freq(tim);

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


TIMHelpers::ClockDomain TIMHelpers::SimpleTimers::get_clock_domain(Tim tim)
{
    switch (tim)
    {
        #if defined(TIM1_BASE)
        case Tim::Tim1: return TIMHelpers::ClockDomain::APB2;
        #endif
        #if defined(TIM2_BASE)
        case Tim::Tim2: return TIMHelpers::ClockDomain::APB1;
        #endif
        #if defined(TIM3_BASE)
        case Tim::Tim3: return TIMHelpers::ClockDomain::APB1;
        #endif
        #if defined(TIM4_BASE)
        case Tim::Tim4: return TIMHelpers::ClockDomain::APB1;
        #endif
        #if defined(TIM5_BASE)
        case Tim::Tim5: return TIMHelpers::ClockDomain::APB1;
        #endif
        #if defined(TIM6_BASE)
        case Tim::Tim6: return TIMHelpers::ClockDomain::APB1;
        #endif
        #if defined(TIM7_BASE)
        case Tim::Tim7: return TIMHelpers::ClockDomain::APB1;
        #endif
        #if defined(TIM8_BASE)
        case Tim::Tim8: return TIMHelpers::ClockDomain::APB2;
        #endif
        #if defined(TIM12_BASE)
        case Tim::Tim12: return TIMHelpers::ClockDomain::APB1;
        #endif
        #if defined(TIM13_BASE)
        case Tim::Tim13: return TIMHelpers::ClockDomain::APB1;
        #endif
        #if defined(TIM14_BASE)
        case Tim::Tim14: return TIMHelpers::ClockDomain::APB1;
        #endif
        #if defined(TIM15_BASE)
        case Tim::Tim15: return TIMHelpers::ClockDomain::APB2;
        #endif
        #if defined(TIM16_BASE)
        case Tim::Tim16: return TIMHelpers::ClockDomain::APB2;
        #endif
        #if defined(TIM17_BASE)
        case Tim::Tim17: return TIMHelpers::ClockDomain::APB2;
        #endif
        default: EG_ASSERT_FAIL("Unsupported timer"); return TIMHelpers::ClockDomain::APB1;
    }
}


#if defined(TIM1_BASE)
extern "C" void TIM1_BRK_IRQHandler()
{
    g_tim1_break_irq.call();
}


extern "C" void TIM1_UP_IRQHandler()
{
    g_tim1_update_irq.call();
}


extern "C" void TIM1_TRG_COM_IRQHandler()
{
    g_tim1_trigger_irq.call();
    g_tim1_commutation_irq.call();
}


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


#if defined(TIM5_BASE)
extern "C" void TIM5_IRQHandler()
{
    g_tim5_irq.call();
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


#if defined(TIM8_BASE) || defined(TIM12_BASE)
extern "C" void TIM8_BRK_TIM12_IRQHandler()
{
    g_tim8_break_irq.call();
    g_tim12_irq.call();
}
#endif


#if defined(TIM8_BASE) || defined(TIM13_BASE)
extern "C" void TIM8_UP_TIM13_IRQHandler()
{
    g_tim8_update_irq.call();
    g_tim13_irq.call();
}
#endif


#if defined(TIM8_BASE) || defined(TIM14_BASE)
extern "C" void TIM8_TRG_COM_TIM14_IRQHandler()
{
    g_tim8_trigger_irq.call();
    g_tim8_commutation_irq.call();
    g_tim14_irq.call();
}
#endif


#if defined(TIM8_BASE)
extern "C" void TIM8_CC_IRQHandler()
{
    g_tim8_capture_irq.call();
}
#endif


#if defined(TIM15_BASE)
extern "C" void TIM15_IRQHandler()
{
    g_tim15_irq.call();
}
#endif


#if defined(TIM16_BASE)
extern "C" void TIM16_IRQHandler()
{
    g_tim16_irq.call();
}
#endif


#if defined(TIM17_BASE)
extern "C" void TIM17_IRQHandler()
{
    g_tim17_irq.call();
}
#endif


} // namespace eg {
