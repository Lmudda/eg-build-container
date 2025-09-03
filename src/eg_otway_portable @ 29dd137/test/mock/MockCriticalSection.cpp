/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2025.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

#if defined(OTWAY_TARGET_PLATFORM_BAREMETAL) 

#include "utilities/CriticalSection.h"

namespace eg {

CriticalSection::CriticalSection()
: m_previous_level{0u}
{
}

CriticalSection::CriticalSection([[maybe_unused]] uint8_t new_ipl)
: m_previous_level{0u}
{
}

uint8_t CriticalSection::platform_raise_interrupt_priority_level([[maybe_unused]] uint8_t new_level)
{
    return 0;
}

void CriticalSection::platform_restore_interrupt_priority_level([[maybe_unused]] uint8_t previous_level)
{
}


void CriticalSection::enter_otway_level()
{
}

void CriticalSection::enter_user_level()
{
}

} // namespace eg {

#endif  // defined(OTWAY_TARGET_PLATFORM_BAREMETAL) 