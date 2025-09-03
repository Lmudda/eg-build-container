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
#include "stm32u5xx_hal.h"
#include "stm32u5xx_hal_gpio.h"


namespace eg {


#if defined(GPIOA_BASE)
uint16_t g_gpioa_pins_used = 0u;
#endif
#if defined(GPIOB_BASE)
uint16_t g_gpiob_pins_used = 0u;
#endif
#if defined(GPIOC_BASE)
uint16_t g_gpioc_pins_used = 0u;
#endif
#if defined(GPIOD_BASE)
uint16_t g_gpiod_pins_used = 0u;
#endif
#if defined(GPIOE_BASE)
uint16_t g_gpioe_pins_used = 0u;
#endif
#if defined(GPIOF_BASE)
uint16_t g_gpiof_pins_used = 0u;
#endif
#if defined(GPIOG_BASE)
uint16_t g_gpiog_pins_used = 0u;
#endif
#if defined(GPIOH_BASE)
uint16_t g_gpioh_pins_used = 0u;
#endif
#if defined(GPIOI_BASE)
uint16_t g_gpioi_pins_used = 0u;
#endif
#if defined(GPIOJ_BASE)
uint16_t g_gpioj_pins_used = 0u;
#endif
#if defined(GPIOK_BASE)
uint16_t g_gpiok_pins_used = 0u;
#endif
#if defined(GPIOL_BASE)
uint16_t g_gpiol_pins_used = 0u;
#endif


static uint16_t* get_pins_used(Port port)
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

    return gpio_pins_used;
}


// This is used to track which pins are currently in use to avoid using 
// them in multiple driver instances.
static void allocate_pin(Port port, Pin pin)
{
    uint16_t* gpio_pins_used = get_pins_used(port);
    if (gpio_pins_used != nullptr)
    {
        uint16_t& used = *gpio_pins_used;
        uint16_t  mask = 1U << static_cast<uint8_t>(pin);
        if ((used & mask) != 0)
        {
            // This pin was already used elsewhere.
            Error_Handler();
        }

        used |= mask;
    }
}


// This is used to track which pins are currently in use to avoid using 
// them in multiple driver instances.
static void deallocate_pin(Port port, Pin pin)
{
    uint16_t* gpio_pins_used = get_pins_used(port);
    if (gpio_pins_used != nullptr)
    {
        uint16_t& used = *gpio_pins_used;
        uint16_t  mask = 1U << static_cast<uint8_t>(pin);
        if ((used & mask) != 0)
        {
            used &= ~mask;
        }
        else
        {
            // This pin wasn't allocated.
            Error_Handler();
        }
    }
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


void GPIOHelpers::configure_as_edge_int_input(Port port, Pin pin, PuPd pupd, OSpeed ospeed)
{
    allocate_pin(port, pin);
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
    init.Pin  = 1U << static_cast<uint8_t>(pin);
    init.Mode = GPIO_MODE_ANALOG;
    init.Pull = GPIO_NOPULL;
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


void GPIOHelpers::deinit(Port port, Pin pin)
{
    deallocate_pin(port, pin);
    HAL_GPIO_DeInit(reinterpret_cast<GPIO_TypeDef*>(port), 1U << static_cast<uint8_t>(pin));
}


} // namespace eg {
