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
#include "stm32h7xx_hal.h"
#include <cstdint>
#include <cstring>


// We have some dependencies on settings in stm32h7xx_hal_conf.h
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
    if (m_conf.miso_config)
    {
        GPIOHelpers::configure_as_alternate(m_conf.miso_config->port, m_conf.miso_config->pin, m_conf.miso_config->alternate);
    }

    if (m_conf.mosi_config)
    {
        GPIOHelpers::configure_as_alternate(m_conf.mosi_config->port, m_conf.mosi_config->pin, m_conf.miso_config->alternate);
    }
    if (m_conf.sck_config)
    {
        GPIOHelpers::configure_as_alternate(m_conf.sck_config->port, m_conf.sck_config->pin, m_conf.miso_config->alternate);
    }

    if (m_conf.cs_config)
    {
        if (m_conf.cs_hardware)
        {
            GPIOHelpers::configure_as_alternate(m_conf.cs_config->port, m_conf.cs_config->pin, m_conf.miso_config->alternate);
        }
        else
        {
            GPIOHelpers::configure_as_output(m_conf.cs_config->port, m_conf.cs_config->pin, m_conf.cs_config->otype, m_conf.cs_config->pupd, m_conf.cs_config->ospeed);
            deassert_cs();
        }
    }
}


void SPIDriverDMABase::spi_init()
{
    SPIHelpers::enable_clock(m_conf.spi, m_conf.spi_clk_src);

    m_hspi.Self                           = this;
    m_hspi.Instance                       = reinterpret_cast<SPI_TypeDef*>(m_conf.spi);
    m_hspi.Init.Mode                      = SPI_MODE_MASTER;
    m_hspi.Init.Direction                 = SPI_DIRECTION_2LINES;
    m_hspi.Init.DataSize                  = static_cast<uint32_t>(m_conf.data_size);
    switch (m_conf.mode)
    {
        case SPIMode::Mode0:
            m_hspi.Init.CLKPolarity       = SPI_POLARITY_LOW;
            m_hspi.Init.CLKPhase          = SPI_PHASE_1EDGE;
            break;
        case SPIMode::Mode1:
            m_hspi.Init.CLKPolarity       = SPI_POLARITY_LOW;
            m_hspi.Init.CLKPhase          = SPI_PHASE_2EDGE;
            break;
        case SPIMode::Mode2:
            m_hspi.Init.CLKPolarity       = SPI_POLARITY_HIGH;
            m_hspi.Init.CLKPhase          = SPI_PHASE_1EDGE;
            break;
        case SPIMode::Mode3:
            m_hspi.Init.CLKPolarity       = SPI_POLARITY_HIGH;
            m_hspi.Init.CLKPhase          = SPI_PHASE_2EDGE;
            break;
    }
    m_hspi.Init.NSS                       = m_conf.cs_hardware ? SPI_NSS_HARD_OUTPUT : SPI_NSS_SOFT;
    m_hspi.Init.BaudRatePrescaler         = static_cast<uint32_t>(m_conf.prescaler);
    m_hspi.Init.FirstBit                  = SPI_FIRSTBIT_MSB;
    m_hspi.Init.TIMode                    = SPI_TIMODE_DISABLE;
    m_hspi.Init.CRCCalculation            = SPI_CRCCALCULATION_DISABLE;
    m_hspi.Init.CRCPolynomial             = 0x0;

    m_hspi.Init.NSSPMode                  = m_conf.cs_interleave ? SPI_NSS_PULSE_ENABLE : SPI_NSS_PULSE_DISABLE;
    m_hspi.Init.NSSPolarity               = SPI_NSS_POLARITY_LOW;

    m_hspi.Init.FifoThreshold             = SPI_FIFO_THRESHOLD_01DATA;

    m_hspi.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    m_hspi.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    m_hspi.Init.MasterSSIdleness           = SPI_MASTER_SS_IDLENESS_00CYCLE;
    m_hspi.Init.MasterInterDataIdleness    = static_cast<uint32_t>(m_conf.midi);

    m_hspi.Init.MasterReceiverAutoSusp    = SPI_MASTER_RX_AUTOSUSP_ENABLE;
    m_hspi.Init.MasterKeepIOState         = SPI_MASTER_KEEP_IO_STATE_ENABLE;
    m_hspi.Init.IOSwap                    = SPI_IO_SWAP_DISABLE;
    // TODO: Add extra config for hardware CS control and check other missing fields
    if (HAL_SPI_Init(&m_hspi) != HAL_OK)
    {
        Error_Handler();
    }

    m_hspi.TxCpltCallback   = &SPIDriverDMABase::TxCpltCallback;
    m_hspi.RxCpltCallback   = &SPIDriverDMABase::RxCpltCallback;
    m_hspi.TxRxCpltCallback = &SPIDriverDMABase::TxRxCpltCallback;
    m_hspi.ErrorCallback    = &SPIDriverDMABase::ErrorCallback;

    SPIHelpers::irq_enable(m_conf.spi, m_conf.spi_prio);
    SPIHelpers::irq_handler(m_conf.spi)->connect<&SPIDriverDMABase::spi_isr>(this);
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
    // Memory data alignment needs to be compatible with the SPI data frame size.
    if (static_cast<uint32_t>(m_conf.data_size) > SPI_DATASIZE_16BIT)
    {
        m_mosi_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        m_mosi_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    }
    else if (static_cast<uint32_t>(m_conf.data_size) > SPI_DATASIZE_8BIT)
    {
        m_mosi_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        m_mosi_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    }
    else
    {
        m_mosi_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        m_mosi_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    }
    m_mosi_dma.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    m_mosi_dma.Init.MemBurst            = DMA_MBURST_SINGLE;
    m_mosi_dma.Init.Mode                = DMA_NORMAL;
    m_mosi_dma.Init.Priority            = DMA_PRIORITY_LOW;
    m_mosi_dma.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&m_mosi_dma) != HAL_OK)
    {
        Error_Handler();
    }

    // Connect the DMA channel to the SPI object - this is a HAL thing.
    m_hspi.hdmatx   = &m_mosi_dma;
    m_mosi_dma.Parent = &m_hspi;

    DMAHelpers::irq_enable(m_conf.mosi_dma, m_conf.mosi_prio);
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
    // Memory data alignment needs to be compatible with the SPI data frame size.
    if (static_cast<uint32_t>(m_conf.data_size) > SPI_DATASIZE_16BIT)
    {
        m_miso_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        m_miso_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    }
    else if (static_cast<uint32_t>(m_conf.data_size) > SPI_DATASIZE_8BIT)
    {
        m_miso_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        m_miso_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    }
    else
    {
        m_miso_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        m_miso_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    }
    m_miso_dma.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    m_miso_dma.Init.MemBurst            = DMA_MBURST_SINGLE;
    m_miso_dma.Init.Mode                = DMA_NORMAL;
    m_miso_dma.Init.Priority            = DMA_PRIORITY_LOW;
    m_miso_dma.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&m_miso_dma) != HAL_OK)
    {
        Error_Handler();
    }

    // Connect the DMA channel to the SPI object - this is a HAL thing.
    m_hspi.hdmarx   = &m_miso_dma;
    m_miso_dma.Parent = &m_hspi;

    DMAHelpers::irq_enable(m_conf.miso_dma, m_conf.miso_prio);
    DMAHelpers::irq_handler(m_conf.miso_dma)->connect<&SPIDriverDMABase::miso_dma_isr>(this);
}


