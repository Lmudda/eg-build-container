/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "gtest/gtest.h"
#include "signals/Signal.h"
#include "utilities/RingBuffer.h"
#include "utilities/CriticalSection.h"
#include "mock/event_loop/TestEventLoop.h"

// If the definition produces an error, you are probably trying to compile it alongside
// other compilation units with their own definitios of these functions. 
namespace eg {
extern void on_assert_triggered(char const* file, uint32_t line, const char* function, const char* message) { 
    // Suppressing unused parameter warning: 
    (void) file; 
    (void) line;
    (void) function;
    (void) message;
}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// A little boiler plate is needed to integrate Signals into the program. 
// This abstraction means that Signals need know nothing about EventLoops 
// or threads or whatever. They fire off an Event by calling an "emit func" 
// and forget about it. It is up to the application to somehow dispatch it 
// back to the Signal for final processing. This can be synchronous, asynchronous 
// or somehow magical.


thread_local TestEventLoop* TestEventLoop::m_self;


namespace {

// This is a bit untidy. The goal is to create a separate thread running an event loop,
// and for that thread to register the relevant emit function when it starts - in order
// to make use of the thread context to set a value in the thread local storage. This 
// could all surely be wrapped up more cleanly. The goal here is to test signals.
TestEventLoop loop1{};
TestEventLoop loop2{};

} // namespace

namespace eg {

// This is required in order to make specifying the target loop optional, which 
// is especially useful in single threaded programs.
IEventLoop* default_event_loop_impl() { return &loop1; } 

// This is required in order to allow Signal::dispatch() to determine in which 
// event loop's context (i.e. which thread) it is running. For a single threaded 
// program this doesn't really add value.
IEventLoop* this_event_loop_impl() { return TestEventLoop::m_self; }

} // namespace eg

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


// A few free functions to use as callbacks. We can check whether the associated 
// values have been changed.
namespace {

int g_callback1_value;
std::thread::id g_callback1_id{};
void callback1(const int& value)
{
    g_callback1_value = value;
    g_callback1_id = std::this_thread::get_id();
}

int g_callback2_value;
std::thread::id g_callback2_id{};
void callback2(const int& value)
{
    g_callback2_value = value;
    g_callback2_id = std::this_thread::get_id();
}

int g_callback3_value;
std::thread::id g_callback3_id{};
[[maybe_unused]] void callback3(const int& value)
{
    g_callback3_value = value;
    g_callback3_id = std::this_thread::get_id();
}

} // namespace {


TEST(SignalThread, TwoEventLoopsInThreads )
{
    using namespace std::chrono_literals;

    eg::Signal<int> signal_a;
    signal_a.connect<callback1>(); // Used the default emit func
    signal_a.connect<callback2>(loop1);

    eg::Signal<int> signal_b;
    signal_b.connect<callback3>(loop2);

    g_callback1_value  = 0;
    g_callback2_value  = 0;
    g_callback3_value  = 0;
    //g_loop1_emit_count = 0;
    //g_loop2_emit_count = 0;

    signal_a.emit(123);
    signal_b.emit(456);
    std::this_thread::sleep_for(20ms);

    EXPECT_TRUE(g_callback1_value == 123);
    EXPECT_TRUE(g_callback2_value == 123);
    EXPECT_TRUE(g_callback3_value == 456);  
    //REQUIRE(g_loop1_emit_count == 1);
    //REQUIRE(g_loop2_emit_count == 1);

    EXPECT_TRUE(g_callback1_id == g_callback2_id);
    EXPECT_TRUE(g_callback1_id != g_callback3_id);
    EXPECT_TRUE(g_callback1_id == loop1.get_id());
    EXPECT_TRUE(g_callback3_id == loop2.get_id());
}


TEST(SignalThread, OneSignalEmitsToTwoThreads)
{
    using namespace std::chrono_literals;

    eg::Signal<int> signal;
    signal.connect<callback1>(); // Used the default emit func
    signal.connect<callback2>(loop1);
    signal.connect<callback3>(loop2);

    g_callback1_value  = 0;
    g_callback2_value  = 0;
    g_callback3_value  = 0;
    //g_loop1_emit_count = 0;
    //g_loop2_emit_count = 0;

    signal.emit(123);
    std::this_thread::sleep_for(20ms);

    EXPECT_TRUE(g_callback1_value == 123);
    EXPECT_TRUE(g_callback2_value == 123);
    EXPECT_TRUE(g_callback3_value == 123);  
    //REQUIRE(g_loop1_emit_count == 1);
    //REQUIRE(g_loop2_emit_count == 1);

    EXPECT_TRUE(g_callback1_id == g_callback2_id);
    EXPECT_TRUE(g_callback1_id != g_callback3_id);
    EXPECT_TRUE(g_callback1_id == loop1.get_id());
    EXPECT_TRUE(g_callback3_id == loop2.get_id());
}