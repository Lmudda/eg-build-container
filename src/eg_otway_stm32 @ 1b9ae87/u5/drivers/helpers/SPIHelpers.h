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
#include "stm32u5xx.h"


namespace eg {


enum class Spi
{
    #if defined(SPI1_BASE)
    Spi1 = SPI1_BASE,
    #endif
    #if defined(SPI2_BASE)
    Spi2 = SPI2_BASE,
    #endif
    #if defined(SPI3_BASE)
    Spi3 = SPI3_BASE,
    #endif
};


struct SPIHelpers
{
    static InterruptHandler* irq_handler(Spi spi);
    static void enable_clock(Spi spi);
    static void irq_enable(Spi channel, uint32_t priority = GlobalsDefs::DefPreemptPrio);
};



} // namespace eg {
    


