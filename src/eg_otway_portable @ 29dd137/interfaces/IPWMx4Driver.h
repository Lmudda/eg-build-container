#pragma once
#include "interfaces/IDigitalOutput.h"
#include "utilities/NonCopyable.h"


namespace eg
{


class IPWMx4Driver : private NonCopyable
{
    public:
        // TODO_AC For one current application, we need to support
        // up to 4 channels and we are not using the inverted outputs (where they exist).
        enum class Channel : uint8_t { Channel1, Channel2, Channel3, Channel4 };

    public:
        virtual ~IPWMx4Driver() = default;

        // A disaibled channel is equivalent to duty cycle of 0%
        virtual void enable(Channel channel) = 0;
        virtual void disable(Channel channel) = 0;

        // Enabled state? PwmManager returns false anyway.
        //virtual bool get_state(Channel channel) = 0;

        // We can use the output as a simple GPIO by setting either 0% or 100%. 
        // Any value greater than 100 will be silently capped at 100. Using 1000 steps
        // gives finer control.
        virtual void set_duty_cycle_percent(Channel channel, uint16_t duty_cycle) = 0;
        virtual void set_duty_cycle_permille(Channel channel, uint16_t duty_cycle) = 0;
};


} // namespace eg
