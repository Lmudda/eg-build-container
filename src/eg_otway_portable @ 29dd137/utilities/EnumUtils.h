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
#include <type_traits>


// Convenience function to Convert an enumerator value to its underlying type for ease of use in 
// places where we need integers. Avoids writing static_cast<> all over the place.
template <typename Enum>
constexpr typename std::underlying_type_t<Enum> to_underlying(Enum e) noexcept 
{
    static_assert(std::is_enum_v<Enum>);
    return static_cast<typename std::underlying_type_t<Enum>>(e);
}


// Convenience functions to converts an enumerator to a specific type.


template <typename Enum>
constexpr typename std::underlying_type_t<Enum> to_u8(Enum e) noexcept 
{
    static_assert(std::is_enum_v<Enum>);
    static_assert(std::is_unsigned_v<std::underlying_type_t<Enum>>);
    static_assert(sizeof(std::underlying_type_t<Enum>) <= sizeof(uint8_t));
    return static_cast<uint8_t>(e);
}


template <typename Enum>
constexpr typename std::underlying_type_t<Enum> to_u16(Enum e) noexcept 
{
    static_assert(std::is_enum_v<Enum>);
    static_assert(std::is_unsigned_v<std::underlying_type_t<Enum>>);
    static_assert(sizeof(std::underlying_type_t<Enum>) <= sizeof(uint16_t));
    return static_cast<uint16_t>(e);
}


template <typename Enum>
constexpr typename std::underlying_type_t<Enum> to_u32(Enum e) noexcept 
{
    static_assert(std::is_enum_v<Enum>);
    static_assert(std::is_unsigned_v<std::underlying_type_t<Enum>>);
    static_assert(sizeof(std::underlying_type_t<Enum>) <= sizeof(uint32_t));
    return static_cast<uint32_t>(e);
}