/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "signals/InterruptHandler.h"
#include "GlobalDefs.h"
#include <cstdint>
#include "stm32h7xx.h"


namespace eg {


enum class Can
{
    #if defined(FDCAN1_BASE)
    FdCan1 = FDCAN1_BASE,
    #endif
    #if defined(FDCAN2_BASE)
    FdCan2 = FDCAN2_BASE,
    #endif
};


struct CANHelpers
{
    enum class IrqType { IT0, IT1 };
    static InterruptHandler* irq_handler(Can can, IrqType type);
    static void irq_enable(Can can, IrqType type, uint32_t priority);
    static void enable_clock(Can can, uint32_t clk_src);
};



} // namespace eg {
    


