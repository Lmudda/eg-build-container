/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "interfaces/IDigitalInput.h"
#include "drivers/helpers/GPIOHelpers.h"
#include "timers/Timer.h"


namespace eg {


// This implementation of IDigitalInput used a software timer to poll the input, and emits 
// events when it changes. Setting the poll period to zero disables polling.
class DigitalInputPolled : public IDigitalInput
{
    public:
        struct Config
        {
            Port     port;                         
            Pin      pin;
            uint8_t  period_ms{5};
            bool     invert{false};
            PuPd     pupd{PuPd::PullDown};
            uint32_t pin_mask{1U << static_cast<uint8_t>(pin)};
        };

    public:
        DigitalInputPolled(const Config& conf);

        bool read() const override;
        SignalProxy<bool> on_change() override;

    private:
        void on_timer();
        
    private:
        const Config& m_conf;
        Signal<bool>  m_on_change;   
        Timer         m_timer;     // Used to debounce the input
        bool          m_state{};   // Last debounced state
};


} // namespace eg {

