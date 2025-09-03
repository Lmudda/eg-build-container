/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/PWMx4Driver.h"
#include "utilities/ErrorHandler.h"
#include "utilities/EnumUtils.h"
#include "logging/Assert.h"


namespace eg {


static uint32_t get_channel_index(PWMx4Driver::Channel channel)
{
    using Channel = PWMx4Driver::Channel;

    switch (channel)
    {
        case Channel::Channel1: return TIM_CHANNEL_1;
        case Channel::Channel2: return TIM_CHANNEL_2;
        case Channel::Channel3: return TIM_CHANNEL_3;
        case Channel::Channel4: return TIM_CHANNEL_4;
    }
    Error_Handler();
    return 0;
}


PWMx4Driver::PWMx4Driver(const Config& conf)
: m_conf{conf}
{
    configure_timer();
    configure_master();
    configure_channels();
    configure_deadtime();
}


void PWMx4Driver::enable(Channel channel)
{
    HAL_TIM_PWM_Start(&m_htim, get_channel_index(channel));
}


void PWMx4Driver::disable(Channel channel)
{
    HAL_TIM_PWM_Stop(&m_htim, get_channel_index(channel));
}


void PWMx4Driver::set_duty_cycle_permille(Channel channel, uint16_t permille)
{
    // We scale the permille range [0, 1000] to the duty cycle range [min_duty, max_duty]
    // so that the full range of 1000 values can be used by the application, but this 
    // translates to a more limited range of output duty cycles. We need to limit the current 
    // through the LEDs.
    uint16_t input    = (permille < 1'000) ? permille : 1'000;
    uint8_t  index    = to_u8(channel);
    // These are numbers in the range 0 to m_conf.period.
    uint16_t min_duty = m_conf.channels[index].min_duty;
    uint16_t max_duty = m_conf.channels[index].max_duty;

    EG_ASSERT(min_duty <= max_duty, "min_duty > max_duty");
    EG_ASSERT(max_duty <= m_conf.period, "max_duty > m_conf.period");

    // input=0 -> output=min_duty; input=1000 -> output=max_duty
    uint32_t output   = min_duty + (uint32_t(max_duty - min_duty) * input) / 1'000;

    __HAL_TIM_SET_COMPARE(&m_htim, get_channel_index(channel), output);
}


void PWMx4Driver::set_duty_cycle_percent(Channel channel, uint16_t percent)
{
    set_duty_cycle_permille(channel, percent * 10);
}


void PWMx4Driver::configure_timer()
{
    TIMHelpers::enable_clock(m_conf.tim, m_conf.clk_src);

    m_htim.Instance               = TIMHelpers::instance(m_conf.tim);
    // Note that the prescale register divides the clock by (TIMx->PSC + 1).
    m_htim.Init.Prescaler         = m_conf.prescale - 1U;
    m_htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
    // Minus 1 because the time period is determined by (TIMx->ARR + 1) ticks. 
    // The counter runs from 0 to ARR, inclusive. 
    m_htim.Init.Period            = m_conf.period - 1U;
    m_htim.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    m_htim.Init.RepetitionCounter = 0;
    m_htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&m_htim) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_TIM_PWM_Init(&m_htim) != HAL_OK)
    {
        Error_Handler();
    }
}


void PWMx4Driver::configure_master()
{
    TIM_MasterConfigTypeDef config{};
    config.MasterOutputTrigger  = TIM_TRGO_RESET;
    config.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    config.MasterSlaveMode      = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&m_htim, &config) != HAL_OK)
    {
        Error_Handler();
    }
}


void PWMx4Driver::configure_channels()
{
    for (uint8_t c = 0; c < 4; ++c)
    {
        const auto& channel = m_conf.channels[c];
        if (channel.used)
        {
            TIM_OC_InitTypeDef config{};
            config.OCMode       = TIM_OCMODE_PWM1;
            config.OCPolarity   = TIM_OCPOLARITY_HIGH;
            config.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
            config.OCFastMode   = TIM_OCFAST_DISABLE;
            config.OCIdleState  = TIM_OCIDLESTATE_RESET;
            config.OCNIdleState = TIM_OCNIDLESTATE_RESET;
            config.Pulse        = 0;
            if (HAL_TIM_PWM_ConfigChannel(&m_htim, &config, get_channel_index(channel.channel)) != HAL_OK)
            {
                Error_Handler();
            }

            GPIOHelpers::configure_as_alternate(channel.port, channel.pin, channel.alt_fn, OType::PushPull);
        }
    }
}


void PWMx4Driver::configure_deadtime()
{
    TIM_BreakDeadTimeConfigTypeDef config{};
    config.OffStateRunMode  = TIM_OSSR_DISABLE;
    config.OffStateIDLEMode = TIM_OSSI_DISABLE;
    config.LockLevel        = TIM_LOCKLEVEL_OFF;
    config.DeadTime         = 0;
    config.BreakState       = TIM_BREAK_DISABLE;
    config.BreakPolarity    = TIM_BREAKPOLARITY_HIGH;
    config.BreakFilter      = 0;
    config.BreakAFMode      = TIM_BREAK_AFMODE_INPUT;
    config.Break2State      = TIM_BREAK2_DISABLE;
    config.Break2Polarity   = TIM_BREAK2POLARITY_HIGH;
    config.Break2Filter     = 0;
    config.Break2AFMode     = TIM_BREAK_AFMODE_INPUT;
    config.AutomaticOutput  = TIM_AUTOMATICOUTPUT_DISABLE;
    if (HAL_TIMEx_ConfigBreakDeadTime(&m_htim, &config) != HAL_OK)
    {
        Error_Handler();
    }
}


} // namespace eg {
