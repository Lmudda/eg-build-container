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


// This implementation of IDigitalInput relies on EXTI interrupts to detect edges, and 
// emits events when the state of the input changes. The input is debounced with a software
// timer. Setting a period of 0ms will disable the debouncing. 
//
// Note that each EXTIx line is configured in the hardware to a monitor the state of Pinx of 
// a particular Port. This means you can't use DigitalInputEXTI for two inputs which have the 
// same pin number but on different ports. This is a hardware limitation of STM32s, but there 
// are usually enough free pins to have all different numbers - talk to your EE!
//
// If you really can't use DigitalInputEXTI, consider a different implementation of IDigital
// input which polls the input on a software or hardware timer.
class DigitalInput : public IDigitalInput
{
    public:
        struct Config
        {
            Port     port;                         
            Pin      pin;
            uint8_t  debounce_ms{5};
            bool     invert{false};
            Priority prio{Priority::Prio14}; // Low but not the lowest. 
            PuPd     pupd{PuPd::PullDown};
            uint32_t pin_mask{1U << static_cast<uint8_t>(pin)};
        };

    public:
        DigitalInput(const Config& conf);

        bool read() const override;
        SignalProxy<bool> on_change() override;

    private:
        void on_interrupt();
        void on_timer();
        
    private:
        const Config& m_conf;
        Signal<bool>  m_on_change;   
        Timer         m_timer;     // Used to debounce the input
        bool          m_state{};   // Last debounced state
};


} // namespace eg {

