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
#include "stm32u5xx_hal.h"


namespace eg {


static InterruptHandler g_dma_channels_irq[16];


DMA_Channel_TypeDef* DMAHelpers::instance(Dma channel)
{
    return reinterpret_cast<DMA_Channel_TypeDef*>(channel);
}


InterruptHandler* DMAHelpers::irq_handler(Dma channel)
{
    uint8_t index;

    switch (channel)
    {
        case Dma::Dma1Channel0:  index =  0; break;
        case Dma::Dma1Channel1:  index =  1; break;
        case Dma::Dma1Channel2:  index =  2; break;
        case Dma::Dma1Channel3:  index =  3; break;
        case Dma::Dma1Channel4:  index =  4; break;
        case Dma::Dma1Channel5:  index =  5; break;
        case Dma::Dma1Channel6:  index =  6; break;
        case Dma::Dma1Channel7:  index =  7; break;
        case Dma::Dma1Channel8:  index =  8; break;
        case Dma::Dma1Channel9:  index =  9; break;
        case Dma::Dma1Channel10: index = 10; break;
        case Dma::Dma1Channel11: index = 11; break;
        case Dma::Dma1Channel12: index = 12; break;
        case Dma::Dma1Channel13: index = 13; break;
        case Dma::Dma1Channel14: index = 14; break;
        case Dma::Dma1Channel15: index = 15; break;
        default: Error_Handler();
    }

    return &g_dma_channels_irq[index];
}


void DMAHelpers::irq_enable(Dma channel, uint32_t priority)
{
    IRQn_Type irqn;
    switch (channel)
    {
        case Dma::Dma1Channel0:  irqn = GPDMA1_Channel0_IRQn; break;
        case Dma::Dma1Channel1:  irqn = GPDMA1_Channel1_IRQn; break;
        case Dma::Dma1Channel2:  irqn = GPDMA1_Channel2_IRQn; break;
        case Dma::Dma1Channel3:  irqn = GPDMA1_Channel3_IRQn; break;
        case Dma::Dma1Channel4:  irqn = GPDMA1_Channel4_IRQn; break;
        case Dma::Dma1Channel5:  irqn = GPDMA1_Channel5_IRQn; break;
        case Dma::Dma1Channel6:  irqn = GPDMA1_Channel6_IRQn; break;
        case Dma::Dma1Channel7:  irqn = GPDMA1_Channel7_IRQn; break;
        case Dma::Dma1Channel8:  irqn = GPDMA1_Channel8_IRQn; break;
        case Dma::Dma1Channel9:  irqn = GPDMA1_Channel9_IRQn; break;
        case Dma::Dma1Channel10: irqn = GPDMA1_Channel10_IRQn; break;
        case Dma::Dma1Channel11: irqn = GPDMA1_Channel11_IRQn; break;
        case Dma::Dma1Channel12: irqn = GPDMA1_Channel12_IRQn; break;
        case Dma::Dma1Channel13: irqn = GPDMA1_Channel13_IRQn; break;
        case Dma::Dma1Channel14: irqn = GPDMA1_Channel14_IRQn; break;
        case Dma::Dma1Channel15: irqn = GPDMA1_Channel15_IRQn; break;
        default: Error_Handler();
    }

    HAL_NVIC_SetPriority(irqn, priority, GlobalsDefs::DefSubPrio);
    HAL_NVIC_EnableIRQ(irqn);    
}


void DMAHelpers::enable_clock(Dma channel)
{
    switch (channel)
    {
        case Dma::Dma1Channel0:
        case Dma::Dma1Channel1:
        case Dma::Dma1Channel2:
        case Dma::Dma1Channel3:
        case Dma::Dma1Channel4:
        case Dma::Dma1Channel5:
        case Dma::Dma1Channel6:
        case Dma::Dma1Channel7:
        case Dma::Dma1Channel8:
        case Dma::Dma1Channel9:
        case Dma::Dma1Channel10:
        case Dma::Dma1Channel11:
        case Dma::Dma1Channel12:
        case Dma::Dma1Channel13:
        case Dma::Dma1Channel14:
        case Dma::Dma1Channel15:
            __HAL_RCC_GPDMA1_CLK_ENABLE();
            break;

        default: Error_Handler();
    }
}

extern "C" void GPDMA1_Channel0_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel0)->call();
}

extern "C" void GPDMA1_Channel1_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel1)->call();
}

extern "C" void GPDMA1_Channel2_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel2)->call();
}

extern "C" void GPDMA1_Channel3_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel3)->call();
}

extern "C" void GPDMA1_Channel4_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel4)->call();
}

extern "C" void GPDMA1_Channel5_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel5)->call();
}

extern "C" void GPDMA1_Channel6_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel6)->call();
}

extern "C" void GPDMA1_Channel7_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel7)->call();
}

extern "C" void GPDMA1_Channel8_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel8)->call();
}

extern "C" void GPDMA1_Channel9_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel9)->call();
}

extern "C" void GPDMA1_Channel10_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel10)->call();
}

extern "C" void GPDMA1_Channel11_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel11)->call();
}

extern "C" void GPDMA1_Channel12_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel12)->call();
}

extern "C" void GPDMA1_Channel13_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel13)->call();
}

extern "C" void GPDMA1_Channel14_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel14)->call();
}

extern "C" void GPDMA1_Channel15_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel15)->call();
}

} // namespace eg {


