/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "utilities/NonCopyable.h"


namespace eg {


// Abstract interace for analogue outputs. This is inherited and implemented in
// the platform-specific files such as AnalogueOutput.cpp.
class IAnalogueOutput : private NonCopyable 
{
    public:
        virtual ~IAnalogueOutput() = default;
        virtual void set(uint32_t value) = 0;
        virtual uint32_t get() = 0;
        virtual void enable() = 0;
        virtual void disable() = 0;
};


}
