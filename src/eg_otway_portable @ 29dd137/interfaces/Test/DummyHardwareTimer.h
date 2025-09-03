/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "interfaces/IHardwareTimer.h"
#include "timers/Timer.h"

using eg::IHardwareTimer;

namespace eg
{
    class DummyHardwareTimer final : public IHardwareTimer
    {
        public:
        
        DummyHardwareTimer()
        : mTimer(UINT32_MAX, eg::Timer::Type::OneShot)
        {
            mTimer.on_update().connect<&DummyHardwareTimer::OnTimerElapsed>(this);
        }

        void enable() override
        {
            mIsEnabled = true;
            mTimer.start();
        }

        void disable() override
        {
            mIsEnabled = false;
            mTimer.stop();
        }

        void change_frequency(uint32_t freq) override
        {
            mFrequency = freq;
        }
	    
	    bool is_running() override
	    {
		    return mIsEnabled;
	    }

        void change_time_period_ms(uint32_t periodMs) override
        {
            mPeriod = periodMs;
            mTimer.set_period(periodMs);
        }

        uint32_t get_duration_left_ms()  override
        {
            return mTimer.get_ticks_remaining();
        }

        eg::SignalProxy<> on_update() override {return eg::SignalProxy<>{ mElapsed };}

        // DUMMY METHODS
        bool DummyIsEnabled() { return mIsEnabled; }
        uint32_t DummyGetFrequency() { return mFrequency; }
        uint32_t DummyGetTimePeriod() { return mPeriod; }

    private:
        eg::Signal<> mElapsed;
        eg::Timer mTimer;
        bool mIsEnabled = false;
        uint32_t mFrequency;
        uint32_t mPeriod;

        
        void OnTimerElapsed() { 
            mIsEnabled = false;
            mElapsed.emit(); 
        }
    };
}