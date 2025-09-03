#include "gtest/gtest.h"
#include "signals/Signal.h"
#include "TestSingleThreadedUtils.h"

// Tracability: 
// PRS-100 defines the signal class.
// PRS-101 defines the signal proxy class.
// PRS-102 defines the event class, which is not exposed to the signal user but 
//         drives the operation of signals. 

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

// Tracability: PRS-92 Event loop interface
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
        ev.dispatch();
    }
    void run() override {}
#if defined(OTWAY_EVENT_LOOP_WATER_MARK)
    uint16_t get_high_water_mark() const { return 0; }
#endif
};

} // namespace


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


// A few free functions to use as callbacks. We can check whether the associated 
// values have been changed.
namespace {

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
[[maybe_unused]] void callback3(const int& value)
{
    g_callback3_value = value;
}

} // namespace {


// A simple class which uses a private member function as a callback.
// We can check whether the member value has been changed.
namespace {

struct Thing
{
public: 
    Thing(eg::Signal<int>& signal) 
    // It is not generally necessary to remember the signals to which we have connected.
    // Added for testing.
    : m_signal{signal}
    { 
        m_conn = signal.connect<&Thing::set>(this);
    }
    int get() const 
    { 
        return m_value; 
    }
    #if defined(OTWAY_TARGET_PLATFORM_BAREMETAL) // No disconnect() for non-baremetal right now
    // This is for test only - want to make sure disconnection works 
    // for a member function.
    void disconnect()
    { 
        m_signal.disconnect(m_conn); 
    }
    #endif // defined(OTWAY_TARGET_PLATFORM_BAREMETAL)
    
private:
    void set(const int& value) 
    { 
        m_value = value; 
    }
private:
    int m_value{};
    // codechecker_intentional [clang-diagnostic-unused-variable, clang-diagnostic-unused-private-field] platform dependent use; gcc doesn't implement [[maybe_unused]] on private member variables.
    eg::Signal<int>& m_signal;
    void* m_conn{};
};

} // namespace


//Fixture for tests
class SignalTest : public testing::Test {
    protected:
    eg::Signal<int> int_signal;        
    TestEventLoop * m_loop;

    virtual void SetUp() 
    {
        m_loop = new TestEventLoop(); 
        eg::CURRENT_EVENT_LOOP = m_loop;
    }

    virtual void TearDown() 
    {
        delete m_loop;
        eg::CURRENT_EVENT_LOOP = nullptr;
    }
};


TEST_F(SignalTest, EmitToFreeFunctions)
{
    [[maybe_unused]] auto conn1 = int_signal.connect<callback1>();
    [[maybe_unused]] auto conn2 = int_signal.connect<callback2>();

    g_callback1_value = 0;
    g_callback2_value = 0;
    g_callback3_value = 0;
    g_test_emit_count = 0;

    int_signal.emit(123);
    EXPECT_TRUE(g_callback1_value == 123);
    EXPECT_TRUE(g_callback2_value == 123);
    EXPECT_TRUE(g_callback3_value == 0);   // This was not changed.
    EXPECT_TRUE(g_test_emit_count == 1);   // One event for two callbacks

    #if defined(OTWAY_TARGET_PLATFORM_BAREMETAL) // No disconnect() for non-baremetal right now
    // Disconnect from the signal.
    int_signal.disconnect(conn1);
    int_signal.emit(256);
    EXPECT_TRUE(g_callback1_value == 123); // This was not changed.
    EXPECT_TRUE(g_callback2_value == 256);
    EXPECT_TRUE(g_callback3_value == 0);   // This was not changed.
    EXPECT_TRUE(g_test_emit_count == 2);   // One more event for the remaining callback

    // Zero connections means no event will be posted.    
    int_signal.disconnect(conn2);
    int_signal.emit(101);
    EXPECT_TRUE(g_callback1_value == 123); // This was not changed.
    EXPECT_TRUE(g_callback2_value == 256); // This was not changed.
    EXPECT_TRUE(g_callback3_value == 0);   // This was not changed.
    EXPECT_TRUE(g_test_emit_count == 2);   // There nothing to emit.
    #endif // defined(OTWAY_TARGET_PLATFORM_BAREMETAL)
}


