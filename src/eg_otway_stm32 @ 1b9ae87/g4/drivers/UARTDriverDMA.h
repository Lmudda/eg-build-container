/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "interfaces/IUARTDriver.h"
#include "drivers/helpers/GPIOHelpers.h"
#include "drivers/helpers/UARTHelpers.h"
#include "drivers/helpers/DMAHelpers.h"
#include "utilities/BlockBuffer.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_uart.h"


namespace eg {


// This driver uses DMA for both TX and RX. More data can be added to the TX buffer while a 
// transer is in progress. When it completes, the code will start another transfer will the new data.
// RX runs continuously on a small circular RX buffer. Any data received is copied into an RXData 
// structure and emitted as an event. There no equivalent of an accumulating RX buffer from which a 
// client can synchronously read. The client instead receives asynchronous events.  
class UARTDriverDMABase : public IUARTDriver
{
public:
    struct Config
    {
        Uart      uart;  
        uint32_t  clk_src;  // e.g. RCC_UART4CLKSOURCE_PCLK1

        uint32_t  baud_rate;

        Port      tx_port;   
        Pin       tx_pin;
        uint8_t   tx_alt_fn; // e.g. GPIO_AF5_UART4 
        Dma       tx_dma;    // DMA channel index.
        uint8_t   tx_req;    // e.g. DMA_REQUEST_UART4_TX 

        Port      rx_port;
        Pin       rx_pin;
        uint8_t   rx_alt_fn; // e.g. GPIO_AF5_UART4 
        Dma       rx_dma;    // DMA channel index. 
        uint8_t   rx_req;    // e.g. DMA_REQUEST_UART4_RX 
    };

public:
    UARTDriverDMABase(const Config& conf, BlockBuffer& tx_buffer); 
    void write(const uint8_t* data, uint16_t length) override;
    void write(const char* data) override;
    SignalProxy<RXData> on_rx_data() override { return SignalProxy<RXData>{m_on_rx_data}; }
    SignalProxy<> on_error() override { return SignalProxy<>{m_on_error}; };

private:
    // Called from the constructor.
    void gpio_init();
    void uart_init();
    void tx_dma_init();
    void rx_dma_init();

    // Invoked from ISRs.
    void tx_dma_isr();
    void rx_dma_isr();
    void uart_isr();

    // Invoked from HAL callbacks (from ISR). 
    void TxCpltCallback();
    void ErrorCallback();
    void RxEventCallback(uint16_t pos);

    // HAL callbacks - just trampoline the calls to non-static members above.
    // It is not necessary, but have used the same names and case as HAL.
    static void TxCpltCallback(UART_HandleTypeDef* uart);
    static void ErrorCallback(UART_HandleTypeDef* uart);
    static void RxEventCallback(UART_HandleTypeDef* uart, uint16_t pos);

private:
    // Nasty trick to add a user value to the handle for a bit of context in callbacks.
    struct UARTHandle : public UART_HandleTypeDef
    {
        UARTDriverDMABase* Self{};
    };
    
    struct DMAHandle : public DMA_HandleTypeDef
    {
        UARTDriverDMABase* Self{};
    };

private:
    const Config& m_conf;
    UARTHandle    m_uart_handle{};
    DMAHandle     m_tx_dma_handle{};
    DMAHandle     m_rx_dma_handle{};

    using TXBuffer = BlockBuffer;
    using TXBlock  = TXBuffer::Block;
    TXBuffer&     m_tx_buffer; 
    TXBlock       m_tx_block{};
    bool          m_tx_busy{false}; // Already managed by HAL I think

    // The RxBuffer is used in a circular mode and continuously 
    // populated with any received bytes. We get HT, TC and IDLE interrupts,
    // which result all in calls to RxEventCallback().
    static constexpr uint8_t RxBufferSize = RXData::MaxData * 2;
    uint8_t        m_rx_buffer[RxBufferSize];
    // Track the last-read position.
    uint16_t       m_rx_pos{};  
    Signal<RXData> m_on_rx_data{};
    Signal<>       m_on_error{};
};


template<uint16_t TxBufferSize>
class UARTDriverDMA : public UARTDriverDMABase
{
public:
    UARTDriverDMA(const Config& conf) 
    : UARTDriverDMABase{conf, m_data}
    {        
    }

private:
    BlockBufferArray<TxBufferSize> m_data;
};



} // namespace eg {
