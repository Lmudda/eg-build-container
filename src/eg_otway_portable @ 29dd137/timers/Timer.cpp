/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#if defined(OTWAY_TARGET_PLATFORM_LINUX)
#include "private/linux/timers/Timer.cpp"
#elif defined(OTWAY_TARGET_PLATFORM_BAREMETAL) || defined(OTWAY_TARGET_PLATFORM_FREERTOS)
#include "private/baremetal/timers/Timer.cpp"
#else
#error OTWAY_TARGET_PLATFORM must be set in CMake. See Otway Portable library README.md.
#endif