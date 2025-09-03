/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <thread>
#include <mutex>
#include <optional>
#include <condition_variable>
#include "signals/Signal.h"
#include "utilities/RingBuffer.h"


// This class combines a thread with a RingBuffer to make an event loop running in 
// another thread. It's a bit easier in FreeRTOS because the RTOS queue is itself 
// threadsafe, and you can block on it. The method used to associate the thread with 
// an emit function is a bit clunky, but does work. Room for improvement. 
class TestEventLoop : public eg::IEventLoop
{
    public:
        TestEventLoop()
        : m_thread{&TestEventLoop::exec_static, this}
        {
        }

        ~TestEventLoop()
        {
            stop();
        }

        void post(const eg::Event& event) override
        {      
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.put(event);
            m_condition.notify_one();
        }

        void run() override
        {
            // Thread was started in the ctor already.
            // For consistency this should not return.
        }

        void stop()
        {
            if (m_thread.joinable())
            {
                m_thread.request_stop();
                m_thread.join();
            }
        }

        std::thread::id get_id() { return m_thread.get_id(); }

        
    #if defined(OTWAY_EVENT_LOOP_WATER_MARK)
        uint16_t get_high_water_mark() const { return 0; }
    #endif

    private:   
        static void exec_static(std::stop_token stoken, TestEventLoop* self)
        {
            // This is thread local storage
            m_self = self;
            self->exec(stoken);
        }

        void exec(std::stop_token stoken)
        { 
            while (!stoken.stop_requested())
            { 
                std::optional<eg::Event> event;

                // We need to lock the queue, take an event, and then unlock the queue 
                // before dispatching the event. This is because the act of dispatching it 
                // may cause further events to be added to the queue, so it needs to be 
                // unlocked.                
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

                if (event.has_value())
                {
                    event->dispatch();
                }
            }
        }

    private:
        std::jthread                        m_thread;
        std::mutex                          m_mutex;
        std::condition_variable_any         m_condition;
        eg::RingBufferArray<eg::Event, 16>  m_queue;

    private:
        friend eg::IEventLoop* eg::default_event_loop_impl();
        friend eg::IEventLoop* eg::this_event_loop_impl();
        static thread_local TestEventLoop* m_self;
};
