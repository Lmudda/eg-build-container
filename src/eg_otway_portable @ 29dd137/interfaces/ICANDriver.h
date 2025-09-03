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


class ICANDriver : private NonCopyable
{
    public:
        struct CANMessage
        {
            // 11 or 29 bits. Assume 11 bits for now. 
            uint32_t id{};       
            bool     remote{};     
            // Up to 8 bytes for classic CAN. Up to 64 bytes for FDCAN.
            uint8_t  length{};
            // Null means the data is in buffer rather than held externally.
            uint8_t* data{};
            // Classic CAN has only up to 8 bytes of data. Support this with a 
            // small self-contained buffer. This is more convenient for emitting 
            // RX messages with a Signal.
            uint8_t  buffer[8];
        };

        enum class CANErrorStatus
        {
            ActiveError,
            PassiveError,
            BusOff
        };

        struct CANStatus
        {
            CANErrorStatus error_status{CANErrorStatus::ActiveError};
        };

        enum class Error
        {
            TxQueueFull,
            RxMessageLost,
            InternalFault
        };

    public:
        virtual ~ICANDriver() = default;
        virtual void queue_message(uint32_t id, const uint8_t* data, uint8_t length) = 0;
        virtual void queue_message(const CANMessage& message) = 0;
        virtual SignalProxy<CANMessage> on_rx_message() = 0;
        virtual SignalProxy<CANStatus> on_status_changed() = 0;
        virtual SignalProxy<Error> on_error() = 0;
};


} // namespace eg {

