/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "drivers/FDCANDriver.h"
#include "drivers/helpers/CANHelpers.h"
#include "drivers/helpers/GPIOHelpers.h"
#include "signals/InterruptHandler.h"
#include "utilities/ErrorHandler.h"
#include "utilities/CriticalSection.h"
#include "stm32g4xx_hal.h"
#include <cstdint>
#include <cstring>


// We have some dependencies on settings in stm32g4xx_hal_conf.h
#ifndef HAL_FDCAN_MODULE_ENABLED
#error HAL_FDCAN_MODULE_ENABLED must be defined to use this driver
#endif
#if (USE_HAL_FDCAN_REGISTER_CALLBACKS == 0)
#error USE_HAL_FDCAN_REGISTER_CALLBACKS must be set to use this driver
#endif


namespace eg {


FDCANDriver::FDCANDriver(const Config& conf)
: m_conf{conf}
{
    m_bus_timer.on_update().connect<&FDCANDriver::on_bus_timer>(this);

    configure_pins();
    configure_fdcan();
    configure_filters();
    configure_callbacks();
    configure_interrupts();

    // Start the FDCAN module
    if (HAL_FDCAN_Start(&m_hfdcan) != HAL_OK)
    {
        Error_Handler();
    }
}


void FDCANDriver::configure_pins()
{
    GPIOHelpers::configure_as_alternate(m_conf.tx_port, m_conf.tx_pin, m_conf.tx_alt);
    GPIOHelpers::configure_as_alternate(m_conf.rx_port, m_conf.rx_pin, m_conf.rx_alt);
}


void FDCANDriver::configure_fdcan()
{
    CANHelpers::enable_clock(m_conf.can, m_conf.clk_src);

    m_hfdcan.Self                      = this;
    m_hfdcan.Instance                  = reinterpret_cast<FDCAN_GlobalTypeDef*>(m_conf.can);

    // At 170MHz this lot gives us a data rate of about 1MHz:
    // 1. ClockDivider        = FDCAN_CLOCK_DIV2  => 85MHz
    // 2. NominalPrescaler    = 17                =>  5MHz
    // 3. SyncSeg (implicit)  = 1
    //    NominalTimeSeg1     = 2
    //    NominalTimeSeg2     = 2
    //    Total bit time      = 5 (1 + 2 + 2)     =>  1MHz

    // NOTE: It turns out that the ClockDivider value is ignored for FDCAN2 because the
    // hardware uses whatever value had been set for FDCAN1 (assuming you are using it).
    // The bit timing is instead configured with NominalPrescaler:

    // At 170MHz this lot gives us a data rate of about 1MHz:
    // 1. ClockDivider        = FDCAN_CLOCK_DIV1  => 170MHz
    // 2. NominalPrescaler    = 34                =>   5MHz
    // 3. SyncSeg (implicit)  = 1
    //    NominalTimeSeg1     = 2
    //    NominalTimeSeg2     = 2
    //    Total bit time      = 5 (1 + 2 + 2)     =>   1MHz
    // TODO_AC: FDCAN prescale assertion here - also move some of these values into the configuration.

    // Basic parameters
    // ----------------
    // Specifies the FDCAN kernel clock divider when the clock calibration is bypassed.
    //m_hfdcan.Init.ClockDivider         = FDCAN_CLOCK_DIV2; // 1MHz
    m_hfdcan.Init.ClockDivider         = FDCAN_CLOCK_DIV4; // 500kHz - Faulhaber default.
    //m_hfdcan.Init.ClockDivider         = FDCAN_CLOCK_DIV8;
    // Classic, or FD with or without bitrate switching. Pretty sure we only need classic CAN.
    m_hfdcan.Init.FrameFormat          = FDCAN_FRAME_CLASSIC;
    // I had a lot of trouble with FDCAN_MODE_NORMAL mode because the TX complete interrupt
    // never fired. Don't know why, but there was no device on the bus to respond to messages.
    // External loop back appears to fix this while still putting the messages on the bus. It
    // remains to be seen how this interacts with RX messages. NOTE Loop back is a test mode which
    // will ignore RX data.
    m_hfdcan.Init.Mode                 = FDCAN_MODE_NORMAL;
    // Not sure when you'd want to enable these.
    m_hfdcan.Init.AutoRetransmission   = ENABLE;
    m_hfdcan.Init.TransmitPause        = ENABLE;
    m_hfdcan.Init.ProtocolException    = DISABLE;
    // Specifies the maximum number of time quanta the FDCAN hardware is allowed to lengthen or shorten a bit to perform resynchronization.
    m_hfdcan.Init.NominalSyncJumpWidth = 1;
    // Specifies the value by which the oscillator frequency is divided for generating the data bit time quanta.
    m_hfdcan.Init.DataPrescaler        = 1;
    // Specifies the maximum number of time quanta the FDCAN hardware is allowed to lengthen or shorten a data bit to perform resynchronization.
    m_hfdcan.Init.DataSyncJumpWidth    = 1;
    // Specifies the number of time quanta in Data Bit Segment 1.
    m_hfdcan.Init.DataTimeSeg1         = 1;
    // Specifies the number of time quanta in Data Bit Segment 2.
    m_hfdcan.Init.DataTimeSeg2         = 1;
    // Specifies the number of standard Message ID filter.
    m_hfdcan.Init.StdFiltersNbr        = 0;
    // Specifies the number of extended Message ID filters.
    m_hfdcan.Init.ExtFiltersNbr        = 0;
    // Tx FIFO/Queue Mode selection: FIFO or Queue - there's a difference?
    m_hfdcan.Init.TxFifoQueueMode      = FDCAN_TX_FIFO_OPERATION;

    // Bit timing parameters
    // ---------------------
    // Specifies the value by which the oscillator frequency is divided for generating the nominal bit time quanta.
    m_hfdcan.Init.NominalPrescaler     = 34;
    // Specifies the number of time quanta in Bit Segment 1.
    m_hfdcan.Init.NominalTimeSeg1      = 2;
    // Specifies the number of time quanta in Bit Segment 2.
    m_hfdcan.Init.NominalTimeSeg2      = 2;

    if (HAL_FDCAN_Init(&m_hfdcan) != HAL_OK)
    {
        Error_Handler();
    }
}


void FDCANDriver::configure_filters()
{
    // Configure Rx filter
    FDCAN_FilterTypeDef filter;
    filter.IdType       = FDCAN_STANDARD_ID;
    filter.FilterIndex  = 0;
    filter.FilterType   = FDCAN_FILTER_MASK;
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter.FilterID1    = 0x321;
    filter.FilterID2    = 0x7FF;
    if (HAL_FDCAN_ConfigFilter(&m_hfdcan, &filter) != HAL_OK)
    {
        Error_Handler();
    }

    // Configure global filter:
    if (HAL_FDCAN_ConfigGlobalFilter(&m_hfdcan,
        FDCAN_ACCEPT_IN_RX_FIFO0,
        FDCAN_ACCEPT_IN_RX_FIFO0,
        FDCAN_FILTER_REMOTE,
        FDCAN_FILTER_REMOTE) != HAL_OK)
    {
        Error_Handler();
    }
}


void FDCANDriver::configure_callbacks()
{
    // Probably don't need all of these callbacks.
    //m_hfdcan.TxEventFifoCallback         = &FDCANDriver::TxEventFifoCallback;
    m_hfdcan.RxFifo0Callback             = &FDCANDriver::RxFifo0Callback;
    //m_hfdcan.RxFifo1Callback             = &FDCANDriver::RxFifo1Callback;
    //m_hfdcan.TxFifoEmptyCallback         = &FDCANDriver::TxFifoEmptyCallback;
    m_hfdcan.TxBufferCompleteCallback    = &FDCANDriver::TxBufferCompleteCallback;
    //m_hfdcan.TxBufferAbortCallback       = &FDCANDriver::TxBufferAbortCallback;
    //m_hfdcan.HighPriorityMessageCallback = &FDCANDriver::HighPriorityMessageCallback;
    //m_hfdcan.TimestampWraparoundCallback = &FDCANDriver::TimestampWraparoundCallback;
    //m_hfdcan.TimeoutOccurredCallback     = &FDCANDriver::TimeoutOccurredCallback;
    m_hfdcan.ErrorCallback               = &FDCANDriver::ErrorCallback;
    m_hfdcan.ErrorStatusCallback         = &FDCANDriver::ErrorStatusCallback;

    // Tell the driver which interrupts to activate. These results in calls to the various callbacks.
    uint32_t int_flags{};

    // These basic interrupts are all we need for normal operation.
    int_flags |= FDCAN_IT_RX_FIFO0_NEW_MESSAGE;
    int_flags |= FDCAN_IT_TX_COMPLETE;

    // This relates to a timeout for reading RX message FIFOs and TX event FIFO.
    // Don't think we need this one.
    //int_flags |= FDCAN_IT_TIMEOUT_OCCURRED;

    // This interrupt calls ErrorCallback on failed transmission (e.g. no ACK when nothing
    // else on the bus).
    int_flags |= FDCAN_IT_ARB_PROTOCOL_ERROR;

    // Didn't see this interrupt for some reason.
    //int_flags |= FDCAN_IT_BUS_OFF;

    // This interrupt calls ErrorStatusCallback when we switch from ActiveError to PassiveError.
    // This happens when the Transmit Error Count reaches 128 (16 failed transmits).
    int_flags |= FDCAN_IT_ERROR_PASSIVE;

    uint32_t buffers = FDCAN_TX_BUFFER0 | FDCAN_TX_BUFFER1 | FDCAN_TX_BUFFER2;
    if (HAL_FDCAN_ActivateNotification(&m_hfdcan, int_flags, buffers) != HAL_OK)
    {
        Error_Handler();
    }

    // Not sure we need this. We appear to be getting spurious timeouts for some reason
    // despite not using TX event FIFO and not receving messages in RX FIFO.
    //HAL_FDCAN_EnableTimeoutCounter(&m_hfdcan);
}


void FDCANDriver::configure_interrupts()
{
    // We can apparently configure which interrupts occur on which vector. Not sure
    // why you would do this except perhaps separate TX from RX.
    using Irq = CANHelpers::IrqType;
    CANHelpers::irq_enable(m_conf.can, Irq::IT0, GlobalsDefs::DefPreemptPrio);
    CANHelpers::irq_handler(m_conf.can, Irq::IT0)->connect<&FDCANDriver::it0_isr>(this);
    //CANHelpers::irq_enable(m_conf.can,  Irq::IT1);
    //CANHelpers::irq_handler(m_contf.can, Irq::IT1)->connect<&FDCANDriver::it1_isr>(this);
}


// This function came about because of the way errors are handled on the CAN bus. It is not
// we documented in the ST Reference manual but I found various sources online which
// improved my understanding and led to a working solution. Could likely be improved with
// more research and testing, but I spent about day on this.
//
//     https://www.csselectronics.com/pages/can-bus-errors-intro-tutorial
//     Very useful explanation of the error handling state machine and error counters.
//
//     tn1367-spc5x-can-errors-management-and-bus-off-recovery-stmicroelectronics.pdf
//     Similar description but from ST, for a different peripheral (MCAN).
//
// The behaviour I see does not seem to match exactly. If I send messages with nothing else
// on the bus, I get 16 ActiveError indications on the bus (16 TX messages). The Error Counter
// Register reaches 0x00100080 and switchd to PassiveError. Fine (see tutorial). However, at this
// point it appears that the peripheral actually enters BusOff mode. Perhaps this is something to
// do with the DAR bit being set (Disable Automatic Retransmit).
//
//     https://community.st.com/t5/stm32-mcus-products/fd-can-how-to-reset-error-counter/m-p/647490
//     Comment from ST states that we must hard reset the peripheral.
//
// Not sure what I think of this as a recovery mechanism but it does seem to work.
//
// From the Reference Manual. This seems a little misleading because I never get to this point.
// I never see the BusOff interrupt and attempting the recovery sequence described here does not
// work.
//
//     The bus-off recovery sequence (see CAN Specification Rev. 2.0 or ISO11898-1) cannot be
//     shortened by setting or clearing the INIT bit of the FDCAN_CCCR register. If the device
//     enters bus-off, it sets the INIT bit of its own, stopping all bus activities. Once INIT has been
//     cleared by the CPU, the device waits for 129 occurrences of bus-idle (129 × 11 consecutive
//     recessive bits) before resuming normal operation. At the end of the bus-off recovery
//     sequence, the error management counters are reset. During the waiting time after clearing
//     INIT, each time a sequence of 11 recessive bits has been monitored, a bit0 error code is
//     written to LEC[2:0] of FDCAN_PSR, enabling the CPU to check up whether the CAN bus is
//     void FDCANDriver::reinitialise_fdcan() stuck at dominant or continuously disturbed, and to
//     monitor the bus-off recovery sequence. The REC[6:0] bitfield of the FDCAN_ECR register is
//     used to count these sequences
//
// The bus is at 500kHz, so 129 * 11 * 2us is ~2800us. I used a software timer of ~4ms to cover
// this, but now that seems redundant. Going to leave it alone for now as we finally have some
// kind of working bus recovery.
void FDCANDriver::reinitialise_fdcan()
{
    // Probably not required but doesn't hurt.
    if (HAL_FDCAN_Stop(&m_hfdcan) != HAL_OK)
    {
        Error_Handler();
    }

    // Hard reset the FDCAN peripheral. NOTE: The relevent clock is used by both FDCAN
    // peripherals, if we are using them. Should probably move this to the CANHelpers.
    __HAL_RCC_FDCAN_FORCE_RESET();
    __HAL_RCC_FDCAN_RELEASE_RESET();

    // Reinitialise the peripheral and whatnot. We don't need to called configure_pins()
    // or configure_interrupts() again.
    m_hfdcan = {};
    configure_fdcan();
    configure_filters();
    configure_callbacks();

    // Start the FDCAN module
    if (HAL_FDCAN_Start(&m_hfdcan) != HAL_OK)
    {
        Error_Handler();
    }
}


void FDCANDriver::queue_message(uint32_t id, const uint8_t* data, uint8_t length)
{
    // NOTE: FDCAN supports up to 64 bytes but only store up to 8 bytes in the message
    // structure. Longer messages assume the data buffer lives elsewhere with a
    // sufficient lifetime to last until transmission. Reception will be similar.
    // We don't want a message structure to be too be to send with a Signal.
    //if (length > 64)
    // In any case we are using CANOpen, which relies on messages only up to 8 bytes.
    if (length > 8)
    {
        Error_Handler();
    }

    CANMessage message{};
    message.id     = id;
    //message.data   = (length <= 8) ? 0 : data;
    if (length <= 8)
    {
        message.length = length;
        std::memcpy(&message.buffer[0], data, length);
    }

    queue_message(message);
}


// Place a message in our manual TX FIFO. This indirection allows us to support
// more consecutive messages than the FDCAN TX FIFO size (3).
void FDCANDriver::queue_message(const CANMessage& message)
{
    // Here we manage the FIFO ourselves and can make it arbitrarily large.
    CriticalSection cs;
    m_tx_buffer.put(message);
    if (!m_tx_busy)
    {
        queue_message();
    }
}


// Take a message from our manual FIFO and stuff it into the FDCAN FIFO.
void FDCANDriver::queue_message()
{
    if (m_bus_off)
    {
        return;
    }

    if (m_tx_buffer.size() > 0)
    {
        m_tx_busy = true;
        const CANMessage& message = m_tx_buffer.front();

        // Some of these settings should perhaps be exposed in the API.
        FDCAN_TxHeaderTypeDef header{};
        header.Identifier          = message.id;
        header.IdType              = FDCAN_STANDARD_ID;
        header.TxFrameType         = message.remote ? FDCAN_REMOTE_FRAME : FDCAN_DATA_FRAME;
        header.DataLength          = message.remote ? 0 : message.length;
        header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
        header.BitRateSwitch       = FDCAN_BRS_OFF;
        header.FDFormat            = FDCAN_CLASSIC_CAN;
        header.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
        header.MessageMarker       = 0;

        // Payload
        const uint8_t* data = (message.data == 0) ? &message.buffer[0] : message.data;

        if (HAL_FDCAN_AddMessageToTxFifoQ(&m_hfdcan, &header, data) != HAL_OK)
        {
            Error_Handler();
        }

        m_tx_buffer.pop();
    }
    else
    {
        m_tx_busy = false;
    }
}


void FDCANDriver::it0_isr()
{
    HAL_FDCAN_IRQHandler(&m_hfdcan);
}


void FDCANDriver::TxEventFifoCallback(uint32_t tx_fifo_its)
{

}


void FDCANDriver::RxFifo0Callback(uint32_t rx_fifo_its)
{
    // For now we assume the maximum data for a CAN message is 8 bytes. In general, this
    // can be up to 64 bytes for FDCAN. CANMessage::buffer is an array of 8 bytes. We
    // need to store longer messages elsewhere, or allocate from a 64-byte chunk pool.

    FDCAN_RxHeaderTypeDef header;
    static uint8_t rx_data[64];
    if (HAL_FDCAN_GetRxMessage(&m_hfdcan, FDCAN_RX_FIFO0, &header, &rx_data[0]) != HAL_OK)
    {
        Error_Handler();
    }

    CANMessage message{};
    message.id     = header.Identifier;
    message.length = header.DataLength;
    memcpy(&message.buffer[0], &rx_data[0], message.length);
    m_on_rx_message.emit(message);
}


void FDCANDriver::RxFifo1Callback(uint32_t rx_fifo_its)
{

}


void FDCANDriver::TxFifoEmptyCallback()
{

}


void FDCANDriver::TxBufferCompleteCallback(uint32_t buffers)
{
    // Current message has been sent. Move on to the next.
    queue_message();
}


void FDCANDriver::TxBufferAbortCallback(uint32_t buffers)
{
}


void FDCANDriver::HighPriorityMessageCallback()
{

}


void FDCANDriver::TimestampWraparoundCallback()
{
}


void FDCANDriver::TimeoutOccurredCallback()
{
    // I seem to be getting this callback about every 31-34ms for some reason
    // when timeout interrupts are enabled.
}


// This callback is called when we get an error (specfically a TX + NAK for now).
void FDCANDriver::ErrorCallback()
{
    //uint32_t error_counters = m_hfdcan.Instance->ECR;
    //HAL_FDCAN_GetErrorCounters();

    // TODO_AC This is the wrong response and can spam the bus when motors are not present.
    // Working for now but review later in the project.
    //queue_message();
}


// This callback is called when the peripherals switches to PassiveError state.
// We respond by resetting the peripherals to clear error counts.
void FDCANDriver::ErrorStatusCallback(uint32_t error_status_its)
{
    m_bus_off = true;
    m_bus_timer.start();
}


void FDCANDriver::on_bus_timer()
{
    // This will reset the bus and clear error counts.
    reinitialise_fdcan();

    m_bus_off = false;
    m_tx_busy = false;
    queue_message();
}


void FDCANDriver::TxEventFifoCallback(FDCAN_HandleTypeDef* hfdcan, uint32_t tx_fifo_its)
{
    auto handle = reinterpret_cast<FDCANDriver::FDCANHandle*>(hfdcan);
    handle->Self->TxEventFifoCallback(tx_fifo_its);
}


void FDCANDriver::RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t rx_fifo_its)
{
    auto handle = reinterpret_cast<FDCANDriver::FDCANHandle*>(hfdcan);
    handle->Self->RxFifo0Callback(rx_fifo_its);
}