void SPIDriverDMABase::queue_transfer(Transfer& trans)
{
    // If the internal buffer is used for TX and/or RX, check for overflow.
    uint32_t buflen = 0;
    buflen += trans.tx_data ? 0u : trans.tx_len;
    buflen += trans.rx_data ? 0u : trans.rx_len;
    if (buflen > Transfer::BufferSize)
    {
        // Maybe return an error code. I prefer to fail
        // fatally because we can find this in testing.
        Error_Handler();
        return;
    }

    // Check for pointless transfer.
    if ((trans.tx_len + trans.rx_len) == 0)
    {
        // Maybe return an error code. I prefer to fail
        // fatally because we can find this in testing.
        Error_Handler();
        return;
    }

    if ((trans.rx_mode == RxMode::TxRx) && (trans.tx_len != trans.rx_len))
    {
        // Simulatenous Tx and Rx, so buffers need to be the same size
        Error_Handler();
        return;
    }

    CriticalSection cs;
    if (m_queue.put(trans))
    {
        if (!m_busy)
        {
            start_transfer();
        }
    }
    else
    {
        // Queue is full. This could potentially happen due to a hardware issue so
        // is not necessarily a programming error. Signal failure.
        m_on_error.emit(trans);
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

        assert_cs();

        if (m_trans.rx_mode == RxMode::TxRx)
        {
            // Have already checked that Tx and Rx buffers are the same size
            m_phase = RxMode::TxRx;
            uint8_t* tx_buffer = m_trans.tx_data ? m_trans.tx_data : &m_trans.buffer[0];
            uint32_t rx_offset = m_trans.tx_data ? 0 : m_trans.tx_len;
            uint8_t* rx_buffer = m_trans.rx_data ? m_trans.rx_data : &m_trans.buffer[rx_offset];
            if (HAL_SPI_TransmitReceive_DMA(&m_hspi, tx_buffer, rx_buffer, m_trans.tx_len) != HAL_OK)
            {
                // TODO: Is this always due to a software error, or could it happen for another reason?
                Error_Handler();
            }
        }
        else
        {
            // Split the TX and RX phases to keep buffers a little smaller. We are half-duplex.
            if (m_trans.tx_len > 0)
            {
                m_phase = RxMode::Tx;

                uint8_t* tx_buffer = m_trans.tx_data ? m_trans.tx_data : &m_trans.buffer[0];
                if (HAL_SPI_Transmit_DMA(&m_hspi, tx_buffer, m_trans.tx_len) != HAL_OK)
                {
                    // TODO: Is this always due to a software error, or could it happen for another reason?
                    Error_Handler();
                }
            }
            else if (m_trans.rx_len > 0)
            {
                m_phase = RxMode::Rx;

                // If the TX buffer is contained within the transfer's internal buffer, we take this
                // into account when the RX buffer is also contained there. We offset by the size of
                // tje TX data. No need for this when the TX buffer is external
                uint32_t rx_offset = m_trans.tx_data ? 0 : m_trans.tx_len;
                uint8_t* rx_buffer = m_trans.rx_data ? m_trans.rx_data : &m_trans.buffer[rx_offset];
                if (HAL_SPI_Receive_DMA(&m_hspi, rx_buffer, m_trans.rx_len) != HAL_OK)
                {
                    // TODO: Is this always due to a software error, or could it happen for another reason?
                    Error_Handler();
                }
            }
        }
    }
    else
    {
        m_busy = false;
    }
}


