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
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <optional>


#if !defined(OTWAY_TARGET_PLATFORM_LINUX)
#error This file is requires OTWAY_TARGET_PLATFORM_LINUX to be defined.
#endif


namespace eg {


class ThreadEventLoop : public IEventLoop
{
    public:
        ThreadEventLoop(const char* name = "loop");
        ~ThreadEventLoop();

        void post(const Event& event) override;
        void run() override {}
        void stop();

    private:
        static void exec_static(std::stop_token stoken, ThreadEventLoop* self, const char* name);
        void exec(std::stop_token stoken);

    private:
        std::jthread                m_thread;
        std::mutex                  m_mutex;
        std::condition_variable_any m_condition;
        std::queue<Event>           m_queue;
};


} // namespace eg {
