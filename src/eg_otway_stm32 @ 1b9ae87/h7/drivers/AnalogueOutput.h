/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "interfaces/IAnalogueOutput.h"
#include "drivers/helpers/GPIOHelpers.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_dac.h"


namespace eg {


class AnalogueOutput : public IAnalogueOutput
{
    public:
        enum class DACChannel
        {
            Out1 = DAC_CHANNEL_1,
            Out2 = DAC_CHANNEL_2
        };


    public:
        struct Config
        {
            Port         port;
            Pin          pin;
            DACChannel   channel;
        };

    public:
        AnalogueOutput(const Config& conf);
        void set(uint32_t value) override;
        uint32_t get() override;
        void enable() override;
        void disable() override;

    private:
    struct DACHandle : public DAC_HandleTypeDef
    {
        AnalogueOutput* Self{};
    };

    private:
        const Config& m_conf;
        DACHandle     m_hdac{};
};


} // namespace eg {
