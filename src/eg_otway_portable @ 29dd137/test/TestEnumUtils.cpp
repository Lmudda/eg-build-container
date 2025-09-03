/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2025.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "gtest/gtest.h"
#include "utilities/EnumUtils.h"

// Traceability: PRS-131 enumeration utilities

// As the enumutils are essentially just static casts, these tests mostly 
// demonstrate their application and ensure they are covered.

// Define test enums for use in unit tests
enum class TestEnum8 : uint8_t
{
    Value0 = 0,
    Value1 = 1,
    Value255 = 255
};

enum class TestEnum16 : uint16_t
{
    Value0 = 0,
    Value1 = 1,
    ValueMax = 65535
};

enum class TestEnum32 : uint32_t
{
    Value0 = 0,
    Value1 = 1,
    ValueMax = 0xFFFFFFFF
};

// Test to_underlying with various enum types
TEST(EnumCastTests, ToUnderlyingReturnsCorrectValue)
{
    EXPECT_EQ(to_underlying(TestEnum8::Value1), static_cast<uint8_t>(1));
    EXPECT_EQ(to_underlying(TestEnum16::ValueMax), static_cast<uint16_t>(65535));
    EXPECT_EQ(to_underlying(TestEnum32::Value0), static_cast<uint32_t>(0));
}

// Test to_u8 with valid enum
TEST(EnumCastTests, ToU8ReturnsCorrectValue)
{
    EXPECT_EQ(to_u8(TestEnum8::Value255), static_cast<uint8_t>(255));
}

// Test to_u16 with valid enum
TEST(EnumCastTests, ToU16ReturnsCorrectValue)
{
    EXPECT_EQ(to_u16(TestEnum16::ValueMax), static_cast<uint16_t>(65535));
}

// Test to_u32 with valid enum
TEST(EnumCastTests, ToU32ReturnsCorrectValue)
{
    EXPECT_EQ(to_u32(TestEnum32::ValueMax), static_cast<uint32_t>(0xFFFFFFFF));
}