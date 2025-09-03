/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/ADCDriver.h"
#include "helpers/ADCHelpers.h"
#include "signals/InterruptHandler.h"
#include "utilities/ErrorHandler.h"
#include "drivers/helpers/GlobalDefs.h"
#include "signals/Signal.h"
#include "stm32g4xx_hal.h"


// We have some dependencies on settings in stm32g4xx_hal_conf.h
#ifndef HAL_ADC_MODULE_ENABLED
#error HAL_ADC_MODULE_ENABLED must be defined to use this driver
#endif
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 0)
#error USE_HAL_ADC_REGISTER_CALLBACKS must be set to use this driver
#endif


namespace eg {


ADCDriver::ADCDriver(const Config& conf)
: m_conf{conf}
{
    ADCHelpers::enable_clock(m_conf.adc);

    // Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
    m_hadc.Self                          = this;
    m_hadc.Instance                      = reinterpret_cast<ADC_TypeDef*>(m_conf.adc);
    m_hadc.Init.ClockPrescaler           = ADC_CLOCK_ASYNC_DIV16;
    m_hadc.Init.Resolution               = ADC_RESOLUTION_12B;
    m_hadc.Init.GainCompensation         = 0;
    m_hadc.Init.ScanConvMode             = ADC_SCAN_DISABLE;
    m_hadc.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;
    m_hadc.Init.LowPowerAutoWait         = DISABLE;
    //m_hadc.Init.LowPowerAutoPowerOff     = DISABLE;
    m_hadc.Init.ContinuousConvMode       = DISABLE;
    m_hadc.Init.NbrOfConversion          = 1;
    m_hadc.Init.DiscontinuousConvMode    = DISABLE;
    m_hadc.Init.ExternalTrigConv         = ADC_SOFTWARE_START;
    m_hadc.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE;
    m_hadc.Init.DMAContinuousRequests    = DISABLE;
    //m_hadc.Init.TriggerFrequencyMode     = ADC_TRIGGER_FREQ_HIGH;
    m_hadc.Init.Overrun                  = ADC_OVR_DATA_PRESERVED;
    //m_hadc.Init.LeftBitShift             = ADC_LEFTBITSHIFT_NONE;
    //m_hadc.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
    m_hadc.Init.OversamplingMode         = DISABLE;
    m_hadc.Init.DataAlign                = ADC_DATAALIGN_RIGHT;
    if (HAL_ADC_Init(&m_hadc) != HAL_OK)
    {
        // Initialization Error
        Error_Handler();
    }

    // if (HAL_ADCEx_Calibration_Start(&m_hadc, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
    // {
    //     // Calibration Error
    //     Error_Handler();
    // }

    m_hadc.ConvCpltCallback = &ADCDriver::ConvCpltCallback;


    ADCHelpers::irq_enable(m_conf.adc, GlobalsDefs::DefPreemptPrio);
    ADCHelpers::irq_handler(m_conf.adc)->connect<&ADCDriver::on_interrupt>(this);
}


bool ADCDriver::start_read(AdcChannel adc_channel)
{
    ADC_ChannelConfTypeDef config{};
    config.Channel      = ADCHelpers::adc_chan_to_stm_adc_chan(adc_channel);
    config.Rank         = ADC_REGULAR_RANK_1;
    // TODO_AC Fix me. This driver has too many hard-coded parameters. 
    // It was written in a hurry. Just need to pull a few parameters into 
    // a configuration struct. Same for the constructor above. Should consider a redesign in which 
    // ADCChannel objects can queue reads in much the same way as I2C transfers are queued.
    // Example application:
    //     The 170MHz system clock reaches the ADC peripheral. 
    //     The /16 prescaler cuts this to 10.625Mhz. This the cycle time 94ns.
    //     The sample time needs to be at least 5us (for reasons). 92.5 cycles gives us 8.7us. 
    //     Would it make sense to use a higher prescaler?
    config.SamplingTime = LL_ADC_SAMPLINGTIME_92CYCLES_5;
    config.SingleDiff   = ADC_SINGLE_ENDED;
    config.OffsetNumber = ADC_OFFSET_NONE;
    config.Offset       = 0;
    if (HAL_ADC_ConfigChannel(&m_hadc, &config) == HAL_OK)
    {
        m_cur_channel = adc_channel;
        return HAL_ADC_Start_IT(&m_hadc) == HAL_OK;
    }
    return false;
}


void ADCDriver::on_interrupt()
{
    HAL_ADC_IRQHandler(&m_hadc);
}


SignalProxy<AdcChannel, uint32_t> ADCDriver::on_reading()
{
    return eg::SignalProxy<AdcChannel, uint32_t>{m_on_reading};
}


void ADCDriver::ConvCpltCallback()
{
    uint32_t adc_value = HAL_ADC_GetValue(&m_hadc);
    m_on_reading.emit(m_cur_channel, adc_value);
}


void ADCDriver::ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    auto handle = reinterpret_cast<ADCDriver::ADCHandle*>(hadc);
    handle->Self->ConvCpltCallback();
}


} // namespace eg
