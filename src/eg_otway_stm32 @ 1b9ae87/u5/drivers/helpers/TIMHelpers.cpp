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
#include "stm32u5xx_hal_rcc.h"


namespace eg {


    // Note that the more advanced timers have multiple vectors.
#if defined(TIM6_BASE)
    static InterruptHandler g_tim5_irq;
#endif


    InterruptHandler* TIMHelpers::irq_handler(Tim tim)
    {
        switch (tim)
        {
#if defined(TIM5_BASE)
        case Tim::Tim5: return &g_tim5_irq;
#endif

        default: Error_Handler();
        }

        return nullptr;
    }


    void TIMHelpers::enable_clock(Tim tim)
    {
        switch (tim)
        {
            // These constants are defined in the CMSIS code rather than HAL.
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

        default: Error_Handler();
        }
    }

    uint32_t TIMHelpers::SimpleTimers::get_input_freq()
    {
        // for a simple clock the clock comes from apb1
        // if the apb prescaler is 0, the timer clock freq is the apb domain frequency
        // otherwise, the timer clock frequency is 2 x apb domain frequency

        uint32_t input_freq = HAL_RCC_GetPCLK1Freq();
        if (APBPrescTable[(RCC->CFGR2 & RCC_CFGR2_PPRE1) >> RCC_CFGR2_PPRE1_Pos] != 0)
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

        ClockConfig config {}
        ;
        config.prescaler = static_cast<uint32_t>(ceilf(ratio / 65536.0f)) - 1;
        config.period = static_cast<uint32_t>(roundf(ratio / float(config.prescaler + 1))) - 1;

        return config;
    }

#if defined(TIM5_BASE)
    extern "C" void TIM5_IRQHandler()
    {
        g_tim5_irq.call();
    }
#endif


} // namespace eg {
