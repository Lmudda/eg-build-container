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
#include <cstdint>


namespace eg {

    enum class AdcChannel
    {
        Channel0,
        Channel1,
        Channel2,
        Channel3,
        Channel4,
        Channel5,
        Channel6,
        Channel7,
        Channel8,
        Channel9,
        Channel10,
        Channel11,
        Channel12,
        Channel13,
        Channel14,
        Channel15,
        Channel16,
        Channel17,
        Channel18,
        Channel19,
        Channel20,
        Channel21,
        Channel22,
        Channel23,
        ChannelVRef,
        ChannelVbat,
        ChannelTemp
    };    

    class IADCDriver : private NonCopyable
    {
        public:
            virtual ~IADCDriver() = default;
            virtual bool start_read(AdcChannel channel) = 0;
            virtual SignalProxy<AdcChannel, uint32_t> on_reading() = 0;
    };


    // Multichannel ADC. Intent is that caller supplies the memory into
    // which the results will be copied. This means that the implementation
    // can't queue requests to start because it has no way of knowing when
    // the supplied memory is free to use. Therefore the intent is that
    // a single read is triggered at a time via start_read (a queuing
    // mechanism could be implemented at a higher level that has the context
    // of when the memory is free to use).
    class IADCDriverMultiChannel : private NonCopyable
    {
        public:
            struct Result
            {
                uint32_t* channel_data;
                uint8_t   channel_count;
            };

        public:
            virtual ~IADCDriverMultiChannel() = default;
            virtual void start_read(Result result) = 0;
            // The ADC resolution, in bits.
            virtual uint8_t get_resolution() const = 0;
            virtual uint8_t get_channel_count() const = 0;
            virtual SignalProxy<Result> on_reading() = 0;
            virtual SignalProxy<Result> on_error() = 0;
    };


} // namespace eg {
