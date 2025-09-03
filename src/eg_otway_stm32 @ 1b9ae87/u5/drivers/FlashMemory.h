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
#include "logging/Assert.h"


namespace eg {


// The class abstracts a region of the physical flash on the STM32. The STM32U5 flash comprises 
// two banks of contiguous 8KB pages with a minimum write size of 16 bytes (the device associates
// some error correction bits with each blocks of 128 bits of data).
class STM32U5FlashMemoryBase : public IFlashMemory
{
public:
    static constexpr uint32_t kPageSize     = 8192U;
    static constexpr uint32_t kWriteSize    = 16U;
    static_assert((kPageSize % kWriteSize) == 0);

    // TODO_AC These constants could be made into template arguments to support different 
    // devices. For now we are supporting STM32U575. 
    static constexpr uint32_t kBank1Address = 0x08000000;
    static constexpr uint32_t kBank1Pages   = 128;
    static constexpr uint32_t kBank1Size    = kBank1Pages * kPageSize;
    static constexpr uint32_t kBank2Address = 0x08100000;
    static constexpr uint32_t kBank2Pages   = 128;
    static constexpr uint32_t kBank2Size    = kBank2Pages * kPageSize;

    STM32U5FlashMemoryBase(uint32_t base_address, uint32_t page_count)
    : m_base_address{base_address}
    , m_page_count{page_count}
    {
    }

    // Note that the pages need not all be the same size, nor even contiguous. 
    uint32_t get_page_count() const override;
    uint32_t get_page_index(uint32_t address) const override;
    uint32_t get_page_address(uint32_t page) const override;
    uint32_t get_page_size(uint32_t page) const override;
    uint32_t get_write_size() const override;

    // Erase the page by index or by any address lying within the page.
    Result erase_address(uint32_t address) override;
    Result erase_page(uint32_t page) override;

    // Write a chunk of data into a flash page.
    Result write(uint32_t address, const uint8_t* data, uint32_t size) override;
    Result write(uint32_t page, uint32_t offset, const uint8_t* data, uint32_t size) override;

    // Read a chunk of data from a flash page. 
    Result read(uint32_t address, uint8_t* data, uint32_t size) const override;
    Result read(uint32_t page, uint32_t offset, uint8_t* data, uint32_t size) const override;
    
    // Direct access to the flash as a pointer to data.
    const uint8_t* get_data(uint32_t address) const override;
    const uint8_t* get_data(uint32_t page, uint32_t offset) const override;

private:
    uint32_t get_bank_address(uint32_t address) const;

private:
    const uint32_t m_base_address;
    const uint32_t m_page_count;
};


// The purpose of this template is just to provide a little compile-time assurance that the 
// underlying block of flash begins on a page boundary.
template <uint32_t kBaseAddress, uint32_t kPageCount>
class STM32U5FlashMemory : public STM32U5FlashMemoryBase
{
public:
    static_assert((kBaseAddress % STM32U5FlashMemoryBase::kPageSize) == 0, 
        "Flash base address should be a multiple of kPageSize");
    static_assert( 
        ((kBaseAddress >= kBank1Address) && (kBaseAddress < (kBank1Address + kBank1Size)) && (kPageCount <= kBank1Pages)) || 
        ((kBaseAddress >= kBank2Address) && (kBaseAddress < (kBank2Address + kBank2Size)) && (kPageCount <= kBank2Pages)), 
        "Flash base address should be in supported range");

    STM32U5FlashMemory()
    : STM32U5FlashMemoryBase(kBaseAddress, kPageCount)
    {        
    }
};


} // namespace eg {

