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


class IUARTDriver : private NonCopyable
{
    public:
        virtual ~IUARTDriver() = default;
        struct RXData 
        {
            static constexpr uint8_t MaxData = 32;
            uint8_t data[MaxData];
            uint8_t length{};
        };

    public:
        virtual SignalProxy<RXData> on_rx_data() = 0;
        virtual SignalProxy<> on_error() = 0;
        virtual void write(const uint8_t* data, uint16_t length) = 0;
        virtual void write(const char* data) = 0;
};


} // namespace eg {

