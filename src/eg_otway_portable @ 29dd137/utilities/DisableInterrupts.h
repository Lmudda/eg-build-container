/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

// Disables *ALL* interrupts on a platform including those outside the control of Otway
// To disable just interrupts under the control of Otway, use a CriticalSection instead

#if defined(OTWAY_TARGET_PLATFORM_LINUX)
#error DisableInterrupts.h should not be used in Linux projects. See CriticalSection.h
#elif defined(OTWAY_TARGET_PLATFORM_FREERTOS) || defined(OTWAY_TARGET_PLATFORM_BAREMETAL)

#include <cstdint>
#include "NonCopyable.h"

namespace eg {
	
class DisableInterrupts : private NonCopyable
{
public:
    DisableInterrupts()
    {
        // Bare metal disable interrupts recursive for all interrupts
        // i.e. Otway controilled interrupts and uncontrolled interrupts
        platform_disable_interrupts();
        ++m_nested;
    }
	~DisableInterrupts()
    {
        --m_nested;
        if (m_nested == 0)
        {
            platform_enable_interrupts();
        }
    }

private:
    // These are declared in the portable library but must be implemented by the
    // application or platform-specific library.
    static void platform_disable_interrupts();
    static void platform_enable_interrupts();

private:
    inline static uint8_t m_nested;
};

} // namespace eg {

#endif
