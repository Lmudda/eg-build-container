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
#include <stm32u5xx.h>


namespace eg {

enum class Dma
{
    Dma1Channel0 = GPDMA1_Channel0_BASE,
    Dma1Channel1 = GPDMA1_Channel1_BASE,
    Dma1Channel2 = GPDMA1_Channel2_BASE,
    Dma1Channel3 = GPDMA1_Channel3_BASE,
    Dma1Channel4 = GPDMA1_Channel4_BASE,
    Dma1Channel5 = GPDMA1_Channel5_BASE,
    Dma1Channel6 = GPDMA1_Channel6_BASE,
    Dma1Channel7 = GPDMA1_Channel7_BASE,
    Dma1Channel8 = GPDMA1_Channel8_BASE,
    Dma1Channel9 = GPDMA1_Channel9_BASE,
    Dma1Channel10 = GPDMA1_Channel10_BASE,
    Dma1Channel11 = GPDMA1_Channel11_BASE,
    Dma1Channel12 = GPDMA1_Channel12_BASE,
    Dma1Channel13 = GPDMA1_Channel13_BASE,
    Dma1Channel14 = GPDMA1_Channel14_BASE,
    Dma1Channel15 = GPDMA1_Channel15_BASE,
};


struct DMAHelpers
{
    static DMA_Channel_TypeDef* instance(Dma channel);
    static InterruptHandler* irq_handler(Dma channel);
    static void irq_enable(Dma channel, uint32_t priority = GlobalsDefs::DefPreemptPrio);
    static void enable_clock(Dma channel);
};


} // namespace eg {
    


