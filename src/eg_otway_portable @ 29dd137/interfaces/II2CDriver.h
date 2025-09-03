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


class II2CDriver : private NonCopyable
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
        virtual ~II2CDriver() = default;
        virtual void queue_transfer(Transfer& trans) = 0;
        virtual SignalProxy<Transfer> on_complete() = 0;
        virtual SignalProxy<Transfer> on_error() = 0;
};


// Added primarily to support the VL53L4CX ToF sensor, which makes synchronous I2C 
// transfers. Would be nice to make it asynchronous...
class II2CDriverBlocking : private NonCopyable
{
    public:
        enum class ErrorCode : uint8_t 
        {
            I2C_ERROR_OK                  = 0,
            I2C_ERROR_ACKNOWLEDGE_FAILURE = 1,
            I2C_ERROR_PERIPHERAL_FAILURE  = 2
        };

    public:
        virtual ~II2CDriverBlocking() = default;
        virtual ErrorCode write(uint16_t address, uint8_t* data, uint16_t length) = 0;
        virtual ErrorCode read(uint16_t address, uint8_t* data, uint16_t length) = 0;
};


// Added to support a sequence of I2C transactions that need to happen back-to-back
// as quickly as possible. The caller provides the memory for a sequence (array) of
// Transfer objects and is responsible for ensuring this remains valid while the
// sequence is queued and being processed. If an error occurs, the on_seq_error
// callback indicates which transfer in the sequence failed.
class II2CDriverSequence : private NonCopyable
{
    public:
        struct Sequence
        {
            uint8_t               length;
            II2CDriver::Transfer* transfers;
        };

    public:
        virtual ~II2CDriverSequence() = default;
        virtual void queue_sequence(Sequence& sequence) = 0;
        virtual void reset() = 0;
        virtual SignalProxy<Sequence> on_seq_complete() = 0;
        virtual SignalProxy<Sequence, uint8_t> on_seq_error() = 0;
};


} // namespace eg {
