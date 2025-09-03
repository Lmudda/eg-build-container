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
#include "interfaces/IADCDriver.h"
#include "stm32h7xx_hal.h"
#include <cstdint>


namespace eg {


enum class Adc : uint32_t
{
    #if defined(ADC1_BASE)
    Adc1 = ADC1_BASE,
    #endif
    #if defined(ADC2_BASE)
    Adc2 = ADC2_BASE,
    #endif
    #if defined(ADC3_BASE)
    Adc3 = ADC3_BASE,
    #endif
};


enum class AdcClockPrescaler : uint32_t
{
    Div1   = ADC_CLOCK_ASYNC_DIV1,
    Div2   = ADC_CLOCK_ASYNC_DIV2,
    Div4   = ADC_CLOCK_ASYNC_DIV4,
    Div8   = ADC_CLOCK_ASYNC_DIV8,
    Div16  = ADC_CLOCK_ASYNC_DIV16,
    Div32  = ADC_CLOCK_ASYNC_DIV32,
    Div64  = ADC_CLOCK_ASYNC_DIV64,
    Div128 = ADC_CLOCK_ASYNC_DIV128,
    Div256 = ADC_CLOCK_ASYNC_DIV256,
};


struct ADCHelpers
{
    static uint32_t adc_chan_to_stm_adc_chan(AdcChannel channel);
    static void enable_clock(Adc adc, uint32_t clk_src);
    static InterruptHandler* irq_handler(Adc adc);
    static void irq_enable(Adc adc, uint32_t priority);
    static uint32_t get_rank(uint8_t rank);
};

	
} // namespace eg {
