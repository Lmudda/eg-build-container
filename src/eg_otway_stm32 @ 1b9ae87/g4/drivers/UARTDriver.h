/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "helpers/GPIOHelpers.h"
#include "helpers/UARTHelpers.h"
#include "interfaces/IUARTDriver.h"
#include "utilities/BlockBuffer.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_uart.h"
#include <cstdint>


namespace eg {


class UARTDriverBase : public IUARTDriver
{
public:
    struct Config
    {
        Uart      uart;     
        uint32_t  clk_src;

        uint32_t  baud_rate;
        
        Port      tx_port;
        Pin       tx_pin;
        uint8_t   tx_alt_fn;
        
        Port      rx_port;
        Pin       rx_pin;
        uint8_t   rx_alt_fn;
    };

public:
    UARTDriverBase(const Config& conf, BlockBuffer& tx_buffer);
    void write(const uint8_t* data, uint16_t length) override;
    void write(const char* data) override;
    SignalProxy<RXData> on_rx_data() override { return SignalProxy<RXData>{m_on_rx_data}; }
    SignalProxy<> on_error() override { return SignalProxy<>{m_on_error}; };

private:
    void irq();
    
    void TxCpltCallback();
    void RxCpltCallback();
    void ErrorCallback();

    static void TxCpltCallback(UART_HandleTypeDef*);
    static void RxCpltCallback(UART_HandleTypeDef*);
    static void ErrorCallback(UART_HandleTypeDef*);

private:
    // Nasty trick to add a user value to the handle for a bit of context in callbacks.
    struct UARTHandle : public UART_HandleTypeDef
    {
        UARTDriverBase* Self{};
    };

private:
    using TXBuffer = BlockBuffer;
    using TXBlock  = TXBuffer::Block;

    const Config&    m_conf;
    UARTHandle       m_huart;
    
    TXBuffer&        m_tx_buffer; 
    TXBlock          m_tx_block;       // Active block being transmitted.
    bool             m_tx_busy{false}; // True if active.

    Signal<RXData>   m_on_rx_data{};
    static constexpr uint16_t kRxBufferSize = RXData::MaxData;
    uint8_t          m_rx_buffer[kRxBufferSize];

    Signal<>         m_on_error{};
};


template<uint16_t TX_BUFFER_SIZE>
class UARTDriver : public UARTDriverBase
{
public:
    UARTDriver(const Config& conf) 
    : UARTDriverBase{conf, m_data}
    {        
    }

private:
    BlockBufferArray<TX_BUFFER_SIZE> m_data;
};


} // namespace eg {
