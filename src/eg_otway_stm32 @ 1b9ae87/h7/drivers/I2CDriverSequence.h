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
#include "helpers/GlobalDefs.h"
#include "utilities/RingBuffer.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_i2c.h"
#include <cstdint>


namespace eg {


// Implements the sequence version of the I2CDriver that is intended for
// executing sequences of I2C transfers quickly. However it still uses emit so
// that the result is processed by the event loop. Although this could be
// combined with the I2CDriver implementation, that would compromise I2CDriver
// for its normal use case. They can always be combined in future if required.
class I2CDriverSequenceBase : public II2CDriverSequence
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

        uint8_t  priority{GlobalsDefs::DefPreemptPrio};
    }; 

public:
    I2CDriverSequenceBase(const Config& conf, RingBuffer<Sequence>& queue);
    void queue_sequence(Sequence& seq) override;
    void reset() override;
    SignalProxy<Sequence> on_seq_complete() override { return SignalProxy<Sequence>{m_on_complete}; }
    SignalProxy<Sequence, uint8_t> on_seq_error() override { return SignalProxy<Sequence, uint8_t>{m_on_error}; }

private:
    // Called from the constructor.
    void gpio_init();
    void gpio_deinit();
    void i2c_init();
    void i2c_deinit();

    void start_sequence();
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
        I2CDriverSequenceBase* Self{};
    };

private:
    using Transfer = II2CDriver::Transfer;

    // Totally arbitrary depth for a queue of pending transfers.
    using Sequences = RingBuffer<Sequence>;

    const Config&     m_conf;
    I2CHandle         m_hi2c{};

    // Active sequence, if any.
    Sequence                  m_seq{};     
    // Position in active sequence.
    uint16_t                  m_seq_pos;
    // Pending sequences, if any.
    Sequences&                m_queue;       
    // True while processing active and pending transfers.
    bool                      m_busy;
    Signal<Sequence>          m_on_complete;
    Signal<Sequence, uint8_t> m_on_error;
};


template <uint16_t QUEUE_SIZE>
class I2CDriverSequence : public I2CDriverSequenceBase
{
public:
    I2CDriverSequence(const Config& conf)
    : I2CDriverSequenceBase{conf, m_data}
    {        
    }

private:
    RingBufferArray<Sequence, QUEUE_SIZE> m_data; 
};


} // namespace eg {



