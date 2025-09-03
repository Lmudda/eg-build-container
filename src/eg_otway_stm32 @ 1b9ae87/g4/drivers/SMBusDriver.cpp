/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/SMBusDriver.h"
#include "drivers/helpers/I2CHelpers.h"
#include "drivers/helpers/GPIOHelpers.h"
#include "signals/InterruptHandler.h"
#include "utilities/ErrorHandler.h"
#include "stm32g4xx_hal.h"
#include <cstdint>
#include <cstring>


// We have some dependencies on settings in stm32g4xx_hal_conf.h
#ifndef HAL_SMBUS_MODULE_ENABLED
#error HAL_SMBUS_MODULE_ENABLED must be defined to use this driver
#endif
#if (USE_HAL_SMBUS_REGISTER_CALLBACKS == 0)
#error USE_HAL_SMBUS_REGISTER_CALLBACKS must be set to use this driver
#endif


namespace eg {


SMBusDriverBase::SMBusDriverBase(const Config& conf, RingBuffer<Transfer>& queue)
: m_conf{conf}
, m_tx_queue{queue}
{
    gpio_init();
    i2c_init();

    HAL_SMBUS_EnableListen_IT(&m_hsmbus);
}


void SMBusDriverBase::gpio_init()
{
    GPIOHelpers::configure_as_alternate(m_conf.scl_port, m_conf.scl_pin, m_conf.scl_alt, OType::OpenDrain);
    GPIOHelpers::configure_as_alternate(m_conf.sda_port, m_conf.sda_pin, m_conf.sda_alt, OType::OpenDrain);
}


void SMBusDriverBase::i2c_init()
{
    I2CHelpers::enable_clock(m_conf.i2c, m_conf.i2c_src);

    m_hsmbus.Self                      = this;
    m_hsmbus.Instance                  = I2CHelpers::instance(m_conf.i2c);
    m_hsmbus.Init.Timing               = m_conf.i2c_timing;
    m_hsmbus.Init.OwnAddress1          = m_conf.i2c_address << 1;

    // These defaults seem fine for now. Could pull out more parameters as we need them.
    m_hsmbus.Init.AddressingMode       = SMBUS_ADDRESSINGMODE_7BIT;
    m_hsmbus.Init.DualAddressMode      = SMBUS_DUALADDRESS_DISABLE;
    m_hsmbus.Init.OwnAddress2          = 0;
    m_hsmbus.Init.OwnAddress2Masks     = SMBUS_OA2_NOMASK;
    m_hsmbus.Init.GeneralCallMode      = SMBUS_GENERALCALL_DISABLE;
    m_hsmbus.Init.NoStretchMode        = SMBUS_NOSTRETCH_DISABLE;

    // SMBus-specific settings not used for the regular I2C driver.
    m_hsmbus.Init.AnalogFilter         = SMBUS_ANALOGFILTER_ENABLE;
    m_hsmbus.Init.PacketErrorCheckMode = SMBUS_PEC_DISABLE; //SMBUS_PEC_ENABLE;
    m_hsmbus.Init.PeripheralMode       = SMBUS_PERIPHERAL_MODE_SMBUS_HOST;
    // TODO_AC What is this and how is it calculated? Appears to be about 25ms.
    m_hsmbus.Init.SMBusTimeout         = 0x0000881B;

    if (HAL_SMBUS_Init(&m_hsmbus) != HAL_OK)
    {
        Error_Handler();
    }

    // Not present in the Cube generated version of this code, but present for the I2C.
    // if (HAL_SMBUS_ConfigAnalogFilter(&m_hsmbus, SMBUS_ANALOGFILTER_ENABLE) != HAL_OK)
    // {
    //     Error_Handler();
    // }
    // if (HAL_SMBUS_ConfigDigitalFilter(&m_hsmbus, 0) != HAL_OK)
    // {
    //     Error_Handler();
    // }

    m_hsmbus.MasterTxCpltCallback = &SMBusDriverBase::MasterTxCpltCallback;
    m_hsmbus.MasterRxCpltCallback = &SMBusDriverBase::MasterRxCpltCallback;
    m_hsmbus.SlaveRxCpltCallback  = &SMBusDriverBase::SlaveRxCpltCallback;
    m_hsmbus.ListenCpltCallback   = &SMBusDriverBase::ListenCpltCallback;
    m_hsmbus.ErrorCallback        = &SMBusDriverBase::ErrorCallback;
    m_hsmbus.AddrCallback         = &SMBusDriverBase::AddrCallback;

    using Irq = I2CHelpers::IrqType;
    I2CHelpers::irq_enable(m_conf.i2c, Irq::Event, GlobalsDefs::DefPreemptPrio);
    I2CHelpers::irq_enable(m_conf.i2c, Irq::Error, GlobalsDefs::DefPreemptPrio);
    I2CHelpers::irq_handler(m_conf.i2c, Irq::Event)->connect<&SMBusDriverBase::event_isr>(this);
    I2CHelpers::irq_handler(m_conf.i2c, Irq::Error)->connect<&SMBusDriverBase::error_isr>(this);
}


void SMBusDriverBase::queue_transfer(Transfer& trans)
{
    // If the internal buffer is used for TX and/or RX, check for overflow.
    uint8_t length = trans.tx_len + trans.rx_len;
    if ((length == 0) || (length > Transfer::BufferSize))
    {
        // Maybe return an error code. I prefer to fail
        // fatally because we can find this in testing.
        Error_Handler();
    }

    CriticalSection cs;
    m_tx_queue.put(trans);
    if (!m_busy)
    {
        start_transfer();
    }
}


void SMBusDriverBase::start_transfer()
{
    CriticalSection cs;
    if (m_tx_queue.size() > 0)
    {
        m_busy     = true;
        m_tx_trans = m_tx_queue.front();
        m_tx_queue.pop();

        HAL_SMBUS_DisableListen_IT(&m_hsmbus);

        //if (m_tx_trans.tx_len > 0)
        {
            // Options flags. Related to restart conditions and PEC. It's not entirely clear when to use what.
            // #define  SMBUS_FIRST_FRAME                      SMBUS_SOFTEND_MODE
            // #define  SMBUS_NEXT_FRAME                       ((uint32_t)(SMBUS_RELOAD_MODE | SMBUS_SOFTEND_MODE))
            // #define  SMBUS_FIRST_AND_LAST_FRAME_NO_PEC      SMBUS_AUTOEND_MODE
            // #define  SMBUS_LAST_FRAME_NO_PEC                SMBUS_AUTOEND_MODE
            // #define  SMBUS_FIRST_AND_LAST_FRAME_WITH_PEC    ((uint32_t)(SMBUS_AUTOEND_MODE | SMBUS_SENDPEC_MODE))
            // #define  SMBUS_LAST_FRAME_WITH_PEC              ((uint32_t)(SMBUS_AUTOEND_MODE | SMBUS_SENDPEC_MODE))
               uint32_t options = (m_tx_trans.rx_len > 0) ? SMBUS_FIRST_FRAME : SMBUS_FIRST_AND_LAST_FRAME_WITH_PEC;
            // Extra byte for the PEC
            uint16_t size    = m_tx_trans.tx_len + ((m_tx_trans.rx_len == 0) ? 1 : 0);
            HAL_SMBUS_Master_Transmit_IT(&m_hsmbus, m_tx_trans.address, &m_tx_trans.buffer[0], size, options);
        }
        // else
        // {
        //     HAL_I2C_Master_Seq_Receive_IT(&m_hsmbus, m_tx_trans.address, &m_tx_trans.buffer[0], m_tx_trans.rx_len, I2C_LAST_FRAME);
        // }
    }
    else
    {
        m_busy = false;
        HAL_SMBUS_EnableListen_IT(&m_hsmbus);
    }
}


void SMBusDriverBase::finish_transfer()
{
    m_on_complete.emit(m_tx_trans);
    start_transfer();
}


void SMBusDriverBase::MasterTxCpltCallback()
{
    if (m_tx_trans.rx_len > 0)
    {
        // Extra byte for the PEC.
        HAL_SMBUS_Master_Receive_IT(&m_hsmbus, m_tx_trans.address, &m_tx_trans.buffer[m_tx_trans.tx_len], m_tx_trans.rx_len + 1, SMBUS_LAST_FRAME_WITH_PEC);
    }
    else
    {
        finish_transfer();
    }
}


void SMBusDriverBase::ErrorCallback()
{
    // TODO_AC SMBusDriverBase::ErrorCallback: need to reset the bus or whatever.
    m_on_error.emit(m_tx_trans);
    start_transfer();
}


void SMBusDriverBase::AddrCallback(uint8_t direction, uint16_t address)
{
    // In listen mode - after a call to HAL_SMBUS_EnableListen_IT() - we receive
    // the ADDR interrupt when our address is seen on the bus. We can also enable
    // the special host address detection. Check busy flag to try to avoid conflicts
    // when sending. Is this actually necessary? How does the arbitration work?
    // TODO_AC Need to properly deal with collisions between sending and receiving.
    address = address << 1;
    if ((!m_busy) && (address == m_hsmbus.Init.OwnAddress1))
    {
        // Receive one byte to identify the command being received.
        m_rx_state = RxState::Command;
        m_rx_trans.address = address;
        HAL_SMBUS_Slave_Receive_IT(&m_hsmbus, &m_rx_trans.buffer[0], 1U, SMBUS_NEXT_FRAME);
    }
    else
    {
        __HAL_SMBUS_GENERATE_NACK(&m_hsmbus);
    }
}


void SMBusDriverBase::SlaveRxCpltCallback()
{
    // This is called when a call to HAL_SMBUS_Slave_Receive_IT() completes. We have
    // a simple state machine to read a command byte, and then the remaining data bytes
    // whose number depends on the command.
    switch (m_rx_state)
    {
        // The command byte has been received.
        case RxState::Command:
            switch (m_rx_trans.buffer[0])
            {
                // TODO_AC These are commands issued by the smart battery. We should instead pass in
                // a lookup table of commands we are listening for (if any). These are used here
                // only to test the receipt of messages from the smart battery (and hence messaages
                // generally). One application actually only cares about Host Notify messages.

                // I think this is the battery telling us its charging current in mA - see the SMBus spec
                // Appendix A "Smart Battery Master Functions", and section 5.2.1. ChargingCurrent()
                case 0x14:
                    // This command has two data bytes to receive.
                    m_rx_trans.tx_len = 0;
                    m_rx_trans.rx_len = 3;
                    break;

                // I think this is the battery telling us its charging voltage in mV - see the SMBus spec
                // Appendix A "Smart Battery Master Functions", and section 5.2.2. ChargingVoltage()
                case 0x15:
                    // This command has one data byte to receive.
                    m_rx_trans.tx_len = 0;
                    m_rx_trans.rx_len = 3;
                    break;

                // I think this is the battery telling us its status/alarm/warning - see the SMBus spec
                // Appendix A "Smart Battery Master Functions".
                // case 0x16:
                //     // This command has one data byte to receive.
                //     m_rx_trans.tx_len = 0;
                //     m_rx_trans.rx_len = 2;
                //     break;
            }
            HAL_SMBUS_Slave_Receive_IT(&m_hsmbus, &m_rx_trans.buffer[1], m_rx_trans.rx_len - 1, SMBUS_NEXT_FRAME);
            m_rx_state = RxState::Data;
            break;

        // The data byte(s) have been received.
        case RxState::Data:
            // Should NAK the last byte?
            m_on_notify.emit(m_rx_trans);
            __HAL_SMBUS_GENERATE_NACK(&m_hsmbus);
            HAL_SMBUS_EnableListen_IT(&m_hsmbus);
            break;
    }
}


void SMBusDriverBase::ListenCpltCallback()
{
    // Appears to be called on STOP seen on the bus but only called for one of the two commands.
    // Should NAK the last byte?
    m_on_notify.emit(m_rx_trans);
    __HAL_SMBUS_GENERATE_NACK(&m_hsmbus);
    HAL_SMBUS_EnableListen_IT(&m_hsmbus);
}


void SMBusDriverBase::MasterRxCpltCallback()
{
    // Called after read has completed. Called from ISR
    finish_transfer();
}


void SMBusDriverBase::MasterTxCpltCallback(SMBUS_HandleTypeDef* hsmbus)
{
    auto handle = reinterpret_cast<SMBusDriverBase::SMBusHandle*>(hsmbus);
    handle->Self->MasterTxCpltCallback();
}


void SMBusDriverBase::MasterRxCpltCallback(SMBUS_HandleTypeDef* hsmbus)
{
    auto handle = reinterpret_cast<SMBusDriverBase::SMBusHandle*>(hsmbus);
    handle->Self->MasterRxCpltCallback();
}


void SMBusDriverBase::SlaveRxCpltCallback(SMBUS_HandleTypeDef* hsmbus)
{
    auto handle = reinterpret_cast<SMBusDriverBase::SMBusHandle*>(hsmbus);
    handle->Self->SlaveRxCpltCallback();
}


void SMBusDriverBase::ListenCpltCallback(SMBUS_HandleTypeDef* hsmbus)
{
    auto handle = reinterpret_cast<SMBusDriverBase::SMBusHandle*>(hsmbus);
    handle->Self->ListenCpltCallback();
}


void SMBusDriverBase::ErrorCallback(SMBUS_HandleTypeDef* hsmbus)
{
    auto handle = reinterpret_cast<SMBusDriverBase::SMBusHandle*>(hsmbus);
    handle->Self->ErrorCallback();
}


void SMBusDriverBase::AddrCallback(SMBUS_HandleTypeDef* hsmbus, uint8_t direction, uint16_t address_match_code)
{
    auto handle = reinterpret_cast<SMBusDriverBase::SMBusHandle*>(hsmbus);
    handle->Self->AddrCallback(direction, address_match_code);
}


// struct Registers
// {
//     I2C_TypeDef i2c;
//     bool is_before_hal;
//     bool is_error;
// };


// uint8_t count = 0;
// Registers registers[32];


void SMBusDriverBase::event_isr()
{
    // if (count < 32)
    // {
    //     registers[count].i2c = *(m_hsmbus.Instance);
    //     registers[count].is_before_hal = true;
    //     ++count;
    // }
    HAL_SMBUS_EV_IRQHandler(&m_hsmbus);
    // if (count < 32)
    // {
    //     registers[count].i2c = *(m_hsmbus.Instance);
    //     ++count;
    // }
}


void SMBusDriverBase::error_isr()
{
    // if (count < 32)
    // {
    //     registers[count].i2c = *(m_hsmbus.Instance);
    //     registers[count].is_before_hal = true;
    //     registers[count].is_error = true;
    //     ++count;
    // }
    HAL_SMBUS_ER_IRQHandler(&m_hsmbus);
    // if (count < 32)
    // {
    //     registers[count].i2c = *(m_hsmbus.Instance);
    //     registers[count].is_error = true;
    //     ++count;
    // }
}


} // namespace eg {
