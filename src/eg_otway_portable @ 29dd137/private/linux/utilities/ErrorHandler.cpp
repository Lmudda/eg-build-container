/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////


// Generic error handler called from many places in the system. 
static volatile bool g_continue;
extern "C" void Error_Handler()
{
    // TODO_AC Error_Handler() This is a place holder to allow Linux code to compile. 
    // Should probably log or throw an exception. Would be nice to change the API
    // to pass error information along, but in an embedded-friendly way.

    // Forever loops are UB in C++. I think this will be OK, and it won't be optimised 
    // away (volatile). Can change the value to continue execution at the call site.
    g_continue = false;
    while (!g_continue);
}
