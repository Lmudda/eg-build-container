/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "ADCHelpers.h"
#include "utilities/ErrorHandler.h"
#include "drivers/helpers/GlobalDefs.h"


namespace eg {


#if defined(ADC1_BASE)
static InterruptHandler g_adc1_handler;
#endif
#if defined(ADC2_BASE)
static InterruptHandler g_adc2_handler;
#endif
#if defined(ADC3_BASE)
static InterruptHandler g_adc3_handler;
#endif


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
        // The following are only available on ADC3 (TODO: constrain this)
        case AdcChannel::ChannelVRef: return ADC_CHANNEL_VREFINT;
        case AdcChannel::ChannelTemp: return ADC_CHANNEL_TEMPSENSOR;
        case AdcChannel::ChannelVbat: return ADC_CHANNEL_VBAT;

        // These enumerators are not represented in the STM32H7 hardware. The common API in
        // IADCDriver.h needs to be more abstract, or refer to channels only through an integer
        // which is the underlying type of an enumeration in the platform-specific code.
        case AdcChannel::Channel20:
        case AdcChannel::Channel21:
        case AdcChannel::Channel22:
        case AdcChannel::Channel23:
            Error_Handler();
            break;
    }

    return ADC_CHANNEL_0;
}


void ADCHelpers::enable_clock(Adc adc, uint32_t clk_src)
{
    // For now we just always use the system clock for the ADCs.
    switch (adc)
    {
        #if defined(ADC1_BASE)
        case Adc::Adc1:
            __HAL_RCC_ADC_CONFIG(clk_src);
            __HAL_RCC_ADC12_CLK_ENABLE();
            break;
        #endif
        #if defined(ADC2_BASE)
        case Adc::Adc2:
            __HAL_RCC_ADC_CONFIG(clk_src);
            __HAL_RCC_ADC12_CLK_ENABLE();
            break;
        #endif
        #if defined(ADC3_BASE)
        case Adc::Adc3:
            __HAL_RCC_ADC_CONFIG(clk_src);
            __HAL_RCC_ADC3_CLK_ENABLE();
            break;
        #endif

        default:
            Error_Handler();
    }
}


InterruptHandler* ADCHelpers::irq_handler(Adc adc)
{
    switch (adc)
    {
        #if defined(ADC1_BASE)
        case Adc::Adc1: return &g_adc1_handler;
        #endif
        #if defined(ADC2_BASE)
        case Adc::Adc2: return &g_adc2_handler;
        #endif
        #if defined(ADC3_BASE)
        case Adc::Adc3: return &g_adc3_handler;
        #endif

        default:
            Error_Handler();
    }

    return nullptr;
}


void ADCHelpers::irq_enable(Adc adc, uint32_t priority)
{
    IRQn_Type irqn;

    switch (adc)
    {
        #if defined(ADC1_BASE)
        case Adc::Adc1: irqn = ADC_IRQn; break;
        #endif
        #if defined(ADC2_BASE)
        case Adc::Adc2: irqn = ADC_IRQn; break;
        #endif
        #if defined(ADC3_BASE)
        case Adc::Adc3: irqn = ADC3_IRQn; break;
        #endif

        default:
            Error_Handler();
	    return;
    }

    HAL_NVIC_SetPriority(irqn, priority, GlobalsDefs::DefSubPrio);
    HAL_NVIC_EnableIRQ(irqn);
}


uint32_t ADCHelpers::get_rank(uint8_t rank)
{
    switch (rank)
    {
        case 1:  return ADC_REGULAR_RANK_1;
        case 2:  return ADC_REGULAR_RANK_2;
        case 3:  return ADC_REGULAR_RANK_3;
        case 4:  return ADC_REGULAR_RANK_4;
        case 5:  return ADC_REGULAR_RANK_5;
        case 6:  return ADC_REGULAR_RANK_6;
        case 7:  return ADC_REGULAR_RANK_7;
        case 8:  return ADC_REGULAR_RANK_8;
        case 9:  return ADC_REGULAR_RANK_9;
        case 10: return ADC_REGULAR_RANK_10;
        case 11: return ADC_REGULAR_RANK_11;
        case 12: return ADC_REGULAR_RANK_12;
        case 13: return ADC_REGULAR_RANK_13;
        case 14: return ADC_REGULAR_RANK_14;
        case 15: return ADC_REGULAR_RANK_15;
        case 16: return ADC_REGULAR_RANK_16;

        // Anything else is an error
        default:
            Error_Handler();
            return ADC_REGULAR_RANK_1;
    }
}


#if defined(ADC1_BASE) || defined(ADC2_BASE)
extern "C" void ADC_IRQHandler()
{
    g_adc1_handler.call();
    g_adc2_handler.call();
}
#endif


#if defined(ADC3_BASE)
extern "C" void ADC3_IRQHandler()
{
    g_adc3_handler.call();
}
#endif


} // namespace eg {
