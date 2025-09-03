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


enum class Uart
{
    #if defined(USART2_BASE)
    Usart2 = USART2_BASE,
    #endif
    #if defined(USART3_BASE)
    Usart3 = USART3_BASE,
    #endif
};


struct UARTHelpers
{
    static USART_TypeDef* instance(Uart uart);
    static InterruptHandler* irq_handler(Uart uart);
    static void irq_enable(Uart uart);
    static void enable_clock(Uart uart);
};


} // namespace eg {
    


