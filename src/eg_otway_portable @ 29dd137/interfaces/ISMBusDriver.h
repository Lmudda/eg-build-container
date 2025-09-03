/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "utilities/NonCopyable.h"
#include "signals/Signal.h"


namespace eg {


class ISMBusDriver : private NonCopyable
{
    public:
        // This structure is predicated on the assumption that an I2C transfer is 
        // always in up to two phases:
        //
        //     TX phase: optionally write some data to the target device 
        //               (e.g. set the current register).
        //     RX phase: optionally read some data from the target device 
        //               (e.g. read the current register value).
        //
        // I don't think I've seen anything more complicated. I've also never seen 
        // any I2C transfers of more than a few bytes, so we have a small fixed 
        // buffer here. It would be simple to extend this to large buffers held 
        // outside the Transfer object (see ISPIDriver2).
        struct Transfer
        {
            // 7-bit address of the I2C device. Never seen 10-bit addresses in real 
            // life so these are not supported. Easy enough to add.
            uint8_t address;
            // Zero means there is no TX phase in the transfer.
            uint8_t tx_len{};
            // Zero means there is no RX phase in the transfer.
            uint8_t rx_len{};
            // Self-contained buffer:
            //     [0..tx_len] is the TX data
            //     [tx_len..(tx_len+rx_len)] is the RX buffer
            // tx_len+rx_len cannot exceed BufferSize. 
            static constexpr uint8_t BufferSize = 16;
            uint8_t buffer[BufferSize];
        };

    public:
        virtual ~ISMBusDriver() = default;
        virtual void queue_transfer(Transfer& trans) = 0;
        virtual SignalProxy<Transfer> on_complete() = 0;
        virtual SignalProxy<Transfer> on_notify() = 0;
        virtual SignalProxy<Transfer> on_error() = 0;

        // TODO_AC We could theoretically directly support the various transaction types specified 
        // by SMBus. The Transfer could have an enum to indicate the type so it is clear 
        // in the completion handler. For now we'll just do enough to support reading words
        // from the battery.
        // void quick_command(uint8_t address);
        // void send_byte(uint8_t address, uint8_t data);
        // void write_byte(uint8_t address, uint8_t command, uint8_t data);
        // void write_word(uint8_t address, uint8_t command, uint16_t data);
        // void write_block(uint8_t address, uint8_t command, uint8_t length, const uint8_t* data);
        // void receive_byte(uint8_t address);
        // void read_byte(uint8_t address, uint8_t command);
        // void read_word(uint8_t address, uint8_t command);
        // void write_block(uint8_t address, uint8_t command);
        // ... there are some others ...
};


} // namespace eg {
