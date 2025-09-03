/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef RTC_MANAGER
#define RTC_MANAGER

#include "interfaces/IRtcManager.h"
#include "stm32u5xx.h"
#include "stm32u5xx_hal.h"
#include "stm32u5xx_hal_rtc.h"
#include "signals/Signal.h"


namespace eg
{
	class RtcManager : public IRtcManager
	{
	public:
		RtcManager();
		~RtcManager() = default;
        
		[[nodiscard]] TimeDate_t GetTimeDate() const override;
        void SetTime(TimeDate_t time) override;
        void SetDate(TimeDate_t date) override;
        void SetTimeDate(TimeDate_t timeDate) override;
		
		eg::SignalProxy<> OnTimeSet()
		{
			return eg::SignalProxy<>{ mTimeChanged };
		}
        
	private:
		RTC_HandleTypeDef m_rtc;
		RTC_DateTypeDef m_rtc_date {};
		RTC_TimeTypeDef m_rtc_time {};
		
		eg::Signal<> mTimeChanged;
    };
}

#endif // !RTC_MANAGER