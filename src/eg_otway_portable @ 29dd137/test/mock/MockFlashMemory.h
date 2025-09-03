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
#include <cstdint>
#include <cstring>
#include <array>
#include <bit>


namespace eg
{


// Simulated flash which represents a contiguous array of fixed-size pages, based
// at some offset. This model is useful for simulating something like the STM32U5 or STM32G4
// flash layout. For variable length pages we might pass an array of page definitions to
// the constructor. That's for another implementation, but should not be hard to do.
template <uint32_t kBaseAddress, uint32_t kPageSize, uint32_t kNumPages, uint32_t kWriteSize>
class MockFlashMemory : public IFlashMemory
{
static_assert(std::has_single_bit(kPageSize));   // It is a power of 2 - STM32G4=2048, STM32U5=8192
static_assert(std::has_single_bit(kWriteSize));  // It is a power of 2 - STM32G4=4, STM32U5=16
static_assert((kBaseAddress % kPageSize) == 0); // This wouldn't be true in general
static_assert(kNumPages > 0);                    // Maybe this should be at least 2 pages

public:
    MockFlashMemory()
    {
        std::memset(m_data.data(), 0xFF, kMemorySize);
    }

    uint32_t get_page_count() const override
    {
        return kNumPages;
    }

    uint32_t get_page_index(uint32_t address) const override
    {
        if (address < kBaseAddress) return kInvalidArgument;
        address -= kBaseAddress;
        if (address >= kMemorySize) return kInvalidArgument;
        return address / kPageSize;
    }

    uint32_t get_page_address(uint32_t page) const override
    {
        if (page >= kNumPages) return kInvalidArgument;
        return kBaseAddress + page * kPageSize;
    }

    uint32_t get_page_size([[maybe_unused]] uint32_t page) const override
    {
        return kPageSize;
    }

    uint32_t get_write_size() const override
    {
        return kWriteSize;
    }

    const uint8_t* get_data(uint32_t address) const override
    {
        if (address < kBaseAddress) return nullptr;
        address -= kBaseAddress;
        if (address >= kMemorySize) return nullptr;
        return &m_data[address];
    }

    const uint8_t* get_data(uint32_t page, uint32_t offset) const override
    {
        if (page >= kNumPages) return nullptr;
        if (offset >= kPageSize) return nullptr;
        return get_data(get_page_address(page) + offset);
    }

    Result write(uint32_t address, const uint8_t* data, uint32_t size) override
    {
        if (address < kBaseAddress) return Result::eInvalidAddress;
        address -= kBaseAddress;

        if (address >= kMemorySize) return Result::eInvalidAddress;
        if ((address + size) > kMemorySize) return Result::eInvalidSize;

        if ((address % kWriteSize) != 0) return Result::eUnalignedAddress;
        if ((size % kWriteSize) != 0) return Result::eUnalignedSize;

        for (uint32_t i = 0; i < size; ++i)
            // codechecker_intentional [core.uninitialized.Assign] static analyser worries that data is &=ing uninitialised stuff. OK in test environment.
            m_data[address + i] &= data[i];
        return Result::eOK;
    }

    Result write(uint32_t page, uint32_t offset, const uint8_t* data, uint32_t size) override
    {
        if (page >= kNumPages) return Result::eInvalidPage;
        if (offset >= kPageSize) return Result::eInvalidOffset;
        return write(get_page_address(page) + offset, data, size);
    }

    Result read(uint32_t address, uint8_t* data, uint32_t size) const override
    {
        if (address < kBaseAddress) return Result::eInvalidAddress;
        address -= kBaseAddress;

        if (address >= kMemorySize) return Result::eInvalidAddress;
        if ((address + size) > kMemorySize) return Result::eInvalidSize;

        // Do we need to care about this for reading?
        // if ((address % kWriteSize) != 0) return Result::eUnalignedAddress;
        // if ((size % kWriteSize) != 0) return Result::eUnalignedSize;

        for (uint32_t i = 0; i < size; ++i)
            data[i] = m_data[address + i];
        return Result::eOK;
    }

    Result read(uint32_t page, uint32_t offset, uint8_t* data, uint32_t size) const override
    {
        if (page >= kNumPages) return Result::eInvalidPage;
        if (offset >= kPageSize) return Result::eInvalidOffset;
        return read(get_page_address(page) + offset, data, size);
    }

    Result erase_address(uint32_t address) override
    {
        //std::cout << "erase " << std::hex << address << std::endl;
        if (address < kBaseAddress) return Result::eInvalidAddress;
        address -= kBaseAddress;

        if (address >= kMemorySize) return Result::eInvalidAddress;

        address = (address / kPageSize) * kPageSize;
        std::memset(&m_data[address], 0xFF, kPageSize);
        return Result::eOK;
    }

    Result erase_page(uint32_t page) override
    {
        if (page >= kNumPages) return Result::eInvalidPage;
        return erase_address(get_page_address(page));
    }

private:
    static constexpr uint32_t kMemorySize = kPageSize * kNumPages;
    std::array<uint8_t, kMemorySize> m_data{};
};


} // namespace eg {
