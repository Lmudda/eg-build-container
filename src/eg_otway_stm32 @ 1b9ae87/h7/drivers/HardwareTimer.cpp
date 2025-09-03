/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/HardwareTimer.h"
#include "stm32h7xx_hal.h"
#include "utilities/ErrorHandler.h"
#include "logging/Assert.h"


// We have some dependencies on settings in stm32h7xx_hal_conf.h
#ifndef HAL_TIM_MODULE_ENABLED
#error HAL_TIM_MODULE_ENABLED must be defined to use this driver
#endif
#if (USE_HAL_TIM_REGISTER_CALLBACKS == 0)
#error USE_HAL_TIM_REGISTER_CALLBACKS must be set to use this driver
#endif


namespace eg {


HardwareTimer::HardwareTimer(const Config& conf)
: m_conf{conf}
{
    TIMHelpers::enable_clock(conf.tim);
    TIMHelpers::ClockConfig clock_config = TIMHelpers::SimpleTimers::calculate_clock_config(conf.tim, conf.freq);

    m_htim.Self                   = this;
    m_htim.Instance               = reinterpret_cast<TIM_TypeDef*>(conf.tim);
    m_htim.Init.Prescaler         = clock_config.prescaler;
    m_htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
    m_htim.Init.Period            = clock_config.period;
    m_htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&m_htim) != HAL_OK)
    {
        Error_Handler();
    }
    
    TIM_MasterConfigTypeDef master{};
    master.MasterOutputTrigger = TIM_TRGO_RESET;
    master.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&m_htim, &master) != HAL_OK)
    {
        Error_Handler();
    }

    m_htim.PeriodElapsedCallback = &HardwareTimer::PeriodElapsedCallback;

    // Note this can't be used with advanced timers TIM1 and TIM8 as they have multiple
    // interrupt vectors rather than a single global. Could extend the config to include type.
    TIMHelpers::irq_handler(m_conf.tim, TIMHelpers::IRQType::Global)->connect<&HardwareTimer::irq>(this);
    TIMHelpers::irq_enable(m_conf.tim, TIMHelpers::IRQType::Global, conf.priority);
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
    
    TIMHelpers::ClockConfig clock_config = TIMHelpers::SimpleTimers::calculate_clock_config(m_conf.tim, freq);
    m_htim.Init.Prescaler = clock_config.prescaler;
    m_htim.Init.Period    = clock_config.period;
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
    

void HardwareTimer::change_time_period_ms(uint32_t period_ms)
{
    // check for overflow (0xffffffff / 10)
    if (429496729u <= period_ms)
    {
        Error_Handler();
    }

    bool restart = false;
    if (HAL_TIM_Base_GetState(&m_htim) == HAL_TIM_STATE_BUSY)
    {
        HAL_TIM_Base_Stop_IT(&m_htim);
        restart = true;
    }
    
    // Set clock to run at 10,000Hz (i.e. 0.1ms)
    auto config = TIMHelpers::SimpleTimers::calculate_clock_config(m_conf.tim, 10'000u);
    m_htim.Init.Prescaler = config.prescaler;
    m_htim.Init.Period    = period_ms * 10u;
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
    uint32_t current_cnt   = __HAL_TIM_GET_COUNTER(&m_htim);
    uint32_t expiry_cnt    = __HAL_TIM_GET_AUTORELOAD(&m_htim) + 1u;
    uint64_t remaining_ms  = 0u;

    if (expiry_cnt >= current_cnt)
    {
        uint32_t clock_freq = TIMHelpers::SimpleTimers::get_input_freq(m_conf.tim);
        remaining_ms        = uint64_t{m_htim.Init.Prescaler} * 1000U * (expiry_cnt - current_cnt) / clock_freq;
        EG_ASSERT(remaining_ms <= 0xFFFF'FFFF, "Timer duration overflow.");
    }

    return static_cast<uint32_t>(remaining_ms);
}


void HardwareTimer::irq()
{
    HAL_TIM_IRQHandler(&m_htim);
}


void HardwareTimer::PeriodElapsedCallback()
{
    // This is a callback under interrupt context
    m_on_update.call();        
}


void HardwareTimer::PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
    auto handle = reinterpret_cast<HardwareTimer::TIMHandle*>(htim);
    handle->Self->PeriodElapsedCallback();
}


} // namespace eg {
