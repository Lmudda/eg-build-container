/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

// Traceability:
// PRS-103 Timer interface and PRS-104 Timer class

#if defined(OTWAY_TARGET_PLATFORM_BAREMETAL) 

#include "gtest/gtest.h"
#include "timers/Timer.h"
#include <iostream>
#include "TestSingleThreadedUtils.h"
namespace {

int g_timer1_count;
void on_timer1()
{
    ++g_timer1_count;
}

int g_timer2_count;
void on_timer2()
{
    ++g_timer2_count;
}

int g_timer3_count;
void on_timer3()
{
    ++g_timer3_count;
}

class TestEventLoop : public eg::IEventLoop
{
public:
    // Typically this function would post the event to some EventLoop's queue, and 
    // we would have a similar function for each EventLoop. Here we just immediately 
    // dispatch the event, bypassing the EventLoop, and thus making Signal::emit() 
    // synchronous.
    void post(const eg::Event& ev) override { ev.dispatch(); }
    void run() override {}
#if defined(OTWAY_EVENT_LOOP_WATER_MARK)
    uint16_t get_high_water_mark() const { return 0; }
#endif
};

} // namespace {

//Fixture for tests
class TimerTest : public testing::Test {
    protected:     
    TestEventLoop * m_loop;

    virtual void SetUp() {
        m_loop = new TestEventLoop(); 
        eg::CURRENT_EVENT_LOOP = m_loop;
    }

    virtual void TearDown() {
        delete m_loop;
        eg::CURRENT_EVENT_LOOP = nullptr;
    }
};

TEST_F(TimerTest, OneTimerStopped)
{
    constexpr int TIMER1_TICKS = 93;

    eg::Timer timer1{TIMER1_TICKS, eg::Timer::Type::Repeating};
    timer1.on_update().connect<on_timer1>();

    g_timer1_count = 0;

    for (int i = 0; i < 1000; ++i)
    {
        EXPECT_TRUE(g_timer1_count == 0);
        eg::tick_software_timers();
    }
}

TEST_F(TimerTest, OneTimerStarted)
{
    constexpr int TIMER1_TICKS = 93;

    eg::Timer timer1{TIMER1_TICKS, eg::Timer::Type::Repeating};
    timer1.on_update().connect<on_timer1>();
    timer1.start();

    g_timer1_count = 0;

    for (int i = 0; i < 1000; ++i)
    {
        EXPECT_TRUE(g_timer1_count == i / TIMER1_TICKS);
        eg::tick_software_timers();
    }
}

TEST_F(TimerTest, GetTicksRemaining)
{
    constexpr int TIMER1_TICKS = 93;

    eg::Timer timer1{TIMER1_TICKS, eg::Timer::Type::Repeating};
    timer1.on_update().connect<on_timer1>();
    timer1.start();

    EXPECT_EQ(timer1.get_ticks_remaining(), TIMER1_TICKS);

    for (int i = 0; i < 1000; ++i)
    {
        if (i == 0 || i % TIMER1_TICKS == 0) EXPECT_EQ(timer1.get_ticks_remaining(), TIMER1_TICKS);
        else EXPECT_EQ(timer1.get_ticks_remaining(), TIMER1_TICKS - (i % TIMER1_TICKS));
        eg::tick_software_timers();
    }
}


TEST_F(TimerTest, OneTimerOneShot)
{
    constexpr int TIMER1_TICKS = 93;

    eg::Timer timer1{TIMER1_TICKS, eg::Timer::Type::OneShot};
    timer1.on_update().connect<on_timer1>();
    timer1.start();

    g_timer1_count = 0;

    for (int i = 0; i < 1000; ++i)
    {
        EXPECT_TRUE(g_timer1_count == (i >= TIMER1_TICKS));
        eg::tick_software_timers();
    }
}


