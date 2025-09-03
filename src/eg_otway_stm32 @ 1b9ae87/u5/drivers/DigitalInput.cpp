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
#include "stm32u5xx_hal.h"


namespace eg {


// Note that this implementation means we support all 16 EXTI lines whether they are used
// or not in the application. This consumes a little RAM for the array of InterruptHandlers.
// If necessary (unlikely) we could tailor the supported lines with #defines determined by the
// application's CMakeLists.txt, or something like that.
static InterruptHandler g_exti_handlers[16];


static IRQn_Type exti_irq_number(Pin pin);


DigitalInput::DigitalInput(const Config& conf)
: m_conf{conf}
, m_timer{m_conf.debounce_ms, eg::Timer::Type::OneShot}
{
    // Configure the pin and interrupt.
    GPIOHelpers::configure_as_edge_int_input(m_conf.port, m_conf.pin, m_conf.pupd);

    // There are some shared IRQs so be careful of priority conflicts.
    // Generally a common low priority is fine for user input.
    IRQn_Type irqn = exti_irq_number(m_conf.pin);
    HAL_NVIC_SetPriority(irqn, static_cast<uint32_t>(m_conf.prio), 0);
    HAL_NVIC_EnableIRQ(irqn);

    // Set a callback for when the interrupt occurs.
    g_exti_handlers[static_cast<uint8_t>(m_conf.pin)].connect<&DigitalInput::on_interrupt>(this);

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
    state = (m_conf.inverted) ? !state : state;
    return state;
}


SignalProxy<bool> DigitalInput::on_change()
{
    return eg::SignalProxy<bool>{m_on_change};
}


static IRQn_Type exti_irq_number(Pin pin)
{
    switch (pin)
    {
        case Pin::Pin0:   return EXTI0_IRQn;
        case Pin::Pin1:   return EXTI1_IRQn;
        case Pin::Pin2:   return EXTI2_IRQn;
        case Pin::Pin3:   return EXTI3_IRQn;
        case Pin::Pin4:   return EXTI4_IRQn;
        case Pin::Pin5:   return EXTI5_IRQn;
        case Pin::Pin6:   return EXTI6_IRQn;
        case Pin::Pin7:   return EXTI7_IRQn;
        case Pin::Pin8:   return EXTI8_IRQn;
        case Pin::Pin9:   return EXTI9_IRQn;
        case Pin::Pin10:  return EXTI10_IRQn;
        case Pin::Pin11:  return EXTI11_IRQn;
        case Pin::Pin12:  return EXTI12_IRQn;
        case Pin::Pin13:  return EXTI13_IRQn;
        case Pin::Pin14:  return EXTI14_IRQn;
        case Pin::Pin15:  return EXTI15_IRQn;
    }

    return EXTI0_IRQn;
}

// This overrides a weakly defined function in the HAL.
void EXTI_Common_Callback(uint16_t mask)
{
    uint8_t pin = 0;

    switch (mask)
    {
        case GPIO_PIN_0:   pin =  0; break;
        case GPIO_PIN_1:   pin =  1; break;
        case GPIO_PIN_2:   pin =  2; break;
        case GPIO_PIN_3:   pin =  3; break;
        case GPIO_PIN_4:   pin =  4; break;
        case GPIO_PIN_5:   pin =  5; break;
        case GPIO_PIN_6:   pin =  6; break;
        case GPIO_PIN_7:   pin =  7; break;
        case GPIO_PIN_8:   pin =  8; break;
        case GPIO_PIN_9:   pin =  9; break;
        case GPIO_PIN_10:  pin = 10; break;
        case GPIO_PIN_11:  pin = 11; break;
        case GPIO_PIN_12:  pin = 12; break;
        case GPIO_PIN_13:  pin = 13; break;
        case GPIO_PIN_14:  pin = 14; break;
        case GPIO_PIN_15:  pin = 15; break;
        default:
            break;
    }

    g_exti_handlers[pin].call();
}

extern "C" void HAL_GPIO_EXTI_Rising_Callback(uint16_t mask)
{
    EXTI_Common_Callback(mask);
}

extern "C" void HAL_GPIO_EXTI_Falling_Callback(uint16_t mask)
{
    EXTI_Common_Callback(mask);
}

// Different families map the EXTI pin to interrupts in different ways.
// It would be nice to capture this more gracefully.
extern "C" void EXTI0_IRQHandler()
{
    // This will call HAL_GPIO_EXTI_Callback() if the pin has trigged this
    // interrupt. It will also reset the interrupt flag.
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}


extern "C" void EXTI1_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}


extern "C" void EXTI2_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}


extern "C" void EXTI3_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}


extern "C" void EXTI4_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}


extern "C" void EXTI5_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
}


extern "C" void EXTI6_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
}


extern "C" void EXTI7_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
}


extern "C" void EXTI8_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
}


extern "C" void EXTI9_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
}


extern "C" void EXTI10_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
}


extern "C" void EXTI11_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
}


extern "C" void EXTI12_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
}


extern "C" void EXTI13_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}


extern "C" void EXTI14_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
}


extern "C" void EXTI15_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}
} // namespace eg {
