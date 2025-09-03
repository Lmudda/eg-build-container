/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/DigitalInputPolled.h"
#include "signals/InterruptHandler.h"
#include "drivers/helpers/GPIOHelpers.h"
#include "stm32u5xx_hal.h"


namespace eg {


DigitalInputPolled::DigitalInputPolled(const Config& conf)
: m_conf{conf}
, m_timer{m_conf.period_ms, eg::Timer::Type::Repeating}
{
    // Configure the pin and interrupt.
    GPIOHelpers::configure_as_input(m_conf.port, m_conf.pin, m_conf.pupd);

    // Set a callback for the debounce timer.
    m_timer.on_update().connect<&DigitalInputPolled::on_timer>(this); 
    if (m_conf.period_ms > 0)
    {
        m_timer.start();
    }

    // Initialise cached status
    m_state = read();
}


void DigitalInputPolled::on_timer()
{
    bool state = read();
    if (state != m_state)
    {
        m_state = state;
        m_on_change.emit(m_state);
    }
}


bool DigitalInputPolled::read() const    
{
    GPIO_TypeDef* gpio = reinterpret_cast<GPIO_TypeDef*>(m_conf.port);
    bool state = (HAL_GPIO_ReadPin(gpio, m_conf.pin_mask) == GPIO_PIN_SET);
    state = (m_conf.invert) ? !state : state;
    return state;
}


SignalProxy<bool> DigitalInputPolled::on_change()
{
    return eg::SignalProxy<bool>{m_on_change};
}


} // namespace eg {


