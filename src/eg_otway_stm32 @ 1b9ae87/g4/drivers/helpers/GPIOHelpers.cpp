/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "GPIOHelpers.h"
#include "utilities/ErrorHandler.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_gpio.h"


namespace eg {


// Note that this implementation means we support all 16 EXTI lines whether they are used 
// or not in the application. This consumes a little RAM for the array of InterruptHandlers.
// If necessary (unlikely) we could tailor the supported lines with #defines determined by the 
// application's CMakeLists.txt, or something like that.
static InterruptHandler g_exti_handlers[16];


#if defined(GPIOA_BASE)
uint16_t g_gpioa_pins_used;
#endif
#if defined(GPIOB_BASE)
uint16_t g_gpiob_pins_used;
#endif
#if defined(GPIOC_BASE)
uint16_t g_gpioc_pins_used;
#endif
#if defined(GPIOD_BASE)
uint16_t g_gpiod_pins_used;
#endif
#if defined(GPIOE_BASE)
uint16_t g_gpioe_pins_used;
#endif
#if defined(GPIOF_BASE)
uint16_t g_gpiof_pins_used;
#endif
#if defined(GPIOG_BASE)
uint16_t g_gpiog_pins_used;
#endif
#if defined(GPIOH_BASE)
uint16_t g_gpioh_pins_used;
#endif
#if defined(GPIOI_BASE)
uint16_t g_gpioi_pins_used;
#endif
#if defined(GPIOJ_BASE)
uint16_t g_gpioj_pins_used;
#endif
#if defined(GPIOK_BASE)
uint16_t g_gpiok_pins_used;
#endif
#if defined(GPIOL_BASE)
uint16_t g_gpiol_pins_used;
#endif


// This is used to track which pins are currently in use to avoid using 
// them in multiple driver instances.
static void allocate_pin(Port port, Pin pin)
{
    uint16_t* gpio_pins_used{};
    switch (port)
    {
        // These constants are defined in the CMSIS code rather than HAL.
        #if defined(GPIOA_BASE)
        case Port::PortA: gpio_pins_used = &g_gpioa_pins_used; break; 
        #endif
        #if defined(GPIOB_BASE)
        case Port::PortB: gpio_pins_used = &g_gpiob_pins_used; break; 
        #endif
        #if defined(GPIOC_BASE)
        case Port::PortC: gpio_pins_used = &g_gpioc_pins_used; break; 
        #endif
        #if defined(GPIOD_BASE)
        case Port::PortD: gpio_pins_used = &g_gpiod_pins_used; break; 
        #endif
        #if defined(GPIOE_BASE)
        case Port::PortE: gpio_pins_used = &g_gpioe_pins_used; break; 
        #endif
        #if defined(GPIOF_BASE)
        case Port::PortF: gpio_pins_used = &g_gpiof_pins_used; break; 
        #endif
        #if defined(GPIOG_BASE)
        case Port::PortG: gpio_pins_used = &g_gpiog_pins_used; break; 
        #endif
        #if defined(GPIOH_BASE)
        case Port::PortH: gpio_pins_used = &g_gpioh_pins_used; break; 
        #endif
        #if defined(GPIOI_BASE)
        case Port::PortI: gpio_pins_used = &g_gpioi_pins_used; break; 
        #endif
        #if defined(GPIOJ_BASE)
        case Port::PortJ: gpio_pins_used = &g_gpioj_pins_used; break; 
        #endif
        #if defined(GPIOK_BASE)
        case Port::PortK: gpio_pins_used = &g_gpiok_pins_used; break; 
        #endif
        #if defined(GPIOL_BASE)
        case Port::PortL: gpio_pins_used = &g_gpiol_pins_used; break; 
        #endif
        default: Error_Handler();
    }

    uint16_t& used = *gpio_pins_used;
    uint16_t  mask = 1U << static_cast<uint8_t>(pin);
    if ((used & mask) != 0)
    {
        // This pin was already used elsewhere.
        Error_Handler();
    }

    used |= mask;
}


uint16_t g_exti_pins_used;
static void allocate_exti(Pin pin)
{
    uint16_t mask = 1U << static_cast<uint8_t>(pin);
    if ((g_exti_pins_used & mask) != 0)
    {
        // This EXTI line was already used elsewhere.
        Error_Handler();
    }
    g_exti_pins_used |= mask; 
}


InterruptHandler* GPIOHelpers::irq_handler(Pin pin)
{
    return &g_exti_handlers[static_cast<uint8_t>(pin)];
}


IRQn_Type GPIOHelpers::irq_number(Pin pin)
{
    switch (pin)
    {
        case Pin::Pin0:  return EXTI0_IRQn;
        case Pin::Pin1:  return EXTI1_IRQn;
        case Pin::Pin2:  return EXTI2_IRQn;
        case Pin::Pin3:  return EXTI3_IRQn;
        case Pin::Pin4:  return EXTI4_IRQn;
        case Pin::Pin5:  
        case Pin::Pin6:  
        case Pin::Pin7:  
        case Pin::Pin8:  
        case Pin::Pin9:  return EXTI9_5_IRQn;
        case Pin::Pin10: 
        case Pin::Pin11: 
        case Pin::Pin12: 
        case Pin::Pin13: 
        case Pin::Pin14: 
        case Pin::Pin15: return EXTI15_10_IRQn;
    }        

    Error_Handler();
    return EXTI0_IRQn;
}


void GPIOHelpers::irq_enable(Pin pin, uint32_t priority)
{
    IRQn_Type irqn = irq_number(pin);
    HAL_NVIC_SetPriority(irqn, priority, GlobalsDefs::DefSubPrio);
    HAL_NVIC_EnableIRQ(irqn);    
}


// These blasted HAL macros for enabling peripherals are a poor
// design when you need generic code. Here we switch on the peripheral
// address. 
void GPIOHelpers::enable_clock(Port port)
{
    switch (port)
    {
        // These constants are defined in the CMSIS code rather than HAL.
        #if defined(GPIOA_BASE)
        case Port::PortA: 
            __HAL_RCC_GPIOA_CLK_ENABLE();
            break;
        #endif
        #if defined(GPIOB_BASE)
        case Port::PortB: 
            __HAL_RCC_GPIOB_CLK_ENABLE();
            break;
        #endif
        #if defined(GPIOC_BASE)
        case Port::PortC: 
            __HAL_RCC_GPIOC_CLK_ENABLE();
            break;
        #endif
        #if defined(GPIOD_BASE)
        case Port::PortD: 
            __HAL_RCC_GPIOD_CLK_ENABLE();
            break;
        #endif
        #if defined(GPIOE_BASE)
        case Port::PortE: 
            __HAL_RCC_GPIOE_CLK_ENABLE();
            break;
        #endif
        #if defined(GPIOF_BASE)
        case Port::PortF: 
            __HAL_RCC_GPIOF_CLK_ENABLE();
            break;
        #endif
        #if defined(GPIOG_BASE)
        case Port::PortG: 
            __HAL_RCC_GPIOG_CLK_ENABLE();
            break;
        #endif
        #if defined(GPIOH_BASE)
        case Port::PortH: 
            __HAL_RCC_GPIOH_CLK_ENABLE();
            break;
        #endif
        #if defined(GPIOI_BASE)
        case Port::PortI: 
            __HAL_RCC_GPIOI_CLK_ENABLE();
            break;
        #endif
        #if defined(GPIOJ_BASE)
        case Port::PortJ: 
            __HAL_RCC_GPIOJ_CLK_ENABLE();
            break;
        #endif
        #if defined(GPIOK_BASE)
        case Port::PortK: 
            __HAL_RCC_GPIOK_CLK_ENABLE();
            break;
        #endif
        #if defined(GPIOL_BASE)
        case Port::PortL: 
            __HAL_RCC_GPIOL_CLK_ENABLE();
            break;
        #endif

        default: Error_Handler();
    }
}


static void configure_pupd_ospeed(GPIO_InitTypeDef& init, PuPd pupd, OSpeed ospeed)
{
    switch (pupd)
    {
        case PuPd::PullUp:   init.Pull = GPIO_PULLUP;   break;
        case PuPd::PullDown: init.Pull = GPIO_PULLDOWN; break;
        case PuPd::NoPull: 
        default:             init.Pull = GPIO_NOPULL;
    }

    switch (ospeed)
    {
        case OSpeed::Low:       init.Speed = GPIO_SPEED_FREQ_LOW;    break;
        case OSpeed::Medium:    init.Speed = GPIO_SPEED_FREQ_MEDIUM; break;
        case OSpeed::High:      init.Speed = GPIO_SPEED_FREQ_HIGH;   break;
        case OSpeed::VeryHigh: 
        default:                init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    }
}


void GPIOHelpers::configure_as_input(Port port, Pin pin, PuPd pupd, OSpeed ospeed)
{
    allocate_pin(port, pin);
    enable_clock(port);

    GPIO_InitTypeDef init {};
    init.Pin   = 1U << static_cast<uint8_t>(pin);
    init.Mode  = GPIO_MODE_INPUT;
    configure_pupd_ospeed(init, pupd, ospeed);
    HAL_GPIO_Init(reinterpret_cast<GPIO_TypeDef*>(port), &init);
}


void GPIOHelpers::configure_as_exti_input(Port port, Pin pin, PuPd pupd, OSpeed ospeed)
{
    allocate_pin(port, pin);
    allocate_exti(pin);
    enable_clock(port);

    GPIO_InitTypeDef init{};
    init.Pin   = 1U << static_cast<uint8_t>(pin);
    init.Mode  = GPIO_MODE_IT_RISING_FALLING;
    configure_pupd_ospeed(init, pupd, ospeed);
    HAL_GPIO_Init(reinterpret_cast<GPIO_TypeDef*>(port), &init);
}


void GPIOHelpers::configure_as_output(Port port, Pin pin, OType otype, PuPd pupd, OSpeed ospeed)
{    
    allocate_pin(port, pin);
    enable_clock(port);

    GPIO_InitTypeDef init{};
    init.Pin   = 1U << static_cast<uint8_t>(pin);
    init.Mode  = (otype == OType::PushPull) ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_OUTPUT_OD;
    configure_pupd_ospeed(init, pupd, ospeed);
    HAL_GPIO_Init(reinterpret_cast<GPIO_TypeDef*>(port), &init);
}


void GPIOHelpers::configure_as_analogue(Port port, Pin pin)
{
    allocate_pin(port, pin);
    enable_clock(port);

    GPIO_InitTypeDef init{};
    init.Pin       = 1U << static_cast<uint8_t>(pin);
    init.Mode      = GPIO_MODE_ANALOG;
    init.Pull      = GPIO_NOPULL;
    init.Speed     = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(reinterpret_cast<GPIO_TypeDef*>(port), &init);
}


void GPIOHelpers::configure_as_alternate(Port port, Pin pin, uint8_t alt_fn, OType otype)
{
    allocate_pin(port, pin);
    enable_clock(port);

    GPIO_InitTypeDef init{};
    init.Pin       = 1U << static_cast<uint8_t>(pin);
    init.Mode      = (otype == OType::PushPull) ? GPIO_MODE_AF_PP : GPIO_MODE_AF_OD;
    init.Pull      = GPIO_NOPULL;
    init.Speed     = GPIO_SPEED_FREQ_LOW;
    init.Alternate = alt_fn;

    HAL_GPIO_Init(reinterpret_cast<GPIO_TypeDef*>(port), &init);
}


// This overrides a weakly defined function in the HAL. 
extern "C" void HAL_GPIO_EXTI_Callback(uint16_t mask)
{
    Pin pin{};

    switch (mask)
    {
        case GPIO_PIN_0:   pin = Pin::Pin0;  break;
        case GPIO_PIN_1:   pin = Pin::Pin1;  break;
        case GPIO_PIN_2:   pin = Pin::Pin2;  break;
        case GPIO_PIN_3:   pin = Pin::Pin3;  break;
        case GPIO_PIN_4:   pin = Pin::Pin4;  break;
        case GPIO_PIN_5:   pin = Pin::Pin5;  break;
        case GPIO_PIN_6:   pin = Pin::Pin6;  break;
        case GPIO_PIN_7:   pin = Pin::Pin7;  break;
        case GPIO_PIN_8:   pin = Pin::Pin8;  break;
        case GPIO_PIN_9:   pin = Pin::Pin9;  break;
        case GPIO_PIN_10:  pin = Pin::Pin10; break;
        case GPIO_PIN_11:  pin = Pin::Pin11; break;
        case GPIO_PIN_12:  pin = Pin::Pin12; break;
        case GPIO_PIN_13:  pin = Pin::Pin13; break;
        case GPIO_PIN_14:  pin = Pin::Pin14; break;
        case GPIO_PIN_15:  pin = Pin::Pin15; break;
        
        default: Error_Handler();
    }

    GPIOHelpers::irq_handler(pin)->call();
}


extern "C" void EXTI0_IRQHandler()
{
    // This will call HAL_GPIO_EXTI_Callback() if the pin has trigged this 
    // interrupt. It will also reset the interrupt flag.
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}


extern "C" void EXTI1_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}


extern "C" void EXTI2_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}


extern "C" void EXTI3_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}


extern "C" void EXTI4_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}


extern "C" void EXTI9_5_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
}


extern "C" void EXTI15_10_IRQHandler()
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}


} // namespace eg {
