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


class ISystemReset : private NonCopyable
{
    public:
        virtual ~ISystemReset() = default;

        // Perform a hard reset of the system. This method should not return.
        virtual void hard_reset() = 0;
};


} // namespace eg
