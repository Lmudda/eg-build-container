/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "HSEMHelpers.h"
#include "utilities/ErrorHandler.h"
#include "GlobalDefs.h"
#include "logging/Assert.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_hsem.h"


namespace eg {


void HSEMHelpers::assert_id(uint8_t sem_id)
{
    EG_ASSERT(sem_id < HSEM_SEMID_MAX, "Invalid semaphore ID");
}


void HSEMHelpers::configure()
{
    // Enable clock
    __HAL_RCC_HSEM_CLK_ENABLE();
}


IRQn_Type HSEMHelpers::irq_number(Core core)
{
    switch (core)
    {
        case Core::CortexM7:  return HSEM1_IRQn;
        case Core::CortexM4:  return HSEM2_IRQn;
    }        

    Error_Handler();
    return HSEM1_IRQn;
}


void HSEMHelpers::irq_enable(Core core, uint32_t priority)
{
    IRQn_Type irqn = irq_number(core);
    HAL_NVIC_SetPriority(irqn, priority, GlobalsDefs::DefSubPrio);
    HAL_NVIC_EnableIRQ(irqn); 
}


// The real signal from which a proxy is returned. See HSEMHelpers::irq_handler().
static Signal<uint32_t>& private_irq_handler()
{
    static Signal<uint32_t> signal;
    return signal;
}


// Use a single signal rather than a 32 separate callbacks as it's far more likely that
// there will only be a small number of semaphores used. This saves on RAM usage in the
// typical case. If lots of semaphore IDs are used, then each registered listener will
// receive multiple redundant calls indicating that other semaphore IDs have been
// released, but these can just be ignored.
SignalProxy<uint32_t> HSEMHelpers::irq_handler()
{
    static SignalProxy<uint32_t> proxy{private_irq_handler()};
    return proxy;
}


// This overrides a weakly defined function in the HAL.
extern "C" void HAL_HSEM_FreeCallback(uint32_t mask)
{
    // Call directly so that this happens under interrupt context.
    private_irq_handler().call(mask);
}


extern "C" void HSEM1_IRQHandler(void)
{
    // Let the ST HAL handle it.
    // This will reset the interrupt flag and call HAL_HSEM_FreeCallback.
    HAL_HSEM_IRQHandler();
}


extern "C" void HSEM2_IRQHandler(void)
{
    // Let the ST HAL handle it.
    // This will reset the interrupt flag and call HAL_HSEM_FreeCallback.
    HAL_HSEM_IRQHandler();
}


} // namespace eg {
