/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once 
#include "signals/Signal.h"
#include <cstdint>


namespace eg {


// This implementation of the software timer relies on the system ticker frequency.
class ITimer
{
public:
    enum class Type { OneShot, Repeating };

public:    
    ITimer() = default;
    virtual ~ITimer() = default;

    // Explicitly delete copy/move 
    ITimer(const ITimer&) = delete;
    ITimer& operator=(const ITimer&) = delete;
    ITimer(ITimer&&) = delete;
    ITimer& operator=(ITimer&&) = delete;

    virtual uint32_t get_period() const = 0;
    virtual void     set_period(uint32_t period) = 0;

    virtual Type get_type() const = 0;
    virtual void set_type(Type type) = 0;
    
    virtual bool is_running() const = 0;
    
    virtual void start() = 0;
    virtual void stop() = 0;

    virtual uint32_t get_ticks_remaining() const = 0;

    virtual SignalProxy<> on_update() = 0;
};


} // namespace eg {
    
    