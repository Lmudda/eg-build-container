/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "ADCDriverBlocking.h"
#include "utilities/ErrorHandler.h"
#include "logging/Assert.h"


// We have some dependencies on settings in stm32h7xx_hal_conf.h
#ifndef HAL_ADC_MODULE_ENABLED
#error HAL_ADC_MODULE_ENABLED must be defined to use this driver
#endif
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 0)
#error USE_HAL_ADC_REGISTER_CALLBACKS must be set to use this driver
#endif


namespace eg {


ADCDriverBlocking::ADCDriverBlocking(const Config& conf)
: m_conf{conf}
, m_current_channel{static_cast<AdcChannel>(-1)}
, m_sampling_time{static_cast<ADCDriverBlocking::SamplingTime>(-1)}
{
    adc_init();

    // Calibration (offset and linearity, should only be done once).
    // NB the ADC voltage regulator must be stable before doing this, which
    // takes a maximum of 10us (so this is fine).
    if (HAL_ADCEx_Calibration_Start(&m_hadc, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED) != HAL_OK)
    {
        Error_Handler();
    };
}


void ADCDriverBlocking::adc_init()
{
    ADCHelpers::enable_clock(m_conf.adc, m_conf.adc_clk_src);

    // TODO: Sort out prescaler, probably want this to be a parameter.
    m_hadc.Self                          = this;
    m_hadc.Instance                      = reinterpret_cast<ADC_TypeDef*>(m_conf.adc);
    m_hadc.Init.ClockPrescaler           = static_cast<uint32_t>(m_conf.prescaler);
    m_hadc.Init.Resolution               = ADC_RESOLUTION_16B;
    m_hadc.Init.ScanConvMode             = ADC_SCAN_DISABLE;
    m_hadc.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;
    m_hadc.Init.LowPowerAutoWait         = DISABLE;
    m_hadc.Init.ContinuousConvMode       = DISABLE;
    m_hadc.Init.NbrOfConversion          = 1u;
    m_hadc.Init.DiscontinuousConvMode    = DISABLE;
    m_hadc.Init.NbrOfDiscConversion      = 0;
    m_hadc.Init.ExternalTrigConv         = ADC_SOFTWARE_START;
    m_hadc.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE;
    m_hadc.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
    m_hadc.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;
    m_hadc.Init.LeftBitShift             = ADC_LEFTBITSHIFT_NONE;
    m_hadc.Init.OversamplingMode         = DISABLE;
    m_hadc.Init.Oversampling.Ratio       = 1;
    if (HAL_ADC_Init(&m_hadc) != HAL_OK)
    {
        Error_Handler();
    }

    // Not using multimode (only applies to ADC1/ADC2)
    if ((m_conf.adc == eg::Adc::Adc1) || (m_conf.adc == eg::Adc::Adc2))
    {
        ADC_MultiModeTypeDef multimode{};
        multimode.Mode = ADC_MODE_INDEPENDENT;
        if (HAL_ADCEx_MultiModeConfigChannel(&m_hadc, &multimode) != HAL_OK)
        {
            Error_Handler();
        }
    }
}


void ADCDriverBlocking::configure_channel(AdcChannel channel, SamplingTime sampling_time)
{
    // TODO: Make this safe for use with multiple AnalogueInput instances.
    if (channel != m_current_channel && m_sampling_time != sampling_time)
    {
        m_current_channel = channel;

        // Configure ADC channel.
        ADC_ChannelConfTypeDef ch_init{};
        ch_init.Channel                = ADCHelpers::adc_chan_to_stm_adc_chan(channel);
        ch_init.Rank                   = ADC_REGULAR_RANK_1;
        ch_init.SamplingTime           = static_cast<uint32_t>(sampling_time);
        ch_init.SingleDiff             = ADC_SINGLE_ENDED;
        ch_init.OffsetNumber           = ADC_OFFSET_NONE;
        ch_init.Offset                 = 0u;
        ch_init.OffsetSignedSaturation = DISABLE;
        if (HAL_ADC_ConfigChannel(&m_hadc, &ch_init) != HAL_OK)
        {
            Error_Handler();
        }
    }
}


bool ADCDriverBlocking::read(uint32_t& result)
{
    // TODO: Make this safe for use with multiple AnalogueInput instances.
    bool success = true;
    if (HAL_ADC_Start(&m_hadc) == HAL_OK)
    {
        if (HAL_ADC_PollForConversion(&m_hadc, 2u) == HAL_OK)
        {
            result = m_hadc.Instance->DR;
        }
        else
        {
            success = false;
        }
    }
    else
    {
        success = false;
    }

    return success;
}


} // namespace eg {
