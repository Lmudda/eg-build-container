/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "utilities/NonCopyable.h"
#include "signals/Signal.h"


namespace eg {


// Simple wrapper for a CMSIS OS queue (which is a wrapper around a FreeRTOS queue). The 
// main purpose is to encapsulate the storage to support statically allocated queues.
template <typename ItemType, uint16_t kNumElems>
class FreeRTOSQueue : public NonCopyable
{
    // Make sure the element size is rounded up to a multiple of 4.
    static constexpr uint16_t kElemSize = ((sizeof(ItemType) + 3) / 4) * 4; 

    public:
        FreeRTOSQueue()
        {
            //m_attr.name;       
            //m_attr.attr_bits;   
            m_attr.cb_mem  = &m_control;    
            m_attr.cb_size = sizeof(m_control);    
            m_attr.mq_mem  = &m_items[0];    
            m_attr.mq_size = kElemSize * kNumElems; 

            m_id = osMessageQueueNew(kNumElems, kElemSize, &m_attr);
       } 

       bool put(const ItemType& item)
       {
            // Not used in the implementation.
            constexpr uint8_t  Priority = 0; 
            // Don't wait at all.
            constexpr uint32_t Timeout  = 0; 

            osStatus_t status = osMessageQueuePut(m_id, &item, Priority, Timeout);  
            return (status == osOK);
       }

       bool get(ItemType& item)
       {
            // Don't care about the priority. Wait forever if the queue is empty.
            osStatus_t status = osMessageQueueGet(m_id, &item, nullptr, osWaitForever);
            return (status == osOK);
       }

    private:
        alignas(uint32_t) StaticQueue_t m_control{};
        alignas(uint32_t) uint8_t       m_items[kElemSize * kNumElems];
        osMessageQueueId_t              m_id{};
        osMessageQueueAttr_t            m_attr{}; 
};


} // namespace eg {
