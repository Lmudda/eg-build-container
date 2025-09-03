/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2025.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cstring>

#include "stm32h7xx_hal_flash.h"

#include "utilities/NonCopyable.h"
#include "interfaces/IFlashMemory.h"
#include "logging/Assert.h"


namespace eg {


// The class abstracts a region of the physical flash on the STM32.
// The template parameters are:
// - kPageSize The size of each sector of flash memory (the size varies between devices, but on each device all pages are equal size)
// - kphysicalPageCount the number of sectors of flash within each bank. This is not the same sas the number of pages managed by this instance of the class
// - kNumBanks The number of banks of flash (up to two)
// - kWriteSize The minimum block size for writing to flash (in bytes)
// - kDeviceId The expected processor ID, which is checked against the value in the CPU's DBGMCU register
template <uint32_t kPageSize, uint32_t kPhysicalPageCount, uint32_t kNumBanks, uint32_t kWriteSize, uint32_t kDeviceId>
class STM32H7FlashMemoryBase : public IFlashMemory
{
public:
    // Check the number of banks is valid
    static_assert(kNumBanks <= 2, "There can be no more than two flash banks defined");

    // The minimum block size for writing flash memory
    static_assert((kPageSize % kWriteSize) == 0);

    // The total size of each bank of flash
    static constexpr uint32_t kBankSize = kPhysicalPageCount * kPageSize;

    STM32H7FlashMemoryBase(uint32_t base_address, uint32_t page_count)
    : m_base_address{base_address}
    , m_page_count{page_count}
    {
        // The device ID is in the first 12 bits of the IDCODE register. See the "Microcontroller debug unit (DBGMCU)"
        // data in the reference manual for details. The ID numbers are taken from the respective reference menuals.
        uint32_t dev_id = DBGMCU->IDCODE & 0xFFFu;
        EG_ASSERT(dev_id == kDeviceId, "Invalid flash memory controller instantiated - incompatible CPU");
    }

    uint32_t get_page_count() const override
    {
        return m_page_count;
    }

    uint32_t get_page_index(uint32_t address) const override
    {
        // All our pages are the same size, so converting an address to a page index
        // is simple integer arithmetic. First remove the base address offset.
        if (address < m_base_address) return kInvalidArgument;
        address -= m_base_address;
        if (address >= (m_page_count * kPageSize)) return kInvalidArgument;
        return address / kPageSize;
    }

    uint32_t get_page_address(uint32_t page) const override
    {
        // All our pages are the same size, so converting an index to a page address
        // is simple integer arithmetic. Remember to add the base address offset.
        if (page >= m_page_count) return kInvalidArgument;
        return m_base_address + page * kPageSize;
    }

    uint32_t get_page_size([[maybe_unused]] uint32_t page) const override
    {
        return kPageSize;
    }

    uint32_t get_write_size() const override
    {
        return kWriteSize;
    }

    Result erase_address(uint32_t address) override
    {
        // Page index within our block of pages rather than from the beginning of the
        // flash bank.
        auto page = get_page_index(address);
        if (page == kInvalidArgument) return Result::eInvalidAddress;

        return erase_page(page);
    }

    Result erase_page(uint32_t page) override {
        auto page_address = get_page_address(page);
        if (page_address == kInvalidArgument) return Result::eInvalidPage;

        auto result = HAL_FLASH_Unlock();
        if (result == HAL_OK)
        {
            __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK1);
            if (kNumBanks == 2)
            {
                __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK2);
            }

            // Calculate the index of the page within the entire bank.
            auto bank_address = get_bank_address();
            auto bank_page    = (page_address - bank_address) / kPageSize;

            // Configure the erase. All pages which we might erase are located in flash bank 2.
            // TODO_AC Add an assertion for this on the address range in the constructor?
            FLASH_EraseInitTypeDef info = { };
            info.TypeErase    = FLASH_TYPEERASE_SECTORS;
            info.NbSectors    = 1;
            info.Banks        = get_bank_number();
            info.Sector       = bank_page;
            info.VoltageRange = FLASH_VOLTAGE_RANGE_4;

