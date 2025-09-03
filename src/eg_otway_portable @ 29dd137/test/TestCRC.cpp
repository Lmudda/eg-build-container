/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "gtest/gtest.h"
#include "utilities/CRC.h"

// Tracability: 
// PRS-106 defines the CRC calculator. 
// PRS-107 defines the CRC lookup table creation, called as part of the tests. 

class CRCTest: public testing::Test
{
protected:
    static constexpr uint8_t  data1[] = { 0x12, 0x34, 0x56, 0x78, 0x09 };
    static constexpr uint32_t size1   = std::size(data1);

    static constexpr uint8_t  data2[] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };
    static constexpr uint32_t size2   = std::size(data2);

    static constexpr uint8_t  data3[] = 
    { 
        // "The cat sat on the mat dreaming of this and that".
        0x54, 0x68, 0x65, 0x20, 0x63, 0x61, 0x74, 0x20, 0x73, 0x61, 0x74, 0x20, 0x6f, 0x6e, 0x20, 0x74, 
        0x68, 0x65, 0x20, 0x6d, 0x61, 0x74, 0x20, 0x64, 0x72, 0x65, 0x61, 0x6d, 0x69, 0x6e, 0x67, 0x20, 
        0x6f, 0x66, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x74, 0x68, 0x61, 0x74, 
    };
    static constexpr uint32_t size3   = std::size(data3);

    // Concatenate data1, data2 and data3.
    static constexpr uint8_t  data4[] = 
    { 
        0x12, 0x34, 0x56, 0x78, 0x09,
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x54, 0x68, 0x65, 0x20, 0x63, 0x61, 0x74, 0x20, 0x73, 0x61, 0x74, 0x20, 0x6f, 0x6e, 0x20, 0x74, 
        0x68, 0x65, 0x20, 0x6d, 0x61, 0x74, 0x20, 0x64, 0x72, 0x65, 0x61, 0x6d, 0x69, 0x6e, 0x67, 0x20, 
        0x6f, 0x66, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x74, 0x68, 0x61, 0x74, 
    };
    static constexpr uint32_t size4   = std::size(data4);
};


// 16-bits. Non-zero initial. No reflection in or out. Zero XOR value.
TEST_F(CRCTest, TestCRC16_CCITT_FALSE)
{
    // These value obtained by using datan for input at https://crccalc.com/
    constexpr uint16_t crc1 = 0x4B7A;
    constexpr uint16_t crc2 = 0x29B1;
    constexpr uint16_t crc3 = 0xC398;
    constexpr uint16_t crc4 = 0x6A2F;

    eg::CRC16_CCITT_FALSE crc;

    // Single pass calculation.
    EXPECT_TRUE(crc.calculate(data1, size1) == crc1);
    EXPECT_TRUE(crc.calculate(data2, size2) == crc2);
    EXPECT_TRUE(crc.calculate(data3, size3) == crc3);
    EXPECT_TRUE(crc.calculate(data4, size4) == crc4);

    // Single update calculation - unwrapped single pass.
    crc.reset();
    crc.update(data1, size1);
    EXPECT_TRUE(crc.finalise() == crc1);
    crc.reset();
    crc.update(data2, size2);
    EXPECT_TRUE(crc.finalise() == crc2);
    crc.reset();
    crc.update(data3, size3);
    EXPECT_TRUE(crc.finalise() == crc3);
    crc.reset();
    crc.update(data4, size4);
    EXPECT_TRUE(crc.finalise() == crc4);

    // Multiple update calculation.
    crc.reset();
    crc.update(data1, size1);
    crc.update(data2, size2);
    crc.update(data3, size3);
    EXPECT_TRUE(crc.finalise() == crc4);
}


// 16-bits. Zero initial. No reflection in or out. Non-zero XOR value.
TEST_F(CRCTest, TestCRC16_DECT_R)
{
    // These value obtained by using datan for input at https://crccalc.com/
    constexpr uint16_t crc1 = 0xB82B;
    constexpr uint16_t crc2 = 0x007E;
    constexpr uint16_t crc3 = 0x0D2C;
    constexpr uint16_t crc4 = 0x1AC8;

    eg::CRC16_DECT_R crc;

    // Single pass calculation.
    EXPECT_TRUE(crc.calculate(data1, size1) == crc1);
    EXPECT_TRUE(crc.calculate(data2, size2) == crc2);
    EXPECT_TRUE(crc.calculate(data3, size3) == crc3);
    EXPECT_TRUE(crc.calculate(data4, size4) == crc4);

    // Single update calculation - unwrapped single pass.
    crc.reset();
    crc.update(data1, size1);
    EXPECT_TRUE(crc.finalise() == crc1);
    crc.reset();
    crc.update(data2, size2);
    EXPECT_TRUE(crc.finalise() == crc2);
    crc.reset();
    crc.update(data3, size3);
    EXPECT_TRUE(crc.finalise() == crc3);
    crc.reset();
    crc.update(data4, size4);
    EXPECT_TRUE(crc.finalise() == crc4);

    // Multiple update calculation.
    crc.reset();
    crc.update(data1, size1);
    crc.update(data2, size2);
    crc.update(data3, size3);
    EXPECT_TRUE(crc.finalise() == crc4);
}


