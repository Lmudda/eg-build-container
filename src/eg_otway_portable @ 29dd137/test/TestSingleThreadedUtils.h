#pragma once
#include "signals/Signal.h"

// Declaring required functions for tests

namespace eg {

extern IEventLoop* CURRENT_EVENT_LOOP;
extern IEventLoop* default_event_loop_impl();
extern IEventLoop* this_event_loop_impl();
extern void on_assert_triggered(char const* file, uint32_t line, const char* function, const char* message);

} // namespace eg