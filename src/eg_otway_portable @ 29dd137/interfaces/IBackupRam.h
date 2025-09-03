/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2025.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>

#include "utilities/NonCopyable.h"


namespace eg
{


class IBackupRam : private NonCopyable
{
    public:
        virtual ~IBackupRam() = default;

    // Get a pointer to the start of backup RAM
    virtual uint8_t* back_ram_ptr() const = 0;

    // Query the size of backup RAM, in bytes
    virtual uint32_t backup_ram_size() const = 0;
};


} // namespace eg
