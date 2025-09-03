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
#include "stm32g4xx.h"


namespace eg {


enum class Dma
{
    Dma1Channel1 = DMA1_Channel1_BASE,
    Dma1Channel2 = DMA1_Channel2_BASE,
    Dma1Channel3 = DMA1_Channel3_BASE,
    Dma1Channel4 = DMA1_Channel4_BASE,
    Dma1Channel5 = DMA1_Channel5_BASE,
    Dma1Channel6 = DMA1_Channel6_BASE,
    Dma1Channel7 = DMA1_Channel7_BASE,
    Dma1Channel8 = DMA1_Channel8_BASE,

    Dma2Channel1 = DMA2_Channel1_BASE,
    Dma2Channel2 = DMA2_Channel2_BASE,
    Dma2Channel3 = DMA2_Channel3_BASE,
    Dma2Channel4 = DMA2_Channel4_BASE,
    Dma2Channel5 = DMA2_Channel5_BASE,
    Dma2Channel6 = DMA2_Channel6_BASE,
    Dma2Channel7 = DMA2_Channel7_BASE,
    Dma2Channel8 = DMA2_Channel8_BASE,
};


struct DMAHelpers
{
    static DMA_Channel_TypeDef* instance(Dma channel);
    static InterruptHandler* irq_handler(Dma channel);
    static void irq_enable(Dma channel, uint32_t priority);
    static void enable_clock(Dma channel);
};


} // namespace eg {
    


