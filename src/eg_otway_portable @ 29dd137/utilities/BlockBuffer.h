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
#include <cstring>
#include "logging/Assert.h"

namespace eg { 


// Sort of like a ring buffer except the blocks of items are added and removed
// en masse. Useful for the UART TX buffer if nowhere else.
class BlockBuffer
{
public: 
    struct Block
    {
        const uint8_t* buffer{}; 
        uint16_t       length{};
        uint16_t       index{};
    };   

public:
    BlockBuffer(uint8_t* buffer, uint16_t buflen)
    : m_buffer{buffer}
    , m_buflen{buflen}
    {
        EG_ASSERT(buflen > 0, "BlockBuffer length must be non-zero"); // To prevent execution of a divide by zero
    }

    uint16_t capacity() const { return m_buflen; }
    uint16_t size() const     { return m_length; }

    // Add a new chunk of data to the buffer. Returns true if it succeeds.
    bool append(Block block)
    {
        return append(block.buffer, block.length);
    }

    // Obtain details of the next block which can be written out as a single 
    // DMA block or whatever.
    // TODO_AC Since we assume the blocks will be consumed in order it would be change the 
    // API from front()/remove() to read_start()/read_complete(). The active block can be stored 
    // as a member so it need not be passed to read_complete. We can assert if read_start() is 
    // called out of sequence. 
    Block front() const
    {
        Block block(nullptr, 0, 0);
        if (m_length > 0)
        {
            block.buffer = &m_buffer[m_getpos];
            block.index  = ++m_index;    
            if (m_putpos > m_getpos)
                block.length = m_length;
            else
                block.length = m_buflen - m_getpos;
        }
        return block;
    }

    // Cleanup up the given block - assumed to have been obtained with an earlier 
    // call to front(). Returns true if the length is compatible.
    bool remove(Block block)
    {
        if (block.length > m_length)
            return false;
        m_length -= block.length;
        m_getpos  = (m_getpos + block.length) % m_buflen;
        return true;
    }

private:
    bool append(const uint8_t* buffer, uint16_t length)
    {
        if (length == 0)
            return true;
        
        // Will all of the new data fit into the TX buffer?
        if ((m_length + length) > m_buflen)
            return false;
        
        // Will the new data wrap around the TX buffer?
        if (uint32_t(m_putpos + length) <= m_buflen)
        {
            std::memcpy(&m_buffer[m_putpos], &buffer[0], length);
            m_length += length;
            // codechecker_false_positive [core.DivideZero] Guarded by assert in constructor.
            m_putpos  = (m_putpos + length) % m_buflen;
        }
        else
        {
            // Split into two parts and wrap around the end.
            uint16_t length1 = m_buflen - m_putpos;
            uint16_t length2 = length - length1;

            std::memcpy(&m_buffer[m_putpos], &buffer[0], length1);
            std::memcpy(&m_buffer[0], &buffer[length1], length2);

            m_length += length;
            m_putpos  = length2;
        }

        return true;
    }    

private:
    uint8_t* const m_buffer{};
    const uint16_t m_buflen{};

    uint16_t m_putpos{};
    uint16_t m_getpos{};
    uint16_t m_length{};    

    mutable uint16_t m_index{};
};


// Simple wrapper for a BlockBuffer which provides a statically allocated 
// buffer for it to work with.
template <uint16_t SIZE>
class BlockBufferArray : public BlockBuffer
{
public:
    static_assert(SIZE > 0, "BlockBuffer size must be non-zero");
    BlockBufferArray()
    : BlockBuffer(&m_array[0], SIZE)
    {        
    }

private:
    uint8_t m_array[SIZE] = {};
};


} // namespace eg { 
