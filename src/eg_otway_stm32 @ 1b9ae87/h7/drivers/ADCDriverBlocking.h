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
#include "helpers/ADCHelpers.h"
#include "helpers/GlobalDefs.h"
#include "helpers/GPIOHelpers.h"
#include "utilities/NonCopyable.h"


namespace eg {


// This is a simple driver which performs a fast blocking read of the selected ADC channel.
// It is intended for use via AnalogueInputBlocking, and not on its own.
// The channels are configured on construction.
// TODO: This implementation is NOT currently safe for use with more than one AnalogueInput.
class ADCDriverBlocking : private eg::NonCopyable
{
    public:
        enum class SamplingTime : uint32_t
        {
            e1p5   = ADC_SAMPLETIME_1CYCLE_5,
            e2p5   = ADC_SAMPLETIME_2CYCLES_5,
            e8p5   = ADC_SAMPLETIME_8CYCLES_5,
            e16p5  = ADC_SAMPLETIME_16CYCLES_5,
            e32p5  = ADC_SAMPLETIME_32CYCLES_5,
            e64p5  = ADC_SAMPLETIME_64CYCLES_5,
            e387p5 = ADC_SAMPLETIME_387CYCLES_5,
            e810p5 = ADC_SAMPLETIME_810CYCLES_5
        };

    public:
        struct Config
        {
            Adc                  adc;
            AdcClockPrescaler    prescaler;
            uint32_t             adc_clk_src;
        };

    public:
        ADCDriverBlocking(const Config& conf);
        void configure_channel(AdcChannel channel, SamplingTime sampling_time);
        bool read(uint32_t& result);

    private:
        // Nasty trick to add a user value to the handle for a bit of context in callbacks.
        struct ADCHandle : public ADC_HandleTypeDef
        {
            ADCDriverBlocking* Self{};
        };

    private:
        void adc_init();
 
    private:
        const Config&  m_conf;
        ADCHandle      m_hadc{};
        AdcChannel     m_current_channel;
        SamplingTime   m_sampling_time;
};


} // namespace eg {
