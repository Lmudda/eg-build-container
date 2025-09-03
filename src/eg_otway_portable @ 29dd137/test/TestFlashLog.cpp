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
#include "drivers/FlashLog.h"
#include "mock/MockFlashMemory.h"
#include <iostream>


/////////////////////////////////////////////////////////////////////////////////////////////
// This set of tests is ...
/////////////////////////////////////////////////////////////////////////////////////////////


// TODO_AC Run the tests again with differnt constants and a differently sized Item to ensure that none of them are hardcoded.
constexpr uint32_t kPageSize     = 0x2000;      // All pages are 8KB
constexpr uint32_t kWriteSize    = 16;          // The 128-bit bus is a feature of STM32U5s 
constexpr uint32_t kBaseAddress  = 0x081F'0000; // Arbitrary  
constexpr uint32_t kNumPages     = 4;           // Arbitrary
using MockU5FlashMemory = eg::MockFlashMemory<kBaseAddress, kPageSize, kNumPages, kWriteSize>;
using IFlashMemory      = eg::IFlashMemory;


// This must be a multiple of kWriteSize in size, or the code won't compile.
struct TestLogItem 
{
    TestLogItem() = default;
    TestLogItem(uint32_t value)
    {
        for (uint8_t i = 0; i < 8; ++i) 
        {
            // Create some pseudorandom-ish data based on simple input.
            data[i] = value;
            value = value * (value + 1);
        }
    }
    uint32_t data[8]{};
};
using TestLog = eg::FlashLog<TestLogItem, kWriteSize>;

// To ease some of the testing below.
bool operator==(const TestLogItem& lhs, const TestLogItem& rhs)
{
    for (uint8_t i = 0; i < 8; ++i) 
        if (lhs.data[i] != rhs.data[i]) return false;
    return true;
}


TEST(FlashLog, FlashLog)
{
    MockU5FlashMemory flash;
    TestLog log{flash};

    // Initialised values of the empty log.
    EXPECT_TRUE(log.get_newest_page() == 0);
    EXPECT_TRUE(log.get_oldest_page() == 0);
    EXPECT_TRUE(log.get_record_count() == 0);
    EXPECT_TRUE(log.get_records_per_page() == ((kPageSize - kWriteSize) / sizeof(TestLogItem)));

    uint32_t count = log.get_records_per_page();

    // Fill up the empty log.
    for (uint32_t p = 0; p < kNumPages; ++p)
    {
        // Write a page full of records.
        for (uint32_t i = 0; i < count; ++i)
        {
            TestLogItem item{};
            log.append_record(item);
            EXPECT_TRUE(log.get_record_count() == (i + 1 + p * count));
            // This page is not yet over-written.
            EXPECT_TRUE(log.get_oldest_page() == 0);
            // We advance through the available pages in the log.
            EXPECT_TRUE(log.get_newest_page() == p);
            // This should never change.
            EXPECT_TRUE(log.get_records_per_page() == ((kPageSize - kWriteSize) / sizeof(TestLogItem)));
        }
    }

    // Add items to a full log.
    for (uint32_t p = 0; p < (kNumPages + 2); ++p)
    {
        // Write a page full of records.
        for (uint32_t i = 0; i < count; ++i)
        {
            TestLogItem item{};
            log.append_record(item);
            // We overwrite the pages as we go along. This discards the oldest records.
            EXPECT_TRUE(log.get_record_count() == (i + 1 + (kNumPages - 1) * count));
            EXPECT_TRUE(log.get_oldest_page() == ((p + 1) % kNumPages));
            // We advance through the available pages in the log.
            EXPECT_TRUE(log.get_newest_page() == (p % kNumPages));
            // This should never change.
            EXPECT_TRUE(log.get_records_per_page() == ((kPageSize - kWriteSize) / sizeof(TestLogItem)));
        }
    }

    // Just make sure that the newest and oldest pages don't have the initial values. 
    EXPECT_TRUE(log.get_newest_page() != 0);
    EXPECT_TRUE(log.get_oldest_page() != 0);
    EXPECT_TRUE(log.get_record_count() != 0);

    // Erase the log. All the initial values should be restored.
    EXPECT_TRUE(log.erase_all_records() == TestLog::Result::eOK); 
    EXPECT_TRUE(log.get_newest_page() == 0);
    EXPECT_TRUE(log.get_oldest_page() == 0);
    EXPECT_TRUE(log.get_record_count() == 0);
    EXPECT_TRUE(log.get_records_per_page() == ((kPageSize - kWriteSize) / sizeof(TestLogItem)));
}


