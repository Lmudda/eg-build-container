/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "utilities/NonCopyable.h"
#include "interfaces/IFlashMemory.h"
#include <type_traits>


namespace eg {


// The log is a journal of fixed-size records. There is a header for each flash page, followed 
// by up to as many records as will fit into the page. When the current page is full we start 
// filling the next page. If the next page is already full, we erase it first (it is the oldest 
// data). The set of pages in the flash are used as a kind of ring buffer.
//
// TODO_AC Create a concrete base class to reduce code duplication. The log doesn't care about the 
// record type other than its size. It is just memcpy'd in and out.
template <typename Record, uint32_t kWriteSize>
class FlashLog : public NonCopyable
{
public:    
    static_assert(std::is_trivially_copyable_v<Record>);
    static_assert((sizeof(Record) % kWriteSize) == 0);
    static constexpr uint32_t kUnusedWord    = 0xFFFF'FFFF;
    static constexpr uint32_t kInvalidPage   = kUnusedWord;
    static constexpr uint32_t kInvalidIndex  = kUnusedWord;
    static constexpr uint32_t kInvalidOffset = kUnusedWord;

    // Each page in the log with any data at all has a header record at the beginning.
    // This makes it simpler to keep track of which pages hold the oldest and newest data.
    // It's a little wasteful if kWriteSize is larger than a uint32_t, but STM32U5 is the 
    // only part I've ever seen this on (so far).
    static_assert(kWriteSize >= sizeof(uint32_t));
    struct alignas(kWriteSize) Header
    {
        uint32_t index{};
        uint32_t unused1{kUnusedWord};
        uint32_t unused2{kUnusedWord};
        uint32_t unused3{kUnusedWord};
    };
    static_assert(std::is_trivially_copyable_v<Header>);
    static_assert((sizeof(Header) % kWriteSize) == 0);

    using Result = IFlashMemory::Result; 

public:    
    FlashLog(IFlashMemory& flash)
    : m_flash{flash}
    {
        initialise();
    }

    Result append_record(const Record& record)
    {
        uint32_t page_size    = m_flash.get_page_size(m_current_page);
        uint32_t page_address = m_flash.get_page_address(m_current_page);

        if ((m_current_offset + sizeof(Record)) > page_size)
        {
            // The page is full. Additional work is needed.
            Header header;

            // Determine the index of the next page.
            m_flash.read(page_address, reinterpret_cast<uint8_t*>(&header), sizeof(header));
            uint32_t next_index = header.index + 1;            

            // Advance to the next page. This may wrap around to the zeroth page.
            uint32_t page_count = m_flash.get_page_count();
            m_current_page   = (m_current_page + 1) % page_count;
            m_current_offset = sizeof(Header);

            // Test whether we need to erase the page. That is, if it already contains data.
            page_address = m_flash.get_page_address(m_current_page);
            m_flash.read(page_address, reinterpret_cast<uint8_t*>(&header), sizeof(header));            
            if (header.index != kInvalidIndex)
            {
                m_flash.erase_address(page_address);
            }

            // Now write the page header. 
            header.index = next_index;
            //std::cout << "header " << std::hex << header.index << std::endl;
            m_flash.write(page_address, reinterpret_cast<const uint8_t*>(&header), sizeof(header));
        }

        // Finally write the new record.
        //std::cout << "record " << std::hex << page_address << " " << m_current_offset << std::endl;
        m_flash.write(page_address + m_current_offset, reinterpret_cast<const uint8_t*>(&record), sizeof(Record)); 
        m_current_offset += sizeof(Record); 

        return Result::eOK;          
    }

    const Record* get_pointer_by_index(uint32_t index) const
    {
        if (index > get_record_count()) return nullptr;

        uint32_t size = get_records_per_page();
        uint32_t page = get_oldest_page();

        // TODO_AC This assumes we have pages all the same size. Reasonable in context.
        page  = (page + index / size) % m_flash.get_page_count();
        index = index % size; 

        uint32_t offset = sizeof(Header) + index * sizeof(Record);
        // TODO_AC Have we already constrained the alignment enough to not need memcpy?
        return reinterpret_cast<const Record*>(m_flash.get_data(page, offset));
    }

    Result read_record_by_index(uint32_t index, Record& record) const
    {
        if (index > get_record_count()) return Result::eInvalidOffset;

        uint32_t size = get_records_per_page();
        uint32_t page = get_oldest_page();

        // TODO_AC This assumes we have pages all the same size. Reasonable in context.
        page  = (page + index / size) % m_flash.get_page_count();
        index = index % size; 

        uint32_t offset = sizeof(Header) + index * sizeof(Record);
        m_flash.read(page, offset, reinterpret_cast<uint8_t*>(&record), sizeof(Record));
        return Result::eOK;
    }

    Result erase_all_records()
    {
        uint32_t page_count = m_flash.get_page_count();
        for (uint32_t page = 0; page < page_count; ++page)
        {
            m_flash.erase_page(page);
        }
        return initialise();
    }

    uint32_t get_records_per_page() const
    {
        // TODO_AC This assumes we have pages all the same size. Reasonable in context.
        return (m_flash.get_page_size(0) - sizeof(Header)) / sizeof(Record);
    }

