/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "interfaces/IDigitalOutput.h"

namespace eg {


// Abstract interace for digital outputs. This is inherited and implemented in 
// the platform-specific files such as DigitalOutput.cpp.
class TestDigitalOutput : public IDigitalOutput
{
    public:  
        TestDigitalOutput()  
            : m_state(false)
        {}

        ~TestDigitalOutput() {}

        void set() {m_state = true;}  
        void reset() {m_state = false;}
        void toggle() {m_state = !m_state;}
        void set_to(bool state) {m_state = state;}   
        bool read() {return m_state;}

    private:
        bool m_state;
};

    
}