/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/HardwareTimer.h"
#include "stm32u5xx_hal.h"
#include "utilities/ErrorHandler.h"


namespace eg {

static HardwareTimer *g_pHwTimer = nullptr;

HardwareTimer::HardwareTimer(const Config& conf)
: m_conf{conf}
{
    TIMHelpers::enable_clock(conf.tim);
    TIMHelpers::ClockConfig clock_config = TIMHelpers::SimpleTimers::calculate_clock_config(conf.freq);

    m_htim.Instance = reinterpret_cast<TIM_TypeDef *>(conf.tim);
    m_htim.Init.Prescaler = clock_config.prescaler;
    m_htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    m_htim.Init.Period = clock_config.period;
    m_htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&m_htim) != HAL_OK)
    {
        Error_Handler();
        Error_Handler();
    }

    TIM_MasterConfigTypeDef sMasterConfig = {};
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&m_htim, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
        Error_Handler();
    }

    TIMHelpers::irq_handler(m_conf.tim)->connect<&HardwareTimer::irq>(this);

    // TODO_AC This could be moved into the help and deduce the IRQ number from peripheral index.
    HAL_NVIC_SetPriority(m_conf.tim_irqn, 0, 0);
    HAL_NVIC_EnableIRQ(m_conf.tim_irqn);
}

void HardwareTimer::enable()
{
    __HAL_TIM_CLEAR_FLAG(&m_htim, TIM_FLAG_UPDATE);
    HAL_TIM_Base_Start_IT(&m_htim);
}

void HardwareTimer::disable()
{
    HAL_TIM_Base_Stop_IT(&m_htim);
}

bool HardwareTimer::is_running()
{
    return HAL_TIM_STATE_BUSY == HAL_TIM_Base_GetState(&m_htim);
}

void HardwareTimer::change_frequency(const uint32_t freq)
{
    bool restart = false;
    if (HAL_TIM_Base_GetState(&m_htim) == HAL_TIM_STATE_BUSY)
    {
        HAL_TIM_Base_Stop_IT(&m_htim);
        restart = true;
    }

    TIMHelpers::ClockConfig clock_config = TIMHelpers::SimpleTimers::calculate_clock_config(freq);
    m_htim.Init.Prescaler = clock_config.prescaler;
    m_htim.Init.Period = clock_config.period;
    __HAL_TIM_SET_COUNTER(&m_htim, 0u);

    if (HAL_TIM_Base_Init(&m_htim) != HAL_OK)
    {
        Error_Handler();
    }

    if (restart)
    {
        HAL_TIM_Base_Start_IT(&m_htim);
    }
}

void HardwareTimer::change_time_period_ms(uint32_t periodMs)
{
    // check for overflow (0xffffffff / 10)
    if (429496729u <= periodMs)
    {
        Error_Handler();
    }

    bool restart = false;
    if (HAL_TIM_Base_GetState(&m_htim) == HAL_TIM_STATE_BUSY)
    {
        HAL_TIM_Base_Stop_IT(&m_htim);
        restart = true;
    }

    // Set clock to run at 10000Hz (i.e. 0.1ms)
    m_htim.Init.Prescaler = __HAL_TIM_CALC_PSC(TIMHelpers::SimpleTimers::get_input_freq(), 10000);
    m_htim.Init.Period = periodMs * 10;
    __HAL_TIM_SET_COUNTER(&m_htim, 0u);

    if (HAL_TIM_Base_Init(&m_htim) != HAL_OK)
    {
        Error_Handler();
    }

    if (restart)
    {
        HAL_TIM_Base_Start_IT(&m_htim);
    }
}

uint32_t HardwareTimer::get_duration_left_ms()
{
    uint32_t curCnt       = __HAL_TIM_GET_COUNTER(&m_htim);
    uint32_t expiryCnt    = __HAL_TIM_GET_AUTORELOAD(&m_htim) + 1u;
    int64_t  remainingCnt = expiryCnt - curCnt;
    uint32_t clock_freq   = TIMHelpers::SimpleTimers::get_input_freq();
    uint32_t remainingMs  = 0u;

    if (remainingCnt >= 0)
    {
        remainingMs = (1000u * remainingCnt * m_htim.Init.Prescaler) / clock_freq;
    }

    return remainingMs;
}

void HardwareTimer::irq()
{
    g_pHwTimer = this;
    HAL_TIM_IRQHandler(&m_htim);
    g_pHwTimer = nullptr;
}

void HardwareTimer::Expired(TIM_HandleTypeDef *htim)
{
    if (&m_htim == htim)
    {
        HAL_TIM_Base_Stop_IT(htim);
        m_on_update.emit();
    }
    else
    {
        Error_Handler();
    }
}


} // namespace eg {

extern "C"
{
    void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
    {
        if (eg::g_pHwTimer)
        {
            eg::g_pHwTimer->Expired(htim);
        }
    }
}
