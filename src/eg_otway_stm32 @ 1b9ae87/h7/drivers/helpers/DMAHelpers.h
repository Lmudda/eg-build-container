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

// ******************
// **** WARNING! ****
// ******************
// The regular DMA (DMA1 and DMA2) on the STM32H7 cannot access the ITCM and DTCM internal RAM.
// STM32CubeMX uses these by default, so you will need to configure the linker to use another
// area of RAM if you want to use these DMA.
enum class Dma
{
    Dma1Stream0 = DMA1_Stream0_BASE,
    Dma1Stream1 = DMA1_Stream1_BASE,
    Dma1Stream2 = DMA1_Stream2_BASE,
    Dma1Stream3 = DMA1_Stream3_BASE,
    Dma1Stream4 = DMA1_Stream4_BASE,
    Dma1Stream5 = DMA1_Stream5_BASE,
    Dma1Stream6 = DMA1_Stream6_BASE,
    Dma1Stream7 = DMA1_Stream7_BASE,

    Dma2Stream0 = DMA2_Stream0_BASE,
    Dma2Stream1 = DMA2_Stream1_BASE,
    Dma2Stream2 = DMA2_Stream2_BASE,
    Dma2Stream3 = DMA2_Stream3_BASE,
    Dma2Stream4 = DMA2_Stream4_BASE,
    Dma2Stream5 = DMA2_Stream5_BASE,
    Dma2Stream6 = DMA2_Stream6_BASE,
    Dma2Stream7 = DMA2_Stream7_BASE,
};


struct DMAHelpers
{
    static DMA_Stream_TypeDef* instance(Dma stream);
    static InterruptHandler* irq_handler(Dma stream);
    static void irq_enable(Dma stream, uint32_t priority);
    static void enable_clock(Dma stream);
};


} // namespace eg {
    


