#include "PwmManager.h"
#include "drivers/helpers/TIMHelpers.h"
#include "drivers/helpers/GPIOHelpers.h"
#include "stm32u5xx_hal_gpio.h"
#include "utilities/ErrorHandler.h"

namespace eg
{
    PwmManager::PwmManager(const Config &conf)
        : mConf(conf)
    {
        TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
        TIM_MasterConfigTypeDef sMasterConfig = { 0 };
        TIM_OC_InitTypeDef sConfigOC = { 0 };

        TIMHelpers::enable_clock(conf.pwm_tim);

        m_htim.Instance =  reinterpret_cast<TIM_TypeDef *>(conf.pwm_tim);
        m_htim.Init.Prescaler = conf.pwm_prescaler - 1;
        m_htim.Init.CounterMode = TIM_COUNTERMODE_UP;
        m_htim.Init.Period = 100 - 1;
        m_htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        m_htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
        if (HAL_TIM_Base_Init(&m_htim) != HAL_OK)
        {
            Error_Handler();
        }

        sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
        if (HAL_TIM_ConfigClockSource(&m_htim, &sClockSourceConfig) != HAL_OK)
        {
            Error_Handler();
        }

        if (HAL_TIM_PWM_Init(&m_htim) != HAL_OK)
        {
            Error_Handler();
        }

        sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
        sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
        if (HAL_TIMEx_MasterConfigSynchronization(&m_htim, &sMasterConfig) != HAL_OK)
        {
            Error_Handler();
        }

        sConfigOC.OCMode = TIM_OCMODE_PWM1;
        sConfigOC.Pulse = conf.pwm_init_period;
        sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
        sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
        if (HAL_TIM_PWM_ConfigChannel(&m_htim, &sConfigOC, static_cast<uint32_t>(conf.pwm_tim_chan)) != HAL_OK)
        {
            Error_Handler();
        }


        GPIO_InitTypeDef init {};

        init.Pin       = 1U << static_cast<uint8_t>(mConf.pwm_pin);
        init.Mode      = GPIO_MODE_AF_PP;
        init.Pull      = GPIO_NOPULL;
        init.Speed     = GPIO_SPEED_FREQ_LOW;
        init.Alternate = conf.pwm_alt;

        GPIOHelpers::enable_clock(mConf.pwm_port);
        GPIO_TypeDef* gpio = reinterpret_cast<GPIO_TypeDef*>(mConf.pwm_port);
        HAL_GPIO_Init(gpio, &init);
    }

    void PwmManager::enable()
    {
        HAL_TIM_PWM_Start(&m_htim, static_cast<uint32_t>(mConf.pwm_tim_chan));
    }

    void PwmManager::disable()
    {
        HAL_TIM_PWM_Stop(&m_htim, static_cast<uint32_t>(mConf.pwm_tim_chan));
    }

    bool PwmManager::get_state()
    {
        return false;
    }

    void PwmManager::set_duty_cycle(const uint8_t& duty_cycle_percentage)
    {
        TIM_OC_InitTypeDef sConfigOC;

        sConfigOC.OCMode = TIM_OCMODE_PWM1;
        sConfigOC.Pulse = duty_cycle_percentage;
        sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
        sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
        if (HAL_TIM_PWM_ConfigChannel(&m_htim, &sConfigOC, static_cast<uint32_t>(mConf.pwm_tim_chan)) != HAL_OK)
        {
            Error_Handler();
        }
    }
}