/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "ADCHelpers.h"


namespace eg {

    uint32_t ADCHelpers::adc_chan_to_stm_adc_chan(AdcChannel channel)
    {
        switch (channel)
        {
        case AdcChannel::Channel0:    return ADC_CHANNEL_0;
        case AdcChannel::Channel1:    return ADC_CHANNEL_1;
        case AdcChannel::Channel2:    return ADC_CHANNEL_2;
        case AdcChannel::Channel3:    return ADC_CHANNEL_3;
        case AdcChannel::Channel4:    return ADC_CHANNEL_4;
        case AdcChannel::Channel5:    return ADC_CHANNEL_5;
        case AdcChannel::Channel6:    return ADC_CHANNEL_6;
        case AdcChannel::Channel7:    return ADC_CHANNEL_7;
        case AdcChannel::Channel8:    return ADC_CHANNEL_8;
        case AdcChannel::Channel9:    return ADC_CHANNEL_9;
        case AdcChannel::Channel10:   return ADC_CHANNEL_10;
        case AdcChannel::Channel11:   return ADC_CHANNEL_11;
        case AdcChannel::Channel12:   return ADC_CHANNEL_12;
        case AdcChannel::Channel13:   return ADC_CHANNEL_13;
        case AdcChannel::Channel14:   return ADC_CHANNEL_14;
        case AdcChannel::Channel15:   return ADC_CHANNEL_15;
        case AdcChannel::Channel16:   return ADC_CHANNEL_16;
        case AdcChannel::Channel17:   return ADC_CHANNEL_17;
        case AdcChannel::Channel18:   return ADC_CHANNEL_18;
        case AdcChannel::Channel19:   return ADC_CHANNEL_19;
        case AdcChannel::Channel20:   return ADC_CHANNEL_20;
        case AdcChannel::Channel21:   return ADC_CHANNEL_21;
        case AdcChannel::Channel22:   return ADC_CHANNEL_22;
        case AdcChannel::Channel23:   return ADC_CHANNEL_23;
        case AdcChannel::ChannelVRef: return ADC_CHANNEL_VREFINT;
        case AdcChannel::ChannelVbat: return ADC_CHANNEL_VBAT;
        case AdcChannel::ChannelTemp: return ADC_CHANNEL_TEMPSENSOR;
        }
        return 0u;
    }

    // Enable the VREFINT buffer which is read via Channel 0. It took some digging in the reference 
    // manual and source code to get this information. 
    // TODO_AC Does this need more work?
    void ADCHelpers::enable_vrefint()
    {
#if defined(ADC12_COMMON_BASE)
        // There is a specific clock in the RCC for the VREFINT buffer. 
        __HAL_RCC_VREF_CLK_ENABLE();
        // The VREFINT read has to be explicitly enabled inthe ADC common control register.
        ADC12_COMMON->CCR |= ADC_CCR_VREFEN;
#endif
    }

	void ADCHelpers::enable_clock(Adc adc)
	{
#if defined(ADC1_BASE)
        if (Adc::Adc1 == adc)
        {
            __HAL_RCC_ADC1_CLK_ENABLE();
        }
#endif
#if defined(ADC4_BASE)
        if (Adc::Adc4 == adc)
        {
            __HAL_RCC_ADC4_CLK_ENABLE();
        }
#endif
#if defined(ADC12_BASE)
        if (Adc::Adc1 == adc || Adc::Adc2 == adc)
        {
            __HAL_RCC_ADC12_CLK_ENABLE();
        }
#endif
    }
} // namespace eg {
