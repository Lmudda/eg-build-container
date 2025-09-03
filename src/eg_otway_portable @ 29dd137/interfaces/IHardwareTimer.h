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


namespace eg {

class IHardwareTimer : private NonCopyable
{
public:
	virtual ~IHardwareTimer() = default;
	virtual void enable() = 0;
	virtual void disable() = 0;
	virtual void change_frequency(uint32_t freq) = 0;
	virtual bool is_running() = 0;
	virtual void change_time_period_ms(uint32_t periodMs) = 0;
  
	/**
	 * @brief return how long is left on the timer in ms until it raises an interrupt 
	 */
	virtual uint32_t get_duration_left_ms() = 0;
	virtual SignalProxy<> on_update() = 0;
};

} // namespace eg