TEST_F(TimerTest, ThreeTimersStarted)
{
    constexpr int TIMER1_TICKS = 93;
    constexpr int TIMER2_TICKS = 101;
    constexpr int TIMER3_TICKS = 113;

    eg::Timer timer1{TIMER1_TICKS, eg::Timer::Type::Repeating};
    timer1.on_update().connect<on_timer1>();
    timer1.start();

    eg::Timer timer2{TIMER2_TICKS, eg::Timer::Type::Repeating};
    timer2.on_update().connect<on_timer2>();
    timer2.start();

    eg::Timer timer3{TIMER3_TICKS, eg::Timer::Type::Repeating};
    timer3.on_update().connect<on_timer3>();
    timer3.start();

    g_timer1_count = 0;
    g_timer2_count = 0;
    g_timer3_count = 0;

    for (int i = 0; i < 1000; ++i)
    {
        EXPECT_TRUE(g_timer1_count == i / TIMER1_TICKS);
        EXPECT_TRUE(g_timer2_count == i / TIMER2_TICKS);
        EXPECT_TRUE(g_timer3_count == i / TIMER3_TICKS);
        eg::tick_software_timers();
    }

    timer2.stop();    

    for (int i = 0; i < 1000; ++i)
    {
        EXPECT_TRUE(g_timer1_count == (1000 + i) / TIMER1_TICKS);
        EXPECT_TRUE(g_timer2_count == (1000 )    / TIMER2_TICKS); // Unchanged as stopped
        EXPECT_TRUE(g_timer3_count == (1000 + i) / TIMER3_TICKS);
        eg::tick_software_timers();
    }
}


TEST_F(TimerTest, ThreeTimersMixed)
{
    constexpr int TIMER1_TICKS = 93;
    constexpr int TIMER2_TICKS = 101;
    constexpr int TIMER3_TICKS = 113;

    eg::Timer timer1{TIMER1_TICKS, eg::Timer::Type::Repeating};
    timer1.on_update().connect<on_timer1>();
    timer1.start();

    eg::Timer timer2{TIMER2_TICKS, eg::Timer::Type::OneShot};
    timer2.on_update().connect<on_timer2>();
    timer2.start();

    eg::Timer timer3{TIMER3_TICKS, eg::Timer::Type::Repeating};
    timer3.on_update().connect<on_timer3>();
    timer3.start();

    g_timer1_count = 0;
    g_timer2_count = 0;
    g_timer3_count = 0;

    for (int i = 0; i < 1000; ++i)
    {
        EXPECT_TRUE(g_timer1_count == i / TIMER1_TICKS);
        EXPECT_TRUE(g_timer2_count == (i >= TIMER2_TICKS));
        EXPECT_TRUE(g_timer3_count == i / TIMER3_TICKS);
        eg::tick_software_timers();
    }

    timer2.stop();    

    for (int i = 0; i < 1000; ++i)
    {
        EXPECT_TRUE(g_timer1_count == (1000 + i) / TIMER1_TICKS);
        EXPECT_TRUE(g_timer2_count == 1);
        EXPECT_TRUE(g_timer3_count == (1000 + i) / TIMER3_TICKS);
        eg::tick_software_timers();
    }
}


TEST_F(TimerTest, ThreeTimersScoped)
{
    constexpr int TIMER1_TICKS = 93;
    constexpr int TIMER2_TICKS = 101;
    constexpr int TIMER3_TICKS = 113;

    g_timer1_count = 0;
    g_timer2_count = 0;
    g_timer3_count = 0;

    eg::Timer timer1{TIMER1_TICKS, eg::Timer::Type::Repeating};
    timer1.on_update().connect<on_timer1>();
    timer1.start();

    { // Scope 1
        eg::Timer timer2{TIMER2_TICKS, eg::Timer::Type::Repeating};
        timer2.on_update().connect<on_timer2>();
        timer2.start();
        
        { // Scope 2
            eg::Timer timer3{TIMER3_TICKS, eg::Timer::Type::Repeating};
            timer3.on_update().connect<on_timer3>();
            timer3.start();

            for (int i = 0; i < 1000; ++i)
            {
                EXPECT_TRUE(g_timer1_count == i / TIMER1_TICKS);
                EXPECT_TRUE(g_timer2_count == i / TIMER2_TICKS);
                EXPECT_TRUE(g_timer3_count == i / TIMER3_TICKS);
                eg::tick_software_timers();
            }
        } // Scope 2

        // timer3 should stop on going out of scope.
        for (int i = 0; i < 1000; ++i)
        {
            EXPECT_TRUE(g_timer1_count == (1000 + i) / TIMER1_TICKS);
            EXPECT_TRUE(g_timer2_count == (1000 + i) / TIMER2_TICKS); 
            EXPECT_TRUE(g_timer3_count == (1000)     / TIMER3_TICKS);
            eg::tick_software_timers();
        }
    } // Scope 1

    // timer2 should stop on going out of scope.
    for (int i = 0; i < 1000; ++i)
    {
        EXPECT_TRUE(g_timer1_count == (2000 + i) / TIMER1_TICKS);
        EXPECT_TRUE(g_timer2_count == (2000)     / TIMER2_TICKS); 
        EXPECT_TRUE(g_timer3_count == (1000)     / TIMER3_TICKS);
        eg::tick_software_timers();
    }
}


