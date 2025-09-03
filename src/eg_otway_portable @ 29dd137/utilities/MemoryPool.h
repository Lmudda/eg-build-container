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


// Note that this class is not thread/interrupt safe. External serialisation must 
// provided where necessary.
template <typename T, uint16_t SIZE>
class MemoryPool
{
private:
    // This union is to ensure that the pool item is large enough to store a pointer.    
    union PoolItem
    {
        uint8_t   data[sizeof(T)];
        PoolItem* next;
    };

public:
    MemoryPool()
    {
        for (uint16_t i = 0U; i < SIZE; ++i)
        {
            free(reinterpret_cast<T*>(&(m_pool[i])));
        }
    }
    
    T* alloc()
    {
        if (!m_free_list)
            return nullptr;

        PoolItem* item = m_free_list;
        m_free_list    = item->next;

        --m_free;
        if (m_free < m_lowest)
        {
            m_lowest = m_free;
        }

        return reinterpret_cast<T*>(item);
    }
    
    bool free(T* ptr)
    {
        if (ptr)
        {
            PoolItem* item = reinterpret_cast<PoolItem*>(ptr);

            // Does this memory belong to this pool?
            if ((item >= &m_pool[0]) && (item < &m_pool[SIZE]))
            {
                // The pointer must have a value matching allocations from this pool.

                // Bonus, walk the free list to check for double deletions.
                //PoolItem* free = m_free_list;
                //while (free)
                //{
                //    free = free->next;
                //}

                item->next  = m_free_list;
                m_free_list = item;
                ++m_free;
                return true;
            }
        }

        return false;
    }

    uint16_t available() const
    {
        return m_free;
    }

    uint16_t low_water_mark() const
    {
        return m_lowest;
    }

private:
    uint16_t  m_free      = 0;
    uint16_t  m_lowest    = SIZE;
    PoolItem* m_free_list = nullptr;
    PoolItem  m_pool[SIZE];
};


} // namespace eg {

