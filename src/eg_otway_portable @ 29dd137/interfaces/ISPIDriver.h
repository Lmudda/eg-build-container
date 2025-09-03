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
#include "interfaces/IDigitalOutput.h"


namespace eg {


class ISPIDriver : private NonCopyable
{
    public:
        // Added to make transfers a little more flexible. There are up to two 
        // blocks of data. Most transfers are optional TX followed by optional RX.
        // We have a use case with an LCD display where we need TX followed by TX 
        // from discontiguous buffers without raising CS. We have another use case
        // where TX and RX are simultaneous (common with ADCs). A more general purpose 
        // solution might have an array of TX/RX + Buffer + Length, but we don't need it.
        enum class RxMode : uint8_t { Rx, Tx, TxRx };

        struct Transfer
        {
            // Chip select. Could be nullptr
            IDigitalOutput* cs{};

            // Pointer to external buffer (typically for large transfers). 
            // Null value means TX comes from buffer[0:tx_len].
            // Better to make this const but the C API doesn't accept that.
            uint8_t* tx_data{};
            // Zero means there is no TX phase in the transfer.
            uint32_t tx_len{};

            // Pointer to external buffer (typically for large transfers). 
            // Null value means RX goes in buffer[tx_len:tx_len+rx_len].
            // Note that this pointer can be used for a second TX buffer (see Mode).
            uint8_t* rx_data{};
            // Zero means there is no RX phase in the transfer.
            uint32_t rx_len{};

            // Determine whether second buffer is TX or RX.
            RxMode rx_mode{RxMode::Rx};

            // Self contained buffer for small transfers such as register reads.  
            // Take care that tx_len+rx_len is at most 16 (where these don't refer
            // to external buffers).
            static constexpr uint8_t BufferSize = 16;
            uint8_t buffer[BufferSize];
        };

    public:
        virtual ~ISPIDriver() = default;
        virtual void queue_transfer(Transfer& trans) = 0;
        virtual SignalProxy<Transfer> on_complete() = 0;
        virtual SignalProxy<Transfer> on_error() = 0;
};


} // namespace eg {
