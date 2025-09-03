/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "ThreadEventLoop.h"


namespace eg {


// Thread local storage to assocate an Emitter with a thread (typically the 
// one owned by an EventLoop). The Emitter is basically a callback for placing 
// an Event into a particular EventLoop's queue. The indirection here decouples 
// Signal<> from EventLoop. We could, in principle, connect Signal to a different 
// type of event handler. 
__thread IEventLoop* g_thread_event_loop;


void set_thread_event_loop(IEventLoop* loop)
{
    g_thread_event_loop = loop;
}

IEventLoop* get_thread_event_loop()
{
    return g_thread_event_loop;
}


ThreadEventLoop::ThreadEventLoop(const char* name)
{
    m_thread = std::jthread{std::jthread{&ThreadEventLoop::exec_static, this, name}};
}


ThreadEventLoop::~ThreadEventLoop()
{
    // Stop the worker thread gracefully when we go out of scope.
    // jthread does not die if this is not here, which seems odd.
    stop();
}


void ThreadEventLoop::post(const Event& event)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(event);
    m_condition.notify_one();
}


void ThreadEventLoop::stop()
{
    if (m_thread.joinable())
    {
        m_thread.request_stop();
        m_thread.join();
    }
}


void ThreadEventLoop::exec_static(std::stop_token stoken, ThreadEventLoop* self, const char* name)
{
    pthread_setname_np(pthread_self(), name);
    set_thread_event_loop(self);
    self->exec(stoken);
}


void ThreadEventLoop::exec(std::stop_token stoken)
{
    while (!stoken.stop_requested())
    {
        std::optional<Event> event{};

        // We need to lock the queue, take an event, and then unlock the queue
        // before dispatching the event. This is because the act of dispatching it
        // may cause further events to be added to the queue, so it needs to be
        // unlocked. The queue is blocked on a condition when it is empty.
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, stoken, [this]{ return m_queue.size() > 0; });
            if (stoken.stop_requested()) break;

            if (m_queue.size() > 0)
            {
                event = m_queue.front();
                m_queue.pop();
            }
        }

        // This is called outside the scope of the mutex lock.
        if (event.has_value())
        {
            event->dispatch();
        }
    }
}


} // namespace eg {
