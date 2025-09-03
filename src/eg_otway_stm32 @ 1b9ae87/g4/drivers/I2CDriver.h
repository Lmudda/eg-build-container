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


class I2CDriverBase : public II2CDriver
{
public:
    struct Config
    {
        I2c       i2c;  
        // Clock source.    
        uint32_t  i2c_src;
        // It looks like a magic number but populates 
        // multiple fields of a single regiser, TIMINGR.
        // See I2CHelpers::make_i2c_timing();
        uint32_t  i2c_timing;
        
        Port      sda_port;   
        Pin       sda_pin;
        uint8_t   sda_alt;  

        Port      scl_port;
        Pin       scl_pin;
        uint8_t   scl_alt;    
    }; 

public:
    I2CDriverBase(const Config& conf, RingBuffer<Transfer>& queue);
    void queue_transfer(Transfer& trans) override;
    SignalProxy<Transfer> on_complete() override { return SignalProxy<Transfer>{m_on_complete}; }
    SignalProxy<Transfer> on_error() override { return SignalProxy<Transfer>{m_on_error}; }

private:
    // Called from the constructor.
    void gpio_init();
    void i2c_init();

    void start_transfer();
    void finish_transfer();

    void event_isr();
    void error_isr();

    void MasterTxCpltCallback();
    void MasterRxCpltCallback();
    void ErrorCallback();

    static void MasterTxCpltCallback(I2C_HandleTypeDef* hi2c);
    static void MasterRxCpltCallback(I2C_HandleTypeDef* hi2c);
    static void ErrorCallback(I2C_HandleTypeDef* hi2c);

private:
    // Nasty trick to add a user value to the handle for a bit of context in callbacks.
    struct I2CHandle : public I2C_HandleTypeDef
    {
        I2CDriverBase* Self{};
    };

private:
    // Totally arbitrary depth for a queue of pending transfers.
    using Transfers = RingBuffer<Transfer>;

    const Config&     m_conf;
    I2CHandle         m_hi2c{};

    // Active transfer, if any.    
    Transfer          m_trans{};     
    // Pending transfers, if any.
    Transfers&        m_queue;       
    // True while processing active and pending transfers.
    bool              m_busy{false}; 
    Signal<Transfer>  m_on_complete;
    Signal<Transfer>  m_on_error;
};


template <uint16_t QUEUE_SIZE>
class I2CDriver : public I2CDriverBase
{
public:
    I2CDriver(const Config& conf)
    : I2CDriverBase{conf, m_data}
    {        
    }

private:
    RingBufferArray<Transfer, QUEUE_SIZE> m_data; 
};


} // namespace eg {



