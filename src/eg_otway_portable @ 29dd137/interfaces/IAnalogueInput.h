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
#include "signals/Signal.h"


namespace eg {


class IAnalogueInput : private NonCopyable
{
};


class IAnalogueInputBlocking : private NonCopyable
{
    public:
        virtual ~IAnalogueInputBlocking() = default;
        virtual bool read(uint32_t& result) = 0;
};


} // namespace eg {
