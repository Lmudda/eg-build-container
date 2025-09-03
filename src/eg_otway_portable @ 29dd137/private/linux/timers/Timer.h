/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "signals/Signal.h"
#include "utilities/NonCopyable.h"
#include <chrono>


namespace eg {


// This class provides a simple interruptible interval timer which can be used to defer operations. It is very
// useful as a member of another class such as a state machine, in which it can be used to generate timeouts
// and other events which drive the state. Simply connect a member function to the exposed Signal object, and
// then start or stop the timer wherever makes sense. See the Blinky example.
class Timer : private NonCopyable
{
    public:
        using Millis = std::chrono::milliseconds;
        enum class Type { OneShot, Repeating };

        ~Timer() { stop(); }

        void start(Millis period, Type type);
        void stop();

        SignalProxy<> on_timer() { return SignalProxy<>{m_signal}; }

    private:
        void emit() { m_signal.emit(); }   // Asynchronous - place an event in one or more event loops

    private:
        // What is the range of this? Maybe use an integer base?
        using TimePoint = std::chrono::steady_clock::time_point;

        // This is used to form a doubly linked list of running timers
        // without needing to dynamically allocated links. Each timer
        // can only appear onced in the list, so it makes sense that
        // it owns the link data at a member.
        struct Link
        {
            TimePoint expiry{};
            Timer*    timer{};
            Link*     next{};
            Link*     prev{};
        };
        friend class TimerQueue;

    private:
        Millis   m_period{};
        Type     m_type{};
        Signal<> m_signal{};
        Link     m_link{};
};


} // namespace eg
