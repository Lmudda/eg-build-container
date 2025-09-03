
/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "interfaces/IInterCoreEventSink.h"
#include "drivers/helpers/HSEMHelpers.h"
#include "drivers/helpers/GlobalDefs.h"
#include <cstdint>


namespace eg {


class InterCoreEventSink : public IInterCoreEventSink
{
    public:
        struct Config 
        {
            uint8_t  sem_id;
            Core     core;
            uint8_t  priority{GlobalsDefs::DefPreemptPrio};
        };

        InterCoreEventSink(const Config& conf);

        SignalProxy<> on_signalled() override;

    private:
        void on_interrupt(const uint32_t &mask);

    private:
        const Config&  m_conf;
        Signal<> m_on_signalled;
        const uint32_t m_sem_mask;
};


} // namespace eg {

