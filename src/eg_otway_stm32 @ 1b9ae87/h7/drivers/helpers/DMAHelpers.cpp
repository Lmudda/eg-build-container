/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "DMAHelpers.h"
#include "utilities/ErrorHandler.h"
#include "stm32h7xx_hal.h"


namespace eg {


static InterruptHandler g_dma_streams_irq[16];


DMA_Stream_TypeDef* DMAHelpers::instance(Dma stream)
{
    return reinterpret_cast<DMA_Stream_TypeDef*>(stream);
}


InterruptHandler* DMAHelpers::irq_handler(Dma stream)
{
    uint8_t index = 0;

    switch (stream)
    {
        case Dma::Dma1Stream0: index =  0; break;
        case Dma::Dma1Stream1: index =  1; break;
        case Dma::Dma1Stream2: index =  2; break;
        case Dma::Dma1Stream3: index =  3; break;
        case Dma::Dma1Stream4: index =  4; break;
        case Dma::Dma1Stream5: index =  5; break;
        case Dma::Dma1Stream6: index =  6; break;
        case Dma::Dma1Stream7: index =  7; break;

        case Dma::Dma2Stream0: index =  8; break;
        case Dma::Dma2Stream1: index =  9; break;
        case Dma::Dma2Stream2: index = 10; break;
        case Dma::Dma2Stream3: index = 11; break;
        case Dma::Dma2Stream4: index = 12; break;
        case Dma::Dma2Stream5: index = 13; break;
        case Dma::Dma2Stream6: index = 14; break;
        case Dma::Dma2Stream7: index = 15; break;

        default: Error_Handler();
    }

    return &g_dma_streams_irq[index];
}


void DMAHelpers::irq_enable(Dma stream, uint32_t priority)
{
    IRQn_Type irqn;
    switch (stream)
    {
        case Dma::Dma1Stream0: irqn = DMA1_Stream0_IRQn; break;
        case Dma::Dma1Stream1: irqn = DMA1_Stream1_IRQn; break;
        case Dma::Dma1Stream2: irqn = DMA1_Stream2_IRQn; break;
        case Dma::Dma1Stream3: irqn = DMA1_Stream3_IRQn; break;
        case Dma::Dma1Stream4: irqn = DMA1_Stream4_IRQn; break;
        case Dma::Dma1Stream5: irqn = DMA1_Stream5_IRQn; break;
        case Dma::Dma1Stream6: irqn = DMA1_Stream6_IRQn; break;
        case Dma::Dma1Stream7: irqn = DMA1_Stream7_IRQn; break;

        case Dma::Dma2Stream0: irqn = DMA2_Stream0_IRQn; break;
        case Dma::Dma2Stream1: irqn = DMA2_Stream1_IRQn; break;
        case Dma::Dma2Stream2: irqn = DMA2_Stream2_IRQn; break;
        case Dma::Dma2Stream3: irqn = DMA2_Stream3_IRQn; break;
        case Dma::Dma2Stream4: irqn = DMA2_Stream4_IRQn; break;
        case Dma::Dma2Stream5: irqn = DMA2_Stream5_IRQn; break;
        case Dma::Dma2Stream6: irqn = DMA2_Stream6_IRQn; break;
        case Dma::Dma2Stream7: irqn = DMA2_Stream7_IRQn; break;

        default:
	        Error_Handler();
	        return;
    }

    HAL_NVIC_SetPriority(irqn, priority, GlobalsDefs::DefSubPrio);
    HAL_NVIC_EnableIRQ(irqn);    
}


void DMAHelpers::enable_clock(Dma stream)
{
    switch (stream)
    {
        case Dma::Dma1Stream0:
        case Dma::Dma1Stream1:
        case Dma::Dma1Stream2:
        case Dma::Dma1Stream3:
        case Dma::Dma1Stream4:
        case Dma::Dma1Stream5:
        case Dma::Dma1Stream6:
        case Dma::Dma1Stream7:
            __HAL_RCC_DMA1_CLK_ENABLE();
            break;

        case Dma::Dma2Stream0:
        case Dma::Dma2Stream1:
        case Dma::Dma2Stream2:
        case Dma::Dma2Stream3:
        case Dma::Dma2Stream4:
        case Dma::Dma2Stream5:
        case Dma::Dma2Stream6:
        case Dma::Dma2Stream7:
            __HAL_RCC_DMA2_CLK_ENABLE();
            break;

        default: Error_Handler();
    }
}


extern "C" void DMA1_Stream0_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Stream0)->call();
}


extern "C" void DMA1_Stream1_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Stream1)->call();
}


extern "C" void DMA1_Stream2_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Stream2)->call();
}


extern "C" void DMA1_Stream3_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Stream3)->call();
}


extern "C" void DMA1_Stream4_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Stream4)->call();
}


extern "C" void DMA1_Stream5_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Stream5)->call();
}


extern "C" void DMA1_Stream6_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Stream6)->call();
}


extern "C" void DMA1_Stream7_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Stream7)->call();
}


extern "C" void DMA2_Stream0_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma2Stream0)->call();
}


extern "C" void DMA2_Stream1_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma2Stream1)->call();
}


extern "C" void DMA2_Stream2_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma2Stream2)->call();
}


extern "C" void DMA2_Stream3_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma2Stream3)->call();
}


extern "C" void DMA2_Stream4_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma2Stream4)->call();
}


extern "C" void DMA2_Stream5_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma2Stream5)->call();
}


extern "C" void DMA2_Stream6_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma2Stream6)->call();
}


extern "C" void DMA2_Stream7_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma2Stream7)->call();
}


} // namespace eg {