TEST_F(SignalTest, EmitToMemberFunctions)
{
    // In real code, the signal would generally be a member of some larger object
    // such as a timer, driver or state machine.

    // Pass signal to constructors to let these objects connect to it.
    Thing thing1{int_signal};
    Thing thing2{int_signal};
    g_test_emit_count = 0;

    EXPECT_TRUE(thing1.get() == 0);
    EXPECT_TRUE(thing2.get() == 0);

    int_signal.emit(123);
    EXPECT_TRUE(thing1.get() == 123);
    EXPECT_TRUE(thing2.get() == 123);
    EXPECT_TRUE(g_test_emit_count == 1);

    #if defined(OTWAY_TARGET_PLATFORM_BAREMETAL) // No disconnect() for non-baremetal right now
    thing1.disconnect();
    int_signal.emit(256);
    EXPECT_TRUE(thing1.get() == 123); // This was not changed.
    EXPECT_TRUE(thing2.get() == 256);
    EXPECT_TRUE(g_test_emit_count == 2);

    thing2.disconnect();
    int_signal.emit(101);
    EXPECT_TRUE(thing1.get() == 123); // This was not changed.
    EXPECT_TRUE(thing2.get() == 256);
    EXPECT_TRUE(g_test_emit_count == 2);
    #endif // defined(OTWAY_TARGET_PLATFORM_BAREMETAL)
}


TEST_F(SignalTest, CallToFreeFunctions)
{
    [[maybe_unused]] auto conn1 = int_signal.connect<callback1>();
    [[maybe_unused]] auto conn2 = int_signal.connect<callback2>();
    g_callback1_value = 0;
    g_callback2_value = 0;
    g_callback3_value = 0;
    g_test_emit_count = 0;

    // call() API does not go via the emit function. This is a synchronous.
    int_signal.call(123);
    EXPECT_TRUE(g_callback1_value == 123);
    EXPECT_TRUE(g_callback2_value == 123);
    EXPECT_TRUE(g_callback3_value == 0);
    EXPECT_TRUE(g_test_emit_count == 0);

    #if defined(OTWAY_TARGET_PLATFORM_BAREMETAL) // No disconnect() for non-baremetal right now
    int_signal.disconnect(conn1);
    int_signal.call(256);
    EXPECT_TRUE(g_callback1_value == 123);
    EXPECT_TRUE(g_callback2_value == 256);
    EXPECT_TRUE(g_callback3_value == 0);
    EXPECT_TRUE(g_test_emit_count == 0);

    int_signal.disconnect(conn2);
    int_signal.call(101);
    EXPECT_TRUE(g_callback1_value == 123);
    EXPECT_TRUE(g_callback2_value == 256);
    EXPECT_TRUE(g_callback3_value == 0);
    EXPECT_TRUE(g_test_emit_count == 0);
    #endif // defined(OTWAY_TARGET_PLATFORM_BAREMETAL)
}


TEST_F(SignalTest, CallToMemberFunctions)
{
    // In real code, the signal would generally be a member of some larger object
    // such as a timer, driver or state machine.
    // Pass signal to constructors to let these objects connect to it.
    Thing thing1{int_signal};
    Thing thing2{int_signal};
    g_test_emit_count = 0;

    EXPECT_TRUE(thing1.get() == 0);
    EXPECT_TRUE(thing2.get() == 0);

    int_signal.call(123);
    EXPECT_TRUE(thing1.get() == 123);
    EXPECT_TRUE(thing2.get() == 123);
    EXPECT_TRUE(g_test_emit_count == 0);

    #if defined(OTWAY_TARGET_PLATFORM_BAREMETAL) // No disconnect() for non-baremetal right now
    thing1.disconnect();
    int_signal.call(256);
    EXPECT_TRUE(thing1.get() == 123); // This was not changed.
    EXPECT_TRUE(thing2.get() == 256);
    EXPECT_TRUE(g_test_emit_count == 0);

    thing2.disconnect();
    int_signal.call(101);
    EXPECT_TRUE(thing1.get() == 123); // This was not changed.
    EXPECT_TRUE(thing2.get() == 256); // This was not changed.
    EXPECT_TRUE(g_test_emit_count == 0);
    #endif // defined(OTWAY_TARGET_PLATFORM_BAREMETAL)
}


