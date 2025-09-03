/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/I2CDriver.h"
#include "drivers/helpers/I2CHelpers.h"
#include "drivers/helpers/GPIOHelpers.h"
#include "signals/InterruptHandler.h"
#include "utilities/ErrorHandler.h"
#include "stm32g4xx_hal.h"
#include <cstdint>
#include <cstring>


// We have some dependencies on settings in stm32g4xx_hal_conf.h
#ifndef HAL_I2C_MODULE_ENABLED
#error HAL_I2C_MODULE_ENABLED must be defined to use this driver
#endif
#if (USE_HAL_I2C_REGISTER_CALLBACKS == 0)
#error USE_HAL_I2C_REGISTER_CALLBACKS must be set to use this driver
#endif


namespace eg {


I2CDriverBase::I2CDriverBase(const Config& conf, RingBuffer<Transfer>& queue)
: m_conf{conf}
, m_queue{queue}
{
    gpio_init();
    i2c_init();
}


void I2CDriverBase::gpio_init()
{
    GPIOHelpers::configure_as_alternate(m_conf.scl_port, m_conf.scl_pin, m_conf.scl_alt, OType::OpenDrain);
    GPIOHelpers::configure_as_alternate(m_conf.sda_port, m_conf.sda_pin, m_conf.sda_alt, OType::OpenDrain);
}


void I2CDriverBase::i2c_init()
{
    I2CHelpers::enable_clock(m_conf.i2c, m_conf.i2c_src);

    m_hi2c.Self                  = this;
    m_hi2c.Instance              = I2CHelpers::instance(m_conf.i2c);
    m_hi2c.Init.Timing           = m_conf.i2c_timing;
    // These defaults seem fine for now.
    m_hi2c.Init.OwnAddress1      = 0;
    m_hi2c.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    m_hi2c.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    m_hi2c.Init.OwnAddress2      = 0;
    m_hi2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    m_hi2c.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    m_hi2c.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&m_hi2c) != HAL_OK)
    {
        Error_Handler();
    }

    // These defaults seem fine for now.
    if (HAL_I2CEx_ConfigAnalogFilter(&m_hi2c, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_I2CEx_ConfigDigitalFilter(&m_hi2c, 0) != HAL_OK)
    {
        Error_Handler();
    }

    m_hi2c.MasterTxCpltCallback = &I2CDriverBase::MasterTxCpltCallback;
    m_hi2c.MasterRxCpltCallback = &I2CDriverBase::MasterRxCpltCallback;
    m_hi2c.ErrorCallback        = &I2CDriverBase::ErrorCallback;

    using Irq = I2CHelpers::IrqType;
    I2CHelpers::irq_enable(m_conf.i2c, Irq::Event,GlobalsDefs::DefPreemptPrio);
    I2CHelpers::irq_enable(m_conf.i2c, Irq::Error, GlobalsDefs::DefPreemptPrio);
    I2CHelpers::irq_handler(m_conf.i2c, Irq::Event)->connect<&I2CDriverBase::event_isr>(this);
    I2CHelpers::irq_handler(m_conf.i2c, Irq::Error)->connect<&I2CDriverBase::error_isr>(this);
}


void I2CDriverBase::queue_transfer(Transfer& trans)
{
    // If the internal buffer is used for TX and/or RX, check for overflow.
    uint16_t length = trans.tx_len + trans.rx_len;
    if ((length == 0u) || (length > Transfer::BufferSize))
    {
        // Maybe return an error code. I prefer to fail
        // fatally because we can find this in testing.
        Error_Handler();
    }

    CriticalSection cs;
    m_queue.put(trans);
    if (!m_busy)
    {
        start_transfer();
    }
}


void I2CDriverBase::start_transfer()
{
    CriticalSection cs;
    if (m_queue.size() > 0)
    {
        m_busy  = true;
        m_trans = m_queue.front();
        m_queue.pop();

        if (m_trans.tx_len > 0)
        {
            // Disable stop bit if we have data to receive after the transmit is done.
            uint32_t auto_stop = (m_trans.rx_len > 0) ? I2C_LAST_FRAME_NO_STOP : I2C_LAST_FRAME;
            HAL_I2C_Master_Seq_Transmit_IT(&m_hi2c, m_trans.address, &m_trans.buffer[0], m_trans.tx_len, auto_stop);
        }
        else
        {
            HAL_I2C_Master_Seq_Receive_IT(&m_hi2c, m_trans.address, &m_trans.buffer[0], m_trans.rx_len, I2C_LAST_FRAME);
        }
    }
    else
    {
        m_busy = false;
    }
}


void I2CDriverBase::finish_transfer()
{
    m_on_complete.emit(m_trans);
    start_transfer();
}


void I2CDriverBase::event_isr()
{
    HAL_I2C_EV_IRQHandler(&m_hi2c);
}


void I2CDriverBase::error_isr()
{
    HAL_I2C_ER_IRQHandler(&m_hi2c);
}


void I2CDriverBase::MasterTxCpltCallback()
{
    if (m_trans.rx_len > 0)
    {
        HAL_I2C_Master_Seq_Receive_IT(&m_hi2c, m_trans.address, &m_trans.buffer[m_trans.tx_len], m_trans.rx_len, I2C_LAST_FRAME);
    }
    else
    {
        finish_transfer();
    }
}


void I2CDriverBase::MasterRxCpltCallback()
{
    // Called after read has completed. Called from ISR
    finish_transfer();
}


void I2CDriverBase::ErrorCallback()
{
    // Called after tranaction has failed. Called from ISR.
    // TODO_AC extract the error code and log it.
    m_on_error.emit(m_trans);
    start_transfer();
}


void I2CDriverBase::MasterTxCpltCallback(I2C_HandleTypeDef* hi2c)
{
    auto handle = reinterpret_cast<I2CDriverBase::I2CHandle*>(hi2c);
    handle->Self->MasterTxCpltCallback();
}


void I2CDriverBase::MasterRxCpltCallback(I2C_HandleTypeDef* hi2c)
{
    auto handle = reinterpret_cast<I2CDriverBase::I2CHandle*>(hi2c);
    handle->Self->MasterRxCpltCallback();
}


void I2CDriverBase::ErrorCallback(I2C_HandleTypeDef* hi2c)
{
    auto handle = reinterpret_cast<I2CDriverBase::I2CHandle*>(hi2c);
    handle->Self->ErrorCallback();
}


} // namespace eg {
