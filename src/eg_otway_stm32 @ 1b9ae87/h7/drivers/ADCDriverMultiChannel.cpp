/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "ADCDriverMultiChannel.h"
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


ADCDriverMultiChannel::ADCDriverMultiChannel(const Config& conf)
: m_conf{conf}
{
    EG_ASSERT(m_conf.channel_count <= 16, "ADC limited to a maximum of 16 channels");

    adc_init();
    dma_init();
    channel_init();

    // Calibration (offset and linearity, should only be done once).
    // NB the ADC voltage regulator must be stable before doing this, which
    // takes a maximum of 10us (so this is fine).
    if (HAL_ADCEx_Calibration_Start(&m_hadc, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED) != HAL_OK)
    {
        Error_Handler();
    };
}


uint8_t ADCDriverMultiChannel::get_resolution() const
{
    return 16u;
}


uint8_t ADCDriverMultiChannel::get_channel_count() const
{
    return m_conf.channel_count;
}


void ADCDriverMultiChannel::adc_init()
{
    ADCHelpers::enable_clock(m_conf.adc, m_conf.adc_clk_src);

    // TODO: Sort out prescaler, probably want this to be a parameter.
    m_hadc.Self                          = this;
    m_hadc.Instance                      = reinterpret_cast<ADC_TypeDef*>(m_conf.adc);
    m_hadc.Init.ClockPrescaler           = static_cast<uint32_t>(m_conf.prescaler);
    // This has to match the return value of get_resolution().
    m_hadc.Init.Resolution               = ADC_RESOLUTION_16B;
    // Want to convert in sequence mode from rank 1 to rank N.
    m_hadc.Init.ScanConvMode             = ADC_SCAN_ENABLE;
    m_hadc.Init.EOCSelection             = ADC_EOC_SEQ_CONV;
    m_hadc.Init.LowPowerAutoWait         = DISABLE;
    // Want single conversion of each channel rather than continously running as
    // we want to control when this is triggered.
    m_hadc.Init.ContinuousConvMode       = DISABLE;
    m_hadc.Init.NbrOfConversion          = m_conf.channel_count;
    m_hadc.Init.DiscontinuousConvMode    = DISABLE;
    m_hadc.Init.NbrOfDiscConversion      = 0;
    m_hadc.Init.ExternalTrigConv         = ADC_SOFTWARE_START;
    m_hadc.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE;
    // Not continuously running, so want oneshot DMA
    // (NB for continuously running, need circular mode with a pair of buffers and
    // copy out of the just-filled buffer when complete or half complete interrupt fires).
    m_hadc.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_ONESHOT;
    m_hadc.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;
    m_hadc.Init.LeftBitShift             = ADC_LEFTBITSHIFT_NONE;
    // Not using hardware oversampling. If required, there are additional parameters
    // to set in the Init.Oversampling struct.
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

    // Register for relevant callbacks
    m_hadc.ConvCpltCallback = &ADCDriverMultiChannel::ConvCpltCallback;
    m_hadc.ErrorCallback    = &ADCDriverMultiChannel::ErrorCallback;

    // Configure ADC interrupt
    ADCHelpers::irq_enable(m_conf.adc, m_conf.adc_prio);
    ADCHelpers::irq_handler(m_conf.adc)->connect<&ADCDriverMultiChannel::adc_isr>(this);
}


