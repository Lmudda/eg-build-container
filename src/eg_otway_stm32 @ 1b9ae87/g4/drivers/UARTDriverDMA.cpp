/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "UARTDriverDMA.h"
#include "utilities/ErrorHandler.h"
#include "utilities/CriticalSection.h"
#include "stm32g4xx_hal_rcc_ex.h"
#include "interfaces/IDigitalOutput.h"
#include "logging/Assert.h"


// We have some dependencies on settings in stm32g4xx_hal_conf.h
#ifndef HAL_UART_MODULE_ENABLED
#error HAL_UART_MODULE_ENABLED must be defined to use this driver
#endif
#if (USE_HAL_UART_REGISTER_CALLBACKS == 0)
#error USE_HAL_UART_REGISTER_CALLBACKS must be set to use this driver
#endif


namespace eg {


UARTDriverDMABase::UARTDriverDMABase(const Config& conf, BlockBuffer& tx_buffer)
: m_conf{conf}
, m_tx_buffer{tx_buffer}
{
    gpio_init();
    uart_init();
    tx_dma_init();
    rx_dma_init();

    // Start listening for received data.
    HAL_UARTEx_ReceiveToIdle_DMA(&m_uart_handle, &m_rx_buffer[0], RxBufferSize);
}


void UARTDriverDMABase::write(const uint8_t* data, uint16_t length)
{
    CriticalSection cs;
    if (!m_tx_buffer.append({data, length}))
        return;

    if (!m_tx_busy)
        TxCpltCallback();
}


void UARTDriverDMABase::write(const char* data)
{
    write(reinterpret_cast<const uint8_t*>(data), strlen(data));
}


void UARTDriverDMABase::TxCpltCallback()
{
    // The transfer finished. Called in ISR context or with interrupts disabled.
    CriticalSection cs;

    // Clean up the just-sent block.
    if (m_tx_busy)
    {
        m_tx_buffer.remove(m_tx_block);
    }

    // Decide whether to start another block.
    if (m_tx_buffer.size() > 0)
    {
        // This test appends a character as each block is sent. It shows how the first write
        // kicks off a transfer and the next writes extend the buffer while the first transfer
        // run. Sometimes (irregular) two characters are added. I think this is due to the
        // data wrapping in the buffer and being split into two chunks. It is less frequent
        // with a shorter test string in main().
        // static uint8_t c = 'A';
        // c = (c == 'Z') ? 'A' : c + 1;
        // m_tx_buffer.append({&c, 1});

        m_tx_busy  = true;
        m_tx_block = m_tx_buffer.front();
        HAL_UART_Transmit_DMA(&m_uart_handle, m_tx_block.buffer, m_tx_block.length);
    }
    else
    {
        m_tx_busy  = false;
        m_tx_block = TXBlock{};
    }
}


void UARTDriverDMABase::RxEventCallback(uint16_t pos)
{
    // This is called for three different interrupts:
    //   HT:   half-transfer       pos will be RxBufferSize / 2
    //   TC:   transfer-complete   pos will be RxBufferSize
    //   IDLE: UART idle-detected  pos will be whatever
    // The RX buffer is used in circualr mode.
    // We copy out the bytes received since the last call and
    // update the position (wrapping at the end).

    EG_ASSERT(pos <= RxBufferSize, "RxEventCallback position overflow");
    // The code previously assumed the following conditions and this led to stack corruption
    // because the length being written to the RXData buffer was sometimes too large. This
    // appears to occur only during debugging because in normal operation the assumptions
    // are valid due to the speed of interrupt processing relative to UART transfer. Debugging
    // can lead to missed interrupts, which in turn can break the assumptions.
    //EG_ASSERT(pos >= m_rx_pos, "RxEventCallback position unexpected");
    //EG_ASSERT((pos - m_rx_pos) <= RXData::MaxData, "RxEventCallback RXData buffer overflow");

    // Circular buffer: find the number of new bytes.
    uint16_t length = (pos + RxBufferSize - m_rx_pos) % RxBufferSize;

    // Loop to emit up to two events. This should only ever come up during debugging.
    while (length > 0)
    {
        RXData rx{};
        rx.length  = (length > RxBufferSize) ? RxBufferSize: length;
        length    -= rx.length;

        // TODO_AC FreeRTOS priority fault? Check the debug hard fault is fixed. I think I fixed the
        // ultimate cause but then started seeing occasionally assertions in FreeRTOS because
        // the interrupt priority was too high. I don't know how this could happen.

        // memcpy does not work properly here, at least when debugging, because the
        // size of the data might exceed the size of RXData::data. It also does not wrap.
        //std::memcpy(&rx.data[0], &m_rx_buffer[m_rx_pos], rx.length);
        for (uint16_t i = 0; i < rx.length; ++i)
        {
            rx.data[i] = m_rx_buffer[(m_rx_pos + i) % RxBufferSize];
        }

        m_rx_pos = (m_rx_pos + rx.length) % RxBufferSize;
        m_on_rx_data.emit(rx);
    }
}


void UARTDriverDMABase::ErrorCallback()
{
    // TODO_AC UARTDriverDMABase::ErrorCallback
    m_on_error.emit();
}



void UARTDriverDMABase::gpio_init()
{
    GPIOHelpers::configure_as_alternate(m_conf.tx_port, m_conf.tx_pin, m_conf.tx_alt_fn);
    GPIOHelpers::configure_as_alternate(m_conf.rx_port, m_conf.rx_pin, m_conf.rx_alt_fn);
}


void UARTDriverDMABase::uart_init()
{
    UARTHelpers::enable_clock(m_conf.uart, m_conf.clk_src);

    m_uart_handle.Self                = this;
    m_uart_handle.Instance            = UARTHelpers::instance(m_conf.uart);
    m_uart_handle.Init.BaudRate       = m_conf.baud_rate;
    // These defaults are fine in almost all cases.
    m_uart_handle.Init.WordLength     = UART_WORDLENGTH_8B;
    m_uart_handle.Init.StopBits       = UART_STOPBITS_1;
    m_uart_handle.Init.Parity         = UART_PARITY_NONE;
    m_uart_handle.Init.Mode           = UART_MODE_TX_RX;
    m_uart_handle.Init.HwFlowCtl      = UART_HWCONTROL_NONE;
    m_uart_handle.Init.OverSampling   = UART_OVERSAMPLING_16;
    m_uart_handle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    m_uart_handle.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    m_uart_handle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&m_uart_handle) != HAL_OK)
    {
        Error_Handler();
    }

    // These defaults are fine for now.
    if (HAL_UARTEx_SetTxFifoThreshold(&m_uart_handle, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&m_uart_handle, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_UARTEx_DisableFifoMode(&m_uart_handle) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_UART_RegisterCallback(&m_uart_handle, HAL_UART_TX_COMPLETE_CB_ID, &UARTDriverDMABase::TxCpltCallback);
    HAL_UART_RegisterCallback(&m_uart_handle, HAL_UART_ERROR_CB_ID, &UARTDriverDMABase::ErrorCallback);
    HAL_UART_RegisterRxEventCallback(&m_uart_handle, &UARTDriverDMABase::RxEventCallback);

    // The interrupt priorities of the DMA TX, DMA RX and Idle Line are all set to the save value. Does this matter?
    UARTHelpers::irq_enable(m_conf.uart, GlobalsDefs::DefPreemptPrio);
    UARTHelpers::irq_handler(m_conf.uart)->connect<&UARTDriverDMABase::uart_isr>(this);
}


void UARTDriverDMABase::tx_dma_init()
{
    DMAHelpers::enable_clock(m_conf.tx_dma);

    m_tx_dma_handle.Self                     = this;
    m_tx_dma_handle.Instance                 = DMAHelpers::instance(m_conf.tx_dma);
    m_tx_dma_handle.Init.Request             = m_conf.tx_req;
    // These values are fine for this driver.
    m_tx_dma_handle.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    m_tx_dma_handle.Init.PeriphInc           = DMA_PINC_DISABLE;
    m_tx_dma_handle.Init.MemInc              = DMA_MINC_ENABLE;
    m_tx_dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    m_tx_dma_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    m_tx_dma_handle.Init.Mode                = DMA_NORMAL;
    m_tx_dma_handle.Init.Priority            = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&m_tx_dma_handle) != HAL_OK)
    {
      Error_Handler();
    }

    // Connect the DMA channel to the UART object - this is a HAL thing.
    m_uart_handle.hdmatx  = &m_tx_dma_handle;
    m_tx_dma_handle.Parent = &m_uart_handle;

    DMAHelpers::irq_enable(m_conf.tx_dma, GlobalsDefs::DefPreemptPrio);
    DMAHelpers::irq_handler(m_conf.tx_dma)->connect<&UARTDriverDMABase::tx_dma_isr>(this);
}


