/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2025.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "gtest/gtest.h"

#if defined(OTWAY_TARGET_PLATFORM_BAREMETAL) 

#include "TestSingleThreadedUtils.h"
#include "signals/InterruptHandler.h"
#include "private/baremetal/signals/Callback.h"

class Thing
{
public:
    Thing() { m_callback.connect<&Thing::function_to_call>(this); };
    ~Thing() = default;
    bool m_function_called = false;
    eg::Callback<void(void)> m_callback;
private:
    void function_to_call() { m_function_called = true; }
};


TEST(InterruptHandler, MethodConnection)
{
    Thing thing;
    ASSERT_FALSE(thing.m_function_called);
    thing.m_callback.call();
    ASSERT_TRUE(thing.m_function_called);
}


TEST(InterruptHandler, CallbackLambdaConnection)
{
    uint32_t cb_count = 0;
    auto cb_lambda = [&](){cb_count++;};
    eg::Callback<void(void)> callback;
    callback.connect(cb_lambda);
    ASSERT_EQ(cb_count, 0);
    callback.call();
    ASSERT_EQ(cb_count, 1);
    callback.disconnect();
}


bool FUNCTION_WAS_CALLED = false;
void example_function() 
{
    FUNCTION_WAS_CALLED = true;
}


TEST(InterruptHandler, StaticConnection)
{
    eg::InterruptHandler handler;
    handler.connect<example_function>();
    ASSERT_FALSE(FUNCTION_WAS_CALLED);
    handler.call();
    ASSERT_TRUE(FUNCTION_WAS_CALLED);
    handler.disconnect();
}

#endif  // defined(OTWAY_TARGET_PLATFORM_BAREMETAL) 