            // Do the erase (this blocks)
            // TODO_AC What is the error value? Log it or something.
            uint32_t error;
            result = HAL_FLASHEx_Erase(&info, &error);

            HAL_FLASH_Lock();
        }

        return (result == HAL_OK) ? Result::eOK : Result::eFlashFailed;
    }


    Result write(uint32_t address, const uint8_t* data, uint32_t size) override
    {
        if (get_page_index(address) == kInvalidArgument) return Result::eInvalidAddress;
        if (((address % kPageSize) + size) > kPageSize)  return Result::eInvalidSize;
        if (size % kWriteSize != 0)    return Result::eUnalignedSize;
        if (address % kWriteSize != 0) return Result::eUnalignedAddress;

        uint32_t data_address = reinterpret_cast<uint32_t>(data);
        uint32_t data_written = 0;

        // Clear any previous errors
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK1);
        if (kNumBanks == 2)
        {
            __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK2);
        }

        auto result  = HAL_FLASH_Unlock();
        while ((result == HAL_OK) && (data_written < size))
        {
            // Note that the first parameter in the following function call is actually unused unless
            // writing to OTP memory (which we're not doing), so its value is unimportant. According to
            // the comments for HAL_FLASH_Program() writes have to be 256-bit aligned for the STM32H74x.
            result         = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, data_address);
            data_written  += kWriteSize;
            data_address  += kWriteSize;
            address       += kWriteSize;
        }

        HAL_FLASH_Lock();

        return (result == HAL_OK) ? Result::eOK : Result::eFlashFailed;
    }

    Result write(uint32_t page, uint32_t offset, const uint8_t* data, uint32_t size) override
    {
        auto page_address = get_page_address(page);
        if (page_address == kInvalidArgument) return Result::eInvalidPage;
        return write(page_address + offset, data, size);
    }


    Result read(uint32_t address, uint8_t* data, uint32_t size) const override
    {
        if (get_page_index(address) == kInvalidArgument) return Result::eInvalidAddress;
        if (((address % kPageSize) + size) > kPageSize)  return Result::eInvalidSize;

        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK1);
        if (kNumBanks == 2)
        {
            __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK2);
        }

        // No need for anything special to read the flash.
        const uint8_t* flash_data = get_data(address);
        std::memcpy(data, flash_data, size);

        return Result::eOK;
    }

    Result read(uint32_t page, uint32_t offset, uint8_t* data, uint32_t size) const override
    {
        auto page_address = get_page_address(page);
        if (page_address == kInvalidArgument) return Result::eInvalidPage;
        return read(page_address + offset, data, size);
    }

    const uint8_t* get_data(uint32_t address) const override
    {
        if (get_page_index(address) == kInvalidArgument) return nullptr;
        return reinterpret_cast<const uint8_t*>(address);
    }

    const uint8_t* get_data(uint32_t page, uint32_t offset) const override
    {
        auto page_address = get_page_address(page);
        if (page_address == kInvalidArgument) return nullptr;
        return get_data(page_address + offset);
    }


private:

    // Get the memory address of the start of the physical bnk of flash memory that
    // this instance manages.
    uint32_t get_bank_address() const
    {
        if ((m_base_address >= kBankAddress[0]) && (m_base_address < (kBankAddress[0] + kBankSize)))
        {
            return kBankAddress[0];
        }

        if ((kNumBanks == 2)
            && (m_base_address >= kBankAddress[1]) && (m_base_address < (kBankAddress[1] + kBankSize)))
        {
            return kBankAddress[1];
        }

        return kInvalidArgument;
    }


    // Get the bank number for the flash region managed by this object. This is required when
    // erasing flash.
    uint32_t get_bank_number() const
    {
        if ((m_base_address >= kBankAddress[0]) && (m_base_address < (kBankAddress[0] + kBankSize)))
        {
            return FLASH_BANK_1;
        }

        if ((kNumBanks == 2)
            && (m_base_address >= kBankAddress[1]) && (m_base_address < (kBankAddress[1] + kBankSize)))
        {
            return FLASH_BANK_2;
        }

        // This is an invalid bank number
        return 0;
    }


