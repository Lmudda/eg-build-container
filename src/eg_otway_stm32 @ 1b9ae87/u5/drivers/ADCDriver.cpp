/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "stm32u5xx_hal.h"
#include "helpers/ADCHelpers.h"
#include "signals/InterruptHandler.h"
#include "signals/Signal.h"
#include "drivers/ADCDriver.h"
#include "utilities/ErrorHandler.h"

#ifndef HAL_ADC_MODULE_ENABLED
#error ADC Module must be enabled in the ST HAL
#endif

namespace eg {

#if defined(ADC1_BASE)
    static InterruptHandler  g_adc1_handler;

    // HAL forces us to use a global as no way of getting class instamnce into IRQ handling functions
    // Currently implementation only uses a single ADC, ADC1
    static ADC_HandleTypeDef g_hadc1;
#endif
#if defined(ADC4_BASE)
    static InterruptHandler  g_adc4_handler;

    // HAL forces us to use a global as no way of getting class instamnce into IRQ handling functions
    // Currently implementation only uses a single ADC, ADC1
    static ADC_HandleTypeDef g_hadc4;
#endif
#if defined(ADC12_BASE)
    static InterruptHandler  g_adc1_handler;
    static InterruptHandler  g_adc2_handler;

    // HAL forces us to use a global as no way of getting class instamnce into IRQ handling functions
    // Currently implementation only uses a single ADC, ADC1
    static ADC_HandleTypeDef g_hadc1;
    static ADC_HandleTypeDef g_hadc2;
#endif
    ADCDriver::ADCDriver(const Config& conf)
    : m_conf{conf}
    {
	    RCC_PeriphCLKInitTypeDef periphClkInit = { 0 };

	    periphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADCDAC;
	    periphClkInit.AdcDacClockSelection = RCC_ADCDACCLKSOURCE_HCLK;
	    if (HAL_RCCEx_PeriphCLKConfig(&periphClkInit) != HAL_OK)
	    {
		    // Initialization Error
	       Error_Handler();
    	}
	    
        ADCHelpers::enable_clock(m_conf.adc);
	    
        // Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
        g_hadc1.Instance                      = reinterpret_cast<ADC_TypeDef*>(m_conf.adc);
        g_hadc1.Init.ClockPrescaler           = ADC_CLOCK_ASYNC_DIV256;
        g_hadc1.Init.Resolution               = ADC_RESOLUTION_14B;
        g_hadc1.Init.GainCompensation         = 0;
        g_hadc1.Init.ScanConvMode             = ADC_SCAN_DISABLE;
        g_hadc1.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;
        g_hadc1.Init.LowPowerAutoWait         = DISABLE;
        g_hadc1.Init.LowPowerAutoPowerOff     = DISABLE;
        g_hadc1.Init.ContinuousConvMode       = DISABLE;
        g_hadc1.Init.NbrOfConversion          = 1;
        g_hadc1.Init.DiscontinuousConvMode    = DISABLE;
        g_hadc1.Init.ExternalTrigConv         = ADC_SOFTWARE_START;
        g_hadc1.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE;
        g_hadc1.Init.DMAContinuousRequests    = DISABLE;
        g_hadc1.Init.TriggerFrequencyMode     = ADC_TRIGGER_FREQ_HIGH;
        g_hadc1.Init.Overrun                  = ADC_OVR_DATA_PRESERVED;
        g_hadc1.Init.LeftBitShift             = ADC_LEFTBITSHIFT_NONE;
        g_hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
        g_hadc1.Init.OversamplingMode         = DISABLE;
        g_hadc1.Init.DataAlign                = ADC_DATAALIGN_RIGHT;
        if (HAL_ADC_Init(&g_hadc1) != HAL_OK)
        {
            // Initialization Error
           Error_Handler();
        }

        // TODO_AC This is hard-coded for Bugaboo for now. Channel 0 of the ADC is used to read 
        // VREFINT so we can more accurately workout VREF for the conversions. 
        ADCHelpers::enable_vrefint();

        g_adc1_handler.connect<&ADCDriver::on_interrupt>(this);

        if (HAL_ADCEx_Calibration_Start(&g_hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK) {
            // Calibration Error
            Error_Handler();
         }

        // Enable interrupts
        HAL_NVIC_SetPriority(ADC1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(ADC1_IRQn);
    }



    bool ADCDriver::start_read(AdcChannel adc_channel)
    {
        ADC_ChannelConfTypeDef sConfig = { 0 };

        sConfig.Channel      = ADCHelpers::adc_chan_to_stm_adc_chan(adc_channel); // Select the ADC channel (e.g., ADC_CHANNEL_0)
        sConfig.Rank         = ADC_REGULAR_RANK_1;
        sConfig.SamplingTime = LL_ADC_SAMPLINGTIME_814CYCLES;
        sConfig.SingleDiff   = ADC_SINGLE_ENDED;
        sConfig.OffsetNumber = ADC_OFFSET_NONE;
        sConfig.Offset       = 0;
        if (HAL_ADC_ConfigChannel(&g_hadc1, &sConfig) == HAL_OK)
        {
            m_cur_channel = adc_channel;
        }

        return HAL_OK == HAL_ADC_Start_IT(&g_hadc1);
    }



    void ADCDriver::on_interrupt()
    {
        uint32_t adcValue = HAL_ADC_GetValue(&g_hadc1);
        m_on_reading.emit(m_cur_channel, adcValue);
    }


    SignalProxy<AdcChannel, uint32_t> ADCDriver::on_reading()
    {
        return eg::SignalProxy<AdcChannel, uint32_t>{m_on_reading};
    }


#if defined(ADC1_BASE)
    extern "C" void ADC1_IRQHandler(void)
    {
        HAL_ADC_IRQHandler(&g_hadc1);
    }
#endif
#if defined(ADC4_BASE)
    extern "C" void ADC4_IRQHandler(void)
    {
        HAL_ADC_IRQHandler(&g_hadc4);
    }
#endif
#if defined(ADC12_BASE)
    extern "C" void ADC1_IRQHandler(void)
    {
        HAL_ADC_IRQHandler(&g_hadc1);
    }
    extern "C" void ADC2_IRQHandler(void)
    {
        HAL_ADC_IRQHandler(&g_hadc2);
    }
#endif


    extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
    {
#if defined(ADC1_BASE)
        if (hadc->Instance == ADC1)
        {
            g_adc1_handler.call();
        }
#endif
#if defined(ADC4_BASE)
        if (hadc->Instance == ADC4)
        {
            g_adc4_handler.call();
        }
#endif
#if defined(ADC12_BASE)
        if (hadc->Instance == ADC1)
        {
            g_adc1_handler.call();
        }
        if (hadc->Instance == ADC2)
        {
            g_adc1_handler.call();
        }
#endif
    }

} // namespace eg
