/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2025.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

// In case a compiler does not have a way to indicate that a path is deliberately unreachable.

#if __has_builtin(__builtin_unreachable)
#define EG_UNREACHABLE __builtin_unreachable();
#else
#error Provide an equivalent to __builtin_unreachable here that is compatible with your compiler. 
#endif