void FDCANDriver::RxFifo1Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t rx_fifo_its)
{
    auto handle = reinterpret_cast<FDCANDriver::FDCANHandle*>(hfdcan);
    handle->Self->RxFifo1Callback(rx_fifo_its);
}


void FDCANDriver::TxFifoEmptyCallback(FDCAN_HandleTypeDef* hfdcan)
{
    auto handle = reinterpret_cast<FDCANDriver::FDCANHandle*>(hfdcan);
    handle->Self->TxFifoEmptyCallback();
}


void FDCANDriver::TxBufferCompleteCallback(FDCAN_HandleTypeDef* hfdcan, uint32_t buffers)
{
    auto handle = reinterpret_cast<FDCANDriver::FDCANHandle*>(hfdcan);
    handle->Self->TxBufferCompleteCallback(buffers);
}


void FDCANDriver::TxBufferAbortCallback(FDCAN_HandleTypeDef* hfdcan, uint32_t buffers)
{
    auto handle = reinterpret_cast<FDCANDriver::FDCANHandle*>(hfdcan);
    handle->Self->TxBufferAbortCallback(buffers);
}


void FDCANDriver::HighPriorityMessageCallback(FDCAN_HandleTypeDef* hfdcan)
{
    auto handle = reinterpret_cast<FDCANDriver::FDCANHandle*>(hfdcan);
    handle->Self->HighPriorityMessageCallback();
}


void FDCANDriver::TimestampWraparoundCallback(FDCAN_HandleTypeDef* hfdcan)
{
    auto handle = reinterpret_cast<FDCANDriver::FDCANHandle*>(hfdcan);
    handle->Self->TimestampWraparoundCallback();
}


void FDCANDriver::TimeoutOccurredCallback(FDCAN_HandleTypeDef* hfdcan)
{
    auto handle = reinterpret_cast<FDCANDriver::FDCANHandle*>(hfdcan);
    handle->Self->TimeoutOccurredCallback();
}


void FDCANDriver::ErrorCallback(FDCAN_HandleTypeDef* hfdcan)
{
    auto handle = reinterpret_cast<FDCANDriver::FDCANHandle*>(hfdcan);
    handle->Self->ErrorCallback();
}


void FDCANDriver::ErrorStatusCallback(FDCAN_HandleTypeDef* hfdcan, uint32_t error_status_its)
{
    auto handle = reinterpret_cast<FDCANDriver::FDCANHandle*>(hfdcan);
    handle->Self->ErrorStatusCallback(error_status_its);
}


} // namespace eg {
