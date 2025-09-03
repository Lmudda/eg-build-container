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

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_crc.h"

#include "interfaces/ICrcDriver.h"

namespace eg {


// Manages the single CRC hardware IP block in the CPU
class CrcDriver : public ICrcDriver
{
public:
    CrcDriver(const Config& config);

    // Initialise the hardware
    void init(const Config& config) override;

    // Accumulate new data as part of an on-going calculation. The length is
    // specified in 32-bit words. Returns the updated CRC.
    uint32_t accumulate(uint32_t const *buffer, uint32_t length) override;

    // Start a new CRC calculation, and return the new CRC. The length is
    // specified in 32-bit words. 
    uint32_t calculate(uint32_t const *buffer, uint32_t length) override;

    // Query the state of the hardware CRC engine
    State get_state() override;

private:

    // Convert the length passed in, in 32-bit words, into the length
    // required for the CRC HAL function. This depends on the input data
    // format specified in the configuration.
    uint32_t required_length(uint32_t length);

    CRC_HandleTypeDef m_crc_instance{};
};


} // namespace eg {
