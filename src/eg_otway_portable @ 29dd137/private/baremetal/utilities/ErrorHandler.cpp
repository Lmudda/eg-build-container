/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "utilities/DisableInterrupts.h"


// Generic error handler called from many places in the system.
// Could change the API so that each application can optionally set an alternative handler.
// Excluded from test coverage as this deliberately hangs your program. LCOV_EXCL_START
static volatile bool g_continue;
void(*g_error_handler)(void) = nullptr;
extern "C" void Error_Handler()
{
    eg::DisableInterrupts di;

    // Allow target specific error handler
    if (g_error_handler)
    {
        g_error_handler();
    }

    // Forever loops are UB in C++. I think this will be OK, and it won't be optimised
    // away (volatile). Can change the value to continue execution at the call site.
    g_continue = false;
    while (!g_continue);
}
// LCOV_EXCL_STOP