    uint32_t get_oldest_page() const 
    {
        uint32_t oldest_page  = kInvalidPage;
        uint32_t oldest_index = kInvalidIndex;

        Header   header;
        uint32_t page_count = m_flash.get_page_count();
        for (uint32_t page = 0; page < page_count; ++page)
        {
            uint32_t address = m_flash.get_page_address(page);
            m_flash.read(address, reinterpret_cast<uint8_t*>(&header), sizeof(header));            
            if (header.index == kInvalidIndex) continue;
            if ((oldest_index == kInvalidIndex) || (header.index < oldest_index))
            {
                oldest_index = header.index;
                oldest_page  = page; 
            }
        }
        return oldest_page;
    }

    uint32_t get_newest_page() const 
    {
        uint32_t newest_page  = kInvalidPage;
        uint32_t newest_index = kInvalidIndex;

        Header   header;
        uint32_t page_count = m_flash.get_page_count();
        for (uint32_t page = 0; page < page_count; ++page)
        {
            uint32_t address = m_flash.get_page_address(page);
            m_flash.read(address, reinterpret_cast<uint8_t*>(&header), sizeof(header));            
            if (header.index == kInvalidIndex) continue;
            if ((newest_index == kInvalidIndex) || (header.index > newest_index))
            {
                newest_index = header.index;
                newest_page  = page; 
            }
        }
        return newest_page;
    }

    uint32_t get_record_count() const
    {
        // TODO_AC This assumes we have pages all the same size. Reasonable in context.
        const uint32_t kRecordsPerPage = get_records_per_page();

        // We can work this out from the oldest page, current page and current offset.
        uint32_t result = 0;
        uint32_t page   = get_oldest_page();
        while (page != m_current_page)
        {
            result += kRecordsPerPage;
            page = (page + 1) % m_flash.get_page_count(); 
        }

        result += (m_current_offset - sizeof(Header)) / sizeof(Record);
        return result;
    }

private:
    Result initialise() 
    {
        // Better to make these static assertions but we would have to template IFlashMemory 
        // Perhaps that's a good idea since we don't really need dynamic polymorphism.
        // Assert that kWriteSize matches m_flash.get_write_size().
        // Assert that (sizeof(Record) - sizeof(Header)) <= m_flash.get_page_size().

        //uint32_t oldest_page = get_oldest_page();
        uint32_t newest_page = get_newest_page();
        // uint32_t oldest_page = kInvalidPage;
        // uint32_t newest_page = kInvalidPage;
        // Header   header;
        // uint32_t page_count = m_flash.get_page_count();
        // for (uint32_t page = 0; page < page_count; ++page)
        // {
        //     uint32_t address = m_flash.get_page_address(page);
        //     m_flash.read(address, reinterpret_cast<uint8_t*>(&header), sizeof(header));            
        //     if (header.index == kInvalidIndex) continue;
        //     oldest_page = ((oldest_page == kInvalidPage) || (oldest_page > page)) ? page : oldest_page; 
        //     newest_page = ((newest_page == kInvalidPage) || (newest_page < page)) ? page : newest_page; 
        // }

        if (newest_page == kInvalidPage)
        {
            // The flash is completely empty, so create a header in the first page.
            // m_current_offset is set to the write location of the next (i.e. first) log record.  
            m_current_page   = 0;
            m_current_offset = sizeof(Header);

            Header header{};
            header.index = 0;
            uint32_t address = m_flash.get_page_address(m_current_page);
            //std::cout << "header " << std::hex << header.index << std::endl;
            m_flash.write(address, reinterpret_cast<const uint8_t*>(&header), sizeof(Header));
        }
        else 
        {
            // There is at least one page with records in it (at least a header). 
            m_current_page   = newest_page;
            m_current_offset = sizeof(Header);

            // Walk the page to find the place where the next log record would be written. 
            uint32_t page_size    = m_flash.get_page_size(m_current_page);
            uint32_t page_address = m_flash.get_page_address(m_current_page);
            while (m_current_offset < page_size)
            {
                constexpr uint32_t kWords = kWriteSize / sizeof(uint32_t);
                
                // We don't need to read the whole record, but just want to check whether a record 
                // is present. Reading one chunk of kWriteSize should be sufficient.
                uint32_t buffer[kWords];
                m_flash.read(page_address + m_current_offset, reinterpret_cast<uint8_t*>(&buffer[0]), kWriteSize);
                uint32_t test = kUnusedWord; 
                for (uint32_t w = 0; w < kWords; ++w)
                    test &= buffer[w];
                if (test == kUnusedWord)
                    break;           

                // Advance to the next record.
                m_current_offset += sizeof(Record);
            }

            // At this point m_current_offset points to the byte after the end of the last record. This could 
            // be the first byte of the next page, or it could be too close to the end of the page to fit another 
            // record in (same thing). In this case, do nothing now, but when it is time to write a record, we will
            // advance to the next page and start writing in it (that page may need to be erased before writing the 
            // header).
        }
        return Result::eOK; 
    }

private:
    IFlashMemory& m_flash;
    uint32_t m_current_page{kInvalidPage};
    uint32_t m_current_offset{kInvalidOffset};
};


} // namespace eg {
