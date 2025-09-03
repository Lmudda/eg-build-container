/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

// Tracability: PRS-97 Ring buffer and PRS-98 Ring buffer array

#include "gtest/gtest.h"
#include "utilities/RingBuffer.h"

using namespace std;

TEST(RingBuffer, FillingTheBuffer)
{
    eg::RingBufferArray<int, 7> buffer;

    EXPECT_TRUE(buffer.size() == 0);
    for (int i = 1; i <= buffer.capacity(); ++i)
    {
        EXPECT_TRUE(buffer.put(1000 + i) == true);
        EXPECT_TRUE(buffer.size() == i);
        EXPECT_TRUE(buffer.capacity() == 7);
        EXPECT_TRUE(buffer.front() == 1001);
    }

    // It is full so cannot put another item.
    EXPECT_TRUE(buffer.put(1009) == false);
    EXPECT_TRUE(buffer.size() == 7);
    EXPECT_TRUE(buffer.capacity() == 7);
    EXPECT_TRUE(buffer.front() == 1001);
}

TEST(RingBuffer, PoppingTheBuffer)
{
    eg::RingBufferArray<int, 7> buffer;

    EXPECT_TRUE(buffer.size() == 0);
    for (int i = 1; i <= buffer.capacity(); ++i)
    {
        EXPECT_TRUE(buffer.put(1000 + i) == true);
        EXPECT_TRUE(buffer.size() == i);
        EXPECT_TRUE(buffer.capacity() == 7);
        EXPECT_TRUE(buffer.front() == 1001);
    }

    for (int i = 1; i <= buffer.capacity(); ++i)
    {
        EXPECT_TRUE(buffer.size() == 8 - i);
        EXPECT_TRUE(buffer.front() == 1000 + i);
        EXPECT_TRUE(buffer.pop() == true);
        EXPECT_TRUE(buffer.capacity() == 7);
    }

    // It is empty so cannot pop another item.
    EXPECT_TRUE(buffer.pop() == false);
    EXPECT_TRUE(buffer.size() == 0);
    EXPECT_TRUE(buffer.capacity() == 7);
}

TEST(RingBuffer, ReadingTheBuffer)
{
    eg::RingBufferArray<int, 7> buffer;

    EXPECT_TRUE(buffer.size() == 0);
    for (int i = 1; i <= buffer.capacity(); ++i)
    {
        EXPECT_TRUE(buffer.put(1000 + i) == true);
        EXPECT_TRUE(buffer.size() == i);
        EXPECT_TRUE(buffer.capacity() == 7);
        EXPECT_TRUE(buffer.front() == 1001);
    }

    for (int i = 1; i <= buffer.capacity(); ++i)
    {
        EXPECT_TRUE(buffer.size() == 8 - i);
        EXPECT_TRUE(buffer.front() == 1000 + i);
        int value{};
        EXPECT_TRUE(buffer.get(value) == true);
        EXPECT_TRUE(value == 1000 + i);
        EXPECT_TRUE(buffer.capacity() == 7);
    }

    // It is empty so cannot read another item.
    int value = 3001;
    EXPECT_TRUE(buffer.get(value) == false);
    EXPECT_TRUE(value == 3001);
    EXPECT_TRUE(buffer.size() == 0);
    EXPECT_TRUE(buffer.capacity() == 7);
}

TEST(RingBuffer, ClearingTheBuffer)
{
    eg::RingBufferArray<int, 7> buffer;

    EXPECT_TRUE(buffer.size() == 0);
    for (int i = 1; i <= buffer.capacity(); ++i)
    {
        EXPECT_TRUE(buffer.put(1000 + i) == true);
        EXPECT_TRUE(buffer.size() == i);
        EXPECT_TRUE(buffer.capacity() == 7);
        EXPECT_TRUE(buffer.front() == 1001);
    }

    buffer.clear();
    EXPECT_TRUE(buffer.size() == 0);
    EXPECT_TRUE(buffer.capacity() == 7);
    EXPECT_TRUE(buffer.pop() == false);

    // Again with a partial fill 
    for (int i = 1; i <= (buffer.capacity() / 2); ++i)
    {
        EXPECT_TRUE(buffer.put(1000 + i) == true);
        EXPECT_TRUE(buffer.size() == i);
        EXPECT_TRUE(buffer.capacity() == 7);
        EXPECT_TRUE(buffer.front() == 1001);
    }

    buffer.clear();
    EXPECT_TRUE(buffer.size() == 0);
    EXPECT_TRUE(buffer.capacity() == 7);
    EXPECT_TRUE(buffer.pop() == false);
}