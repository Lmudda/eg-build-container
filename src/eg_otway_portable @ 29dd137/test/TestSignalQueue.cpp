#include "gtest/gtest.h"
#include "signals/Signal.h"
#include "utilities/RingBuffer.h"
#include "TestSingleThreadedUtils.h"

// Tracability:
// PRS-92 Event loop interface.
// PRS-102 Event class directly.

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// A little boiler plate is needed to integrate Signals into the program. 
// This abstraction means that Signals need know nothing about EventLoops 
// or threads or whatever. They fire off an Event by calling an "emit func" 
// and forget about it. It is up to the application to somehow dispatch it 
// back to the Signal for final processing. This can be synchronous, asynchronous 
// or somehow magical.

namespace {

int g_test_emit_count;
class TestEventLoop : public eg::IEventLoop
{
public:
    // Typically this function would post the event to some EventLoop's queue, and 
    // we would have a similar function for each EventLoop. Here we just immediately 
    // dispatch the event, bypassing the EventLoop, and thus making Signal::emit() 
    // synchronous.
    void post(const eg::Event& ev) override
    {
        ++g_test_emit_count;   
        m_queue.put(ev);
    }

    void run() override
    {
        eg::Event ev;
        while (m_queue.get(ev))
        {
            ev.dispatch();
        }
    }

    // For testing
    void run_one()
    {
        eg::Event ev;
        if (m_queue.get(ev))
        {
            ev.dispatch();
        }
    }

    // For testing
    int pending() const { return m_queue.size(); }

#if defined(OTWAY_EVENT_LOOP_WATER_MARK)
    uint16_t get_high_water_mark() const { return 0; }
#endif

private:
    // We'll fake the event loop by creating a queue of pending events and them 
    // explicitly processing the queue to dispatch the events.
    eg::RingBufferArray<eg::Event, 8> m_queue;
};

// A few free functions to use as callbacks. We can check whether the associated 
// values have been changed.

int g_callback1_value;
void callback1(const int& value)
{
    g_callback1_value = value;
}

int g_callback2_value;
void callback2(const int& value)
{
    g_callback2_value = value;
}

int g_callback3_value;
void callback3(const int& value)
{
    g_callback3_value = value;
}

} // namespace {

class SignalQueueTest: public testing::Test
{
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

TEST_F(SignalQueueTest, SinglePendingEventInTheQueue)
{
    eg::Signal<int> signal;
    signal.connect<callback1>();
    signal.connect<callback2>();

    g_callback1_value = 0;
    g_callback2_value = 0;
    g_callback3_value = 0;
    g_test_emit_count = 0;
    EXPECT_TRUE(m_loop->pending() == 0);

    signal.emit(123);
    EXPECT_TRUE(m_loop->pending() == 1);
    EXPECT_TRUE(g_callback1_value == 0);
    EXPECT_TRUE(g_callback2_value == 0);
    EXPECT_TRUE(g_callback3_value == 0);  
    EXPECT_TRUE(g_test_emit_count == 1); // Emit called 

    m_loop->run();
    EXPECT_TRUE(m_loop->pending() == 0);
    EXPECT_TRUE(g_callback1_value == 123); // Event dispatched
    EXPECT_TRUE(g_callback2_value == 123);
    EXPECT_TRUE(g_callback3_value == 0);   
    EXPECT_TRUE(g_test_emit_count == 1);   
}


TEST_F(SignalQueueTest, TwoPendingEventsInTheQueueOne)
{
    eg::Signal<int> signal_a;
    eg::Signal<int> signal_b;
    signal_a.connect<callback1>();
    signal_b.connect<callback2>();
    signal_b.connect<callback3>();

    g_callback1_value = 0;
    g_callback2_value = 0;
    g_callback3_value = 0;
    g_test_emit_count = 0;
    EXPECT_TRUE(m_loop->pending() == 0);

    signal_a.emit(123);
    EXPECT_TRUE(m_loop->pending() == 1);
    EXPECT_TRUE(g_test_emit_count == 1);  
    EXPECT_TRUE(g_callback1_value == 0);
    EXPECT_TRUE(g_callback2_value == 0);
    EXPECT_TRUE(g_callback3_value == 0);  

    signal_b.emit(456);
    EXPECT_TRUE(m_loop->pending() == 2);
    EXPECT_TRUE(g_test_emit_count == 2);
    EXPECT_TRUE(g_callback1_value == 0);
    EXPECT_TRUE(g_callback2_value == 0);
    EXPECT_TRUE(g_callback3_value == 0);  

    m_loop->run_one();
    EXPECT_TRUE(m_loop->pending() == 1);
    EXPECT_TRUE(g_test_emit_count == 2);
    EXPECT_TRUE(g_callback1_value == 123);
    EXPECT_TRUE(g_callback2_value == 0);
    EXPECT_TRUE(g_callback3_value == 0);   

    m_loop->run_one();
    EXPECT_TRUE(m_loop->pending() == 0);
    EXPECT_TRUE(g_test_emit_count == 2);
    EXPECT_TRUE(g_callback1_value == 123);
    EXPECT_TRUE(g_callback2_value == 456);
    EXPECT_TRUE(g_callback3_value == 456);   
}

TEST_F(SignalQueueTest, TwoPendingEventsInTheQueueTwo)
{
    eg::Signal<int> signal_a;
    eg::Signal<int> signal_b;
    signal_a.connect<callback1>();
    signal_b.connect<callback2>();
    signal_b.connect<callback3>();

    g_callback1_value = 0;
    g_callback2_value = 0;
    g_callback3_value = 0;
    g_test_emit_count = 0;
    EXPECT_TRUE(m_loop->pending() == 0);

    signal_a.emit(123);
    EXPECT_TRUE(m_loop->pending() == 1);
    EXPECT_TRUE(g_test_emit_count == 1);  
    EXPECT_TRUE(g_callback1_value == 0);
    EXPECT_TRUE(g_callback2_value == 0);
    EXPECT_TRUE(g_callback3_value == 0);  

    signal_b.emit(456);
    EXPECT_TRUE(m_loop->pending() == 2);
    EXPECT_TRUE(g_test_emit_count == 2);
    EXPECT_TRUE(g_callback1_value == 0);
    EXPECT_TRUE(g_callback2_value == 0);
    EXPECT_TRUE(g_callback3_value == 0);  

    m_loop->run();
    EXPECT_TRUE(m_loop->pending() == 0);
    EXPECT_TRUE(g_test_emit_count == 2);
    EXPECT_TRUE(g_callback1_value == 123);
    EXPECT_TRUE(g_callback2_value == 456);
    EXPECT_TRUE(g_callback3_value == 456);   
}