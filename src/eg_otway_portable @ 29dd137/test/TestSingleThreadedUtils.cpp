#include "signals/Signal.h"
#include "TestSingleThreadedUtils.h"

// Defining required functions for tests

namespace eg {

IEventLoop* CURRENT_EVENT_LOOP = nullptr;

// This is required in order to make specifying the target loop optional, which 
// is especially useful in single threaded programs.
IEventLoop* default_event_loop_impl() { return CURRENT_EVENT_LOOP; } 

// This is required in order to allow Signal::dispatch() to determine in which 
// event loop's context (i.e. which thread) it is running. For a single threaded 
// program this doesn't really add value.
IEventLoop* this_event_loop_impl() { return CURRENT_EVENT_LOOP; }

void on_assert_triggered(char const* file, uint32_t line, const char* function, const char* message) { 
    // Suppressing unused parameter warning: 
    (void) file; 
    (void) line;
    (void) function;
    (void) message;
}

} // namespace eg