/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "signals/Signal.h"
#include "GlobalDefs.h"
#include <cstdint>
#include "stm32h7xx.h"


namespace eg {


enum class Core : uint8_t
{
    CortexM7,
    CortexM4
};

struct HSEMHelpers
{
public:
    static void assert_id(uint8_t sem_id);
    static void configure();
    static IRQn_Type irq_number(Core core);
    static void irq_enable(Core core, uint32_t priority);
    
    // Note that this will be called under interrupt context
    static SignalProxy<uint32_t> irq_handler();
};


} // namespace eg {
