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
#include "signals/Signal.h"
#include <cstdint>


namespace eg {


// Simple alternative to IRtcManager with the addition of access to backup registers.
// Added to avoid breaking existing projects, and to change the naming convention to 
// match the rest of Otway.
class IRTCDriver : public NonCopyable
{
    public:
        struct Date
        {
            uint16_t year;
            uint8_t  month;
            uint8_t  day;     
        };

        struct Time
        {
            uint8_t  hour;    
            uint8_t  minute;
            uint8_t  second;
        };

        struct DateTime
        {
            Date date;
            Time time;
        };

    public:
        virtual ~IRTCDriver() = default;

        [[nodiscard]] virtual DateTime get_datetime() const = 0;

        virtual void set_time(const Time& time) = 0;
        virtual void set_date(const Date& date) = 0;
        virtual void set_datetime(const DateTime& datetime) = 0;

        virtual eg::SignalProxy<> on_time_set() = 0;

        // Simple access to backup registers. For now the driver is assumed to 
        // do nothing special with the anti-tamper features of the device. The
        // backup register are in the same power domain as the RTC and are 
        // accessed (at least with ST HAL) through the same peripheral proxy object. 
        //
        // The number of backup registers depend on the platform, so it is not possible
        // to enforce a correct index here with an enumeration. It is the responsibility 
        // of the implmentation to assert that the index is valid.
        virtual uint32_t get_backup_register(uint32_t index) const = 0;
        virtual void set_backup_register(uint32_t index, uint32_t value) = 0;
};


} // namespace eg


