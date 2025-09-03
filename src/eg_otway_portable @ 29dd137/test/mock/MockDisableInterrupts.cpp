/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2025.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

#if defined(OTWAY_TARGET_PLATFORM_BAREMETAL) 

#include "utilities/DisableInterrupts.h"
namespace eg 
{ 
void DisableInterrupts::platform_disable_interrupts() { }
void DisableInterrupts::platform_enable_interrupts() { }
}
#endif  // defined(OTWAY_TARGET_PLATFORM_BAREMETAL) 