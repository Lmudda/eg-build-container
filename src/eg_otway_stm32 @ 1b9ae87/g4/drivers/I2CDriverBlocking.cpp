/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/I2CDriverBlocking.h"
#include "drivers/helpers/I2CHelpers.h"
#include "drivers/helpers/GPIOHelpers.h"
#include "signals/InterruptHandler.h"
#include "utilities/ErrorHandler.h"
#include "stm32g4xx_hal.h"
#include <cstdint>
#include <cstring>


// We have some dependencies on settings in stm32g4xx_hal_conf.h
#ifndef HAL_I2C_MODULE_ENABLED
#error HAL_I2C_MODULE_ENABLED must be defined to use this driver
#endif
#if (USE_HAL_I2C_REGISTER_CALLBACKS == 0)
#error USE_HAL_I2C_REGISTER_CALLBACKS must be set to use this driver
#endif


namespace eg {


I2CDriverBlocking::I2CDriverBlocking(const Config& conf)
: m_conf{conf}
{
    gpio_init();
    i2c_init();
}


void I2CDriverBlocking::gpio_init()
{
    GPIOHelpers::configure_as_alternate(m_conf.scl_port, m_conf.scl_pin, m_conf.scl_alt, OType::OpenDrain);
    GPIOHelpers::configure_as_alternate(m_conf.sda_port, m_conf.sda_pin, m_conf.sda_alt, OType::OpenDrain);
}


void I2CDriverBlocking::i2c_init()
{
    I2CHelpers::enable_clock(m_conf.i2c, m_conf.i2c_src);

    m_hi2c.Instance              = I2CHelpers::instance(m_conf.i2c);
    m_hi2c.Init.Timing           = m_conf.i2c_timing; 
    // These defaults seem fine for now.
    m_hi2c.Init.OwnAddress1      = 0;
    m_hi2c.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    m_hi2c.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    m_hi2c.Init.OwnAddress2      = 0;
    m_hi2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    m_hi2c.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    m_hi2c.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&m_hi2c) != HAL_OK)
    {
        Error_Handler();
    }

    // These defaults seem fine for now.
    if (HAL_I2CEx_ConfigAnalogFilter(&m_hi2c, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_I2CEx_ConfigDigitalFilter(&m_hi2c, 0) != HAL_OK)
    {
        Error_Handler();
    }
}


I2CDriverBlocking::ErrorCode I2CDriverBlocking::write(uint16_t address, uint8_t* data, uint16_t length)
{
    ErrorCode result = ErrorCode::I2C_ERROR_OK;    
    if (HAL_I2C_Master_Transmit(&m_hi2c, address, data, length, I2C_POLLING_TIMEOUT) != HAL_OK)
    {
        if (HAL_I2C_GetError(&m_hi2c) != HAL_I2C_ERROR_AF)
        {
            result = ErrorCode::I2C_ERROR_ACKNOWLEDGE_FAILURE;
        }
        else
        {
            result = ErrorCode::I2C_ERROR_PERIPHERAL_FAILURE;
        }
    }
    return result;
}


I2CDriverBlocking::ErrorCode I2CDriverBlocking::read(uint16_t address, uint8_t* data, uint16_t length)
{
    ErrorCode result = ErrorCode::I2C_ERROR_OK;    
    if (HAL_I2C_Master_Receive(&m_hi2c, address, data, length, I2C_POLLING_TIMEOUT) != HAL_OK)
    {
        if (HAL_I2C_GetError(&m_hi2c) != HAL_I2C_ERROR_AF)
        {
            result = ErrorCode::I2C_ERROR_ACKNOWLEDGE_FAILURE;
        }
        else
        {
            result = ErrorCode::I2C_ERROR_PERIPHERAL_FAILURE;
        }
    }
    return result;
}


} // namespace eg {