void ADCDriverMultiChannel::dma_init()
{
    DMAHelpers::enable_clock(m_conf.dma);

    m_hdma.Self                     = this;
    m_hdma.Instance                 = DMAHelpers::instance(m_conf.dma);
    m_hdma.Init.Request             = m_conf.dma_req;
    m_hdma.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    m_hdma.Init.PeriphInc           = DMA_PINC_DISABLE;
    m_hdma.Init.MemInc              = DMA_MINC_ENABLE;
    // Copying from 32-bit peripheral register to 32-bit target memory
    m_hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    m_hdma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    m_hdma.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    m_hdma.Init.MemBurst            = DMA_MBURST_SINGLE;
    m_hdma.Init.Mode                = DMA_NORMAL;
    m_hdma.Init.Priority            = DMA_PRIORITY_LOW;
    m_hdma.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&m_hdma) != HAL_OK)
    {
        Error_Handler();
    }

    // Connect the DMA channel to the ADC object - this is a HAL thing.
    m_hadc.DMA_Handle = &m_hdma;
    m_hdma.Parent = &m_hadc;

    DMAHelpers::irq_enable(m_conf.dma, m_conf.dma_prio);
    DMAHelpers::irq_handler(m_conf.dma)->connect<&ADCDriverMultiChannel::dma_isr>(this);
}


void ADCDriverMultiChannel::channel_init()
{
    // Configure the selected channels.
    for (uint8_t i = 0u; i < m_conf.channel_count; i++)
    {
        auto& channel_conf = m_conf.channel_config[i];

        // Configure pin.
        // Some pins are special (PA0, PA1, PC2, PC3) and have an analogue switch.
        // See section 12.3.13 of the STM32H7 reference manual RM0399.
        if ((channel_conf.port == eg::Port::PortA) && (channel_conf.pin == eg::Pin::Pin0))
        {
            HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA0, SYSCFG_SWITCH_PA0_OPEN);
        }
        else if ((channel_conf.port == eg::Port::PortA) && (channel_conf.pin == eg::Pin::Pin1))
        {
            HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA0, SYSCFG_SWITCH_PA0_OPEN);
        }
        else if ((channel_conf.port == eg::Port::PortC) && (channel_conf.pin == eg::Pin::Pin2))
        {
            HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC2, SYSCFG_SWITCH_PC2_OPEN);
        }
        else if ((channel_conf.port == eg::Port::PortC) && (channel_conf.pin == eg::Pin::Pin3))
        {
            HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC3, SYSCFG_SWITCH_PC3_OPEN);
        }
        else
        {
            GPIOHelpers::configure_as_analogue(channel_conf.port, channel_conf.pin);
        }

        // Configure ADC channel.
        ADC_ChannelConfTypeDef ch_init{};
        ch_init.Channel                = ADCHelpers::adc_chan_to_stm_adc_chan(channel_conf.channel);
        ch_init.Rank                   = ADCHelpers::get_rank(i + 1u);
        // Set to maximum sampling time for now. May want to make this configurable.
        ch_init.SamplingTime           = ADC_SAMPLETIME_810CYCLES_5;
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


void ADCDriverMultiChannel::start_read(Result result)
{
    if (result.channel_count == m_conf.channel_count)
    {
        m_result = result;

        // The following relies on the DMA copying 16 bits of ADC data to 32 bit values.
        // The stride pattern for this is configured in dma_init().
        if (HAL_ADC_Start_DMA(&m_hadc, result.channel_data, m_conf.channel_count) != HAL_OK)
        {
            // Immediate failure
            m_on_error.emit(result);
        };
    }
    else
    {
        // Attempt to read the wrong number of channels
        m_on_error.emit(result);
    }
}


void ADCDriverMultiChannel::ConvCpltCallback()
{
    m_on_reading.emit(m_result);
}


void ADCDriverMultiChannel::ErrorCallback()
{
    m_on_error.emit(m_result);
}


void ADCDriverMultiChannel::adc_isr()
{
    HAL_ADC_IRQHandler(&m_hadc);
}


void ADCDriverMultiChannel::dma_isr()
{
    HAL_DMA_IRQHandler(&m_hdma);
}


void ADCDriverMultiChannel::ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    auto handle = reinterpret_cast<ADCDriverMultiChannel::ADCHandle*>(hadc);
    handle->Self->ConvCpltCallback();
}


void ADCDriverMultiChannel::ErrorCallback(ADC_HandleTypeDef* hadc)
{
    auto handle = reinterpret_cast<ADCDriverMultiChannel::ADCHandle*>(hadc);
    handle->Self->ErrorCallback();
}


} // namespace eg {
