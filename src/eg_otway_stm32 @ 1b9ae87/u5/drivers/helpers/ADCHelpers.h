/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "GlobalDefs.h"
#include <cstdint>
#include "stm32u5xx_hal.h"
#include "interfaces/IADCDriver.h"


namespace eg {

	constexpr uint16_t AdcMin14Bits  = 0u;
	constexpr uint16_t AdcMax14Bits  = 0x3fff;

    enum class Adc : uint32_t
    {
        #if defined(ADC1_BASE)
        Adc1 = ADC1_BASE,
        #endif
        #if defined(ADC4_BASE)
        Adc4 = ADC4_BASE,
        #endif
        #if defined(ADC12_BASE)
        Adc12 = Adc12_BASE,
        #endif
    };

	struct ADCHelpers
	{
		static uint32_t adc_chan_to_stm_adc_chan(AdcChannel channel);
		static void enable_clock(Adc adc);
        static void enable_vrefint();
	};

	
} // namespace eg {
