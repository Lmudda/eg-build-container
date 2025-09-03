/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "interfaces/IPWMx4Driver.h"
#include "helpers/TIMHelpers.h"
#include "helpers/GPIOHelpers.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_tim.h"
#include <cstdint>


namespace eg {


// Supports up to four channels on the same timer with potentially different duty cycles.
// All channels necessarily have the same frequency.
class PWMx4Driver : public IPWMx4Driver
{
public:
    using Channel = IPWMx4Driver::Channel;

    struct ChannelConfig
    {
        bool     used{false};
        Channel  channel{Channel::Channel1};
        Port     port{};
        Pin      pin{};
        uint8_t  alt_fn{};
        uint16_t min_duty{0};
        uint16_t max_duty{1000};
    };

    struct Config
    {
        // Peripheral and clock source.    
        Tim           tim{};
        uint32_t      clk_src{};
        // Default period (shared by all channels).
        // TODO_AC Better to specify the PWM frequency and get period from clock.
        uint16_t      prescale{170}; // e.g. scale a 170MHz clock to 1MHz input
        uint16_t      period{1000};  // e.g. a period of 1000 cycles is 1kHz PWM with 1MHz input 
        ChannelConfig channels[4];
    }; 

public:
    PWMx4Driver(const Config& conf);

    void enable(Channel channel) override;
    void disable(Channel channel) override;
    void set_duty_cycle_permille(Channel channel, uint16_t duty_cycle) override;
    void set_duty_cycle_percent(Channel channel, uint16_t duty_cycle) override;

private:
    void configure_timer();
    void configure_master();
    void configure_channels();
    void configure_deadtime();
    void configure_pins();

private:
    const Config&     m_conf;
    TIM_HandleTypeDef m_htim{};
};


} // namespace eg {
