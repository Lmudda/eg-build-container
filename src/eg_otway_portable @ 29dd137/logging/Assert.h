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


// EG_ASSERT does not execute the predicate (i.e. has no side-effects) when asserts are disabled.
// "do { stuff } while (0)" is more commonly used than "( stuff )".
#undef EG_ASSERT
#if defined(OTWAY_DISABLE_ASSERTS)
#define EG_ASSERT(predicate, message) (static_cast<void>(0))
#else
#define EG_ASSERT(predicate, message) do { if (!(predicate)) ::eg::assert_triggered(__FILE__, __LINE__, __func__, message); } while (false)
#endif


// EG_VERIFY executes the predicate (i.e. may have side-effects) when asserts are disabled.
// "do { stuff } while (0)" is more commonly used than "( stuff )".
#undef EG_VERIFY
#if defined(OTWAY_DISABLE_ASSERTS)
#define EG_VERIFY(predicate, message) do { static_cast<void>(predicate); } while (false)
#else
#define EG_VERIFY(predicate, message) EG_ASSERT(predicate, message)
#endif


// Not sure how useful this is but a harmless convenience.
#define EG_ASSERT_FAIL(message) EG_ASSERT(false, message)


namespace eg {


// This is called whenever an assertion fails. It logs a message and then calls on_assert_triggered().
void assert_triggered(const char* file, uint32_t line, const char* function, const char* message);  

// Including the filename might explode the image size because the full path is included. Another 
// effect is that binaries compiled on different systems are not necessarily the same because of full paths.
// There appears to be no simple way to limit the filename to exclude the path.
//void assert_triggered_with_file(char const* file, uint32_t line, const char* function, const char* message);    

// This is called by assert_triggered() whenever an assertion fails. The implementation is left for each
// application to provide because different systems may want to behave differently with watchdogs or 
// whatever. The implementation could theoretically do nothing, but it should almost certainly stop the 
// execution of the application in some way. No default implementation is provided so that applications
// will see a linker error if they don't implement this function.
void on_assert_triggered(char const* file, uint32_t line, const char* function, const char* message);    


} // namespace eg {
