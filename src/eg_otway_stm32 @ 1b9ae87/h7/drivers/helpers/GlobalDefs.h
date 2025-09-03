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

#error "Not implemented yet for FreeRTOS"

#elif defined(OTWAY_TARGET_PLATFORM_BAREMETAL) 

#include "stm32h7xx_hal.h"

namespace eg::GlobalsDefs {

// Priority level of Otway. Any interrupt prirorities at or below this level will be disabled
// when Otway enters a critical section and are thus under the control of Otway. For now
// Otway will block all interrupts under it's control when in a critical section.
// In future, this could be extended to only explicity disable the interrupt priority level
// of the highest priority interrupt(s) that are members of the critical section.
//
// Any interrupt priorities above this priority will pre-empt any Otway operations
// and so care must be exercised over their use and interation with any Otway or
// user code.
//
// NB The convention for priorities on the ARM/STM32H7 platform are that lower numerical
// values are HIGHER priority - i.e. 0 == Highest Priority, then 1, 2, 3 etc.
constexpr uint8_t MaxPreemptPrio = (((1u << __NVIC_PRIO_BITS) / 2u) - 1u); // defaulted to middle of available levels

// DefPreemptPrio is the default priority at which interrupts under the control of Otway are configured.
// For now set to the maximum controlled priority
constexpr uint8_t DefPreemptPrio = MaxPreemptPrio;

constexpr uint8_t DefSubPrio = 0; // default sub-priority level for NVIC (essentailly defines irq arbitration order)

// UserPrio is the priority at which application code operates and is the lowest
// priority level. This is the highest BASEPRI value the target supports.
constexpr uint8_t UserPrio = ((1u << __NVIC_PRIO_BITS) - 1u); // lowest priority level (i.e. highest BASEPRI value)


} // namespace eg::GlobalsDefs {


#endif