TEST_F(SignalTest, ConnectViaAdapter)
{
    // The purpose of this adapter is to hide the call() and emit() methods 
    // from those who need access to make connections but should not call
    // those methods. Effectively makes the call() and emit() private to the 
    // owner of the Signal object.
    eg::SignalProxy<int> int_proxy{int_signal};

    [[maybe_unused]] auto conn1 = int_proxy.connect<callback1>();
    [[maybe_unused]] auto conn2 = int_proxy.connect<callback2>();

    g_callback1_value = 0;
    g_callback2_value = 0;
    g_callback3_value = 0;
    g_test_emit_count = 0;

    // call() API does not go via the emit function. This is a synchronous.
    int_signal.call(123);
    EXPECT_TRUE(g_callback1_value == 123);
    EXPECT_TRUE(g_callback2_value == 123);
    EXPECT_TRUE(g_callback3_value == 0);
    EXPECT_TRUE(g_test_emit_count == 0);

    #if defined(OTWAY_TARGET_PLATFORM_BAREMETAL) // No disconnect() for non-baremetal right now
    int_proxy.disconnect(conn1);
    int_signal.call(256);
    EXPECT_TRUE(g_callback1_value == 123);
    EXPECT_TRUE(g_callback2_value == 256);
    EXPECT_TRUE(g_callback3_value == 0);
    EXPECT_TRUE(g_test_emit_count == 0);

    int_proxy.disconnect(conn2);
    int_signal.call(101);
    EXPECT_TRUE(g_callback1_value == 123);
    EXPECT_TRUE(g_callback2_value == 256);
    EXPECT_TRUE(g_callback3_value == 0);
    EXPECT_TRUE(g_test_emit_count == 0);
    #endif // defined(OTWAY_TARGET_PLATFORM_BAREMETAL)
}


[[maybe_unused]] static void callback_void()
{    
}


[[maybe_unused]] static void callback_double(const double&)
{    
}

#if defined(OTWAY_TARGET_PLATFORM_BAREMETAL) // Pool tests only relevant for baremetal target

TEST_F(SignalTest, PoolSizeMatchesMaxSignalLinks)
{
    // May need updating if MAX_SIGNAL_LINKS becomes a build argument
    EXPECT_EQ(eg::SignalBase::pool_free(), eg::SignalBase::pool_size());
}

TEST_F(SignalTest, DestructorFreesConnectionsZeroArgument)
{
    const uint16_t pool_size = eg::SignalBase::pool_free();
    {
        // Zero argument specialisation
        eg::Signal<> signal;
        signal.connect<callback_void>();
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 1);
        signal.connect<callback_void>();
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 2);
        signal.connect<callback_void>();
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 3);
        signal.connect<callback_void>();
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 4);
    }
    // Signal has gone out of scope
    EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size);
}

TEST_F(SignalTest, DestructorFreesConnectionsSingleArgument)
{
    const uint16_t pool_size = eg::SignalBase::pool_free();
    {
        // Single argument specialisation
        eg::Signal<double> signal;
        signal.connect<callback_double>();
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 1);
        signal.connect<callback_double>();
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 2);
        signal.connect<callback_double>();
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 3);
        signal.connect<callback_double>();
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 4);
    }
    // Signal has gone out of scope
    EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size);
}


TEST_F(SignalTest, DisconnectFreesConnectionsZeroArgument)
{
    const uint16_t pool_size = eg::SignalBase::pool_free();
    {
        // Zero argument specialisation
        eg::Signal<> signal;
        auto conn1 = signal.connect<callback_void>();
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 1);
        auto conn2 = signal.connect<callback_void>();
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 2);
        auto conn3 = signal.connect<callback_void>();
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 3);
        auto conn4 = signal.connect<callback_void>();
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 4);

        signal.disconnect(conn4);
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 3);
        signal.disconnect(conn3);
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 2);
        signal.disconnect(conn1);
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 1);
        signal.disconnect(conn2);
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size);
    }
    // Signal has gone out of scope
    EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size);
}

TEST_F(SignalTest, DisconnectFreesConnectionsSingleArgument)
{
    const uint16_t pool_size = eg::SignalBase::pool_free();
    {
        // Single argument specialisation
        eg::Signal<double> signal;
        auto conn1 = signal.connect<callback_double>();
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 1);
        auto conn2 = signal.connect<callback_double>();
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 2);
        auto conn3 = signal.connect<callback_double>();
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 3);
        auto conn4 = signal.connect<callback_double>();
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 4);

        signal.disconnect(conn4);
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 3);
        signal.disconnect(conn3);
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 2);
        signal.disconnect(conn1);
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size - 1);
        signal.disconnect(conn2);
        EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size);
    }
    // Signal has gone out of scope
    EXPECT_TRUE(eg::SignalBase::pool_free() == pool_size);
}

#endif // defined(OTWAY_TARGET_PLATFORM_BAREMETAL) 

int g_int_arg;
bool g_bool_arg;
void two_arguments(const int& i, const bool& b)
{
    g_int_arg  = i;
    g_bool_arg = b;
}


TEST_F(SignalTest, SignalWithTwoArguments)
{
    eg::Signal<int, bool> sig;
    sig.connect<two_arguments>();
    g_int_arg  = 0;
    g_bool_arg = false;
    sig.emit(1234, true);
    EXPECT_TRUE(g_int_arg == 1234);
    EXPECT_TRUE(g_bool_arg == true);
}