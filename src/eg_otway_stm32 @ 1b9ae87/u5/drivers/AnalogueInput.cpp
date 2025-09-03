/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "stm32u5xx_hal.h"
#include "drivers/AnalogueInput.h"
#include "helpers/ADCHelpers.h"
#include "helpers/GPIOHelpers.h"
#include "signals/InterruptHandler.h"
#include "signals/Signal.h"

// Simple class to configure an IO as an analogue input. See ADC Driver to obtain a reading
// from a configured analogue input

namespace eg {

    AnalogueInput::AnalogueInput(const Config& conf)
    : m_conf{conf}
    {
        GPIOHelpers::configure_as_analogue(m_conf.port, m_conf.pin);
    }

} // namespace eg
