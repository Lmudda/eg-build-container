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


namespace eg {


// TODO_AC Using and interface for the backend is not really consistent with using simple
// callbacks for the timestamps. 
class ILoggerBackend : private NonCopyable
{
    public:
        // TODO_AC I think adding the new_line argument was a mistake. The idea was to allow
        // the logger formatting to use a small buffer more than once for the same message 
        // (prefix and then the actual message).
        virtual void write(const char* message, bool new_line = false) = 0;

        virtual ~ILoggerBackend() = default; // Added to satisfy static analyser. 
};


} // namespace eg {    