// 16-bits. Zero initial. Reflection in and out. Non-zero XOR value.
TEST_F(CRCTest, TestCRC16_DNP)
{
    // These value obtained by using datan for input at https://crccalc.com/
    constexpr uint16_t crc1 = 0x4535;
    constexpr uint16_t crc2 = 0xEA82;
    constexpr uint16_t crc3 = 0xEB6B;
    constexpr uint16_t crc4 = 0xBB8C;

    eg::CRC16_DNP crc;

    // Single pass calculation.
    EXPECT_TRUE(crc.calculate(data1, size1) == crc1);
    EXPECT_TRUE(crc.calculate(data2, size2) == crc2);
    EXPECT_TRUE(crc.calculate(data3, size3) == crc3);
    EXPECT_TRUE(crc.calculate(data4, size4) == crc4);

    // Single update calculation - unwrapped single pass.
    crc.reset();
    crc.update(data1, size1);
    EXPECT_TRUE(crc.finalise() == crc1);
    crc.reset();
    crc.update(data2, size2);
    EXPECT_TRUE(crc.finalise() == crc2);
    crc.reset();
    crc.update(data3, size3);
    EXPECT_TRUE(crc.finalise() == crc3);
    crc.reset();
    crc.update(data4, size4);
    EXPECT_TRUE(crc.finalise() == crc4);

    // Multiple update calculation.
    crc.reset();
    crc.update(data1, size1);
    crc.update(data2, size2);
    crc.update(data3, size3);
    EXPECT_TRUE(crc.finalise() == crc4);
}


// 32-bits. Non-zero initial. Reflection in and out. Non-zero XOR value.
TEST_F(CRCTest, TestCRC32)
{
    // These value obtained by using datan for input at https://crccalc.com/
    constexpr uint32_t crc1 = 0x55404551;
    constexpr uint32_t crc2 = 0xCBF43926;
    constexpr uint32_t crc3 = 0x4EABCF7C;
    constexpr uint32_t crc4 = 0x24CAFA72;

    eg::CRC32 crc;

    // Single pass calculation.
    EXPECT_TRUE(crc.calculate(data1, size1) == crc1);
    EXPECT_TRUE(crc.calculate(data2, size2) == crc2);
    EXPECT_TRUE(crc.calculate(data3, size3) == crc3);
    EXPECT_TRUE(crc.calculate(data4, size4) == crc4);

    // Single update calculation - unwrapped single pass.
    crc.reset();
    crc.update(data1, size1);
    EXPECT_TRUE(crc.finalise() == crc1);
    crc.reset();
    crc.update(data2, size2);
    EXPECT_TRUE(crc.finalise() == crc2);
    crc.reset();
    crc.update(data3, size3);
    EXPECT_TRUE(crc.finalise() == crc3);
    crc.reset();
    crc.update(data4, size4);
    EXPECT_TRUE(crc.finalise() == crc4);

    // Multiple update calculation.
    crc.reset();
    crc.update(data1, size1);
    crc.update(data2, size2);
    crc.update(data3, size3);
    EXPECT_TRUE(crc.finalise() == crc4);
}


TEST_F(CRCTest, TestCRC8_BLUETOOTH)
{
    // These value obtained by using datan for input at https://crccalc.com/
    constexpr uint8_t crc1 = 0xAD;
    constexpr uint8_t crc2 = 0x26;
    constexpr uint8_t crc3 = 0xB9;
    constexpr uint8_t crc4 = 0x15;

    eg::CRC8_BLUETOOTH crc;

    // Single pass calculation.
    EXPECT_TRUE(crc.calculate(data1, size1) == crc1);
    EXPECT_TRUE(crc.calculate(data2, size2) == crc2);
    EXPECT_TRUE(crc.calculate(data3, size3) == crc3);
    EXPECT_TRUE(crc.calculate(data4, size4) == crc4);

    // Single update calculation - unwrapped single pass.
    crc.reset();
    crc.update(data1, size1);
    EXPECT_TRUE(crc.finalise() == crc1);
    crc.reset();
    crc.update(data2, size2);
    EXPECT_TRUE(crc.finalise() == crc2);
    crc.reset();
    crc.update(data3, size3);
    EXPECT_TRUE(crc.finalise() == crc3);
    crc.reset();
    crc.update(data4, size4);
    EXPECT_TRUE(crc.finalise() == crc4);

    // Multiple update calculation.
    crc.reset();
    crc.update(data1, size1);
    crc.update(data2, size2);
    crc.update(data3, size3);
    EXPECT_TRUE(crc.finalise() == crc4);
}


TEST_F(CRCTest, TestCRC8_AUTOSAR)
{
    // These value obtained by using datan for input at https://crccalc.com/
    constexpr uint8_t crc1 = 0xC2;
    constexpr uint8_t crc2 = 0xDF;
    constexpr uint8_t crc3 = 0x16;
    constexpr uint8_t crc4 = 0xF8;

    eg::CRC8_AUTOSAR crc;

    // Single pass calculation.
    EXPECT_TRUE(crc.calculate(data1, size1) == crc1);
    EXPECT_TRUE(crc.calculate(data2, size2) == crc2);
    EXPECT_TRUE(crc.calculate(data3, size3) == crc3);
    EXPECT_TRUE(crc.calculate(data4, size4) == crc4);

    // Single update calculation - unwrapped single pass.
    crc.reset();
    crc.update(data1, size1);
    EXPECT_TRUE(crc.finalise() == crc1);
    crc.reset();
    crc.update(data2, size2);
    EXPECT_TRUE(crc.finalise() == crc2);
    crc.reset();
    crc.update(data3, size3);
    EXPECT_TRUE(crc.finalise() == crc3);
    crc.reset();
    crc.update(data4, size4);
    EXPECT_TRUE(crc.finalise() == crc4);

    // Multiple update calculation.
    crc.reset();
    crc.update(data1, size1);
    crc.update(data2, size2);
    crc.update(data3, size3);
    EXPECT_TRUE(crc.finalise() == crc4);
}


