
/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "interfaces/IInterCoreEventSource.h"
#include <cstdint>


namespace eg {


class InterCoreEventSource : public IInterCoreEventSource
{
    public:
        InterCoreEventSource(uint8_t sem_id);

        bool signal() override;

    private:
        const uint8_t m_sem_id;
};


} // namespace eg {

