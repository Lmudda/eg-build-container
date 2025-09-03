/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "signals/InterruptHandler.h"
#include "GlobalDefs.h"
#include <cstdint>
#include "stm32h7xx.h"


namespace eg {


enum class Tim
{
    #if defined(TIM1_BASE)
    Tim1 = TIM1_BASE,
    #endif
    #if defined(TIM2_BASE)
    Tim2 = TIM2_BASE,
    #endif
    #if defined(TIM3_BASE)
    Tim3 = TIM3_BASE,
    #endif
    #if defined(TIM4_BASE)
    Tim4 = TIM4_BASE,
    #endif
    #if defined(TIM5_BASE)
    Tim5 = TIM5_BASE,
    #endif
    #if defined(TIM6_BASE)
    Tim6 = TIM6_BASE,
    #endif
    #if defined(TIM7_BASE)
    Tim7 = TIM7_BASE,
    #endif
    #if defined(TIM8_BASE)
    Tim8 = TIM8_BASE,
    #endif
    #if defined(TIM12_BASE)
    Tim12 = TIM12_BASE,
    #endif
    #if defined(TIM13_BASE)
    Tim13 = TIM13_BASE,
    #endif
    #if defined(TIM14_BASE)
    Tim14 = TIM14_BASE,
    #endif
    #if defined(TIM15_BASE)
    Tim15 = TIM15_BASE,
    #endif
    #if defined(TIM16_BASE)
    Tim16 = TIM16_BASE,
    #endif
    #if defined(TIM17_BASE)
    Tim17 = TIM17_BASE,
    #endif
};




struct TIMHelpers
{
    // Note that some vector are shared by two timers, or a timer and some other 
    // peripheral, such as DAC. 
    enum class IRQType
    {
        // Most timers have only a single interrupt vector
        Global, 
        // TIM1, TIM8 are advanced-control timers and have multiple interrupt vectors
        Break,
        Update,
        Trigger,
        Commutation,
        Capture
    };

    enum class ClockDomain
    {
        APB1,
        APB2
    };

    static InterruptHandler* irq_handler(Tim tim, IRQType type);
    static void irq_enable(Tim tim, IRQType type, uint32_t priority);
    static void enable_clock(Tim tim, uint32_t clk_src = 0);
    static TIM_TypeDef* instance(Tim tim);

    struct ClockConfig
    {
        uint32_t prescaler;
        uint32_t period;
    };

    struct SimpleTimers
    {
        static uint32_t get_input_freq(Tim tim);
        static ClockConfig calculate_clock_config(Tim tim, uint32_t output_freq);
        static ClockDomain get_clock_domain(Tim tim);
    }; 
};


} // namespace eg {
    


