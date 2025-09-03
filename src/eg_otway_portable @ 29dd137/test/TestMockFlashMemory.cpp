/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "gtest/gtest.h"
#include "interfaces/IFlashMemory.h"
#include "mock/MockFlashMemory.h"
#include <iostream>


/////////////////////////////////////////////////////////////////////////////////////////////
// This set of tests is a pre-requisite for testing flash log and similar abstractions. 
// MockFlashMemory implements the IFlashMemory API with a simulated flash - basically an array 
// of bytes. It might better switch to a vector-based implementation to avoid blowing the stack 
// in tests.
/////////////////////////////////////////////////////////////////////////////////////////////


constexpr uint32_t kPageSize     = 0x2000;      // All pages are 8KB
constexpr uint32_t kWriteSize    = 16;          // The 128-bit bus is a feature of STM32U5s 

constexpr uint32_t kBaseAddress1 = 0x0500'0000; // Arbitrary
constexpr uint32_t kNumPages1    = 5;           // Arbitrary
using MockU5FlashMemory1 = eg::MockFlashMemory<kBaseAddress1, kPageSize, kNumPages1, kWriteSize>;

constexpr uint32_t kBaseAddress2 = 0x081F'0000; // Arbitrary  
constexpr uint32_t kNumPages2    = 9;           // Arbitrary
using MockU5FlashMemory2 = eg::MockFlashMemory<kBaseAddress2, kPageSize, kNumPages2, kWriteSize>;

constexpr uint32_t kBaseAddress3 = 0x080C'0000; // Arbitrary 
constexpr uint32_t kNumPages3    = 16;          // Arbitrary
using MockU5FlashMemory3 = eg::MockFlashMemory<kBaseAddress3, kPageSize, kNumPages3, kWriteSize>;

using IFlashMemory = eg::IFlashMemory;


TEST(MockFlashMemory, BasicProperties)
{
    MockU5FlashMemory1 flash;
    EXPECT_TRUE(flash.get_page_count() == kNumPages1);
    
    for (uint32_t a = kBaseAddress1; a < (kBaseAddress1 + kNumPages1 * kPageSize); a += 2371)
    {
        EXPECT_TRUE(flash.get_page_index(a) == ((a - kBaseAddress1) / kPageSize));
    }
    EXPECT_TRUE(flash.get_page_index(0) == IFlashMemory::kInvalidArgument);
    EXPECT_TRUE(flash.get_page_index(kBaseAddress1 - 1) == IFlashMemory::kInvalidArgument);
    EXPECT_TRUE(flash.get_page_index(kBaseAddress1 + 8192 * kNumPages1) == IFlashMemory::kInvalidArgument);

    for (uint32_t p = 0; p < kNumPages1; ++p)
    {
        EXPECT_TRUE(flash.get_page_address(p) == (kBaseAddress1 + 8192 * p));
        EXPECT_TRUE(flash.get_page_address(p + kNumPages1) == IFlashMemory::kInvalidArgument);
        EXPECT_TRUE(flash.get_page_size(p) == 8192);
    }

    EXPECT_TRUE(flash.get_write_size() == 16);
}


TEST(MockFlashMemory, GetData)
{
    MockU5FlashMemory1 flash;
    const uint8_t* data = flash.get_data(kBaseAddress1);
    EXPECT_TRUE(data != nullptr);

    // Must be initialised to all FF.
    bool all_ff = true;
    for (uint32_t offset = 0; offset < (kNumPages1 * kPageSize); ++offset)
        all_ff = all_ff & (data[offset] == 0xFF);
    EXPECT_TRUE(all_ff);

    // Sample a bunch of addresses in the range.
    for (uint32_t offset = 0; offset < (kNumPages1 * kPageSize); offset += 893)
    {
        // Raw data pointer from address has the expected offset.
        const uint8_t* data2 = flash.get_data(kBaseAddress1 + offset);
        EXPECT_TRUE(data2 != nullptr);
        EXPECT_TRUE((data2 - data) == offset);

        // Raw data pointer from [page, offset] matches the pointer from address.
        auto page = offset / kPageSize;
        auto off  = offset % kPageSize;
        const uint8_t* data3 = flash.get_data(page, off);
        EXPECT_TRUE(data3 == data2);
    }

    // Null pointer if the address is out of range.
    EXPECT_TRUE(flash.get_data(0) == nullptr);
    EXPECT_TRUE(flash.get_data(kBaseAddress1 - 1) == nullptr);
    EXPECT_TRUE(flash.get_data(kBaseAddress1 + kNumPages1 * kPageSize) == nullptr);

    // Null pointer if the page or offset is out of range.
    EXPECT_TRUE(flash.get_data(0, 8192) == nullptr);
    EXPECT_TRUE(flash.get_data(kNumPages1, 0) == nullptr);
}


