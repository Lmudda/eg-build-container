/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "interfaces/IDigitalInput.h"

namespace eg {

    class TestDigitalInput : public IDigitalInput
    {
      public:
        TestDigitalInput()
            : m_state(false)
        {
        }

        bool read() const { return m_state; }
        SignalProxy<bool> on_change() { return SignalProxy(m_on_change); }

        // test methods
        void set_on()
        {
            m_state = true;
            m_on_change.emit(m_state);
        }

        void set_off()
        {
            m_state = false;
            m_on_change.emit(m_state);
        }

      private:
        bool m_state;
        Signal<bool> m_on_change;
    };

} // namespace eg {

