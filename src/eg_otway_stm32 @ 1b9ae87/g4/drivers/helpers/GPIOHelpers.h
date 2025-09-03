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


// Need some sort of platform/device specific includes to make this compile.
// The base addesses are in CMSIS rather than HAL code. 
enum class Port : uint32_t
{
    #if defined(GPIOA_BASE)
    PortA = GPIOA_BASE,
    #endif
    #if defined(GPIOB_BASE)
    PortB = GPIOB_BASE,
    #endif
    #if defined(GPIOC_BASE)
    PortC = GPIOC_BASE,
    #endif
    #if defined(GPIOD_BASE)
    PortD = GPIOD_BASE,
    #endif
    #if defined(GPIOE_BASE)
    PortE = GPIOE_BASE,
    #endif
    #if defined(GPIOF_BASE)
    PortF = GPIOF_BASE,
    #endif
    #if defined(GPIOG_BASE)
    PortG = GPIOG_BASE,
    #endif
    #if defined(GPIOH_BASE)
    PortH = GPIOH_BASE,
    #endif
    #if defined(GPIOI_BASE)
    PortI = GPIOI_BASE,
    #endif
    #if defined(GPIOJ_BASE)
    PortJ = GPIOJ_BASE,
    #endif
    #if defined(GPIOK_BASE)
    PortK = GPIOK_BASE,
    #endif
    #if defined(GPIOL_BASE)
    PortL = GPIOL_BASE,
    #endif
};


enum class Pin : uint8_t    
{ 
   Pin0, Pin1, Pin2,  Pin3,  Pin4,  Pin5,  Pin6,  Pin7, 
   Pin8, Pin9, Pin10, Pin11, Pin12, Pin13, Pin14, Pin15 
};    


enum class Mode : uint8_t   { Input, Output, AltFn, Analog };    
enum class OType : uint8_t  { PushPull, OpenDrain };
enum class OSpeed : uint8_t { Low, Medium, High, VeryHigh };
enum class PuPd : uint8_t   { NoPull, PullUp, PullDown };
// enum class AltFn : uint8_t   
// { 
//     // It would be nice to create names corresponding to the peripherals.
//     // HAL does this but prefer to keep HAL out of the API.
//     Fn0, Fn1, Fn2,  Fn3,  Fn4,  Fn5,  Fn6,  Fn7, 
//     Fn8, Fn9, Fn10, Fn11, Fn12, Fn13, Fn14, Fn15 
// };


// Used for interrupt priority for digital inputs.
enum class Priority : uint8_t 
{ 
    Prio0, Prio1, Prio2,  Prio3,  Prio4,  Prio5,  Prio6,  Prio7, 
    Prio8, Prio9, Prio10, Prio11, Prio12, Prio13, Prio14, Prio15 
};


struct GPIOHelpers
{
    static IRQn_Type irq_number(Pin pin);
    static InterruptHandler* irq_handler(Pin pin);
    static void irq_enable(Pin pin, uint32_t priority);
    static void enable_clock(Port port);

    // NOTE: Some balance here between sensible defaults but realistic options to override.
    // Or pass a struct with all the settings but with defaults most of the time.
    static void configure_as_input(Port port, Pin pin, PuPd pupd, OSpeed ospeed = OSpeed::Low);
    static void configure_as_exti_input(Port port, Pin pin, PuPd pupd, OSpeed ospeed = OSpeed::Low);
    static void configure_as_output(Port port, Pin pin, OType otype, PuPd pupd, OSpeed ospeed = OSpeed::Low);
    static void configure_as_analogue(Port port, Pin pin);
    static void configure_as_alternate(Port port, Pin pin, uint8_t alt_fn, OType otype = OType::PushPull);
};


} // namespace eg {
