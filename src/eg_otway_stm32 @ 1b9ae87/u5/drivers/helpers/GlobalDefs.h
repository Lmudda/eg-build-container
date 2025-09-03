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

// NOTE: We need a way to make this library work with or without the presence of FreeRTOS.
// At least, FreeRTOS is present in the ST code, but might not be used in the application, and
// that might affect parts of the implementation, such as how critical sections work.
//#include "FreeRTOSConfig.h"


namespace eg::GlobalsDefs {


constexpr uint32_t MaxPreemptPrio = 0; //configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
constexpr uint32_t DefPreemptPrio = MaxPreemptPrio;
constexpr uint32_t DefSubPrio     = 0;   


static_assert(DefPreemptPrio >= 0); //configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
static_assert(MaxPreemptPrio >= 0); //configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);


} // namespace eg::GlobalsDefs {
    


