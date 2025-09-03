/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "AnalogueOutput.h"
#include "utilities/ErrorHandler.h"
#include "stm32h7xx_hal_dac.h"


namespace eg {


// 12-bit DAC
static constexpr uint32_t kDACMaxValue = 0xFFFu;
static constexpr uint32_t kDACAlignment = DAC_ALIGN_12B_R;


// TODO: Review the performance of this. The ST HAL is quite bloated as it is designed
// to cover all possible use cases. We're only doing something simple, so there is scope
// for implementing via direct register accesses if the overhead is too much for the
// audio generation use case.
AnalogueOutput::AnalogueOutput(const Config& conf)
: m_conf{conf}
{
    GPIOHelpers::configure_as_analogue(m_conf.port, m_conf.pin);

    // Enable clock
    __HAL_RCC_DAC12_CLK_ENABLE();

    // Initialise DAC
    m_hdac.Self = this;
    m_hdac.Instance = DAC1;
    if (HAL_DAC_Init(&m_hdac) != HAL_OK)
    {
        Error_Handler();
    }

    // Configure channel
    DAC_ChannelConfTypeDef channel_conf{};
    channel_conf.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
    channel_conf.DAC_Trigger = DAC_TRIGGER_NONE;
    channel_conf.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    channel_conf.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
    channel_conf.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
    if (HAL_DAC_ConfigChannel(&m_hdac, &channel_conf, static_cast<uint32_t>(m_conf.channel)))
    {
        Error_Handler();
    }
}


void AnalogueOutput::set(uint32_t value)
{
    if (value > kDACMaxValue)
    {
        value = kDACMaxValue;
    }
    HAL_DAC_SetValue(&m_hdac, static_cast<uint32_t>(m_conf.channel), kDACAlignment, value);
}


uint32_t AnalogueOutput::get()
{
    return HAL_DAC_GetValue(&m_hdac, static_cast<uint32_t>(m_conf.channel));
}


void AnalogueOutput::enable()
{
    HAL_DAC_Start(&m_hdac, static_cast<uint32_t>(m_conf.channel));
}


void AnalogueOutput::disable()
{
    HAL_DAC_Stop(&m_hdac, static_cast<uint32_t>(m_conf.channel));
}


} // namespace eg {
