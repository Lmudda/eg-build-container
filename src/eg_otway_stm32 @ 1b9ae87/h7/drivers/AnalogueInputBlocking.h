/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "interfaces/IAnalogueInput.h"
#include "interfaces/IADCDriver.h"
#include "drivers/ADCDriverBlocking.h"
#include "helpers/GPIOHelpers.h"


namespace eg {


// Simple class to configure an IO as an analogue input. See ADC Driver to obtain a reading
// from a configured analogue input.
//
// TODO_AC This class serves no purpose. It is nothing more than a wrapper around a call to 
// GPIOHelpers::configure_as_analogue(). It would be better to initialise this with a
// compatible IADCDriver reference, and for it to have the start and on_reading members. Then
// it would serve as a simple abstraction for a single input read. Could possibly add a Timer
// member to automatically make regular reads, such as reading the battery voltage every second.
// There are many other ways to use ADC inputs and this would be only one possible use case. We'd
// end up with suite of input classes for various use cases.
// TODO: This implementation is NOT currently safe for use with more than one AnalogueInput.
class AnalogueInputBlocking : public IAnalogueInputBlocking
{
    public:
        struct Config
        {
            Port                            port;             
            Pin                             pin;
            AdcChannel                      channel;
            ADCDriverBlocking::SamplingTime sampling_time;
        };

    public:
        AnalogueInputBlocking(const Config& conf, ADCDriverBlocking& adc);
        bool read(uint32_t& result) override;

    private:
        Config             m_conf;
        ADCDriverBlocking& m_adc;
};


} // namespace eg
