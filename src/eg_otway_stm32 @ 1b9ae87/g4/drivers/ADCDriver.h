/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "interfaces/IADCDriver.h"
#include "helpers/ADCHelpers.h"


namespace eg {


// This is a simple driver which perform a single read of a single channel (at a time).
// Need to think about how to generalise this to share the ADC instance with multiple channels.
class ADCDriver : public IADCDriver
{
    public:
        struct Config
        {
            Adc adc;
        };

    public:
        ADCDriver(const Config& conf);
        bool start_read(AdcChannel adc_channel) override;
        SignalProxy<AdcChannel, uint32_t> on_reading() override;

    private:
        // Nasty trick to add a user value to the handle for a bit of context in callbacks.
        struct ADCHandle : public ADC_HandleTypeDef
        {
            ADCDriver* Self{};
        };

    private:
        void on_interrupt(void);

        void ConvCpltCallback();
        static void ConvCpltCallback(ADC_HandleTypeDef* hadc);
 
    private:
        const Config&                 m_conf;
        ADCHandle                     m_hadc{};
        AdcChannel                    m_cur_channel{};
        Signal<AdcChannel, uint32_t>  m_on_reading{};
};


} // namespace eg {
