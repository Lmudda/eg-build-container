/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2025.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "gtest/gtest.h"
#include "utilities/BlockBuffer.h"

//Traceability: PRS-128 Block buffer
//Traceability: PRS-129 Block buffer array

using namespace eg;

class BlockBufferTest : public ::testing::Test 
{
protected:
    static constexpr uint16_t BUFFER_SIZE = 16;
    uint8_t buffer[BUFFER_SIZE]{};
    BlockBuffer blockBuffer{buffer, BUFFER_SIZE};
};

TEST_F(BlockBufferTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(blockBuffer.capacity(), BUFFER_SIZE);
    EXPECT_EQ(blockBuffer.size(), 0);
}

TEST_F(BlockBufferTest, AppendWithinCapacitySucceeds) 
{
    uint8_t data[] = {1, 2, 3, 4};
    EXPECT_TRUE(blockBuffer.append({data, sizeof(data), 0}));
    EXPECT_EQ(blockBuffer.size(), sizeof(data));
}

TEST_F(BlockBufferTest, AppendExceedingCapacityFails) 
{
    uint8_t data[BUFFER_SIZE + 1] = {};
    EXPECT_FALSE(blockBuffer.append({data, sizeof(data), 0}));
}

TEST_F(BlockBufferTest, AppendWrapsAroundCorrectly) 
{
    uint8_t data1[BUFFER_SIZE - 4] = {};
    uint8_t data2[8] = {};

    EXPECT_TRUE(blockBuffer.append({data1, sizeof(data1), 0}));
    EXPECT_TRUE(blockBuffer.remove(blockBuffer.front()));  // Clear space
    EXPECT_TRUE(blockBuffer.append({data2, sizeof(data2), 0}));
}

TEST_F(BlockBufferTest, FrontReturnsCorrectBlock) 
{
    uint8_t data[] = {10, 20, 30};
    blockBuffer.append({data, sizeof(data), 0});
    auto block = blockBuffer.front();

    ASSERT_NE(block.buffer, nullptr);
    EXPECT_EQ(block.length, sizeof(data));
    EXPECT_EQ(block.index, 1);
}

TEST_F(BlockBufferTest, FrontReturnsEmptyBlockWhenEmpty) 
{
    auto block = blockBuffer.front();
    EXPECT_EQ(block.buffer, nullptr);
    EXPECT_EQ(block.length, 0);
}

TEST_F(BlockBufferTest, RemoveValidBlockSucceeds) 
{
    uint8_t data[] = {5, 6, 7};
    blockBuffer.append({data, sizeof(data), 0});
    auto block = blockBuffer.front();
    EXPECT_TRUE(blockBuffer.remove(block));
    EXPECT_EQ(blockBuffer.size(), 0);
}

TEST_F(BlockBufferTest, RemoveOversizedBlockFails) 
{
    uint8_t data[] = {1, 2};
    blockBuffer.append({data, sizeof(data), 0});
    BlockBuffer::Block invalidBlock{data, static_cast<uint16_t>(sizeof(data) + 1), 0};
    EXPECT_FALSE(blockBuffer.remove(invalidBlock));
}

TEST_F(BlockBufferTest, OverfillBuffer) 
{
    // Fill buffer almost to capacity to force wrap-around
    uint8_t data1[BUFFER_SIZE - 4] = {};
    ASSERT_TRUE(blockBuffer.append({data1, sizeof(data1), 0}));

    // Remove some data to move m_getpos forward
    auto block1 = blockBuffer.front();
    ASSERT_TRUE(blockBuffer.remove(block1));

    // Append more data to wrap around
    uint8_t data2[8] = {};
    ASSERT_TRUE(blockBuffer.append({data2, sizeof(data2), 0}));

    // Now m_putpos <= m_getpos, which will trip the alternative path when we call front().  
    auto block2 = blockBuffer.front();

    // Because we completely filled the buffer, the block we get now is the remainder of the size.
    EXPECT_EQ(block2.length, BUFFER_SIZE - sizeof(data1));
    EXPECT_EQ(block2.index, 2);
}

TEST_F(BlockBufferTest, AppendEmptyBlockReturnsTrue) 
{
    uint8_t dummyData[] = {42}; 
    BlockBuffer::Block emptyBlock{dummyData, 0, 0};

    EXPECT_TRUE(blockBuffer.append(emptyBlock));
    EXPECT_EQ(blockBuffer.size(), 0);  
}

// For BlockBufferArray, check it initialises as expected.
TEST(BlockBufferArrayTest, InitializesWithStaticBuffer) 
{
    BlockBufferArray<32> staticBuffer;
    EXPECT_EQ(staticBuffer.capacity(), 32);
    EXPECT_EQ(staticBuffer.size(), 0);
}