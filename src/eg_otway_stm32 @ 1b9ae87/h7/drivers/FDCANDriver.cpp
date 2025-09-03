/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024-2025.  All rights reserved.
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
#include "stm32h7xx_hal.h"
#include <cstdint>
#include <cstring>


// We have some dependencies on settings in stm32h7xx_hal_conf.h
#ifndef HAL_FDCAN_MODULE_ENABLED
#error HAL_FDCAN_MODULE_ENABLED must be defined to use this driver
#endif
#if (USE_HAL_FDCAN_REGISTER_CALLBACKS == 0)
#error USE_HAL_FDCAN_REGISTER_CALLBACKS must be set to use this driver
#endif


namespace eg {


// This implementation is for CAN-Classic only, so payload is constrained to 8 bytes.
static constexpr uint8_t kMaxMessageLength = 8u;


FDCANDriver::FDCANDriver(const Config& conf)
: m_conf{conf}
, m_emitted_count{0u}
{
    configure_pins();
    configure_fdcan();
    configure_filters();
    configure_callbacks();
    configure_interrupts();
    configure_self_callback();

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
    m_hfdcan.Init.FrameFormat          = FDCAN_FRAME_CLASSIC;
    m_hfdcan.Init.Mode                 = FDCAN_MODE_NORMAL;
    m_hfdcan.Init.AutoRetransmission   = ENABLE;
    m_hfdcan.Init.TransmitPause        = ENABLE;
    m_hfdcan.Init.ProtocolException    = DISABLE;

    // NB FDCAN1 and FDCAN2 share the same input clock.
    m_hfdcan.Init.NominalPrescaler     = m_conf.prescaler;
    m_hfdcan.Init.NominalSyncJumpWidth = m_conf.sync_jump_width;
    m_hfdcan.Init.NominalTimeSeg1      = m_conf.time_seg1;
    m_hfdcan.Init.NominalTimeSeg2      = m_conf.time_seg2;
    // Data phase values set to same as arbitration phase for CAN-Classic.
    m_hfdcan.Init.DataPrescaler        = m_hfdcan.Init.NominalPrescaler;
    m_hfdcan.Init.DataSyncJumpWidth    = m_hfdcan.Init.NominalSyncJumpWidth;
    m_hfdcan.Init.DataTimeSeg1         = m_hfdcan.Init.NominalTimeSeg1;
    m_hfdcan.Init.DataTimeSeg2         = m_hfdcan.Init.NominalTimeSeg2;

    // Implementation uses RxFIFO0 only. Rx buffers are not used.
    // Memory usage is (for 11-bit with 8 byte data fields):
    // |-------------------------------------------------------------------|
    // | Item               | Number used | Words per use | Words required |
    // |--------------------+-------------+---------------+----------------|
    // | 11-bit filters     | up to 128   | 1             | up to 128      |
    // | 29-bit filters     | 0           | 2             | 0              |
    // | Rx FIFO0           | 64          | 4             | 256            |
    // | Rx FIFO1           | 0           | 4             | 0              |
    // | Rx buffers         | 0           | 4             | 0              |
    // | Tx event FIFO      | 0           | 2             | 0              |
    // | Tx FIFO / Tx queue | 32          | 4             | 128            |
    // | Tx buffers         | 0           | 4             | 0              |
    // | Trigger memory     | 0           | 2             | 0              |
    // |====================|=============|===============|================|
    // |                                            TOTAL | 512            |
    // |-------------------------------------------------------------------|
    // Note that words per use in message FIFOs/buffers increases if the
    // messages are more than 8 bytes (there are 2 words of overhead for
    // each message).
    // There is a total of 2560 words (10kBytes) available that has to be
    // shared between FDCAN1 and FDCAN2. So usage here is within this limits.
    // 
    // For now, just divide memory between the two FDCAN peripherals. We could
    // do this more dynamically based on configured usage, but it doesn't matter
    // for the current implementation.
    m_hfdcan.Init.MessageRAMOffset     = m_conf.can == Can::FdCan1 ? 0u : 1280u;    // In words
    m_hfdcan.Init.StdFiltersNbr        = (m_conf.rx_filters != nullptr) ? m_conf.rx_filter_count : 0u;
    m_hfdcan.Init.ExtFiltersNbr        = 0u;
    m_hfdcan.Init.RxFifo0ElmtsNbr      = 64u;   // Maximum
    m_hfdcan.Init.RxFifo0ElmtSize      = FDCAN_DATA_BYTES_8;
    m_hfdcan.Init.RxFifo1ElmtsNbr      = 0u;
    m_hfdcan.Init.RxFifo1ElmtSize      = FDCAN_DATA_BYTES_8;
    m_hfdcan.Init.RxBuffersNbr         = 0u;
    m_hfdcan.Init.RxBufferSize         = FDCAN_DATA_BYTES_8;
    m_hfdcan.Init.TxEventsNbr          = 0u;
    m_hfdcan.Init.TxBuffersNbr         = 0u;
    m_hfdcan.Init.TxFifoQueueElmtsNbr  = 32u;   // Maximum
    m_hfdcan.Init.TxFifoQueueMode      = (m_conf.tx_buffer_mode == TxBufferMode::Fifo) ? FDCAN_TX_FIFO_OPERATION : FDCAN_TX_QUEUE_OPERATION;
    m_hfdcan.Init.TxElmtSize           = FDCAN_DATA_BYTES_8;

    if (HAL_FDCAN_Init(&m_hfdcan) != HAL_OK)
    {
        Error_Handler();
    }    
}


void FDCANDriver::configure_filters()
{
    static constexpr uint32_t kMax11BitFilters = 128u;

    // Configure any Rx filters
    if (m_conf.rx_filters != nullptr)
    {
        if (m_conf.rx_filter_count > kMax11BitFilters)
        {
            Error_Handler();
        }
        
        for (uint8_t i = 0; i < m_conf.rx_filter_count; i++)
        {
            auto& filter_conf = m_conf.rx_filters[i];

            FDCAN_FilterTypeDef filter;
            filter.IdType           = FDCAN_STANDARD_ID;
            filter.FilterIndex      = i;
            filter.FilterType       = static_cast<uint32_t>(filter_conf.type);
            filter.FilterConfig     = static_cast<uint32_t>(filter_conf.match_action);
            filter.FilterID1        = filter_conf.id1;
            filter.FilterID2        = filter_conf.id2;
            filter.RxBufferIndex    = 0;
            filter.IsCalibrationMsg = 0;
            if (HAL_FDCAN_ConfigFilter(&m_hfdcan, &filter) != HAL_OK)
            {
                Error_Handler();
            }
        }
    }

    // Configure global filter, i.e. what to do with messages that don't match
    // the filters, and what to do with remote frames.
    // This implementation rejects 29-bit frames.
    if (HAL_FDCAN_ConfigGlobalFilter(&m_hfdcan, 
        static_cast<uint32_t>(m_conf.rx_global_filter_action),               // How non-matching standard (11-bit) messages are treated
        FDCAN_REJECT,                                                        // How non-matching extended (29-bit) messages are treated
        m_conf.rx_accept_remote ? FDCAN_FILTER_REMOTE : FDCAN_REJECT_REMOTE, // Whether to filter or reject remote 11-bit ID frames
        FDCAN_REJECT_REMOTE) != HAL_OK)                                      // Whether to filter or reject remote 29-bit ID frames
    {
        Error_Handler();
    }
}


void FDCANDriver::configure_callbacks()
{
    // Only a subset of available callbacks are registered because the others
    // aren't relevant for this constrained implementation.
    m_hfdcan.RxFifo0Callback             = &FDCANDriver::RxFifo0Callback;
    m_hfdcan.ErrorCallback               = &FDCANDriver::ErrorCallback;
    m_hfdcan.ErrorStatusCallback         = &FDCANDriver::ErrorStatusCallback;
    
    // Tell the driver which interrupts to activate. These results in calls to the various callbacks.
    uint32_t int_flags{};

    // This basic interrupt is all we need for normal operation.
    int_flags |= FDCAN_IT_RX_FIFO0_NEW_MESSAGE;

    // Want to know if a message was lost due to RXFIFO0 being full (don't care if it's full,
    // but do care if we lose a message as a result, as the caller may want to do something
    // about it).
    int_flags |= FDCAN_IT_RX_FIFO0_MESSAGE_LOST;

    // This interrupt calls ErrorStatusCallback when we switch from ActiveError to PassiveError
    // (or vice-versa). The switch from ActiveError to PassiveError happens when the Transmit
    // Error Count (TEC) or the Receive Error Count (REC) reaches 128 (NB if the transmitter
    // raises the primary error flag this adds 8 to TEC; if the receiver raises the primary error
    // flag this adds 8 to REC; if the receive raises the secondary error flag this adds 1 to REC;
    // otherwise successfully sending or receiving subtracts 1 from TEC or REC respectively).
    // The switch from PassiveError to ActiveError happens when the both the TEC and the REC have
    // dropped below 128 as a result of successful transmits or receives.
    int_flags |= FDCAN_IT_ERROR_PASSIVE;

    // This interrupt calls ErrorStatusCallback when we switch from PassiveError to BusOff
    // (or vice-versa). The switch from PassiveError to BusOff happens when the Transmit Error
    // Count (TEC) exceeds 255. At this point, we have to reset, after which there should be
    // another interrupt indicating the switch from BusOff to ActiveError.
    int_flags |= FDCAN_IT_BUS_OFF;    

    // Some things that shouldn't happen, but we want to know if they do (these call ErrorCallback).
    // NB it's possible to get interrupts for arbitration and data protocol errors (potentially due
    // to activity from elsewhere on the bus), but this current implementation doesn't need to do
    // anything with this information (it just handles the cases where the error status changes due
    // to the Transmit or Receive Error counts exceeding the limit).
    int_flags |= FDCAN_IT_RAM_ACCESS_FAILURE;
    int_flags |= FDCAN_IT_RAM_WATCHDOG;
    int_flags |= FDCAN_IT_RESERVED_ADDRESS_ACCESS;

    // Use all 32 transmit buffers (for maximum FIFO / Queue length).
    // Each buffer is represented by a bit.
    uint32_t buffers = 0xFFFF'FFFF;
    
    // Activate the selected notifications
    if (HAL_FDCAN_ActivateNotification(&m_hfdcan, int_flags, buffers) != HAL_OK)
    {
        Error_Handler();
    }
}


void FDCANDriver::configure_interrupts()
{
    // We can configure which interrupts occur on which vector (potentially useful for
    // prioritising specific things) although this isn't required for this implementation.
    using Irq = CANHelpers::IrqType;
    CANHelpers::irq_enable(m_conf.can, Irq::IT0, m_conf.priority);
    CANHelpers::irq_handler(m_conf.can, Irq::IT0)->connect<&FDCANDriver::it0_isr>(this);
}


void FDCANDriver::configure_self_callback()
{
    if (m_conf.rx_throttle_level > 0u)
    {
        // Register for notification that an emitted RX event has been handled.
        // This is part of the mechanism for throttling events arising from
        // received CAN messages to prevent possible event loop overflow.
        on_rx_message().connect<&FDCANDriver::on_rx_event_handled>(this);
    }
}


void FDCANDriver::on_rx_event_handled(const CANMessage&)
{
    CriticalSection cs;
    if (m_emitted_count > 0u)
    {
        m_emitted_count = m_emitted_count - 1u;
    }

    // Anything in the queue to emit?
    CANMessage next_message;
    if (m_rx_emit_queue.get(next_message))
    {
        m_emitted_count = m_emitted_count + 1u;
        m_on_rx_message.emit(next_message);
    }
}


void FDCANDriver::emit_or_queue_rx(const CANMessage& message)
{
    CriticalSection cs;

    // Can this received message be emitted immediately?
    if (m_conf.rx_throttle_level == 0u)
    {
        // Emit immediately because throttling is disabled.
        m_on_rx_message.emit(message);
    }
    else if (m_emitted_count < m_conf.rx_throttle_level)
    {
        // Throttling enabled, but OK to emit immediately.
        m_emitted_count = m_emitted_count + 1u;
        m_on_rx_message.emit(message);
    }
    else
    {
        // The maximum number of messages have already been emitted and none of
        // them have been handled yet. Attempt to queue the message. It will be
        // emitted later as soon as the next emitted message is handled.
        if (m_rx_emit_queue.put(message) == false)
        {
            // Failed queue the message, so just drop it.
            // Could count the number dropped if required.
        }

        // Could also manage a high water mark if required.
    }
}


void FDCANDriver::queue_message(uint32_t id, const uint8_t* data, uint8_t length)
{
    // NOTE: FDCAN supports up to 64 bytes but only store up to 8 bytes in the message 
    // structure. Longer messages assume the data buffer lives elsewhere with a 
    // sufficient lifetime to last until transmission. Reception will be similar.
    // We don't want a message structure to be too big to send with a Signal.
    // In any case, this implementation is constrained to CAN-Classic, which relies
    // on messages only up to 8 bytes.
    if (length > kMaxMessageLength)
    {
        Error_Handler();
    }

    CANMessage message{};
    message.id     = id;
    message.data   = nullptr;
    message.length = length;
    std::memcpy(&message.buffer[0], data, length);

    queue_message(message);
}


// Place a message into the CAN TX Queue (which is a priority queue). If more than
// 32 messages are required for any reason, then it would be straightforward to add
// a manual priority queue here.
void FDCANDriver::queue_message(const CANMessage& message)
{
    if (message.length > kMaxMessageLength)
    {
        Error_Handler();
    }

    FDCAN_TxHeaderTypeDef header{};
    header.Identifier          = message.id;
    header.IdType              = FDCAN_STANDARD_ID;
    header.TxFrameType         = message.remote ? FDCAN_REMOTE_FRAME : FDCAN_DATA_FRAME;
    header.DataLength          = message.remote ? 0 : message.length;
    header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;  // Only relevant for CAN FD
    header.BitRateSwitch       = FDCAN_BRS_OFF;     // Only relevant for CAN FD
    header.FDFormat            = FDCAN_CLASSIC_CAN;
    header.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
    header.MessageMarker       = 0;

    // Payload
    const uint8_t* data = (message.data == nullptr) ? &message.buffer[0] : message.data;

    // This needs to be protected by a critical section in case an error occurs
    // as a result of an interrupt handler which then clears m_hfdcan.ErrorCode.
    CriticalSection cs;
    if (HAL_FDCAN_AddMessageToTxFifoQ(&m_hfdcan, &header, data) != HAL_OK)
    {
        if ((m_hfdcan.ErrorCode & HAL_FDCAN_ERROR_FIFO_FULL) == HAL_FDCAN_ERROR_FIFO_FULL)
        {
            m_on_error.emit(Error::TxQueueFull);
            m_hfdcan.ErrorCode &= ~HAL_FDCAN_ERROR_FIFO_FULL;
        }
        else
        {
            // Usage error of some kind
            Error_Handler();
        }
    }
}


void FDCANDriver::it0_isr()
{    
    HAL_FDCAN_IRQHandler(&m_hfdcan);
}


void FDCANDriver::RxFifo0Callback(uint32_t rx_fifo_its)
{
    // Meaning of rx_fifo_its for Rx FIFO0:
    //   bit 0: set means new message
    //   bit 1: set means fill level reached watermark
    //   bit 2: set means Rx FIFO0 full
    //   bit 3: set means Rx FIFO0 message lost (or attempt to write size of zero to FIFO0)
    // NB other bits are set for Rx FIFO1.
    static constexpr uint32_t kRxFifo0NewMessageMask = FDCAN_IR_RF0N_Msk;
    static constexpr uint32_t kRxFifo0MessageLostMask = FDCAN_IR_RF0L_Msk;


    if ((rx_fifo_its & kRxFifo0NewMessageMask) == kRxFifo0NewMessageMask)
    {
        // Retrieve the message(s). There could be more than one (particularly if interrupts
        // have been disabled and messages are being received rapidly). If another message
        // arrives after this function is entered then there should be a new interrupt,
        // because the interrupt flag was cleared by the ST HAL just before calling this
        // function.
        //
        // This implementation avoids using a while loop to empty the RxFIFO as a guard
        // against possible denial of service attacks via CAN, although this is perhaps
        // still achievable by careful timing (and would require access to the CAN bus or
        // the ability to compromise one of the other nodes on the bus, both of which offer
        // simpler ways of attacking the overall function of the device anyway).
        uint32_t num_messages = HAL_FDCAN_GetRxFifoFillLevel(&m_hfdcan, FDCAN_RX_FIFO0);
        FDCAN_RxHeaderTypeDef header;
        static uint8_t rx_data[64];
        for (uint32_t i = 0u; i < num_messages; i++)
        {
            if (HAL_FDCAN_GetRxMessage(&m_hfdcan, FDCAN_RX_FIFO0, &header, &rx_data[0]) != HAL_OK)
            {
                // The ways in which this can fail suggest something hasn't been configured
                // correctly or has got badly out of sync.
                Error_Handler();
            }

            // For now we assume the maximum data for a CAN message is 8 bytes. In general, this
            // can be up to 64 bytes for FDCAN. CANMessage::buffer is an array of 8 bytes. We 
            // need to store longer messages elsewhere, or allocate from a 64-byte chunk pool. 
            if (header.DataLength <= 8u)
            {
                CANMessage message{};
                message.id     = header.Identifier;
                message.length = header.DataLength;
                memcpy(&message.buffer[0], &rx_data[0], message.length);
                emit_or_queue_rx(message);
            }
            else
            {
                // Unsupported message type, just drop it.
            }
        }
    }

    if ((rx_fifo_its & kRxFifo0MessageLostMask) != 0u)
    {
        // Lost a message to to Rx FIFO0 being full.
        m_on_error.emit(Error::RxMessageLost);
    }
}


// This callback is called when we get an error. The actual error is specified in
// the ErrorCode field of the HAL FDCAN handle. This should normally be one of:
//    - ELO (overflow of CAN error logging counter occurred)
//    - WDI (message RAM watchdog event due to missing READY)
//    - PEA (protocol error in arbitration phase detected)
//    - PED (protocol error in data phase detected)
//    - ARA (access to reserved address occurred)
//    - Various errors related to time-triggered CAN (out of scope for this driver)
//    - Various errors relating to HAL issues that might not have been cleared following previous failures
//
// Interrupts are only configured for WDI, ARA
// The ErrorCode field is not cleared by the HAL (except on init, deinit and when starting),
// so it needs to be cleared here after it has been checked.
void FDCANDriver::ErrorCallback()
{
    // The faults are encoded in the ErrorCode field of the handle.
    auto error = m_hfdcan.ErrorCode;

    // Only care about reporting CAN message RAM issues.
    if (((error & HAL_FDCAN_ERROR_RAM_ACCESS)    != 0u) ||
        ((error & HAL_FDCAN_ERROR_RAM_WDG)       != 0u) ||
        ((error & HAL_FDCAN_ERROR_RESERVED_AREA) != 0u))
    {
        m_on_error.emit(Error::InternalFault);
    }

    // Need to clear the error flag otherwise we'll keep getting this callback after every interrupt.
    // TODO: Not sure this is actually correct because various calls to HAL_FDCAN_XXX() will set
    //       this value. For now, work around by using a CriticalSection from the main context
    //       whenever we care about the value of m_hfdcan.ErrorCode.
    m_hfdcan.ErrorCode = HAL_FDCAN_ERROR_NONE;
}


// This callback is called when the peripheral switches to or from
// PassiveError and BusOff states.
void FDCANDriver::ErrorStatusCallback(uint32_t error_status_its)
{
    // Check for passive error signal (either set or reset).
    if ((error_status_its & FDCAN_IR_EP) == FDCAN_IR_EP)
    {
        if (READ_BIT(m_hfdcan.Instance->PSR, FDCAN_PSR_EP))
        {
            // Entered passive error state (TEC or REC)
            m_on_status_changed.emit(CANStatus{CANErrorStatus::PassiveError});
        }
        else
        {
            // Passive error state cleared following sufficient successful transmits.
            m_on_status_changed.emit(CANStatus{CANErrorStatus::ActiveError});
        }
    }

    // Check for bus off signal (either set or reset).
    if ((error_status_its & FDCAN_IR_BO) == FDCAN_IR_BO)
    {
        if (READ_BIT(m_hfdcan.Instance->PSR, FDCAN_PSR_BO))
        {
            // Entered bus off state.
            // See description of FDCAN_PSR register in RM0399. This explains
            // that once the CPU clears FDCAN_CCCR.INIT, the device will wait
            // for 129 occurrences of bus idle before resuming normal operation.
            CLEAR_BIT(m_hfdcan.Instance->CCCR, FDCAN_CCCR_INIT);
            m_on_status_changed.emit(CANStatus{CANErrorStatus::BusOff});
        }
        else
        {
            // Left bus off state.
            if (READ_BIT(m_hfdcan.Instance->PSR, FDCAN_PSR_EP))
            {
                // Not really expecting this after bus recovery, but here for completeness.
                m_on_status_changed.emit(CANStatus{CANErrorStatus::PassiveError});
            }
            else
            {
                m_on_status_changed.emit(CANStatus{CANErrorStatus::ActiveError});
            }
        }
    }
}


void FDCANDriver::RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t rx_fifo_its)
{
    auto handle = reinterpret_cast<FDCANDriver::FDCANHandle*>(hfdcan);
    handle->Self->RxFifo0Callback(rx_fifo_its);
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
