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
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_fdcan.h"
#include <cstdint>


namespace eg {


class FDCANDriver : public ICANDriver
{
public:
    struct Config
    {
        Can       can;  
        uint32_t  clk_src; 

        Port      tx_port;
        Pin       tx_pin;
        uint8_t   tx_alt; 

        Port      rx_port;
        Pin       rx_pin;
        uint8_t   rx_alt;    
    }; 

public:
    FDCANDriver(const Config& conf);
    void queue_message(uint32_t id, const uint8_t* data, uint8_t length);
    void queue_message(const CANMessage& message);
    SignalProxy<CANMessage> on_rx_message() { return SignalProxy<CANMessage>{m_on_rx_message}; }

private:
    void configure_pins();
    void configure_fdcan();
    void configure_filters();
    void configure_callbacks();
    void configure_interrupts();

    // This is called when the error count causes us to move to PassiveError state (I think).
    // It seems that the only way to clear the error counter register to prevent the BusOff 
    // state is to hard reset the peripheral. 
    void reinitialise_fdcan();

    void it0_isr();
    void queue_message();

    // We probably don't need all of these callbacks. The most essential ones are 
    // RxFifo0Callback() and TxBufferCompleteCallback(). 
    void TxEventFifoCallback(uint32_t tx_fifo_its);
    void RxFifo0Callback(uint32_t rx_fifo_its);
    void RxFifo1Callback(uint32_t rx_fifo_its);
    void TxFifoEmptyCallback();
    void TxBufferCompleteCallback(uint32_t buffers);
    void TxBufferAbortCallback(uint32_t buffers);
    void HighPriorityMessageCallback();
    void TimestampWraparoundCallback();
    void TimeoutOccurredCallback();
    void ErrorCallback();
    void ErrorStatusCallback(uint32_t error_status_its);

    static void TxEventFifoCallback(FDCAN_HandleTypeDef* hfdcan, uint32_t tx_fifo_its);
    static void RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t rx_fifo_its);
    static void RxFifo1Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t rx_fifo_its);
    static void TxFifoEmptyCallback(FDCAN_HandleTypeDef* hfdcan);
    static void TxBufferCompleteCallback(FDCAN_HandleTypeDef* hfdcan, uint32_t buffers);
    static void TxBufferAbortCallback(FDCAN_HandleTypeDef* hfdcan, uint32_t buffers);
    static void HighPriorityMessageCallback(FDCAN_HandleTypeDef* hfdcan);
    static void TimestampWraparoundCallback(FDCAN_HandleTypeDef* hfdcan);
    static void TimeoutOccurredCallback(FDCAN_HandleTypeDef* hfdcan);
    static void ErrorCallback(FDCAN_HandleTypeDef* hfdcan);
    static void ErrorStatusCallback(FDCAN_HandleTypeDef* hfdcan, uint32_t error_status_its);

    void on_bus_timer();

private:
    // Nasty trick to add a user value to the handle for a bit of context in callbacks.
    struct FDCANHandle : public FDCAN_HandleTypeDef
    {
        FDCANDriver* Self{};
    };

private:
    const Config&      m_conf;
    FDCANHandle        m_hfdcan{};

    using TXBuffer = RingBufferArray<CANMessage, 16>;    
    TXBuffer           m_tx_buffer{};
    bool               m_tx_busy{};

    Signal<CANMessage> m_on_rx_message{};

    bool               m_bus_off{};
    eg::Timer          m_bus_timer{5, eg::Timer::Type::OneShot};
};


} // namespace eg {




