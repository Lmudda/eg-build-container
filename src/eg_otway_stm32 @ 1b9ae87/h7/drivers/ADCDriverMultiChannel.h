/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "interfaces/IADCDriver.h"
#include "helpers/ADCHelpers.h"
#include "helpers/DMAHelpers.h"
#include "helpers/GlobalDefs.h"
#include "helpers/GPIOHelpers.h"


namespace eg {


// This is a simple driver which performs a pre-programmed read of multiple channels.
// The channels are configured on construction.
class ADCDriverMultiChannel : public IADCDriverMultiChannel
{
    public:
        struct ChannelConfig
        {
            AdcChannel channel;
            eg::Port   port;
            eg::Pin    pin;
        };

        struct Config
        {
            Adc                  adc;
            AdcClockPrescaler    prescaler;
            uint32_t             adc_clk_src;
            uint32_t             adc_prio{GlobalsDefs::DefPreemptPrio};
            const ChannelConfig* channel_config;
            uint8_t              channel_count;
            Dma                  dma;
            uint32_t             dma_req;
            uint32_t             dma_prio{GlobalsDefs::DefPreemptPrio};
        };

    public:
        ADCDriverMultiChannel(const Config& conf);
        uint8_t get_resolution() const override;
        uint8_t get_channel_count() const override;
        void start_read(Result result) override;
        SignalProxy<Result> on_reading() override { return SignalProxy<Result>(m_on_reading); }
        SignalProxy<Result> on_error() override { return SignalProxy<Result>(m_on_error); }

    private:
        // Nasty trick to add a user value to the handle for a bit of context in callbacks.
        struct ADCHandle : public ADC_HandleTypeDef
        {
            ADCDriverMultiChannel* Self{};
        };

        struct DMAHandle : public DMA_HandleTypeDef
        {
            ADCDriverMultiChannel* Self{};
        };

    private:
        void adc_init();
        void dma_init();
        void channel_init();

        void adc_isr();
        void dma_isr();

        void ConvCpltCallback();
        void ErrorCallback();
        static void ConvCpltCallback(ADC_HandleTypeDef* hadc);
        static void ErrorCallback(ADC_HandleTypeDef* hadc);
 
    private:
        const Config&  m_conf;
        ADCHandle      m_hadc{};
        DMAHandle      m_hdma{};
        Signal<Result> m_on_reading{};
        Signal<Result> m_on_error{};
        Result         m_result;
};


} // namespace eg {
