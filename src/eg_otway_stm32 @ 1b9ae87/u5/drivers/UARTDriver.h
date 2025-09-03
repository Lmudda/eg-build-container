/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "helpers/GPIOHelpers.h"
#include "helpers/UARTHelpers.h"
#include "interfaces/IUARTDriver.h"
#include "utilities/BlockBuffer.h"
#include "signals/Signal.h"
#include "stm32u5xx_hal.h"
#include "stm32u5xx_hal_uart.h"
#include <cstdint>


namespace eg {


extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef*);
extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef*);
extern "C" void HAL_UART_ErrorCallback(UART_HandleTypeDef*);


// I'm not in love with this implementation as working with HAL involves a lot of pain to properly 
// encapsulate it. Worse, the HAL code is not capable of continuous background receive simultaneously 
// with writes. These features should be othogonal as with SPL version. Might do better with LL code.
class UARTDriverBase : public IUARTDriver
{
public:
    struct Config
    {
        Uart      uart;

        uint32_t  baud_rate;

        Port      tx_port;   
        Pin       tx_pin;
        uint8_t   tx_alt;  

        Port      rx_port;
        Pin       rx_pin;
        uint8_t   rx_alt;  
    };

public:
    UARTDriverBase(const Config& conf, BlockBuffer& tx_buffer);
    void write(const uint8_t* data, uint16_t length) override;
    void write(const char* data) override;
    SignalProxy<RXData> on_rx_data() override { return SignalProxy<RXData>{m_on_rx_data}; } 
 
private:
    void irq();
    void TxCpltCallback();
    void RxCpltCallback();
    void ErrorCallback();

    friend void HAL_UART_TxCpltCallback(UART_HandleTypeDef*);
    friend void HAL_UART_RxCpltCallback(UART_HandleTypeDef*);
    friend void HAL_UART_ErrorCallback(UART_HandleTypeDef*);

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

    Signal<RXData>   m_on_rx_data;
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
