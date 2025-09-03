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


enum class I2c
{
    #if defined(I2C1_BASE)
    I2c1 = I2C1_BASE,
    #endif
    #if defined(I2C2_BASE)
    I2c2 = I2C2_BASE,
    #endif
    #if defined(I2C3_BASE)
    I2c3 = I2C3_BASE,
    #endif
    #if defined(I2C4_BASE)
    I2c4 = I2C4_BASE,
    #endif
};


struct I2CHelpers
{
    enum class IrqType { Event, Error };

    static I2C_TypeDef* instance(I2c i2c);
    static InterruptHandler* irq_handler(I2c i2c, IrqType type);
    static void irq_enable(I2c i2c, IrqType type, uint32_t priority);
    static void enable_clock(I2c i2c, uint32_t clk_src);
    static void disable_clock(I2c i2c);

    // Input is the peripheral clock frequency. The target bus frequency is 100kHz. 
    static constexpr uint32_t make_i2c_timing_100kHz(uint32_t i2c_clk_mhz)
    {
        // STM32Cube calculates this, presumably from the peripheral clock frequency, and 
        // stuffs a magic number into the code. The format of the register I2C_TIMINGR is 
        // explained in the Reference Manual: 
        //                                           0x 20 B0 D9 FF   example
        //     Timing prescaler - 1  PRESC    [31:28]   2- -- -- --   scales I2CCLK to create t_prescale
        //     Reserved                                 -0 -- -- --
        //     Data setup time       SCLDEL   [23:20]   -- B- -- --   units of t_prescale
        //     Data hold time        SDADEL   [19:16]   -- -0 -- --   units of t_prescale 
        //     SCL high period - 1   SCLH     [15:8]    -- -- D9 --   units of t_prescale
        //     SCL low period - 1    SCLL     [7:0]     -- -- -- FF   units of t_prescale
        // 
        // Reversed engineered this calculation from Cube values and a look at the data sheet.
 
        // 250ns is given in the Reference Manual as the minimum setup time for standard mode (100kHz).
        constexpr uint32_t SetupTime = 250;
        // 0ns is given in the Reference Manual as the minimum hold time for standard mode.
        constexpr uint32_t HoldTime  = 0;
        // 10,000ns is the clock period for standard mode (100kHz). It appears that the duty 
        // cycle is assymetric according to Cube: 40% high, 60% low.
        constexpr uint32_t ClockHiTime = 4000;
        constexpr uint32_t ClockLoTime = 6000;
    
        uint8_t presc = 0;
        while (presc < 16)
        {
            double  tpresc_ns  = 1000.0 * (presc + 1) / i2c_clk_mhz;
            uint32_t setup     = uint32_t(double(SetupTime) / tpresc_ns + 0.5) - 1; 
            uint32_t hold      = HoldTime;
            uint32_t scl_high  = uint32_t(double(ClockHiTime) / tpresc_ns + 0.5) - 1;
            uint32_t scl_low   = uint32_t(double(ClockLoTime) / tpresc_ns + 0.5) - 1;

            if ((scl_high < 256) && (scl_low < 256))
            {
                uint32_t result = 
                    ((presc & 0xF) << 28) | 
                    ((setup & 0xF) << 20) | 
                    ((hold & 0xF) << 16) | 
                    ((scl_high & 0xFF) << 8) | 
                    (scl_low & 0xFF);  
                return result;
            }

            ++presc;
        }
        return 0;
    }

    // TODO_AC Generalise this calculation to any bus frequency. Study the reference manual.
    static constexpr uint32_t make_i2c_timing_400kHz(uint32_t i2c_clk_mhz)
    {
        // 250ns is given in the Reference Manual as the minimum setup time for standard mode (100kHz).
        constexpr uint32_t SetupTime = 100;
        // 0ns is given in the Reference Manual as the minimum hold time for standard mode.
        constexpr uint32_t HoldTime  = 0;
        // 2,500ns is the clock period for fast mode (400kHz). It appears that the duty 
        // cycle is assymetric according to Cube: 40% high, 60% low.
        constexpr uint32_t ClockHiTime = 1000;
        constexpr uint32_t ClockLoTime = 1500;
    
        uint8_t presc = 0;
        while (presc < 16)
        {
            double  tpresc_ns  = 1000.0 * (presc + 1) / i2c_clk_mhz;
            uint32_t setup     = uint32_t(double(SetupTime) / tpresc_ns + 0.5) - 1; 
            uint32_t hold      = HoldTime;
            uint32_t scl_high  = uint32_t(double(ClockHiTime) / tpresc_ns + 0.5) - 1;
            uint32_t scl_low   = uint32_t(double(ClockLoTime) / tpresc_ns + 0.5) - 1;

            if ((scl_high < 256) && (scl_low < 256))
            {
                uint32_t result = 
                    ((presc & 0xF) << 28) | 
                    ((setup & 0xF) << 20) | 
                    ((hold & 0xF) << 16) | 
                    ((scl_high & 0xFF) << 8) | 
                    (scl_low & 0xFF);  
                return result;
            }

            ++presc;
        }
        return 0;
    }
};


} // namespace eg {
    