TEST_F(TimerTest, OneTimerChangingTypeAndPeriod)
{
    constexpr int TIMER1_TICKS_A = 93;
    constexpr int TIMER1_TICKS_B = 107;

    eg::Timer timer1{TIMER1_TICKS_A, eg::Timer::Type::Repeating};
    timer1.on_update().connect<on_timer1>();

    EXPECT_TRUE(timer1.get_type() == eg::Timer::Type::Repeating);
    EXPECT_TRUE(timer1.get_period() == TIMER1_TICKS_A);

    timer1.set_type(eg::Timer::Type::OneShot);
    EXPECT_TRUE(timer1.get_type() == eg::Timer::Type::OneShot);
    EXPECT_TRUE(timer1.is_running() == false);
    timer1.set_type(eg::Timer::Type::Repeating);
    EXPECT_TRUE(timer1.get_type() == eg::Timer::Type::Repeating);
    EXPECT_TRUE(timer1.is_running() == false);

    timer1.set_period(TIMER1_TICKS_B);
    EXPECT_TRUE(timer1.get_period() == TIMER1_TICKS_B);
    EXPECT_TRUE(timer1.is_running() == false);
    timer1.set_period(TIMER1_TICKS_A);
    EXPECT_TRUE(timer1.get_period() == TIMER1_TICKS_A);
    EXPECT_TRUE(timer1.is_running() == false);
    
    EXPECT_TRUE(timer1.is_running() == false);
    timer1.start();
    EXPECT_TRUE(timer1.is_running() == true);
    timer1.stop();
    EXPECT_TRUE(timer1.is_running() == false);

    timer1.start();
    EXPECT_TRUE(timer1.is_running() == true);
    timer1.set_period(TIMER1_TICKS_B);
    EXPECT_TRUE(timer1.is_running() == false);
    timer1.start();
    EXPECT_TRUE(timer1.is_running() == true);
    timer1.set_type(eg::Timer::Type::OneShot);
    EXPECT_TRUE(timer1.is_running() == false);

    timer1.set_period(TIMER1_TICKS_A);
    timer1.set_type(eg::Timer::Type::Repeating);
    timer1.start();

    g_timer1_count = 0;
    for (int i = 0; i < 1000; ++i)
    {
        EXPECT_TRUE(g_timer1_count == i / TIMER1_TICKS_A);
        eg::tick_software_timers();
    }

    timer1.set_period(TIMER1_TICKS_B);
    timer1.start();

    g_timer1_count = 0;
    for (int i = 0; i < 1000; ++i)
    {
        EXPECT_TRUE(g_timer1_count == i / TIMER1_TICKS_B);
        eg::tick_software_timers();
    }
}


TEST_F(TimerTest, PeriodOfOne)
{
    constexpr int TIMER1_TICKS_A = 1;

    eg::Timer timer1{TIMER1_TICKS_A, eg::Timer::Type::Repeating};
    timer1.on_update().connect<on_timer1>();
    timer1.start();

    g_timer1_count = 0;
    for (int i = 0; i < 1000; ++i)
    {
        EXPECT_TRUE(g_timer1_count == i / TIMER1_TICKS_A);
        eg::tick_software_timers();
    }
}


#endif // defined(OTWAY_TARGET_PLATFORM_BAREMETAL) 