void UARTDriverDMABase::rx_dma_init()
{
    DMAHelpers::enable_clock(m_conf.rx_dma);

    m_rx_dma_handle.Self                     = this;
    m_rx_dma_handle.Instance                 = DMAHelpers::instance(m_conf.rx_dma);
    m_rx_dma_handle.Init.Request             = m_conf.rx_req;
    // These values are fine for this driver.
    m_rx_dma_handle.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    m_rx_dma_handle.Init.PeriphInc           = DMA_PINC_DISABLE;
    m_rx_dma_handle.Init.MemInc              = DMA_MINC_ENABLE;
    m_rx_dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    m_rx_dma_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    // Circular mode works much better with HAL_UARTEx_ReceiveToIdle_DMA().
    m_rx_dma_handle.Init.Mode                = DMA_CIRCULAR;
    m_rx_dma_handle.Init.Priority            = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&m_rx_dma_handle) != HAL_OK)
    {
        Error_Handler();
    }

    // Connect the DMA channel to the UART object - this is a HAL thing.
    m_uart_handle.hdmarx  = &m_rx_dma_handle;
    m_rx_dma_handle.Parent = &m_uart_handle;

    DMAHelpers::irq_enable(m_conf.rx_dma, GlobalsDefs::DefPreemptPrio);
    DMAHelpers::irq_handler(m_conf.rx_dma)->connect<&UARTDriverDMABase::rx_dma_isr>(this);
}


