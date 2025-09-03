/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "interfaces/IUARTDriver.h"
#include "logging/ILoggerBackend.h"


namespace eg {


// This is basically /dev/null for logging on the STM32. 
// Added to avoid conflict when using the debug UART for the 
// comms stack to talk to the Blindsite application rather than USB. 
class NullLoggerBackend : public ILoggerBackend
{
    public:
        NullLoggerBackend()
        {
        }

        void write([[maybe_unused]] const char* message, [[maybe_unused]] bool new_line = false) override
        {
        }
};


} // namespace eg {    