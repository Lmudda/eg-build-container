/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "signals/InterruptHandler.h"
#include "GlobalDefs.h"
#include <cstdint>
#include "stm32u5xx.h"
#include "stm32u5xx_hal.h"


namespace eg {


	enum class Tim
	{
#if defined(TIM1_BASE)
		Tim1 = TIM1_BASE,
#endif
#if defined(TIM2_BASE)
		Tim2 = TIM2_BASE,
#endif
#if defined(TIM3_BASE)
		Tim3 = TIM3_BASE,
#endif
#if defined(TIM4_BASE)
		Tim4 = TIM4_BASE,
#endif
#if defined(TIM5_BASE)
		Tim5 = TIM5_BASE,
#endif
#if defined(TIM6_BASE)
		Tim6 = TIM6_BASE,
#endif
	};

	enum class TimChannel
	{
		TimChannel1   = TIM_CHANNEL_1,
		TimChannel2   = TIM_CHANNEL_2,
		TimChannel3   = TIM_CHANNEL_3,
		TimChannel4   = TIM_CHANNEL_4,
		TimChannel5   = TIM_CHANNEL_5,
		TimChannel6   = TIM_CHANNEL_6,
		TimChannelAll = TIM_CHANNEL_ALL
	};
		

	struct TIMHelpers
	{
		static InterruptHandler* irq_handler(Tim tim);
		static void enable_clock(Tim tim);

		struct ClockConfig
		{
			uint32_t prescaler;
			uint32_t period;
		};

		struct SimpleTimers
		{
			static uint32_t get_input_freq();
			static ClockConfig calculate_clock_config(uint32_t output_freq);
		}; 
	};


} // namespace eg {
    


