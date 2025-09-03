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
#include "stm32g4xx_hal.h"


namespace eg {


static InterruptHandler g_dma_channels_irq[16];


DMA_Channel_TypeDef* DMAHelpers::instance(Dma channel)
{
    return reinterpret_cast<DMA_Channel_TypeDef*>(channel);
}


// NOTE: DMA channel automatic allocation. This is possible for G4 because the 
// channels can all be used for any DMA request thanks to the mux.
// Dma DMAHelpers::allocate_channel()
// {
//     static uint16_t s_used_dma_channels = 0;

//     uint8_t index = 0;
//     while (index < 16)
//     {
//         uint16_t mask = 1U << index;
//         if ((s_used_dma_channels & mask) == 0)
//         {
//             s_used_dma_channels |= mask;
//             break;
//         }
//         ++index;
//     }

//     if (index >= 16)
//     {
//         // We have run out of channels.
//         Error_Handler();
//     }

//     switch (index)
//     {
//         case  0: return Dma::Dma1Channel1;
//         case  1: return Dma::Dma1Channel2;
//         case  2: return Dma::Dma1Channel3;
//         case  3: return Dma::Dma1Channel4;
//         case  4: return Dma::Dma1Channel5;
//         case  5: return Dma::Dma1Channel6;
//         case  6: return Dma::Dma1Channel7;
//         case  7: return Dma::Dma1Channel8;

//         case  8: return Dma::Dma2Channel1;
//         case  9: return Dma::Dma2Channel2;
//         case 10: return Dma::Dma2Channel3;
//         case 11: return Dma::Dma2Channel4;
//         case 12: return Dma::Dma2Channel5;
//         case 13: return Dma::Dma2Channel6;
//         case 14: return Dma::Dma2Channel7;
//         case 15: return Dma::Dma2Channel8;

//         default: Error_Handler();
//     }
// }


InterruptHandler* DMAHelpers::irq_handler(Dma channel)
{
    uint8_t index = 0;

    switch (channel)
    {
        case Dma::Dma1Channel1: index =  0; break;
        case Dma::Dma1Channel2: index =  1; break;
        case Dma::Dma1Channel3: index =  2; break;
        case Dma::Dma1Channel4: index =  3; break;
        case Dma::Dma1Channel5: index =  4; break;
        case Dma::Dma1Channel6: index =  5; break;
        case Dma::Dma1Channel7: index =  6; break;
        case Dma::Dma1Channel8: index =  7; break;

        case Dma::Dma2Channel1: index =  8; break;
        case Dma::Dma2Channel2: index =  9; break;
        case Dma::Dma2Channel3: index = 10; break;
        case Dma::Dma2Channel4: index = 11; break;
        case Dma::Dma2Channel5: index = 12; break;
        case Dma::Dma2Channel6: index = 13; break;
        case Dma::Dma2Channel7: index = 14; break;
        case Dma::Dma2Channel8: index = 15; break;

        default: Error_Handler();
    }

    return &g_dma_channels_irq[index];
}


void DMAHelpers::irq_enable(Dma channel, uint32_t priority)
{
    IRQn_Type irqn;
    switch (channel)
    {
        case Dma::Dma1Channel1: irqn = DMA1_Channel1_IRQn; break;
        case Dma::Dma1Channel2: irqn = DMA1_Channel2_IRQn; break;
        case Dma::Dma1Channel3: irqn = DMA1_Channel3_IRQn; break;
        case Dma::Dma1Channel4: irqn = DMA1_Channel4_IRQn; break;
        case Dma::Dma1Channel5: irqn = DMA1_Channel5_IRQn; break;
        case Dma::Dma1Channel6: irqn = DMA1_Channel6_IRQn; break;
        case Dma::Dma1Channel7: irqn = DMA1_Channel7_IRQn; break;
        case Dma::Dma1Channel8: irqn = DMA1_Channel8_IRQn; break;

        case Dma::Dma2Channel1: irqn = DMA2_Channel1_IRQn; break;
        case Dma::Dma2Channel2: irqn = DMA2_Channel2_IRQn; break;
        case Dma::Dma2Channel3: irqn = DMA2_Channel3_IRQn; break;
        case Dma::Dma2Channel4: irqn = DMA2_Channel4_IRQn; break;
        case Dma::Dma2Channel5: irqn = DMA2_Channel5_IRQn; break;
        case Dma::Dma2Channel6: irqn = DMA2_Channel6_IRQn; break;
        case Dma::Dma2Channel7: irqn = DMA2_Channel7_IRQn; break;
        case Dma::Dma2Channel8: irqn = DMA2_Channel8_IRQn; break;

        default: 
            Error_Handler();
            return;
    }

    HAL_NVIC_SetPriority(irqn, priority, GlobalsDefs::DefSubPrio);
    HAL_NVIC_EnableIRQ(irqn);    
}


void DMAHelpers::enable_clock(Dma channel)
{
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    switch (channel)
    {
        case Dma::Dma1Channel1:
        case Dma::Dma1Channel2:
        case Dma::Dma1Channel3:
        case Dma::Dma1Channel4:
        case Dma::Dma1Channel5:
        case Dma::Dma1Channel6:
        case Dma::Dma1Channel7:
        case Dma::Dma1Channel8:
            __HAL_RCC_DMA1_CLK_ENABLE();
            break;

        case Dma::Dma2Channel1:
        case Dma::Dma2Channel2:
        case Dma::Dma2Channel3:
        case Dma::Dma2Channel4:
        case Dma::Dma2Channel5:
        case Dma::Dma2Channel6:
        case Dma::Dma2Channel7:
        case Dma::Dma2Channel8:
            __HAL_RCC_DMA2_CLK_ENABLE();
            break;

        default: Error_Handler();
    }
}


extern "C" void DMA1_Channel1_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel1)->call();
}


extern "C" void DMA1_Channel2_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel2)->call();
}


extern "C" void DMA1_Channel3_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel3)->call();
}


extern "C" void DMA1_Channel4_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel4)->call();
}


extern "C" void DMA1_Channel5_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel5)->call();
}


extern "C" void DMA1_Channel6_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel6)->call();
}


extern "C" void DMA1_Channel7_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel7)->call();
}


extern "C" void DMA1_Channel8_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma1Channel8)->call();
}


extern "C" void DMA2_Channel1_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma2Channel1)->call();
}


extern "C" void DMA2_Channel2_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma2Channel2)->call();
}


extern "C" void DMA2_Channel3_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma2Channel3)->call();
}


extern "C" void DMA2_Channel4_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma2Channel4)->call();
}


extern "C" void DMA2_Channel5_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma2Channel5)->call();
}


extern "C" void DMA2_Channel6_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma2Channel6)->call();
}


extern "C" void DMA2_Channel7_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma2Channel7)->call();
}


extern "C" void DMA2_Channel8_IRQHandler()
{
    DMAHelpers::irq_handler(Dma::Dma2Channel8)->call();
}


} // namespace eg {


