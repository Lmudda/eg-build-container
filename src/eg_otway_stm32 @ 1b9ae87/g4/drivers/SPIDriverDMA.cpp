/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/SPIDriverDMA.h"
#include "drivers/helpers/SPIHelpers.h"
#include "drivers/helpers/GPIOHelpers.h"
#include "signals/InterruptHandler.h"
#include "utilities/ErrorHandler.h"
#include "stm32g4xx_hal.h"
#include <cstdint>
#include <cstring>


// We have some dependencies on settings in stm32g4xx_hal_conf.h
#ifndef HAL_SPI_MODULE_ENABLED
#error HAL_SPI_MODULE_ENABLED must be defined to use this driver
#endif
#if (USE_HAL_SPI_REGISTER_CALLBACKS == 0)
#error USE_HAL_SPI_REGISTER_CALLBACKS must be set to use this driver
#endif


namespace eg {


SPIDriverDMABase::SPIDriverDMABase(const Config& conf, RingBuffer<Transfer>& queue)
: m_conf{conf}
, m_queue{queue}
{
    gpio_init();
    spi_init();
    mosi_dma_init();
    miso_dma_init();
}


void SPIDriverDMABase::gpio_init()
{
    GPIOHelpers::configure_as_alternate(m_conf.miso_port, m_conf.miso_pin, m_conf.miso_alt);
    GPIOHelpers::configure_as_alternate(m_conf.mosi_port, m_conf.mosi_pin, m_conf.mosi_alt);
    GPIOHelpers::configure_as_alternate(m_conf.sck_port,  m_conf.sck_pin,  m_conf.sck_alt);
}


void SPIDriverDMABase::spi_init()
{
    SPIHelpers::enable_clock(m_conf.spi);

    m_hspi.Self                   = this;
    m_hspi.Instance               = reinterpret_cast<SPI_TypeDef*>(m_conf.spi);
    m_hspi.Init.Mode              = SPI_MODE_MASTER;
    m_hspi.Init.Direction         = SPI_DIRECTION_2LINES;
    m_hspi.Init.DataSize          = SPI_DATASIZE_8BIT;
    m_hspi.Init.CLKPolarity       = SPI_POLARITY_HIGH;
    m_hspi.Init.CLKPhase          = SPI_PHASE_2EDGE;
    m_hspi.Init.NSS               = SPI_NSS_SOFT;
    m_hspi.Init.BaudRatePrescaler = static_cast<uint32_t>(m_conf.prescaler);
    m_hspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    m_hspi.Init.TIMode            = SPI_TIMODE_DISABLE;
    m_hspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    m_hspi.Init.CRCPolynomial     = 10;
    HAL_SPI_Init(&m_hspi);

    m_hspi.TxCpltCallback = &SPIDriverDMABase::TxCpltCallback;
    m_hspi.RxCpltCallback = &SPIDriverDMABase::RxCpltCallback;
    m_hspi.ErrorCallback  = &SPIDriverDMABase::ErrorCallback;
}


void SPIDriverDMABase::mosi_dma_init()
{
    DMAHelpers::enable_clock(m_conf.mosi_dma);

    m_mosi_dma.Self                     = this;
    m_mosi_dma.Instance                 = DMAHelpers::instance(m_conf.mosi_dma);
    m_mosi_dma.Init.Request             = m_conf.mosi_req;
    // These values are fine for this driver.
    m_mosi_dma.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    m_mosi_dma.Init.PeriphInc           = DMA_PINC_DISABLE;
    m_mosi_dma.Init.MemInc              = DMA_MINC_ENABLE;
    m_mosi_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    m_mosi_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    m_mosi_dma.Init.Mode                = DMA_NORMAL;
    m_mosi_dma.Init.Priority            = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&m_mosi_dma) != HAL_OK)
    {
        Error_Handler();
    }

    // Connect the DMA channel to the UART object - this is a HAL thing.
    m_hspi.hdmatx   = &m_mosi_dma;
    m_mosi_dma.Parent = &m_hspi;

    DMAHelpers::irq_enable(m_conf.mosi_dma, GlobalsDefs::DefPreemptPrio);
    DMAHelpers::irq_handler(m_conf.mosi_dma)->connect<&SPIDriverDMABase::mosi_dma_isr>(this);
}


void SPIDriverDMABase::miso_dma_init()
{
    DMAHelpers::enable_clock(m_conf.miso_dma);

    m_miso_dma.Self                     = this;
    m_miso_dma.Instance                 = DMAHelpers::instance(m_conf.miso_dma);
    m_miso_dma.Init.Request             = m_conf.miso_req;
    // These values are fine for this driver.
    m_miso_dma.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    m_miso_dma.Init.PeriphInc           = DMA_PINC_DISABLE;
    m_miso_dma.Init.MemInc              = DMA_MINC_ENABLE;
    m_miso_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    m_miso_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    // Circular mode works much better with HAL_UARTEx_ReceiveToIdle_DMA().
    m_miso_dma.Init.Mode                = DMA_NORMAL;
    m_miso_dma.Init.Priority            = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&m_miso_dma) != HAL_OK)
    {
        Error_Handler();
    }

    // Connect the DMA channel to the UART object - this is a HAL thing.
    m_hspi.hdmarx   = &m_miso_dma;
    m_miso_dma.Parent = &m_hspi;

    DMAHelpers::irq_enable(m_conf.miso_dma, GlobalsDefs::DefPreemptPrio);
    DMAHelpers::irq_handler(m_conf.miso_dma)->connect<&SPIDriverDMABase::miso_dma_isr>(this);
}


