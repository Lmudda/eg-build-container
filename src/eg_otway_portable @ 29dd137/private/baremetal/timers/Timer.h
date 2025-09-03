/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once 
#include "timers/ITimer.h"
#include "signals/Signal.h"
#include "utilities/NonCopyable.h"
#include <cstdint>


namespace eg {


// Can be easily modified to use a variable duration ticker so that we don't
// get interrupts which do nothing but decrement a counter. Use a hardware timer 
// for the ticker.   
    
    
// Call this from the system ticker handler to drive the software timers.
// This is declared extern "C" so that it can be called by the FreeRTOS version
// of SysTick_Handler().
extern "C" void tick_software_timers();


// This implementation of the software timer relies on the system ticker frequency.
class Timer : public ITimer, private NonCopyable
{
//public:
//    enum class Type { OneShot, Repeating };

public:    
    Timer(uint32_t period, Type type);
    ~Timer();

    uint32_t get_period() const          { return m_period; }
    void     set_period(uint32_t period) { stop(); m_period = period; }

    Type get_type() const     { return m_type; } 
    void set_type(Type type)  { stop(); m_type = type; }
    
    bool is_running() const   { return m_is_running; }
    
    void start();
    void stop();

    // This is broken - need to walk the linked list and add up the differentials.
    // Could instead use the absolute tick count at which the timer fires. Rather 
    // than count down, we compare tick count to target time. Then this function 
    // trivial to implement. But... need to be careful about counter rollover.
    // Or use a uint64_t for the target times and tick counter...
    uint32_t get_ticks_remaining() const;

    SignalProxy<> on_update() { return SignalProxy<>{m_on_update}; }
    
    static uint32_t get_tick_count() { return m_tick_count; } 

private:
    // This is called from, for example, the SysTick ISR, or from a worker thread.
    void emit();

private:
    uint32_t   m_period{1};
    Type       m_type{Type::OneShot};
    bool       m_is_running{false};
    Signal<>   m_on_update;
    
    friend void tick_software_timers();
    static uint32_t m_tick_count; 

private:
    // The following data is used to maintain a doubly linked list of running timers. Each timer 
    // **must** appear only once in the linked list. Recurring timers are re-inserted when they
    // fire. The list is effectively a priority queue, with the timer periods converted to 
    // differentials, and those which will fire sooner nearer the head of the list.
    struct TimerLink
    {
        uint32_t   ticks{0u};       // Remaining ticks of the hardware timer before this timer fires 
                                    // (only decrement if this is the head of the queue).
        Timer*     timer{nullptr};  // The timer which this link represents.
        TimerLink* next{nullptr};   // The next timer after this one, if any. 
        TimerLink* prev{nullptr};   // The timer before this one, if any.
    };
    TimerLink m_link{};
    
    friend class TimerQueue;
};


} // namespace eg {
    
    