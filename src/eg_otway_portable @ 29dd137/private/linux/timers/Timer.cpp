/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "timers/Timer.h"
#include <condition_variable>
#include <mutex>
#include <thread>
//#include </usr/include/pthread.h>


namespace eg {


// This is an implementation detail of Timer which maintains a double linked list of running timers.
// Internally runs a thread which waits till soonest-expiring timer expiry. This wait is interrupted
// whenever the queue is modified, so that we can start waiting on the new soonest-expiring timer.
// The queue thread waits forever if there are no running timers, or until the queue itself is
// destroyed.
class TimerQueue : private NonCopyable
{
    public:
        TimerQueue();
        ~TimerQueue();

        // These are called by the Timer implemenation.
        void insert(Timer* timer);
        void remove(Timer* timer);

    private:
        // Queue management
        void insert_impl(Timer* timer);
        void remove_impl(Timer* timer);
        void reinsert_impl(Timer::Link* timer);
        void on_timeout();

        // Thread execution
        static void exec_static(std::stop_token stoken, TimerQueue* self);
        void exec(std::stop_token stoken);

    private:
        std::jthread                m_thread;
        std::mutex                  m_mutex;
        std::condition_variable_any m_condition;
        bool                        m_changed{};
        Timer::Link*                m_head{};
};


// Meyers "singleton". This is initialised on first use and can be used in the
// same ways that a Singleton would be. But the class is not a Singleton, so
// testing can used scoped instances.
TimerQueue& timer_queue()
{
    static TimerQueue queue;
    return queue;
}

static TimerQueue* debug_timers;

TimerQueue::TimerQueue()
{
    debug_timers = this;
    m_thread = std::jthread{&TimerQueue::exec_static, this};
}


TimerQueue::~TimerQueue()
{
    // Stop the worker thread gracefully when we go out of scope.
    m_thread.request_stop();
    m_thread.join();
}


void TimerQueue::exec_static(std::stop_token stoken, TimerQueue* self)
{
    // Forward to a member function to allow access to member data and methods.
    self->exec(stoken);
}


void TimerQueue::exec(std::stop_token stoken)
{
    // This executes in a worker thread which monitors the list of running timers.
    while (true)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (!m_head)
        {
            // Wake up when the queue is modified (i.e. a timer is started) or the queue is destroyed.
            m_condition.wait(lock, stoken, [this]{ return m_changed; });
        }
        else
        {
            auto expiry = m_head->expiry;
            // Wake up when the head timer expires, the queue is modified (i.e. a timer is started or stopped),
            // or the queue is destroyed.
            m_condition.wait_until(lock, stoken, expiry, [this]{ return m_changed; });
        }
        if (stoken.stop_requested()) break;

        // Starting or stopping a timer forces a "spurious" wake up so we can restart the waiting
        // with the revised m_head expiry time. Check here for any that have actually expired.
        if (m_head && (m_head->expiry <= std::chrono::steady_clock::now()))
        {
            on_timeout();
        }

        // This stop condition is set whenever a timer is started or stopped, forcing the current
        // wait to stop.
        m_changed = false;
    }
}


void TimerQueue::on_timeout()
{
    // Already locked as this is called from TimerQueue::exec() after the condition variable forces a wakeup.
    //std::lock_guard<std::mutex> lock(m_mutex);

    if (m_head)
    {
        auto now = std::chrono::steady_clock::now();
        while (m_head && (m_head->expiry <= now))
        {
            // Remove the head item when its tick count reaches zero, and emit an event from the
            // software timer it represents.
            Timer::Link* link = m_head;
            m_head = m_head->next;
            if (m_head)
            {
                m_head->prev = nullptr;
            }

            // Note that emit() is called with m_mutex locked. This is fine since we are just
            // placing an event in one or more queues (in EventLoops). There is a potential for
            // deadlock if we use call() instead (synchronous): if a timer callback tries to
            // start or stop a timer.
            link->timer->emit();

            link->next = nullptr;
            link->prev = nullptr;

            // Re-insert the item if if is for a recurring timer.
            if (link->timer->m_type == Timer::Type::Repeating)
            {
                // This would possibly drift forward in time due to delays.
                // link->expiry = now + link->timer->m_period;
                link->expiry += link->timer->m_period;
                reinsert_impl(link);
            }
        }
    }
}


void TimerQueue::insert(Timer* timer)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    timer->m_link.expiry = std::chrono::steady_clock::now() + timer->m_period;
    remove_impl(timer);
    insert_impl(timer);

    // Wake up the thread
    m_changed = true;
    m_condition.notify_one();
}


void TimerQueue::remove(Timer* timer)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    remove_impl(timer);

    // Wake up the thread
    m_changed = true;
    m_condition.notify_one();
}


void TimerQueue::insert_impl(Timer* timer)
{
    // Already locked by caller
    //std::lock_guard<std::mutex> lock(m_mutex);

    Timer::Link& link = timer->m_link;
    link.timer = timer;
    link.prev  = nullptr;
    link.next  = nullptr;
    reinsert_impl(&link);
}


void TimerQueue::reinsert_impl(Timer::Link* link)
{
    // Already locked by caller
    //std::lock_guard<std::mutex> lock(m_mutex);

    link->prev  = nullptr;
    link->next  = nullptr;
    Timer::Link* prev = nullptr;
    Timer::Link* next = m_head;

    // Walk the list to find where to insert the new running timer.
    // They are ordered with the soonest-to-expire at the head.
    while (next)
    {
        if (next->expiry < link->expiry)
        {
            prev = next;
            next = next->next;
        }
        else
        {
            break;
        }
    }

    if (prev)
    {
        link->prev = prev;
        prev->next = link;
    }
    else
    {
        m_head = link;
    }

    if (next)
    {
        link->next = next;
        next->prev = link;
    }
}


void TimerQueue::remove_impl(Timer* timer)
{
    // Already locked by caller
    //std::lock_guard<std::mutex> lock(m_mutex);

    Timer::Link* link = m_head;


    // Walk the list to find the link corresponding to out timer, and then unhook it.
    while (link)
    {
        if (link->timer != timer)
        {
            link = link->next;
        }
        else
        {
            if (link->prev)
            {
                link->prev->next = link->next;
            }
            else
            {
                // This could be nullptr.
                m_head = link->next;
            }

            if (link->next)
            {
                link->next->prev = link->prev;
            }

            // link has been removed from the linked list.
            link->prev  = nullptr;
            link->next  = nullptr;

            // The timer cannot appear more than once in the list. This is true because
            // there is a one to one relationship between timers and linked objects.
            break;
        }
    }
}


void Timer::start(Millis period, Type type)
{
    // Period cannot be zero. What is reasonable minimum in Linux?
    if (period.count() < 1) return;

    m_period = period;
    m_type   = type;
    timer_queue().insert(this);
}


void Timer::stop()
{
    timer_queue().remove(this);
}


} // namespace eg
