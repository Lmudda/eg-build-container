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


// Provides event sink functionality for communicating between cores.
class IInterCoreEventSink : private NonCopyable
{
    public:
        virtual ~IInterCoreEventSink() = default;
        virtual SignalProxy<> on_signalled() = 0;
};


} // namespace eg {
