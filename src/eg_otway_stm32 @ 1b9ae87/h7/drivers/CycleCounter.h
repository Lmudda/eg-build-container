/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "utilities/NonCopyable.h"
#include <cstdint>

namespace eg {


// Simple wrapper around the cycle counter
class CycleCounter : private NonCopyable
{
    public:
        CycleCounter(uint32_t expiry_us);

        uint32_t start();
        uint32_t stop();
        uint32_t get_count() const;
        bool     has_expired();

    private:
        uint32_t m_expiry_cycles;
        uint32_t m_start_cycle;
        bool     m_has_expired;
};


} // namespace eg {

