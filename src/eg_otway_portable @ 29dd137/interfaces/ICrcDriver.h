/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2025.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>

#include "utilities/NonCopyable.h"


namespace eg
{


class ICrcDriver : private NonCopyable
{
    public:
        virtual ~ICrcDriver() = default;

        // The length of the polynomial
        enum class Length
        {
            e_7_bit,
            e_8_bit,
            e_16_bit,
            e_32_bit,
        };

        enum class InputInversion
        {
            e_none,              // No inversion
            e_8_bit_inversion,   // bit-reversal done on each byte
            e_16_bit_inversion,  // bit-reversal done on 16-bit half-words
            e_32_bit_inversion   // bit-reversal done on 32-bit words
        };

        // The method of inversion performed on output data
        enum class OutputInversion
        {
            e_none,              // No inversion
            e_32_bit_inversion   // bit-reversal done on 32-bit words
        };

        // The input data format
        enum class InputDataFormat
        {
            e_8_bit,   //!< Data is a stream of bytes
            e_16_bit,  //!< Data is a stream of 16-bit half-words 
            e_32_bit,  //!< Data is a stream of 32-bit half-words 
        };

        // Configuration of the CRC IP block
        struct Config
        {
            // The default initial value is kDefaultInitialValue
            uint32_t initial_value;

            // The default polynomial value is kDefaultPolynomial
            uint32_t polynomial;

            // The default CRC length is kDefaultCrclength
            Length crc_length;

            // The default iniput inversion is kDefaultInputInversion
            InputInversion input_inversion;

            // the default output inversion is kDefaultOutputInversion
            OutputInversion output_inversion;

            // The default data format is kDefaultInputDataFormat;
            InputDataFormat input_data_format;
        };

        // Initialise the hardware
        virtual void init(const Config& config) = 0;

        // Accumulate new data as part of an on-going calculation. The length is
        // specified in 32-bit words. Returns the updated CRC.
        virtual uint32_t accumulate(uint32_t const *buffer, uint32_t length) = 0;

        // Start a new CRC calculation, and return the new CRC. The length is
        // specified in 32-bit words. 
        virtual uint32_t calculate(uint32_t const *buffer, uint32_t length) = 0;

        // The state of the hardware engine
        enum class State 
        {
            eReset,    //!< In reset, not initialised
            eIdle,     //!< Idle ready to be used
            eBusy,     //!< Busy working
            eTimeout,  //!< Operation timed out
            eError,    //!, In an error condition
        };

        // Query the state of the hardware CRC engine
        virtual State get_state() = 0;

        // Default initial value of the CRC
        static constexpr uint32_t kDefaultInitialValue = 0xFFFFFFFFu;

        // Default polynomial for the CRC - the CRC-32 Ethernet polynomial
        static constexpr uint32_t kDefaultPolynomial = 0x4C11DB7u;

        // Default length for the CRC
        static constexpr Length kDefaultCrclength = Length::e_32_bit;

        // Default input inversion method
        static constexpr InputInversion kDefaultInputInversion = InputInversion::e_none;

        // Default output inversion method
        static constexpr OutputInversion kDefaultOutputInversion = OutputInversion::e_none;

        // Default input data format
        static constexpr InputDataFormat kDefaultInputDataFormat = InputDataFormat::e_32_bit;
};


} // namespace eg