void SPIDriverDMABase::assert_cs()
{
    if (m_trans.cs)
    {
        if (!m_conf.cs_hardware)
        {
            m_trans.cs->set();
        }
        else
        {
            // Can't use software and hardware CS together
            Error_Handler();
        }
    }
    else
    {
        if (!m_conf.cs_hardware)
        {
            if (m_conf.cs_config)
            {
                // use default CS config
                GPIO_TypeDef* gpio = reinterpret_cast<GPIO_TypeDef*>(m_conf.cs_config->port);
                HAL_GPIO_WritePin(gpio, m_conf.cs_config->pin_mask, m_conf.cs_config->invert ? GPIO_PIN_RESET : GPIO_PIN_SET);
            }
        }
        // else using hw cs so nothing to do
    }
}


void SPIDriverDMABase::deassert_cs()
{
    if (m_trans.cs)
    {
        if (!m_conf.cs_hardware)
        {
            m_trans.cs->reset();
        }
        else
        {
            // Can't use software and hardware CS together
            Error_Handler();
        }
    }
    else
    {
        if (!m_conf.cs_hardware)
        {
            if (m_conf.cs_config)
            {
                // use default CS config
                GPIO_TypeDef* gpio = reinterpret_cast<GPIO_TypeDef*>(m_conf.cs_config->port);
                HAL_GPIO_WritePin(gpio, m_conf.cs_config->pin_mask, m_conf.cs_config->invert ? GPIO_PIN_SET : GPIO_PIN_RESET);
            }
        }
        // else using hw cs so nothing to do
    }
}

void SPIDriverDMABase::finish_transfer()
{
    CriticalSection cs;
    deassert_cs();
    m_on_complete.emit(m_trans);
    start_transfer();
}


void SPIDriverDMABase::spi_isr()
{
    HAL_SPI_IRQHandler(&m_hspi);
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
            if (HAL_SPI_Receive_DMA(&m_hspi, rx_buffer, m_trans.rx_len) != HAL_OK)
            {
                // TODO: Is this always due to a software error, or could it happen for another reason?
                Error_Handler();
            }
        }
        else
        {
            // Results in another call to TxCpltCallback().
            if (HAL_SPI_Transmit_DMA(&m_hspi, rx_buffer, m_trans.rx_len) != HAL_OK)
            {
                // TODO: Is this always due to a software error, or could it happen for another reason?
                Error_Handler();
            }
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


void SPIDriverDMABase::TxRxCpltCallback()
{
    // Called after simultaneous write/read has completed. Called from ISR.
    finish_transfer();
}


void SPIDriverDMABase::ErrorCallback()
{
    // Called in case of some error.
    deassert_cs();
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


void SPIDriverDMABase::TxRxCpltCallback(SPI_HandleTypeDef* hspi)
{
    auto handle = reinterpret_cast<SPIDriverDMABase::SPIHandle*>(hspi);
    handle->Self->TxRxCpltCallback();
}


void SPIDriverDMABase::ErrorCallback(SPI_HandleTypeDef* hspi)
{
    auto handle = reinterpret_cast<SPIDriverDMABase::SPIHandle*>(hspi);
    handle->Self->ErrorCallback();
}


} // namespace eg {
