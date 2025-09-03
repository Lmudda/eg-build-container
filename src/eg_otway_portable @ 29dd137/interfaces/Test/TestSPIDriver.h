/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "interfaces/ISPIDriver.h"
#include "utilities/RingBuffer.h"
#include "logging/Assert.h"


namespace eg
{

    class TestSpiDriver : public ISPIDriver
    {
    public:
        TestSpiDriver()
            : m_busy(false)
        {
        }

        void queue_transfer(Transfer& trans)
        {
            m_queue.put(trans);
            if (!m_busy)
            {
                m_busy = true;
            }
        }

        SignalProxy<Transfer> on_complete() { return SignalProxy(m_on_complete); }
        SignalProxy<Transfer> on_error() { return SignalProxy(m_on_error); }

        // Emit the front of the transfer queue without modifying 
        bool TransferFinish()
        {
            Transfer current_transfer;
            bool data_to_get = m_queue.get(current_transfer);

            if (!data_to_get) return false;

            m_on_complete.emit(current_transfer);
            if (m_queue.size() == 0) m_busy = false;
            return true;
        }

        // Emit the front of the transfer queue, adjusting its Rx buffer in-place. 
        bool TransferFinishWithModifiedRxData(uint8_t* buffer, uint8_t len)
        {
            Transfer current_transfer;
            bool data_to_get = m_queue.get(current_transfer);

            if (!data_to_get) return false;

            // Modify Rx data 
            for (int i = 0; i < len; i++) current_transfer.rx_data[i] = buffer[i];

            m_on_complete.emit(current_transfer);
            if (m_queue.size() == 0) m_busy = false;
            return true;
        }

        // Emit the front of the transfer queue, adjusting buffers
        bool TransferFinishWithModifiedBuffer(uint8_t* buffer, uint8_t len)
        {
            Transfer current_transfer;
            bool data_to_get = m_queue.get(current_transfer);

            if (!data_to_get) return false;

            // Modify buffer
            for (int i = 0; i < len; i++) current_transfer.buffer[i] = buffer[i];

            m_on_complete.emit(current_transfer);
            if (m_queue.size() == 0) m_busy = false;
            return true;
        }

        // Raise error and clear queue
        bool TransferError()
        {
            Transfer current_transfer;
            bool data_to_get = m_queue.get(current_transfer);

            if (!data_to_get) return false;

            m_on_error.emit(current_transfer);
            if (m_queue.size() == 0) m_busy = false;
            return true;
        }

        const Transfer& GetFrontItem()
        {
            return m_queue.front();
        }

        uint16_t GetQueueSize()
        {
            return m_queue.size();
        }

    private:
        eg::RingBufferArray<Transfer, 16> m_queue;
        Signal<Transfer> m_on_complete;
        Signal<Transfer> m_on_error;
        bool m_busy;
    };

} // namespace eg
