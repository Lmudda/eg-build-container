#pragma once
#include "interfaces/IDigitalOutput.h"
#include "utilities/NonCopyable.h"


namespace eg
{


class IPwmManager : private NonCopyable
{
    public:
        virtual ~IPwmManager() = default;

        virtual void enable() = 0;
        virtual void disable() = 0;

        virtual bool get_state() = 0;

        virtual void set_duty_cycle(const uint8_t& duty_cycle_percentage) = 0;
};


} // namespace eg
