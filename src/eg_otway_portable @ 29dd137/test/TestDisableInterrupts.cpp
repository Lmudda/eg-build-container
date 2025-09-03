/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2025.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#if defined(OTWAY_TARGET_PLATFORM_BAREMETAL) 

#include "gtest/gtest.h"
#include "utilities/DisableInterrupts.h"
#include "TestSingleThreadedUtils.h"

namespace eg 
{ 

bool MOCK_INTERRUPTS_DISABLED = false; 

void DisableInterrupts::platform_enable_interrupts() 
{ 
    MOCK_INTERRUPTS_DISABLED = false; 
}
void DisableInterrupts::platform_disable_interrupts() 
{ 
    MOCK_INTERRUPTS_DISABLED = true; 
}
}


TEST(DisableInterrupts, TypicalUsage)
{
    EXPECT_FALSE(eg::MOCK_INTERRUPTS_DISABLED);
    {
        eg::DisableInterrupts di;
        EXPECT_TRUE(eg::MOCK_INTERRUPTS_DISABLED);
    }
    EXPECT_FALSE(eg::MOCK_INTERRUPTS_DISABLED);
}


TEST(DisableInterrupts, SafeToNest)
{
    EXPECT_FALSE(eg::MOCK_INTERRUPTS_DISABLED);
    {
        eg::DisableInterrupts di;
        EXPECT_TRUE(eg::MOCK_INTERRUPTS_DISABLED);
        {
            eg::DisableInterrupts di;
            EXPECT_TRUE(eg::MOCK_INTERRUPTS_DISABLED);
        }
        EXPECT_TRUE(eg::MOCK_INTERRUPTS_DISABLED);
    }
    EXPECT_FALSE(eg::MOCK_INTERRUPTS_DISABLED);
}


#endif  // defined(OTWAY_TARGET_PLATFORM_BAREMETAL) 