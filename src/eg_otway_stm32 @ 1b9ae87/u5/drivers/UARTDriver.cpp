/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/UARTDriver.h"
#include "helpers/GPIOHelpers.h"
#include "signals/InterruptHandler.h"
#include "utilities/ErrorHandler.h"
#include "stm32u5xx_hal.h"
#include <cstdint>
#include <cstring>


// We have some dependencies on settings in stm32u5xx_hal_conf.h
#ifndef HAL_UART_MODULE_ENABLED
#error HAL_UART_MODULE_ENABLED must be defined to use this driver
#endif
#if (USE_HAL_UART_REGISTER_CALLBACKS == 0)
#error USE_HAL_UART_REGISTER_CALLBACKS must be set to use this driver
#endif


namespace eg {


UARTDriverBase::UARTDriverBase(const Config& conf, BlockBuffer& tx_buffer)
: m_conf{conf}
, m_tx_buffer{tx_buffer}
{
    GPIOHelpers::configure_as_alternate(m_conf.tx_port, m_conf.tx_pin, m_conf.tx_alt);
    GPIOHelpers::configure_as_alternate(m_conf.rx_port, m_conf.rx_pin, m_conf.rx_alt);

    UARTHelpers::enable_clock(m_conf.uart);

    // We might want some of these options in the config.
    m_huart.Self              = this;
    m_huart.Instance          = reinterpret_cast<USART_TypeDef*>(m_conf.uart);
    m_huart.Init.BaudRate     = m_conf.baud_rate;
    m_huart.Init.WordLength   = UART_WORDLENGTH_8B;
    m_huart.Init.StopBits     = UART_STOPBITS_1;
    m_huart.Init.Parity       = UART_PARITY_NONE;
    m_huart.Init.Mode         = UART_MODE_TX_RX;
    m_huart.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    m_huart.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&m_huart) != HAL_OK)
    {
        Error_Handler();
    }

    UARTHelpers::irq_enable(m_conf.uart);
    UARTHelpers::irq_handler(m_conf.uart)->connect<&UARTDriverBase::irq>(this);
}


void UARTDriverBase::write(const uint8_t* data, uint16_t length)
{
    CriticalSection cs;
    if (!m_tx_buffer.append({data, length}))
        return;

    if (!m_tx_busy)
        TxCpltCallback();
}


void UARTDriverBase::write(const char* data)
{
    write(reinterpret_cast<const uint8_t*>(data), strlen(data));
}


void UARTDriverBase::irq()
{
    HAL_UART_IRQHandler(&m_huart);
}


void UARTDriverBase::TxCpltCallback()
{
    CriticalSection cs;

    // Clean up the just-sent block.
    if (m_tx_busy)
    {
        m_tx_buffer.remove(m_tx_block);
    }

    // Decide whether to start another block.
    if (m_tx_buffer.size() > 0)
    {
        m_tx_busy  = true;
        m_tx_block = m_tx_buffer.front();
        HAL_UART_Transmit_IT(&m_huart, m_tx_block.buffer, m_tx_block.length);
    }
    else
    {
        m_tx_busy  = false;
        m_tx_block = TXBlock{};
    }
}


void UARTDriverBase::RxCpltCallback()
{
    // Called after read has completed.
}


void UARTDriverBase::ErrorCallback()
{
    // Called in case of some error.
}


// We need a simple way to trampoline these calls into the relevant instances of
// UARTDriver. UART_HandleTypeDef stupidly does not have a user data value to use
// for context.
extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    auto handle = reinterpret_cast<UARTDriverBase::UARTHandle*>(huart);
    handle->Self->TxCpltCallback();
}


extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    auto handle = reinterpret_cast<UARTDriverBase::UARTHandle*>(huart);
    handle->Self->RxCpltCallback();
}


extern "C" void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    auto handle = reinterpret_cast<UARTDriverBase::UARTHandle*>(huart);
    handle->Self->ErrorCallback();
}


} // namespace eg {
