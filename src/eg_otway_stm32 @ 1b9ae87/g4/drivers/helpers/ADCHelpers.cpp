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
        case AdcChannel::ChannelVRef: return ADC_CHANNEL_VREFINT;
        case AdcChannel::ChannelVbat: return ADC_CHANNEL_VBAT;

        // These enumerators are not represented in the STM32G4 hardware. The common API in
        // IADCDriver.h needs to be more abstract, or refer to channels only through an integer
        // which is the underlying type of an enumeration in the platform-specific code.
        case AdcChannel::Channel19:
        case AdcChannel::Channel20:
        case AdcChannel::Channel21:
        case AdcChannel::Channel22:
        case AdcChannel::Channel23:
        case AdcChannel::ChannelTemp:
            Error_Handler();
            break;
    }

    return ADC_CHANNEL_0;
}


void ADCHelpers::enable_clock(Adc adc)
{
    // For now we just always use the system clock for the ADCs.
    switch (adc)
    {
        #if defined(ADC1_BASE)
        case Adc::Adc1:
            __HAL_RCC_ADC12_CONFIG(RCC_ADC12CLKSOURCE_SYSCLK);
            __HAL_RCC_ADC12_CLK_ENABLE();
            break;
        #endif
        #if defined(ADC2_BASE)
        case Adc::Adc2:
            __HAL_RCC_ADC12_CONFIG(RCC_ADC12CLKSOURCE_SYSCLK);
            __HAL_RCC_ADC12_CLK_ENABLE();
            break;
        #endif
        #if defined(ADC3_BASE)
        case Adc::Adc3:
            __HAL_RCC_ADC12_CONFIG(RCC_ADC345CLKSOURCE_SYSCLK);
            __HAL_RCC_ADC345_CLK_ENABLE();
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
        case Adc::Adc1: irqn = ADC1_2_IRQn; break;
        #endif
        #if defined(ADC2_BASE)
        case Adc::Adc2: irqn = ADC1_2_IRQn; break;
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


#if defined(ADC1_BASE) || defined(ADC1_BASE)
extern "C" void ADC1_2_IRQHandler()
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
