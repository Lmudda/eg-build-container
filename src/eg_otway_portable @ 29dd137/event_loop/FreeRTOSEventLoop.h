/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "freertos/FreeRTOSQueue.h"
#include "freertos/FreeRTOSThread.h"
#include "signals/Signal.h"
#include "task.h"


#if !defined(OTWAY_TARGET_PLATFORM_FREERTOS)
#error This file requires OTWAY_TARGET_PLATFORM_FREERTOS to be defined.
#endif


namespace eg {


// This event loop implementation uses a private FreeRTOS thread to run the loop in its
// own execution context.
template <uint32_t StackSizeBytes, uint32_t EventQueueSize>
class FreeRTOSEventLoop : public IEventLoop
{
    public:
        FreeRTOSEventLoop(const char* name, osPriority_t priority)
        : m_thread{name, this, priority}
        {

        }
        //~FreeRTOSEventLoop();

        void post(const Event& event) override
        {
            m_thread.post(event);
        }

        void run() override
        {            
            // Nothing to do here. Could create the thread without starting it?
        }

    private:
        using ThreadBase = FreeRTOSThread<StackSizeBytes>;
        using Queue      = FreeRTOSQueue<Event, EventQueueSize>;

        class Thread : public ThreadBase
        {
        public:
            Thread(const char* name, IEventLoop* loop, osPriority_t priority) 
            : ThreadBase{name, priority}
            {
                // The TLS thread ID used when dispatching events to be handled in this thread.
                // Is it safe to do this outside of the thread's execution context?
                static_assert(configNUM_THREAD_LOCAL_STORAGE_POINTERS >= 1);
                TaskHandle_t id = static_cast<TaskHandle_t>(ThreadBase::id());
                vTaskSetThreadLocalStoragePointer(id, 0, loop);
            }

            void post(const Event& event) 
            {
                // This should be thread safe in FreeRTOS. 
                // If not, we can add a critical section.
                m_queue.put(event);
            }

        private:
            void execute() override
            {
                // The TLS thread ID used when dispatching events to be handled in this thread.
                // static_assert(configNUM_THREAD_LOCAL_STORAGE_POINTERS >= 1);
                // TaskHandle_t id = static_cast<TaskHandle_t>(FreeRTOSThread::id());
                // vTaskSetThreadLocalStoragePointer(id, 0, &m_loop);

                while (true)
                {
                    // This should be thread safe in FreeRTOS. 
                    // If not, we can add a critical section.
                    // Blocks when the queue is empty.
                    if (m_queue.get(m_event))
                    {
                        m_event.dispatch();
                    }
                }
            }

        private:
            // Pending events
            Queue m_queue{}; 
            // Active event - made a member to take it off the stack
            Event m_event{};
        };

    private:
        Thread m_thread;            
};


} // namespace eg {


