/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/DigitalInput.h"
#include "signals/InterruptHandler.h"
#include "drivers/helpers/GPIOHelpers.h"
#include "utilities/EnumUtils.h"
#include "stm32h7xx_hal.h"


namespace eg {


DigitalInput::DigitalInput(const Config& conf)
: m_conf{conf}
, m_timer{m_conf.debounce_ms, eg::Timer::Type::OneShot}
{
    // Configure the pin and interrupt.
    GPIOHelpers::configure_as_input(m_conf.port, m_conf.pin, m_conf.pupd, IntMode::Interrupt);

    // There are some shared IRQs so be careful of priority conflicts.
    // Generally a common low priority is fine for user input.
    GPIOHelpers::irq_enable(m_conf.pin, to_u32(m_conf.prio));

    // Set a callback for when the interrupt occurs.
    GPIOHelpers::irq_handler(m_conf.pin)->connect<&DigitalInput::on_interrupt>(this);

    // Set a callback for the debounce timer.
    m_timer.on_update().connect<&DigitalInput::on_timer>(this); 

    // Initialise cached status
    m_state = read();
}


void DigitalInput::on_interrupt()
{
    if (m_conf.debounce_ms > 0)
    {
        m_timer.start();
    }
    else
    {
        on_timer();
    }
}


void DigitalInput::on_timer()
{
    bool state = read();
    if (state != m_state)
    {
        m_state = state;
        m_on_change.emit(m_state);
    }
}


bool DigitalInput::read() const    
{
    GPIO_TypeDef* gpio = reinterpret_cast<GPIO_TypeDef*>(m_conf.port);
    bool state = (HAL_GPIO_ReadPin(gpio, m_conf.pin_mask) == GPIO_PIN_SET);
    state = (m_conf.invert) ? !state : state;
    return state;
}


SignalProxy<bool> DigitalInput::on_change()
{
    return eg::SignalProxy<bool>{m_on_change};
}


} // namespace eg {


