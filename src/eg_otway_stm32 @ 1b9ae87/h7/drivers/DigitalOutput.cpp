/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "DigitalOutput.h"
#include "stm32h7xx_hal_gpio.h"


namespace eg {


DigitalOutput::DigitalOutput(const Config& conf)
: m_conf{conf}
{
    GPIOHelpers::configure_as_output(m_conf.port, m_conf.pin, m_conf.otype, m_conf.pupd, m_conf.ospeed);
}   


void DigitalOutput::set()
{
    // TODO_AC DigitalOutput::set() could move invert calculation to compile time?
    GPIO_TypeDef* gpio = reinterpret_cast<GPIO_TypeDef*>(m_conf.port);
    HAL_GPIO_WritePin(gpio, m_conf.pin_mask, m_conf.invert ? GPIO_PIN_RESET : GPIO_PIN_SET);
}


void DigitalOutput::reset()
{
    GPIO_TypeDef* gpio = reinterpret_cast<GPIO_TypeDef*>(m_conf.port);
    HAL_GPIO_WritePin(gpio, m_conf.pin_mask, m_conf.invert ? GPIO_PIN_SET : GPIO_PIN_RESET);
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
    //bool result = HAL_GPIO_ReadOutPin(gpio, m_conf.pin_mask) == GPIO_PIN_SET;
    bool result = ((gpio->ODR & m_conf.pin_mask) != 0);
    return m_conf.invert ? !result : result;
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
