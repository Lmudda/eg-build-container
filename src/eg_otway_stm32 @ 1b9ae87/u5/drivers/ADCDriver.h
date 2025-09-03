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

    class ADCDriver : public IADCDriver
    {
        public:
            struct Config
            {
                Adc        adc;
            };

        public:
            ADCDriver(const Config& conf);

            bool start_read(AdcChannel adc_channel) override;
            SignalProxy<AdcChannel, uint32_t> on_reading() override;

        private:
	        void on_interrupt(void);
	
            const Config&                 m_conf;
	        AdcChannel                    m_cur_channel;
            Signal<AdcChannel, uint32_t>  m_on_reading;
    };

} // namespace eg {
