/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "logging/Assert.h"
#include "logging/Logger.h"
#include <cinttypes> // C99 special format string macros.


namespace eg {


void assert_triggered(const char* file, uint32_t line, const char* function, const char* message)
{
    Logger::log<Logger::Level::Assert>(file, line, function, "%s", message);
    on_assert_triggered(file, line, function, message);   
}    


} // namespace eg {
