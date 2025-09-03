/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <cstdint>


namespace eg { 


// Simple implementation of a ring buffer of objects of some type T.
template <typename T>
class RingBuffer
{   
public:
    RingBuffer(T* buffer, uint16_t buflen)
    : m_buffer{buffer}
    , m_buflen{buflen}
    {
    }
    
    // Read the next item in the buffer without removing it.
    // This assumes that there is something in the buffer to read.
    const T& front() const
    {
        return m_buffer[m_get_pos];
    }

    // Remove the next item, if any, from the reing buffer with returning its value.
    bool pop()
    {
        if (m_length > 0U)
        {
            m_get_pos = (m_get_pos + 1U) % m_buflen; 
            --m_length;
            return true;
        }
        return false;
    }

    // Retrieve the next item, if any, from the buffer, and return whether 
    // there was something to retrieve.
    bool get(T& dat)
    {
        if (m_length > 0U)
        {
            dat = m_buffer[m_get_pos];
            m_get_pos = (m_get_pos + 1U) % m_buflen;
            --m_length;
            return true;
        }
        return false;
    }
    
    // Place an item in the ring buffer, if there is space, and return whether 
    // this operation was successful.
    bool put(const T& dat)
    {
        if (m_length < m_buflen)
        {
            m_buffer[m_put_pos] = dat;
            m_put_pos = (m_put_pos + 1U) % m_buflen; 
            ++m_length;
            return true;
        }
        return false;
    }
    
    void clear()
    {
        m_put_pos = 0U;
        m_get_pos = 0U;
        m_length  = 0U;
    }

    uint16_t size() const
    {
        return m_length;
    }
    
    uint16_t capacity() const
    {
        return m_buflen;
    }
    
private:
    T* const       m_buffer{};    
    const uint16_t m_buflen;

    uint16_t m_put_pos{};
    uint16_t m_get_pos{};
    uint16_t m_length{};
};


// Wraps a static buffer for a RingBuffer. Create and pass to 
// driver constructor requiring a RingBuffer<T>& argument.
template <typename T, uint16_t SIZE>
class RingBufferArray: public RingBuffer<T>
{
public:
    RingBufferArray()
    : RingBuffer<T>{&m_items[0], SIZE}
    {        
    }

private:
    T m_items[SIZE] = {};
};


} // namespace eg {








