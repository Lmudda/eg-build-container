/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "utilities/NonCopyable.h"
#include <cstdint>
#include "signals/Signal.h"


namespace eg {


// TODO_AC Refactor the names to match Otway code. This affects Bugaboo. 
// The struct should be DateTime and should be nested. Ditch the brief comments.

/**
 * @brief Date and Time in 24hr format
 */
struct TimeDate_t
{
    uint8_t  DayInMonth;
    uint8_t  Month;
    uint16_t Year;
    uint8_t  Hour;
    uint8_t  Minute;
    uint8_t  Seconds;
};


class IRtcManager : public NonCopyable
{
    public:
        virtual ~IRtcManager() = default;

        /**
         * @brief Get the current time and date
         * @returns Current time and date as a struct
         */
        [[nodiscard]] virtual TimeDate_t GetTimeDate() const = 0;

        /**
         * @brief Set the time on the RTC
         * Only the time portion of the struct is looked at
         * @param time Time (as a struct) to set on the RTC
         */
        virtual void SetTime(TimeDate_t time) = 0;

        /**
         * @brief Set the date on the RTC
         * Only the date portion of the struct is looked at
         * @param date Date (as a struct) to set on the RTC
         */
        virtual void SetDate(TimeDate_t date) = 0;

        /**
         * @brief Set the date on the RTC
         * Only the date portion of the struct is looked at
         * @param date Date (as a struct) to set on the RTC
         */
        virtual void SetTimeDate(TimeDate_t timeDate) = 0;
	
	    /**
	     * @brief Event raised when the time has been set
	     **/
	    virtual eg::SignalProxy<> OnTimeSet() = 0;
};


} // namespace eg


