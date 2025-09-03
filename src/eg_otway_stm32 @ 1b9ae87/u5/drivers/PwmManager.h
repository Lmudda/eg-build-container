#ifndef PWM_MANAGER_H
#define PWM_MANAGER_H

#include "interfaces/IPwmManager.h"
#include "helpers/GPIOHelpers.h"
#include "drivers/helpers/TIMHelpers.h"
#include "stm32u5xx_hal.h"

namespace eg
{
	class PwmManager : public IPwmManager
	{
	public:
		struct Config
		{
			Port       pwm_port;
			Pin        pwm_pin;
			uint8_t    pwm_alt;
			Tim        pwm_tim;
			TimChannel pwm_tim_chan;
			uint32_t   pwm_prescaler;
			uint32_t   pwm_init_period;
		};

		PwmManager(const Config& conf);

		void enable() override;
		void disable() override;

		bool get_state() override;

		void set_duty_cycle(const uint8_t& duty_cycle_percentage) override;
		
	private:
		Config            mConf;
		uint32_t          m_frequency;	    
		TIM_HandleTypeDef m_htim;
	  
	};
} // namespace eg

#endif // !PWM_MANAGER_H
