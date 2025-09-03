
/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "interfaces/IInterCoreCriticalSection.h"
#include "drivers/helpers/HSEMHelpers.h"
#include "drivers/helpers/GlobalDefs.h"
#include "timers/Timer.h"
#include <cstdint>


namespace eg {


class InterCoreCriticalSection : public IInterCoreCriticalSection
{
    public:
        struct Config 
        {
            uint8_t  sem_id;
            Core     core;
            uint8_t  priority{GlobalsDefs::DefPreemptPrio};
        };

        InterCoreCriticalSection(const Config& conf);

        void lock(uint32_t timeout_ms) override;
        void unlock() override;
        SignalProxy<> on_locked() override;
        SignalProxy<> on_lock_timeout() override;

    private:
        void on_released_interrupt(const uint32_t &mask);
        void on_timer();
        bool try_lock();

    private:
        enum class State
        {
            Unlocked,
            Pending,
            Locked
        };

    private:
        const Config&  m_conf;
        Signal<> m_on_locked;
        Signal<> m_on_lock_timeout;
        Timer m_timer;
        const uint32_t m_sem_mask;
        volatile State m_state;
};


} // namespace eg {

