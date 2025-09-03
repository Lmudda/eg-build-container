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


// Abstract interace for digital outputs. This is inherited and implemented in 
// the platform-specific files such as DigitalOutput.cpp.
class IDigitalOutput : private NonCopyable 
{
    public:    
        virtual ~IDigitalOutput() = default;
        virtual void set() = 0;  
        virtual void reset() = 0;
        virtual void toggle() = 0;
        virtual void set_to(bool state) = 0;   
        virtual bool read() = 0;
};

    
}
