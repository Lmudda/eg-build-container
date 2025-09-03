/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "interfaces/IAnalogueInput.h"
#include "helpers/GPIOHelpers.h"


namespace eg {

    class AnalogueInput : public IAnalogueInput
    {
        public:
            struct Config
            {
                Port       port;             
                Pin        pin;
            };

        public:
            AnalogueInput(const Config& conf);
	
        private:
	        Config m_conf;	
    };

} // namespace eg
