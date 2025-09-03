/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "DigitalOutput.h"
#include "stm32u5xx_hal_gpio.h"


namespace eg {


DigitalOutput::DigitalOutput(const Config& conf)
: m_conf{conf}
{
    GPIO_InitTypeDef init{};
    init.Pin = m_conf.pin_mask;

    // The mode and otype are rolled into one in the HAL code.
    // We assumed the mode is Output here.
    switch (m_conf.otype)
    {
        case OType::OpenDrain: init.Mode = GPIO_MODE_OUTPUT_OD; break;
        case OType::PushPull: 
        default:               init.Mode = GPIO_MODE_OUTPUT_PP;
    }

    switch (m_conf.pupd)
    {
        case PuPd::PullUp:   init.Pull = GPIO_PULLUP;   break;
        case PuPd::PullDown: init.Pull = GPIO_PULLDOWN; break;
        case PuPd::NoPull: 
        default:             init.Pull = GPIO_NOPULL;
    }

    switch (m_conf.ospeed)
    {
        case OSpeed::Low:       init.Speed = GPIO_SPEED_FREQ_LOW;    break;
        case OSpeed::Medium:    init.Speed = GPIO_SPEED_FREQ_MEDIUM; break;
        case OSpeed::High:      init.Speed = GPIO_SPEED_FREQ_HIGH;   break;
        case OSpeed::VeryHigh: 
        default:                init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    }

    GPIOHelpers::enable_clock(conf.port);
    GPIO_TypeDef* gpio = reinterpret_cast<GPIO_TypeDef*>(conf.port);
    HAL_GPIO_Init(gpio, &init);
}   


void DigitalOutput::set()
{
    GPIO_TypeDef* gpio = reinterpret_cast<GPIO_TypeDef*>(m_conf.port);
    HAL_GPIO_WritePin(gpio, m_conf.pin_mask, GPIO_PIN_SET);
}


void DigitalOutput::reset()
{
    GPIO_TypeDef* gpio = reinterpret_cast<GPIO_TypeDef*>(m_conf.port);
    HAL_GPIO_WritePin(gpio, m_conf.pin_mask, GPIO_PIN_RESET);
}


void DigitalOutput::toggle()
{
    GPIO_TypeDef* gpio = reinterpret_cast<GPIO_TypeDef*>(m_conf.port);
    HAL_GPIO_TogglePin(gpio, m_conf.pin_mask);
}


void DigitalOutput::set_to(bool state)
{
    if (state)
    {
        set();
    }
    else
    {
        reset();
    }
}


bool DigitalOutput::read()
{
    GPIO_TypeDef* gpio = reinterpret_cast<GPIO_TypeDef*>(m_conf.port);
    //return HAL_GPIO_ReadOutPin(gpio, m_conf.pin_mask) == GPIO_PIN_SET;
    return (gpio->ODR & m_conf.pin_mask) != 0;
}


// This duplicates the HAL_GPIO_ReadPin() method, but for ODR  
// static GPIO_PinState HAL_GPIO_ReadOutPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
// {
//   GPIO_PinState bitstatus;

//   /* Check the parameters */
//   assert_param(IS_GPIO_PIN(GPIO_Pin));

//   if((GPIOx->ODR & GPIO_Pin) != (uint32_t)GPIO_PIN_RESET)
//   {
//     bitstatus = GPIO_PIN_SET;
//   }
//   else
//   {
//     bitstatus = GPIO_PIN_RESET;
//   }
//   return bitstatus;
// }


} // namespace eg {
