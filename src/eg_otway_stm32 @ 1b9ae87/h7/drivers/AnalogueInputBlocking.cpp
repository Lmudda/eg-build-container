/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/AnalogueInputBlocking.h"
#include "helpers/ADCHelpers.h"
#include "helpers/GPIOHelpers.h"
#include "stm32h7xx_hal.h"


namespace eg {


AnalogueInputBlocking::AnalogueInputBlocking(const Config& conf, ADCDriverBlocking& adc)
: m_conf{conf}
, m_adc{adc}
{
    // Configure pin.
    // Some pins are special (PA0, PA1, PC2, PC3) and have an analogue switch.
    // See section 12.3.13 of the STM32H7 reference manual RM0399.
    if ((m_conf.port == eg::Port::PortA) && (m_conf.pin == eg::Pin::Pin0))
    {
        HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA0, SYSCFG_SWITCH_PA0_OPEN);
    }
    else if ((m_conf.port == eg::Port::PortA) && (m_conf.pin == eg::Pin::Pin1))
    {
        HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA0, SYSCFG_SWITCH_PA0_OPEN);
    }
    else if ((m_conf.port == eg::Port::PortC) && (m_conf.pin == eg::Pin::Pin2))
    {
        HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC2, SYSCFG_SWITCH_PC2_OPEN);
    }
    else if ((m_conf.port == eg::Port::PortC) && (m_conf.pin == eg::Pin::Pin3))
    {
        HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC3, SYSCFG_SWITCH_PC3_OPEN);
    }
    else
    {
        GPIOHelpers::configure_as_analogue(m_conf.port, m_conf.pin);
    }
}


bool AnalogueInputBlocking::read(uint32_t& result)
{
    m_adc.configure_channel(m_conf.channel, m_conf.sampling_time);
    return m_adc.read(result);
}

} // namespace eg
