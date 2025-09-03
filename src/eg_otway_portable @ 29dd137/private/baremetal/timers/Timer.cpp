/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "timers/Timer.h"
#include "utilities/CriticalSection.h"
#include "utilities/NonCopyable.h"
#include <cstdint>


namespace eg {


// This class manages a doubly linked list of TimerLink objects. A hardware timer ISR decrements a tick
// count at the head of the list, and emits the relevant timer signal. The tick counts for the second and
// subsequent items are differential, so it is only necessary to decrement the head item.
class TimerQueue : public NonCopyable
{
public:
    void insert(Timer* timer);
    void remove(Timer* timer);

private:
    friend void tick_software_timers();
    void on_systick();
    void re_insert(Timer::TimerLink* link);

private:
    Timer::TimerLink* m_head{};
};


// Using a function rather than a global delays initialisation until after the task's emit function has been set.
static TimerQueue& timer_queue()
{
    static TimerQueue s_queue;
    return s_queue;
}


// Call this from the OS ticker
void tick_software_timers()
{
    ++Timer::m_tick_count;
    timer_queue().on_systick();
}


uint32_t Timer::m_tick_count = 0;


Timer::Timer(uint32_t period, Type type)
: m_period{period}
, m_type{type}
, m_is_running{false}
{
}


// Timers generally live forever, but should clean up the timer queue if they do go out of scope.
// Call the equivalent of Timer::stop() in the deconstructor to avoid a call to a virtual function during deconstruction. 
Timer::~Timer()
{
    CriticalSection cs;
    timer_queue().remove(this); 
    m_is_running = false;
}


void Timer::start()
{
    // Also performs restart() if timer is running already.
    CriticalSection cs;
    timer_queue().remove(this);
    timer_queue().insert(this);
    m_is_running = true;
}


void Timer::stop()
{
    CriticalSection cs;
    timer_queue().remove(this);
    m_is_running = false;
}


uint32_t Timer::get_ticks_remaining() const
{
    // Don't want a the list to be modified while doing this calculation
    // (for example, if systick fires while reading the remaining time
    // from a repeating timer that is at the head of the list, it could be
    // moved to the tail of the list and give a spurious result).
    CriticalSection cs;

    const TimerLink* link = &m_link;

    // The timeout cannot exceed 32 bits, so it is safe to accumulate in a uint32_t.
    uint32_t ticks_remaining = 0u;
    while (link != nullptr)
    {
        ticks_remaining += link->ticks;
        link = link->prev;
    }

    return ticks_remaining;
}


void Timer::emit()
{
    m_on_update.emit();
}


// This function is called when SysTick fires. This is called from an ISR.
void TimerQueue::on_systick()
{
    // This is a low priority interrupt. We don't want higher priority interrupts modifying
    // the linked list while we are doing it. Events may be posted from here, but this is
    // safe because we are in interrupt mode (OS functions hard fault if interrupts disabled
    // in thread mode).
    CriticalSection cs;

    if (m_head)
    {
        --m_head->ticks;

        while (m_head && (m_head->ticks == 0))
        {
            // Remove the head item when its tick count reaches zero, and emit an event from the
            // software timer it represents.
            Timer::TimerLink* link = m_head;
            m_head = m_head->next;
            if (m_head)
            {
                m_head->prev = 0;
            }

            link->timer->emit();
            link->next = 0;
            link->prev = 0;

            // Re-insert the item if if is for a recurring timer.
            if (link->timer->get_type() == Timer::Type::Repeating)
            {
                re_insert(link);
            }
            else
            {
                // Make sure a OneShot timer is marked as not running once it fires.
                link->timer->m_is_running = false;
            }
        }
    }
}


void TimerQueue::re_insert(Timer::TimerLink* link)
{
    // This is called from thread mode or interrupt mode. No events are posted so
    // disabled interrupts for the duration is safe.
    CriticalSection cs;

    link->prev  = 0;
    link->next  = 0;

    uint32_t ticks = link->timer->get_period();

    Timer::TimerLink* prev = 0;
    Timer::TimerLink* next = m_head;
    while (next)
    {
        // Find where to insert the item into the list. It is basically a priority queue with sooner
        // timeouts nearer the head.
        if (next->ticks < ticks)
        {
            // Reduce the number of ticks we need to account for by the ticks for each
            // earlier entry in the linked list.
            ticks -= next->ticks;
            prev   = next;
            next   = next->next;
        }
        else
        {
            // Reduce the number of ticks the next entry needs to account for by the ticks we
            // are going to account for.
            next->ticks -= ticks;
            break;
        }
    }

    link->ticks = ticks;

    // We are either at the head of the queue or we need to update the previous
    // item so that we are now its next item.
    if (prev)
    {
        link->prev = prev;
        prev->next = link;
    }
    else
    {
        m_head = link;
    }

    // We are either at the tail of the queue or we need to update the next item
    // so that we are now its prev item.
    if (next)
    {
        link->next = next;
        next->prev = link;
    }
}


void TimerQueue::insert(Timer* timer)
{
    // This is called from thread mode or interrupt mode. No events are posted so
    // disabled interrupts for the duration is safe.
    CriticalSection cs;

    Timer::TimerLink* link = &(timer->m_link);
    link->timer = timer;
    link->prev  = 0;
    link->next  = 0;
    re_insert(link);
}


void TimerQueue::remove(Timer* timer)
{
    // This is called from thread mode or interrupt mode. No events are posted so
    // disabled interrupts for the duration is safe.
    CriticalSection cs;

    Timer::TimerLink* link = 0;
    Timer::TimerLink* prev = 0;
    Timer::TimerLink* next = m_head;
    while (next)
    {
        // Walk the linked list to find instances of our timer.
        if (next->timer != timer)
        {
            prev = next;
            next = next->next;
        }
        else
        {
            // We have found an item we need to remove from the list.
            link = next;
            next = link->next;

            // If the timer to be removed is the head, its next item becomes the
            // head. Otherwise, its previous item's next is updated.
            if (prev)
            {
                prev->next = next;
            }
            else
            {
                // This could be 0.
                m_head = next;
            }

            // If the timer to be removed is not the tail, its next item's prev is
            // updated. The ticks are updated to account for the ticks on the timer
            // we are removing.
            if (next)
            {
                next->ticks += link->ticks;
                next->prev   = prev;
            }

            // link has been removed from the linked list.
            link->prev  = 0;
            link->next  = 0;

            // The timer cannot appear more than once in the list. This is true because
            // there is a one to one relationship between timers and linked objects.
            break;
        }
    }
}


} // namespace eg {
