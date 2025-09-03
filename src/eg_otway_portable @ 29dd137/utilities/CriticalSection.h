/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "NonCopyable.h"
#if defined(OTWAY_TARGET_PLATFORM_LINUX)
#include <mutex>
#elif defined(OTWAY_TARGET_PLATFORM_FREERTOS)
//#include "FreeRTOS.h"
#elif defined(OTWAY_TARGET_PLATFORM_BAREMETAL) 
#endif


namespace eg { 


// This class is intended to provide a common critical section API for different platforms. 
// Simply declare an instance at the beginning of the scope and Presto! you have a critical 
// section until the end of that scope.
//
// A critical section is a kind of lightweight system-wide mutex which is quick and simple, but 
// typically blocks more of the system (i.e. it's a blunt instrument).
//
// - Bare metal systems typically disable interrupts to serialise access to data which is used 
//   in both application code and ISRs. Some architectures allow only interrupts below a given 
//   priority to be disabled, so as not to interfere with latency of key high priority interrupts.
//   I've never made use of this feature directly.
//
// - FreeRTOS has a concept of a critical section in its API. This basically just disables 
//   interrupts, but makes use of the ability to leave high priority interrupts enabled on 
//   devices which have this feature (e.g. ARM Cortex-M4).
// - Linux does not have a concept of a critical section, but this can be simulated easily with 
//   a shared mutex.


#if defined(OTWAY_TARGET_PLATFORM_LINUX)


class CriticalSection : private NonCopyable
{
    public:
        CriticalSection()
        {
            m_mutex.lock();
        }
        ~CriticalSection()
        {
            m_mutex.unlock();
        }       

    private:
        // Or maybe recursive_mutex;
        inline static std::mutex m_mutex{};
};


#elif defined(OTWAY_TARGET_PLATFORM_FREERTOS)


class CriticalSection : private NonCopyable
{
    public:
        CriticalSection()
        {
            // FreeRTOS enter critical recursive
            platform_enter_critical();
            ++m_nested;
        }
        ~CriticalSection()
        {
            --m_nested;
            if (m_nested == 0)
            {
                platform_exit_critical();
            }
        }       

    private: 
        static void platform_enter_critical();
        static void platform_exit_critical();

    private:
        inline static uint8_t m_nested;  
};


#elif defined(OTWAY_TARGET_PLATFORM_BAREMETAL)

class CriticalSection : private NonCopyable
{
	public:
		CriticalSection();
		CriticalSection(uint8_t new_ipl);
		~CriticalSection()
		{
			platform_restore_interrupt_priority_level(m_previous_level);
		}       

    // These may need to be declared in the application or platform-specific library.
	static void enter_otway_level();
	static void enter_user_level();
	
	private: 
		// These are declared in the portable library but must be implemented by the 
		// application or platform-specific library.
		uint8_t platform_raise_interrupt_priority_level(uint8_t new_level);
		void platform_restore_interrupt_priority_level(uint8_t prev_level);

	private:
		uint8_t m_previous_level;  
};


#endif


} // namespace eg { 
