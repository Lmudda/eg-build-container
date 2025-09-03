/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once 
#include "interfaces/IRTCDriver.h"
#include "signals/Signal.h"
#include "stm32g4xx.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_rtc.h"


namespace eg {


class RTCDriver : public IRTCDriver
{
    public:
        enum class Clock { eLSI, eLSE };
        struct Config 
        {
            Clock clk_src{Clock::eLSI};
        };

        static constexpr uint32_t kNumBackupRegisters = 32; 

    public:
        RTCDriver(const Config& conf);
        ~RTCDriver() = default;

        [[nodiscard]] DateTime get_datetime() const override;

        void set_time(const Time& time) override;
        void set_date(const Date& date) override;
        void set_datetime(const DateTime& datetime) override;
	
	    eg::SignalProxy<> on_time_set() override
		{
			return eg::SignalProxy<>{m_on_time_set};
		}

        uint32_t get_backup_register(uint32_t index) const override;
        void set_backup_register(uint32_t index, uint32_t value) override;

    private:
        mutable RTC_HandleTypeDef m_rtc{};
        Signal<> m_on_time_set;
};


} // namespace eg
