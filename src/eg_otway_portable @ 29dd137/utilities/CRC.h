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
#include <array>


// This module defines CRC, a template for performing CRC calculations.


namespace eg {


// This function generates the lookup table for a CRC calculator. The 
// table is returned as an array of 256 elements. The function is 
// used below to generator tables at compile time. 
template <typename T, T Polynomial>
consteval std::array<T, 256U> make_crc_table()
{
    constexpr uint16_t kByteBits  = 8U;
    constexpr uint16_t kValueBits = kByteBits * sizeof(T);
    constexpr T        kHighBit   = 1U << (kValueBits - 1U);
    constexpr uint16_t kTableSize = 256U;

    std::array<T, kTableSize> table{};

    // Calculate the remainder for all possible dividends
    T remainder;
    for (uint16_t dividend = 0; dividend < kTableSize; ++dividend)
    {
        // Start with the dividend followed by zeros
        remainder = (T)(dividend << (kValueBits - kByteBits));

        // Perform modulo-2 division, one bit at a time
        for (uint8_t bit = kByteBits; bit > 0; --bit)
        {
            // Try to divide the current data bit
            if (remainder & kHighBit)
            {
                remainder = (T)((remainder << 1) ^ Polynomial);
            }
            else
            {
                remainder = (T)(remainder << 1);
            }
        }

        // Store the result in the table
        table[dividend] = remainder;
    }

    return table;
}


// This structure represents a lookup table which might be shared by two or more CRC 
// calculators. The table contents depend only on the CRC value type and the polynomial.
// These can be the same for otherwise different CRC definitions. For example, compare 
// CRC-16/AUG-CCITT and CRC-16/CCITT-FALSE, which differ only in the InitialValue. 
// See https://crccalc.com/ for many other CRC definitions. Sharing the lookup table 
// through this type saves a bit of flash in the unlikely event that we have two 
// CRCs in a project with the same polynomial.
template <typename T, T Polynomial>
struct CRCTable
{
    static constexpr std::array<T, 256U> kTable = make_crc_table<T, Polynomial>(); 
};


// This class is used to calculated CRCs based on the definition passed in the template arguments.
// We could factor out a common base for CRCs with the same data type, to avoid duplication
// of the methods. This an optimisation we are unlikely to need since a project will typically 
// use only one CRC.
template <typename T, T Polynomial, T InitialValue, bool ReflectIn, bool ReflectOut, T FinalXorValue>
class CRCCalculator
{
private:
    static constexpr uint16_t kByteBits  = 8U;
    static constexpr uint16_t kValueBits = kByteBits * sizeof(T);
    static constexpr uint16_t kTableSize = 256U;
    using Table = CRCTable<T, Polynomial>;

public:
    // All in one calculation. A convenience function.
    T calculate(const uint8_t* data, uint32_t length)
    {
        //if (!data && length > 0) Error_Handler();

        reset();
        update(data, length);
        return finalise();
    }

    // Convenience helper to avoid casting in the client code. 
    template <typename U>
    T calculate(const U& data)
    {
        return calculate(reinterpret_cast<uint8_t*>(&data), sizeof(U));
    }

    void reset() 
    { 
        m_value = InitialValue;
    }

    // Can be called multiple times before getting a final CRC by calling finalise().
    void update(const uint8_t* data, uint32_t length)
    {        
        //if (!data && length > 0) Error_Handler();

        // Divide the message by the polynomial, one byte at a time.
        for(uint32_t i = 0 ; i < length ; ++i)
        {
            uint8_t byte = 0;
            if constexpr (ReflectIn)
            {
                byte = reflect_byte(data[i]);
            }
            else
            {
                byte = data[i];
            }

            byte    = byte ^ static_cast<uint8_t>(m_value >> (kValueBits - kByteBits));
            m_value = Table::kTable[byte] ^ (T)(m_value << kByteBits);
        }
    }

    // Convenience helper to avoid casting in the client code. 
    template <typename U>
    void update(U data)
    {
        update(reinterpret_cast<uint8_t*>(&data), sizeof(U));
    }

    // Called to perform the final reflection and XOR to obtain the final result. 
    // This does not change m_value so, could be called again after more calls to 
    // update().
    T finalise()
    {
        T value = m_value;
        if constexpr (ReflectOut)
        {
            value = reflect_value(value);
        }

        return value ^ FinalXorValue;
    }

private:
    template <typename U> 
    U reflect_bits(U data)
    {
        constexpr uint8_t kDataBits = sizeof(U) * kByteBits;

        // Reflect the data about the centre bit.
        U reflection = 0;
        for (uint8_t bit = 0; bit < kDataBits; ++bit)
        {
            // If the LSB bit is set, set the reflection of it.
            if ((data & 0x01) != 0)
            {
                reflection |= (1 << ((kDataBits - 1) - bit));
            }
            data = (data >> 1);
        }
        return reflection;
    }

    uint8_t reflect_byte(uint8_t data)
    {
        return reflect_bits(data);
    }

    T reflect_value(T data)
    {
        return reflect_bits(data);
    }

private:
    T m_value{InitialValue};
};


// Convenience types for a selection of CRCs. See https://crccalc.com/.
using CRC16_ARC         = CRCCalculator<uint16_t, 0x8005, 0x0000, true,  true,  0x0000>;
using CRC16_AUG_CCITT   = CRCCalculator<uint16_t, 0x1021, 0x1D0F, false, false, 0x0000>;
using CRC16_BUYPASS     = CRCCalculator<uint16_t, 0x8005, 0x0000, false, false, 0x0000>;
using CRC16_CCITT_FALSE = CRCCalculator<uint16_t, 0x1021, 0xFFFF, false, false, 0x0000>;
using CRC16_CDMA2000    = CRCCalculator<uint16_t, 0xC867, 0xFFFF, false, false, 0x0000>;
using CRC16_DDS_110     = CRCCalculator<uint16_t, 0x8005, 0x800D, false, false, 0x0000>;
using CRC16_DECT_R      = CRCCalculator<uint16_t, 0x0589, 0x0000, false, false, 0x0001>;
using CRC16_DECT_X      = CRCCalculator<uint16_t, 0x0589, 0x0000, false, false, 0x0000>;
using CRC16_DNP         = CRCCalculator<uint16_t, 0x3D65, 0x0000, true,  true,  0xFFFF>;

using CRC32             = CRCCalculator<uint32_t, 0x04C1'1DB7, 0xFFFF'FFFF, true,  true,  0xFFFF'FFFF>;
using CRC32_BZIP2       = CRCCalculator<uint32_t, 0x04C1'1DB7, 0xFFFF'FFFF, false, false, 0xFFFF'FFFF>;
using CRC32_JAMCRC      = CRCCalculator<uint32_t, 0x04C1'1DB7, 0xFFFF'FFFF, true,  true,  0x0000'0000>;

using CRC8_BLUETOOTH    = CRCCalculator<uint8_t, 0xA7, 0x00, true,  true,  0x00>;
using CRC8_AUTOSAR      = CRCCalculator<uint8_t, 0x2F, 0xFF, false, false, 0xFF>;


} // namespace eg {
