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
    #if defined(SPI4_BASE)
    Spi4 = SPI4_BASE,
    #endif
    #if defined(SPI5_BASE)
    Spi5 = SPI5_BASE,
    #endif
    #if defined(SPI6_BASE)
    Spi6 = SPI6_BASE,
    #endif
};


struct SPIHelpers
{
    static InterruptHandler* irq_handler(Spi spi);
    static void irq_enable(Spi spi, uint32_t priority);

    // Note that the caller is responsible for setting up any clock sources (e.g. PLLs).
    // This is different to the ST HAL and has been done like this to avoid having to called
    // the huge HAL_RCCEx_PeriphCLKConfig function (and avoids having to do things like
    // pass in the PLL config to this function).
    static void enable_clock(Spi spi, uint32_t clk_src);
};


} // namespace eg {
    