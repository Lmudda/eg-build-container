/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "utilities/CriticalSection.h"
#include "drivers/helpers/GlobalDefs.h"
#include "logging/Assert.h"


namespace eg {


#if defined(OTWAY_TARGET_PLATFORM_FREERTOS)

#error "Not implemented yet for FreeRTOS"

#endif


#if defined(OTWAY_TARGET_PLATFORM_BAREMETAL)

CriticalSection::CriticalSection()
{
    m_previous_level = platform_raise_interrupt_priority_level(GlobalsDefs::MaxPreemptPrio);
}

CriticalSection::CriticalSection(uint8_t new_ipl)
{
    m_previous_level = platform_raise_interrupt_priority_level(new_ipl);
}

uint8_t CriticalSection::platform_raise_interrupt_priority_level(uint8_t new_level)
{
    EG_ASSERT(new_level >= GlobalsDefs::MaxPreemptPrio, "Bad IPL level in CriticalSection");

    // Data synchronisation barrier - required to ensure all writes to memory are
    // complete in the CPU pipeline before the priority level is raised
    __DSB();
    // Store the current priority level to return
    unsigned int previous_level = __get_BASEPRI() >> (8u - __NVIC_PRIO_BITS);
    // Set the new priority level if requested is higher (higher pri is lower value)
    if (new_level < previous_level)
    {
        __set_BASEPRI(new_level << (8u - __NVIC_PRIO_BITS));
        // Instruction synchronisation barrier - required to ensure the MSR instruction used
        // to set the BASEPRI register has been executed in the pipeline (and thus actioned)
        // before any further operations are completed by the CPU.
        __ISB();
    }

    return previous_level;
}

void CriticalSection::platform_restore_interrupt_priority_level(uint8_t previous_level)
{
    EG_ASSERT(previous_level >= GlobalsDefs::MaxPreemptPrio, "Bad IPL level in CriticalSection");

    // Data synchronisation barrier - required to ensure all writes to memory are
    // complete in the CPU pipeline before the priority level is reduced
    __DSB();
    // Restore the priority level
    __set_BASEPRI(previous_level << (8u - __NVIC_PRIO_BITS));
    // Instruction synchronisation barrier - required to ensure the MSR instruction used
    // to set the BASEPRI register has been executed in the pipeline (and thus actioned)
    // before any further operations are completed by the CPU.
    __ISB();
}


void CriticalSection::enter_otway_level()
{
    // Set the priority level
    __set_BASEPRI(eg::GlobalsDefs::MaxPreemptPrio << (8u - __NVIC_PRIO_BITS));
    // Instruction synchronisation barrier - required to ensure the MSR instruction used
    // to set the BASEPRI register has been executed in the pipeline (and thus actioned)
    // before any further operations are completed by the CPU.
    __ISB();
}

void CriticalSection::enter_user_level()
{
    // Set the priority level
    __set_BASEPRI(eg::GlobalsDefs::UserPrio << (8u - __NVIC_PRIO_BITS));
    // Instruction synchronisation barrier - required to ensure the MSR instruction used
    // to set the BASEPRI register has been executed in the pipeline (and thus actioned)
    // before any further operations are completed by the CPU.
    __ISB();
}

#endif


} // namespace eg {