TEST(MockFlashMemory, WriteData)
{
    MockU5FlashMemory1 flash;

    auto write_block = [&](uint32_t address, uint32_t size, uint8_t value)
    {
        std::array<uint8_t, 64> buffer{};
        for (uint32_t i = 0; i < size; ++i)
            buffer[i] = value;
        return flash.write(address, buffer.data(), size);
    };

    auto read_block = [&](uint32_t address, uint32_t size, uint8_t value)
    {
        bool result = true;
        std::array<uint8_t, 64> buffer{};
        flash.read(address, buffer.data(), size);
        for (uint32_t i = 0; i < size; ++i)
            result = result & (buffer[i] == value);
        return result;
    };

    auto is_erased = [&](uint32_t address)
    {
        [[maybe_unused]] auto page = flash.get_page_index(address);
        const uint8_t* data = flash.get_data(address);

        bool result = true;
        for (uint32_t i = 0; i < kPageSize; ++i)
            result = result & (data[i] == 0xFF);
        return result;
    };

    // All pages should be clear
    for (auto p = 0u; static_cast<uint32_t>(p) < kNumPages1; ++p)
        EXPECT_TRUE(is_erased(kBaseAddress1 + p * kPageSize));

    // Writes must be properly aligned and have a matching length.
    EXPECT_TRUE(write_block(kBaseAddress1 + 23, 32, 0x00) == IFlashMemory::Result::eUnalignedAddress);
    EXPECT_TRUE(write_block(kBaseAddress1 + 23, 17, 0x00) == IFlashMemory::Result::eUnalignedAddress);
    EXPECT_TRUE(write_block(kBaseAddress1 + 32, 17, 0x00) == IFlashMemory::Result::eUnalignedSize);
    EXPECT_TRUE(write_block(kBaseAddress1 + 32, 32, 0x00) == IFlashMemory::Result::eOK);

    // All but the first page should be clear
    for (auto p = 1; static_cast<uint32_t>(p) < kNumPages1; ++p)
        EXPECT_TRUE(is_erased(kBaseAddress1 + p * kPageSize));

    // Check that the earlier successful write only affected the target bytes.
    EXPECT_TRUE(read_block(kBaseAddress1,      32, 0xFF));
    EXPECT_TRUE(read_block(kBaseAddress1 + 32, 32, 0x00));
    EXPECT_TRUE(read_block(kBaseAddress1 + 64, 32, 0xFF));

    // Check that writes to the same bytes are ANDed together.
    EXPECT_TRUE(write_block(kBaseAddress1 + 8192, 32, 0xA7) == IFlashMemory::Result::eOK);
    EXPECT_TRUE(read_block(kBaseAddress1  + 8192, 32, 0xA7));
    EXPECT_TRUE(write_block(kBaseAddress1 + 8192, 32, 0xFF) == IFlashMemory::Result::eOK);
    EXPECT_TRUE(read_block(kBaseAddress1  + 8192, 32, 0xA7));
    EXPECT_TRUE(write_block(kBaseAddress1 + 8192, 32, 0x33) == IFlashMemory::Result::eOK);
    EXPECT_TRUE(read_block(kBaseAddress1  + 8192, 32, 0x23));

    // All but the first two pages should be clear
    for (auto p = 2; static_cast<uint32_t>(p) < kNumPages1; ++p)
        EXPECT_TRUE(is_erased(kBaseAddress1 + p * kPageSize));

    // // Check that erase works - and only affect the current page.
    EXPECT_TRUE(flash.erase_address(kBaseAddress1) == IFlashMemory::Result::eOK);
    EXPECT_TRUE(is_erased(kBaseAddress1));
    EXPECT_TRUE(!is_erased(kBaseAddress1 + 8192));
    EXPECT_TRUE(read_block(kBaseAddress1 + 8192, 32, 0x23));

    // All the other pages should be clear
    for (auto p = 2; static_cast<uint32_t>(p) < kNumPages1; ++p)
        EXPECT_TRUE(is_erased(kBaseAddress1 + p * kPageSize));

    EXPECT_TRUE(flash.erase_address(kBaseAddress1 + 8192) == IFlashMemory::Result::eOK);
    EXPECT_TRUE(is_erased(kBaseAddress1));
    EXPECT_TRUE(is_erased(kBaseAddress1 + 8192));

    // All pages should be clear
    for (auto p = 0u; static_cast<uint32_t>(p) < kNumPages1; ++p)
        EXPECT_TRUE(is_erased(kBaseAddress1 + p * kPageSize));

    EXPECT_TRUE(flash.erase_address(0) == IFlashMemory::Result::eInvalidAddress);
    EXPECT_TRUE(flash.erase_address(kBaseAddress1 - 1) == IFlashMemory::Result::eInvalidAddress);
    EXPECT_TRUE(flash.erase_address(kBaseAddress1 + kNumPages1 * kPageSize) == IFlashMemory::Result::eInvalidAddress);

    // All pages should be in range.
    for (auto p = 0u; static_cast<uint32_t>(p) < kNumPages1; ++p)
        EXPECT_TRUE(flash.erase_page(p) == IFlashMemory::Result::eOK);
    EXPECT_TRUE(flash.erase_page(kNumPages1 + 1) == IFlashMemory::Result::eInvalidPage);
}


