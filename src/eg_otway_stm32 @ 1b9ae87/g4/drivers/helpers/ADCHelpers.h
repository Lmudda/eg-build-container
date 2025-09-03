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
#include "stm32g4xx_hal.h"
#include <cstdint>


namespace eg {


constexpr uint16_t kAdcMin14Bits = 0x0000;
constexpr uint16_t kAdcMax14Bits = 0x3FFF;


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


struct ADCHelpers
{
    static uint32_t adc_chan_to_stm_adc_chan(AdcChannel channel);
    static void enable_clock(Adc adc);
    static InterruptHandler* irq_handler(Adc adc);
    static void irq_enable(Adc adc, uint32_t priority);
};

	
} // namespace eg {