void UARTDriverDMABase::tx_dma_isr()
{
    HAL_DMA_IRQHandler(&m_tx_dma_handle);
}


void UARTDriverDMABase::rx_dma_isr()
{
    HAL_DMA_IRQHandler(&m_rx_dma_handle);
}


void UARTDriverDMABase::uart_isr()
{
    HAL_UART_IRQHandler(&m_uart_handle);
}


// Invoked from HAL_DMA_IRQHandler() when the TX transfer is complete.
void UARTDriverDMABase::TxCpltCallback(UART_HandleTypeDef* uart)
{
    auto handle = reinterpret_cast<UARTDriverDMABase::UARTHandle*>(uart);
    handle->Self->TxCpltCallback();
}


void UARTDriverDMABase::ErrorCallback(UART_HandleTypeDef* uart)
{
    auto handle = reinterpret_cast<UARTDriverDMABase::UARTHandle*>(uart);
    handle->Self->ErrorCallback();
}


// Invoked from HAL_UART_IRQHandler() (IDLE interrupt) and HAL_DMA_IRQHandler() (HT and TC interrupts).
// The HAL design seems a little confusing.
void UARTDriverDMABase::RxEventCallback(UART_HandleTypeDef* uart, uint16_t pos)
{
    auto handle = reinterpret_cast<UARTDriverDMABase::UARTHandle*>(uart);
    handle->Self->RxEventCallback(pos);
}


} // namespace eg {