TEST(FlashLog, ReadLog)
{
    MockU5FlashMemory flash;
    TestLog log{flash};

    // Write some records.
    for (uint32_t i = 0; i < 100; ++i)
    {
        TestLogItem item{i + 1};
        log.append_record(item);
    }
    for (uint32_t i = 0; i < 100; ++i)
    {
        TestLogItem temp{i + 1};
        TestLogItem item{0};
        auto result = log.read_record_by_index(i , item);
        EXPECT_TRUE(result == TestLog::Result::eOK);
        EXPECT_TRUE(temp == item);
        const TestLogItem* pitem = log.get_pointer_by_index(i);
        EXPECT_TRUE(temp == *pitem);
    }

    // Write some more records.
    for (uint32_t i = 100; i < 200; ++i)
    {
        TestLogItem item{i + 1};
        log.append_record(item);
    }
    for (uint32_t i = 0; i < 200; ++i)
    {
        TestLogItem temp{i + 1};
        TestLogItem item{0};
        auto result = log.read_record_by_index(i , item);
        EXPECT_TRUE(result == TestLog::Result::eOK);
        EXPECT_TRUE(temp == item);
        const TestLogItem* pitem = log.get_pointer_by_index(i);
        EXPECT_TRUE(temp == *pitem);
    }

    // Should not be able to read a record which does not exist.
    TestLogItem item{0};
    auto result = log.read_record_by_index(201 , item);
    EXPECT_TRUE(result == TestLog::Result::eInvalidOffset);

    // Should not be able to point to a record which does not exist.
    const TestLogItem* pitem = log.get_pointer_by_index(201);
    EXPECT_TRUE(pitem == nullptr);

    // Add enough records to move the next page.
    auto count = log.get_records_per_page();
    for (uint32_t i = 200; i < (count + 200); ++i)
    {
        TestLogItem item{i + 1};
        log.append_record(item);
    }
    for (uint32_t i = 0; i < (count + 200); ++i)
    {
        TestLogItem temp{i + 1};
        TestLogItem item{0};
        auto result = log.read_record_by_index(i , item);
        EXPECT_TRUE(result == TestLog::Result::eOK);
        EXPECT_TRUE(temp == item);
        const TestLogItem* pitem = log.get_pointer_by_index(i);
        EXPECT_TRUE(temp == *pitem);
    }

    // Add enough records to start erasing pages. We should wrap around so that the first few pages 
    // are erased. This means the 0th record advances by (count * erased pages). This loop will mean 
    // we have written 6 full pages of records plus 200. The first three pages should have been 
    // overwritten (two fully, one partially).
    for (uint32_t i = (count + 200); i < (count * 6 + 200); ++i)
    {
        TestLogItem item{i + 1};
        log.append_record(item);
    }
    for (uint32_t i = 0; i < 200; ++i)
    {
        // The oldest record (index 0) should now be the the (count * 2)th item written.
        TestLogItem temp{i + 1 + count * 3};
        TestLogItem item{0};
        auto result = log.read_record_by_index(i , item);
        //std::cout << std::hex << item.data[0] << " "<< temp.data[0] << std::endl;
        EXPECT_TRUE(result == TestLog::Result::eOK);
        EXPECT_TRUE(temp == item);
        const TestLogItem* pitem = log.get_pointer_by_index(i);
        EXPECT_TRUE(temp == *pitem);
    }

    // We want to confirm that the memory is written as expected. The tests on the mock flash 
    // should do this but a redundant test doesn't hurt.
    // Oldest records are in page 3 at this point.
    const uint8_t* memory = flash.get_data(flash.get_page_address(3));
    for (uint32_t i = 0; i < count; ++i)    
    {        
        TestLogItem  temp{i + 1 + count * 3};
        const TestLogItem& item = *reinterpret_cast<const TestLogItem*>(memory + 16 + i * sizeof(TestLogItem));
        EXPECT_TRUE(temp == item);
    }
    // Wrap around to page 0
    memory = flash.get_data(flash.get_page_address(0));
    for (uint32_t i = 0; i < count; ++i)    
    {        
        TestLogItem  temp{i + 1 + count * 4};
        const TestLogItem& item = *reinterpret_cast<const TestLogItem*>(memory + 16 + i * sizeof(TestLogItem));
        EXPECT_TRUE(temp == item);
    }
    // Advance to page 1
    memory = flash.get_data(flash.get_page_address(1));
    for (uint32_t i = 0; i < count; ++i)    
    {        
        TestLogItem  temp{i + 1 + count * 5};
        const TestLogItem& item = *reinterpret_cast<const TestLogItem*>(memory + 16 + i * sizeof(TestLogItem));
        EXPECT_TRUE(temp == item);
    }
    // Advance to page 2 - only partially full
    memory = flash.get_data(flash.get_page_address(2));
    for (uint32_t i = 0; i < 200; ++i)    
    {        
        TestLogItem  temp{i + 1 + count * 6};
        const TestLogItem& item = *reinterpret_cast<const TestLogItem*>(memory + 16 + i * sizeof(TestLogItem));
        EXPECT_TRUE(temp == item);
    }
}


TEST(FlashLog, InitialiseWithExistingRecords)
{
    MockU5FlashMemory flash;
    {
        TestLog log{flash};

        // Write some records.
        for (uint32_t i = 0; i < 100; ++i)
        {
            TestLogItem item{i + 1};
            log.append_record(item);
        }

        // Let it leave scope and deconstruct.
    }
    // Make a new log with the same memory.
    TestLog log2{flash};
}