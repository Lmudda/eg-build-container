/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "stm32h7xx.h"
#include <cstdint>
#include "utilities/DisableInterrupts.h"


// These functions are declared in eg_otway_portable but left to be implemented
// along with the platform-specific drivers.
namespace eg {

void DisableInterrupts::platform_disable_interrupts()
{
    // Data synchronisation barrier - required to ensure all writes to memory are
    // complete in the CPU pipeline before the priority level is raised
    __DSB();
    // Disable interrupts
    __disable_irq(); // sets PRIMASK to 1
    // Instruction synchronisation barrier - required to ensure the CPSID instruction used
    // to set the PRIMASK register has been executed in the pipeline (and thus actioned)
    // before any further operations are completed by the CPU.
    __ISB();
}


void DisableInterrupts::platform_enable_interrupts()
{
    // Data synchronisation barrier - required to ensure all writes to memory are
    // complete in the CPU pipeline before the priority level is reduced
    __DSB();
    // Restore the priority level
    __enable_irq(); // sets PRIMASK to 0
    // Instruction synchronisation barrier - required to ensure the CPSIE instruction used
    // to set the PRIMASK register has been executed in the pipeline (and thus actioned)
    // before any further operations are completed by the CPU.
    __ISB();
}


} // namespace eg {
