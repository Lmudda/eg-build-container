/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "interfaces/IHardwareTimer.h"
#include "drivers/helpers/TIMHelpers.h"
#include "signals/Signal.h"
#include "stm32u5xx_hal.h"


namespace eg
{


class HardwareTimer : public IHardwareTimer
{
public:
    struct Config
    {
        // Driver can be re-used with another TIM
        Tim       tim;
        IRQn_Type tim_irqn;
        uint32_t freq;
    };

public:
    HardwareTimer(const Config& conf);
    void enable() override;
    void disable() override;
	bool is_running() override;

    void change_frequency(uint32_t freq) override;
	void change_time_period_ms(uint32_t periodMs);
	uint32_t get_duration_left_ms();
    SignalProxy<> on_update() override { return SignalProxy<>{m_on_update}; }

private:
    void irq(void);
	void Expired(TIM_HandleTypeDef *htim);
	
	friend void ::HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
	
    const Config&      m_conf;
    TIM_HandleTypeDef  m_htim;
    Signal<>           m_on_update;
};


} // namespace eg {
