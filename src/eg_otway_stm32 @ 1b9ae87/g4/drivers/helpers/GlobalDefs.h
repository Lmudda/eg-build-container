/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <cstdint>


#if defined(OTWAY_TARGET_PLATFORM_FREERTOS)
#include "FreeRTOSConfig.h"


namespace eg::GlobalsDefs {


constexpr uint32_t MaxPreemptPrio = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
constexpr uint32_t DefPreemptPrio = MaxPreemptPrio + 2; // Totally arbitrary.
constexpr uint32_t DefSubPrio     = 0;   

static_assert(MaxPreemptPrio >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
static_assert(DefPreemptPrio >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);


} // namespace eg::GlobalsDefs {


#elif defined(OTWAY_TARGET_PLATFORM_BAREMETAL) 


namespace eg::GlobalsDefs {


constexpr uint32_t MaxPreemptPrio = 0;
constexpr uint32_t DefPreemptPrio = MaxPreemptPrio + 2; // Totally arbitrary.
constexpr uint32_t DefSubPrio     = 0;   


} // namespace eg::GlobalsDefs {


#endif


