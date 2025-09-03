/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "SPIHelpers.h"
#include "utilities/ErrorHandler.h"
#include "stm32g4xx_hal.h"


namespace eg {


#if defined(SPI1_BASE)
static InterruptHandler g_spi1_irq;
#endif
#if defined(SPI2_BASE)
static InterruptHandler g_spi2_irq;
#endif
#if defined(SPI3_BASE)
static InterruptHandler g_spi3_irq;
#endif


InterruptHandler* SPIHelpers::irq_handler(Spi spi)
{
    switch (spi)
    {
        #if defined(SPI1_BASE)
        case Spi::Spi1: return &g_spi1_irq;
        #endif
        #if defined(SPI2_BASE)
        case Spi::Spi2: return &g_spi2_irq;
        #endif
        #if defined(SPI3_BASE)
        case Spi::Spi3: return &g_spi3_irq;
        #endif

        default: Error_Handler();
    }

    return nullptr;
}


void SPIHelpers::irq_enable(Spi spi, uint32_t priority)
{
    IRQn_Type irqn;

    switch (spi)
    {
        #if defined(SPI1_BASE)
        case Spi::Spi1: irqn = SPI1_IRQn; break;
        #endif
        #if defined(SPI2_BASE)
        case Spi::Spi2: irqn = SPI2_IRQn; break;
        #endif
        #if defined(SPI3_BASE)
        case Spi::Spi3: irqn = SPI3_IRQn; break;
        #endif

        default: 
            Error_Handler();
            return;
    }    

    HAL_NVIC_SetPriority(irqn, priority, GlobalsDefs::DefSubPrio);
    HAL_NVIC_EnableIRQ(irqn);    
}


void SPIHelpers::enable_clock(Spi spi)
{
    switch (spi)
    {
        // These constants are defined in the CMSIS code rather than HAL.
        #if defined(SPI1_BASE)
        case Spi::Spi1: 
            __HAL_RCC_SPI1_CLK_ENABLE();
            break;
        #endif
        #if defined(SPI2_BASE)
        case Spi::Spi2: 
            __HAL_RCC_SPI2_CLK_ENABLE();
            break;
        #endif
        #if defined(SPI3_BASE)
        case Spi::Spi3: 
            __HAL_RCC_SPI3_CLK_ENABLE();
            break;
        #endif

        default: Error_Handler();
    }
}


#if defined(SPI1_BASE)
extern "C" void SPI1_IRQHandler()
{
    g_spi1_irq.call();
}
#endif


#if defined(SPI2_BASE)
extern "C" void SPI2_IRQHandler()
{
    g_spi2_irq.call();
}
#endif


#if defined(SPI3_BASE)
extern "C" void SPI3_IRQHandler()
{
    g_spi3_irq.call();
}
#endif


} // namespace eg {


