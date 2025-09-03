/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "interfaces/IDigitalOutput.h"
#include "drivers/helpers/GPIOHelpers.h"


namespace eg {


class DigitalOutput : public IDigitalOutput
{
    public:
        struct Config 
        { 
            Port     port; 
            Pin      pin;
            //Mode     mode{Mode::Output}; // Not required
            OType    otype{OType::PushPull};
            OSpeed   ospeed{OSpeed::High};
            PuPd     pupd{PuPd::NoPull};
            uint32_t pin_mask{1U << static_cast<uint8_t>(pin)};
        };

    public:
        DigitalOutput(const Config& conf);    
        void set() override;
        void reset() override;
        void toggle() override;
        void set_to(bool state) override;
        bool read() override;

    private:
        const Config& m_conf;
};


} // namespace eg {
