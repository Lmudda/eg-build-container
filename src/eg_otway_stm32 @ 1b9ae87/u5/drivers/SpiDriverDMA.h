/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "interfaces/ISPIDriver.h"
#include "helpers/SPIHelpers.h"
#include "helpers/DMAHelpers.h"
#include "helpers/GPIOHelpers.h"
#include "utilities/RingBuffer.h"
#include "stm32u5xx_hal.h"
#include "stm32u5xx_hal_spi.h"
#include <cstdint>


namespace eg {


// Wrapper for the DMA HAL API for SPI. 
class SPIDriverDMABase : public ISPIDriver
{
public:
    struct Config
    {
        Spi       spi;  

	    uint32_t  prescaler;
	    
        Port      miso_port;  
        Pin       miso_pin;
        uint8_t   miso_alt;   
        Dma       miso_dma;    
        uint8_t   miso_req;   

        Port      mosi_port;
        Pin       mosi_pin;
        uint8_t   mosi_alt;    
        Dma       mosi_dma;   
        uint8_t   mosi_req;   

        Port      sck_port;
        Pin       sck_pin;
        uint8_t   sck_alt;    
    }; 

public:
    SPIDriverDMABase(const Config& conf, RingBuffer<Transfer>& queue);
    void queue_transfer(Transfer& trans) override;
    SignalProxy<Transfer> on_complete() override { return SignalProxy<Transfer>{m_on_complete}; }
    SignalProxy<Transfer> on_error() override { return SignalProxy<Transfer>{m_on_error}; }

private:
    // Called from the constructor.
    void gpio_init();
    void spi_init();
    void mosi_dma_init();
    void miso_dma_init();

    void start_transfer();
    void finish_transfer();

    void mosi_dma_isr();
    void miso_dma_isr();

    void spi_isr();

    void TxCpltCallback();
    void RxCpltCallback();
    void ErrorCallback();

    // HAL callbacks - just trampoline the calls to non-static members above.
    static void TxCpltCallback(SPI_HandleTypeDef* uart);
    static void RxCpltCallback(SPI_HandleTypeDef* uart);
    static void ErrorCallback(SPI_HandleTypeDef* uart);

private:
    // Nasty trick to add a user value to the handle for a bit of context in callbacks.
    struct SPIHandle : public SPI_HandleTypeDef
    {
        SPIDriverDMABase* Self{};
    };

    struct DMAHandle : public DMA_HandleTypeDef
    {
        SPIDriverDMABase* Self{};
    };

private:
    // Totally arbitrary depth for a queue of pending transfers.
    using Transfers = RingBuffer<Transfer>;

    const Config&     m_conf;
    SPIHandle         m_hspi{};
    DMAHandle         m_mosi_dma{}; 
    DMAHandle         m_miso_dma{}; 

    // Active transfer, if any.    
    Transfer          m_trans{};     
    // Pending transfers, if any.
    Transfers&        m_queue;       
    // True while processing active and pending transfers.
    bool              m_busy{false}; 
    Signal<Transfer>  m_on_complete;
    Signal<Transfer>  m_on_error;
    // This is used to track which of the two phases/buffers 
    // we last sent. Needed to prevent repetition in the TX callback.
    RxMode            m_phase{};        
};


template <uint16_t QUEUE_SIZE>
class SPIDriverDMA : public SPIDriverDMABase
{
public:
    SPIDriverDMA(const Config& conf)
    : SPIDriverDMABase{conf, m_data}
    {        
    }

private:
    RingBufferArray<Transfer, QUEUE_SIZE> m_data; 
};


} // namespace eg {



