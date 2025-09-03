/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "gtest/gtest.h"
#include "utilities/MemoryPool.h"
#include <set>
#include <vector>
#include <algorithm>
#include <random>

// Tracability: PRS-105 Memory pool

namespace {

struct TestStruct
{
    uint32_t field1;
    uint32_t field2;
    uint32_t field3;
    uint32_t field4;
};

} // namespace {


TEST(MemoryPoolTest, AllocateAndFree)
{
    constexpr int POOL_SIZE = 32;
    eg::MemoryPool<TestStruct, POOL_SIZE> pool;

    std::set<TestStruct*> allocated;

    for (int i = 0; i < POOL_SIZE; ++i)
    {
        EXPECT_TRUE(pool.available() == POOL_SIZE - i);

        auto p = pool.alloc();
        EXPECT_TRUE(p != nullptr);
        EXPECT_TRUE(allocated.find(p) == allocated.end());
        allocated.insert(p);
    }

    // The pool is exhausted.
    EXPECT_TRUE(pool.available() == 0);
    auto p = pool.alloc();
    EXPECT_TRUE(p == nullptr);

    auto allocated2 = allocated; // To not invalidate iterators
    for (auto p: allocated)
    {
        EXPECT_TRUE(pool.available() == POOL_SIZE - allocated2.size());
        EXPECT_TRUE(p != nullptr);
        EXPECT_TRUE(pool.free(p) == true);
        allocated2.erase(p);
    }
}


TEST(MemoryPoolTest, RandomisedOrdering)
{
    constexpr int POOL_SIZE = 32;
    eg::MemoryPool<TestStruct, POOL_SIZE> pool;
    
    auto rng = std::default_random_engine{};

    for (int loop = 0; loop < 100; ++loop)
    {
        std::vector<TestStruct*> allocated;
        while (pool.available() > 0)
        {
            auto p = pool.alloc();
            EXPECT_TRUE(p != nullptr);
            allocated.push_back(p);
            EXPECT_TRUE(pool.available() == POOL_SIZE - allocated.size());
        }

        std::shuffle(allocated.begin(), allocated.end(), rng);

        for (int i = 0; i < (int)allocated.size(); ++i)
        {
            auto p = allocated[i];
            EXPECT_TRUE(p != nullptr);
            EXPECT_TRUE(pool.free(p) == true);
            EXPECT_TRUE(pool.available() == i + 1);
        }
    }
}


TEST(MemoryPoolTest, FreeNullPtrReturnsFalse)
{
    constexpr int POOL_SIZE = 32;
    eg::MemoryPool<TestStruct, POOL_SIZE> pool;
    EXPECT_FALSE(pool.free(nullptr));
}