protected:
    // The address in memory of the flash banks (for devices with one bank,
    // only the first element is applicable).
    static constexpr uint32_t kBankAddress[2]{0x08000000u, 0x08100000u};

private:

    // The base address for this instance. This is not necesasrily the base address
    // of a bank of flash - the instance could be restricted to s subsection of a bank.
    const uint32_t m_base_address;

    // The number of pages of flash memory managed by this instance. This is not
    // necessarily the same as the number of pages in the bank of flash.
    const uint32_t m_page_count;
};


// Class to manage flash memory on STM32H745/755 and STM32H747/757 devices. The device ID for
// these CPUs is 0x450 (see section 63.5.8 of the reference manual).
template <uint32_t kBaseAddress, uint32_t kPageCount>
class STM32H74xFlashMemory : public STM32H7FlashMemoryBase<0x20000u, 8u, 2u, 32u, 0x450u>
{
public:
    static_assert((kBaseAddress % 0x20000u) == 0,
        "Flash base address should be a multiple of kPageSize");
    static_assert(kPageCount <= 8u, "Flash page count should be in supported range");
    static_assert(
        ((kBaseAddress >= kBankAddress[0]) && (kBaseAddress < (kBankAddress[0] + kBankSize))) ||
        ((kBaseAddress >= kBankAddress[1]) && (kBaseAddress < (kBankAddress[1] + kBankSize))),
        "Flash base address should be in supported range");

    STM32H74xFlashMemory()
        : STM32H7FlashMemoryBase(kBaseAddress, kPageCount)
    {
    }
};


// Class to manage flash memory on STM32H7A3/7B3/7B0 devices. The device ID for these CPUs is
// 0x480 (see section 64.5.7 of the reference manual).
template <uint32_t kBaseAddress, uint32_t kPageCount>
class STM32H7AxFlashMemory : public STM32H7FlashMemoryBase<0x400u, 128u, 2u, 16u, 0x480u>
{
public:
    static_assert((kBaseAddress % 0x400u) == 0,
        "Flash base address should be a multiple of kPageSize");
    static_assert(kPageCount <= 128u, "Flash page count should be in supported range");
    static_assert(
        ((kBaseAddress >= kBankAddress[0]) && (kBaseAddress < (kBankAddress[0] + kBankSize))) ||
        ((kBaseAddress >= kBankAddress[1]) && (kBaseAddress < (kBankAddress[1] + kBankSize))),
        "Flash base address should be in supported range");

    STM32H7AxFlashMemory()
        : STM32H7FlashMemoryBase(kBaseAddress, kPageCount)
    {
    }
};


// Class to manage flash memory on STM32H72x/STM32H73x devices. The device ID for these CPUs is
// 0x483 (see section 65.5.7 of the reference manual).
template <uint32_t kBaseAddress, uint32_t kPageCount>
class STM32H72xFlashMemory : public STM32H7FlashMemoryBase<0x20000u, 8u, 1u, 32u, 0x483u>
{
public:
    static_assert((kBaseAddress % 0x20000u) == 0,
        "Flash base address should be a multiple of kPageSize");
    static_assert(kPageCount <= 8u, "Flash page count should be in supported range");
    static_assert(
        (kBaseAddress >= kBankAddress[0]) && (kBaseAddress < (kBankAddress[0] + kBankSize)),
        "Flash base address should be in supported range");

    STM32H72xFlashMemory()
        : STM32H7FlashMemoryBase(kBaseAddress, kPageCount)
    {
    }
};

} // namespace eg {

