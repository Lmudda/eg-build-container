/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "interfaces/II2CDriver.h"
#include "helpers/I2CHelpers.h"
#include "helpers/GPIOHelpers.h"
#include "utilities/RingBuffer.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_i2c.h"
#include <cstdint>


namespace eg {


class I2CDriverBlocking : public II2CDriverBlocking
{
public:
    struct Config
    {
        I2c       i2c;      
        uint32_t  i2c_src;
        uint32_t  i2c_timing;
        
        Port      sda_port;   
        Pin       sda_pin;
        uint8_t   sda_alt;  

        Port      scl_port;
        Pin       scl_pin;
        uint8_t   scl_alt;    
    }; 

public:
    I2CDriverBlocking(const Config& conf);
    ErrorCode write(uint16_t address, uint8_t* data, uint16_t length);
    ErrorCode read(uint16_t address, uint8_t* data, uint16_t length);

private:
    // Called from the constructor.
    void gpio_init();
    void i2c_init();

private:
    static constexpr uint16_t I2C_POLLING_TIMEOUT = 0x1000;

    const Config&     m_conf;
    I2C_HandleTypeDef m_hi2c{};
};


} // namespace eg {