void SPIDriverDMABase::queue_transfer(Transfer& trans)
{
    // If the internal buffer is used for TX and/or RX, check for overflow.
    uint32_t buflen = 0;
    buflen += trans.tx_data ? 0 : trans.tx_len;
    buflen += trans.rx_data ? 0 : trans.rx_len;
    if (buflen > Transfer::BufferSize)
    {
        // Maybe return an error code. I prefer to fail
        // fatally because we can find this in testing.
        Error_Handler();
        return;
    }

    // Pointless transfer. Should report an error.
    if ((trans.tx_len + trans.rx_len) == 0)
    {
        return;
    }

    CriticalSection cs;
    m_queue.put(trans);
    if (!m_busy)
    {
        start_transfer();
    }
}


void SPIDriverDMABase::start_transfer()
{
    CriticalSection cs;
    if (m_queue.size() > 0)
    {
        m_busy  = true;
        m_trans = m_queue.front();
        m_queue.pop();

        if (m_trans.cs)
        {
            m_trans.cs->reset();
        }

        // Split the TX and RX phases to keep buffers a little smaller. We are half-duplex.
        if (m_trans.tx_len > 0)
        {
            m_phase = RxMode::Tx;

            uint8_t* tx_buffer = m_trans.tx_data ? m_trans.tx_data : &m_trans.buffer[0];
            HAL_SPI_Transmit_DMA(&m_hspi, tx_buffer, m_trans.tx_len);
        }
        else if (m_trans.rx_len > 0)
        {
            m_phase = RxMode::Rx;

            // If the TX buffer is contained within the transfer's internal buffer, we take this
            // into account when the RX buffer is also contained there. We offset by the size of
            // tje TX data. No need for this when the TX buffer is external
            uint32_t rx_offset = m_trans.tx_data ? 0 : m_trans.tx_len;
            uint8_t* rx_buffer = m_trans.rx_data ? m_trans.rx_data : &m_trans.buffer[rx_offset];
            HAL_SPI_Receive_DMA(&m_hspi, rx_buffer, m_trans.rx_len);
        }
    }
    else
    {
        m_busy = false;
    }
}


void SPIDriverDMABase::finish_transfer()
{
    CriticalSection cs;

    if (m_trans.cs)
    {
        m_trans.cs->set();
    }
    m_on_complete.emit(m_trans);
    start_transfer();
}


void SPIDriverDMABase::mosi_dma_isr()
{
    HAL_DMA_IRQHandler(&m_mosi_dma);
}


void SPIDriverDMABase::miso_dma_isr()
{
    HAL_DMA_IRQHandler(&m_miso_dma);
}


void SPIDriverDMABase::TxCpltCallback()
{
    // Called after write has completed. Called from ISR.
    // Extended so that the RX phase can be used to TX a second chunk of data.
    // In this case we need to check the phase to avoid endless repetition of
    // the RX data block (when it is used for TX).
    if ((m_trans.rx_len > 0) && (m_phase == RxMode::Tx))
    {
        m_phase = RxMode::Rx;

        uint32_t rx_offset = m_trans.tx_data ? 0 : m_trans.tx_len;
        uint8_t* rx_buffer = m_trans.rx_data ? m_trans.rx_data : &m_trans.buffer[rx_offset];

        if (m_trans.rx_mode == RxMode::Rx)
        {
            // Results in a call to RxCpltCallback().
            HAL_SPI_Receive_DMA(&m_hspi, rx_buffer, m_trans.rx_len);
        }
        else
        {
            // Results in another call to TxCpltCallback().
            HAL_SPI_Transmit_DMA(&m_hspi, rx_buffer, m_trans.rx_len);
        }
    }
    else
    {
        finish_transfer();
    }
}


void SPIDriverDMABase::RxCpltCallback()
{
    // Called after read has completed. Called from ISR.
    finish_transfer();
}


void SPIDriverDMABase::ErrorCallback()
{
    // Called in case of some error.
    if (m_trans.cs)
    {
        m_trans.cs->set();
    }
    m_on_error.emit(m_trans);
    start_transfer();
}


void SPIDriverDMABase::TxCpltCallback(SPI_HandleTypeDef* hspi)
{
    auto handle = reinterpret_cast<SPIDriverDMABase::SPIHandle*>(hspi);
    handle->Self->TxCpltCallback();
}


void SPIDriverDMABase::RxCpltCallback(SPI_HandleTypeDef* hspi)
{
    auto handle = reinterpret_cast<SPIDriverDMABase::SPIHandle*>(hspi);
    handle->Self->RxCpltCallback();
}


void SPIDriverDMABase::ErrorCallback(SPI_HandleTypeDef* hspi)
{
    auto handle = reinterpret_cast<SPIDriverDMABase::SPIHandle*>(hspi);
    handle->Self->ErrorCallback();
}


} // namespace eg {
