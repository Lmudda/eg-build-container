/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "utilities/RingBuffer.h"
#include "signals/Signal.h"
#include "utilities/NonCopyable.h"
#include "utilities/CriticalSection.h"
#include "utilities/ErrorHandler.h"
#include "logging/Assert.h"
#include "logging/Logger.h"


// This class could be used on FreeRTOS or Linux systems, but you probably wouldn't.
// #if !defined(OTWAY_TARGET_PLATFORM_BAREMETAL)
// #error This file is requires OTWAY_TARGET_PLATFORM_BAREMETAL to be defined.
// #endif


namespace eg {


template <uint8_t QUEUE_SIZE>
class BareMetalEventLoop : public eg::IEventLoop
{
public:
    BareMetalEventLoop()
    {
	    // Raise the priority to Otway level to prevent any interrupts firing until the
	    // main loop is started
        CriticalSection::enter_otway_level();
    }

    void post(const eg::Event& ev) override
    {
        CriticalSection cs;
        if (m_queue.put(ev) == false)
        {
            // Failed to add to queue, this is a terminal error as an event has been lost.
            EG_ASSERT_FAIL("Event queue has overflowed!");
            Error_Handler(); // LCOV_EXCL_LINE
        }

#if defined(OTWAY_EVENT_LOOP_WATER_MARK)
        // Update high water mark.
        const uint16_t queue_size = m_queue.size();
        if (queue_size > m_high_water_mark)
        {
            m_high_water_mark = queue_size;
        }
#endif
    }

    void run() override
    {
	    // Enter user level. After this point controlled interrupts will be enabled.
        CriticalSection::enter_user_level();

        while (true)
        {
            Event event;
            bool  valid;
            {
                CriticalSection cs;
                valid = m_queue.get(event);
            }

            if (valid)
            {
                event.dispatch();
            }
        }
    }

#if defined(OTWAY_EVENT_LOOP_WATER_MARK)
    uint16_t get_high_water_mark() const override
    {
        return m_high_water_mark;
    }
#endif    

private:
    eg::RingBufferArray<eg::Event, QUEUE_SIZE> m_queue;
#if defined(OTWAY_EVENT_LOOP_WATER_MARK)
    uint16_t                                   m_high_water_mark;
#endif
};


} // namespace eg {
