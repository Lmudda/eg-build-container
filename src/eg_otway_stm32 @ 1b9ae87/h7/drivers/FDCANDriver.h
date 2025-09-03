/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "interfaces/ICANDriver.h"
#include "helpers/CANHelpers.h"
#include "helpers/GPIOHelpers.h"
#include "utilities/RingBuffer.h"
#include "timers/Timer.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_fdcan.h"
#include <cstdint>


namespace eg {


// Simple implementation of ICANDriver using the STM32H7 FDCAN peripheral.
// Implementation is currently limited to CAN with standard (11-bit) messages.
// More sophisticated FD-CAN (with extended 29-bit and higher speeds) is not
// supported currently, but could be added if required.
// Implementation uses a FIFO for receipt which permits a FIFO of up to
// 64 received messages in the order that they were received. An alternative
// would be to use dedicated Rx buffers, each of which can have its own filter,
// but each buffer can only hold a single arriving message and so has to be
// serviced before the next message arrives (newly arriving messages are discarded).
// Similarly we use a FIFO/queue rather than dedicated Tx buffers for transmit so that
// we can handle more than one message of a given ID. Choose a FIFO for strict ordering
// of transmission according to the FIFO, or choose a queue to allow the element with
// the lowest ID to be sent first (i.e. it's a priority queue where items of the same
// priority get sent in FIFO order).
//
// Useful documents:
//  AN5348 - Implementation of CAN-FD in STM32 devices
//  RM0399 - Reference manual for STM32H747, section 59
class FDCANDriver : public ICANDriver
{
public:
    enum class RxFilterType
    {
        Range = FDCAN_FILTER_RANGE, // id1 and id2 specify the range (id1 is min, id2 is max)
        Dual  = FDCAN_FILTER_DUAL,  // id1 and id2 specify two specific ids
        Mask  = FDCAN_FILTER_MASK,  // Classic filter: id1 specifies filter and id2 specifies mask
    };

    enum class RxFilterMatchAction
    {
        Accept = FDCAN_FILTER_TO_RXFIFO0,   // NB implementation is currently hardcoded to use FIFO0
        Reject = FDCAN_FILTER_REJECT
    };

    enum class RxGlobalFilterAction
    {
        Accept = FDCAN_ACCEPT_IN_RX_FIFO0,  // NB implementation is currently hardcoded to use FIFO0
        Reject = FDCAN_REJECT
    };

    // Simple abstraction of the receive filter.
    struct RxFilterConfig
    {
        RxFilterType        type;
        RxFilterMatchAction match_action;
        uint32_t            id1;
        uint32_t            id2;
    };

    // How the transmit buffer memory is handled.
    enum class TxBufferMode
    {
        Fifo,           // True FIFO
        PriorityQueue   // Lower-value IDs are prioritised for transmission
        // NB, we could also add buffers here, but then would need a
        // mechanism for configuring which buffer is for which ID.
    };

    struct Config
    {
        Can                   can;
        uint8_t               priority{GlobalsDefs::DefPreemptPrio};
        uint32_t              clk_src;
        uint32_t              prescaler;
        uint32_t              sync_jump_width;
        uint32_t              time_seg1;
        uint32_t              time_seg2;

        Port                  tx_port;
        Pin                   tx_pin;
        uint8_t               tx_alt;

        Port                  rx_port;
        Pin                   rx_pin;
        uint8_t               rx_alt;

        // If rx_filters is nullptr or rx_filter_count is zero,
        // then all received messages will be accepted.
        const RxFilterConfig* rx_filters;
        uint8_t               rx_filter_count;
        RxGlobalFilterAction  rx_global_filter_action;
        bool                  rx_accept_remote;
        // The maximum number of received messages that can be emitted to the
        // event loop but still remain unhandled (zero to disable throttling).
        uint8_t               rx_throttle_level;

        TxBufferMode          tx_buffer_mode;
    };

public:
    FDCANDriver(const Config& conf);
    void queue_message(uint32_t id, const uint8_t* data, uint8_t length) override;
    void queue_message(const CANMessage& message) override;
    SignalProxy<CANMessage> on_rx_message() override { return SignalProxy<CANMessage>{m_on_rx_message}; }
    SignalProxy<CANStatus> on_status_changed() override { return SignalProxy<CANStatus>{m_on_status_changed}; }
    SignalProxy<Error> on_error() override { return SignalProxy<Error>{m_on_error}; }

private:
    void configure_pins();
    void configure_fdcan();
    void configure_filters();
    void configure_callbacks();
    void configure_interrupts();
    void configure_self_callback();

    void on_rx_event_handled(const CANMessage&);
    void emit_or_queue_rx(const CANMessage& message);

    void it0_isr();

    // We probably don't need all of these callbacks. The most essential ones are 
    // RxFifo0Callback() and TxBufferCompleteCallback(). 
    void RxFifo0Callback(uint32_t rx_fifo_its);
    void ErrorCallback();
    void ErrorStatusCallback(uint32_t error_status_its);

    static void RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t rx_fifo_its);
    static void ErrorCallback(FDCAN_HandleTypeDef* hfdcan);
    static void ErrorStatusCallback(FDCAN_HandleTypeDef* hfdcan, uint32_t error_status_its);

private:
    // Nasty trick to add a user value to the handle for a bit of context in callbacks.
    struct FDCANHandle : public FDCAN_HandleTypeDef
    {
        FDCANDriver* Self{};
    };

private:
    // The size of the queue for received messages that haven't yet been
    // emitted.
    static constexpr uint8_t kEmitQueueSize = 64u;
    using RxEmitQueue = RingBufferArray<CANMessage, kEmitQueueSize>;

    const Config&         m_conf;
    FDCANHandle           m_hfdcan{};
    FDCAN_TxHeaderTypeDef m_tx_header;
    
    RxEmitQueue           m_rx_emit_queue;
    volatile uint8_t      m_emitted_count;

    Signal<CANMessage>    m_on_rx_message{};
    Signal<CANStatus>     m_on_status_changed{};
    Signal<Error>         m_on_error{};
};


} // namespace eg {




