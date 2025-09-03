/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "FlashMemory.h"
#include "stm32u5xx_hal_flash.h"
#include "logging/Assert.h"
#include "logging/Logger.h"
#include <cstring>


namespace eg {


using Result = IFlashMemory::Result;


uint32_t STM32U5FlashMemoryBase::get_page_count() const
{
    return m_page_count;
}


uint32_t STM32U5FlashMemoryBase::get_page_index(uint32_t address) const
{
    // All our pages are the same size, so converting an address to a page index
    // is simple integer arithmetic. First remove the base address offset.
    if (address < m_base_address) return kInvalidArgument;
    address -= m_base_address;
    if (address >= (m_page_count * kPageSize)) return kInvalidArgument;
    return address / kPageSize;
}


uint32_t STM32U5FlashMemoryBase::get_bank_address(uint32_t address) const
{
    if ((m_base_address >= kBank1Address) && (m_base_address < (kBank1Address + kBank1Size))) 
        return kBank1Address;
    if ((m_base_address >= kBank2Address) && (m_base_address < (kBank2Address + kBank2Size))) 
        return kBank2Address;
    return kInvalidArgument;
}


uint32_t STM32U5FlashMemoryBase::get_page_address(uint32_t page) const
{
    // All our pages are the same size, so converting an index to a page address 
    // is simple integer arithmetic. Remember to add the base address offset.
    if (page >= m_page_count) return kInvalidArgument;
    return m_base_address + page * kPageSize;
}


uint32_t STM32U5FlashMemoryBase::get_page_size(uint32_t page) const
{
    return kPageSize;
}


uint32_t STM32U5FlashMemoryBase::get_write_size() const
{
    return kWriteSize;
}


Result STM32U5FlashMemoryBase::erase_address(uint32_t address)
{
    // Page index within our block of pages rather than from the beginning of the 
    // flash bank. 
    auto page = get_page_index(address);
    if (page == kInvalidArgument) return Result::eInvalidAddress;

    return erase_page(page);
}


Result STM32U5FlashMemoryBase::erase_page(uint32_t page)
{
    auto page_address = get_page_address(page);
    if (page_address == kInvalidArgument) return Result::eInvalidPage;

    auto result = HAL_FLASH_Unlock();
    if (result == HAL_OK)
    {
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

        // Calculate the index of the page within the entire bank.
        auto bank_address = get_bank_address(page_address);
        auto bank_page    = (page_address - bank_address) / kPageSize;

        // Configure the erase. All pages which we might erase are located in flash bank 2.
        // TODO_AC Add an assertion for this on the address range in the constructor?
        FLASH_EraseInitTypeDef info = {};
        info.TypeErase = FLASH_TYPEERASE_PAGES;
        info.NbPages   = 1;        
        info.Banks     = FLASH_BANK_2;
        info.Page      = bank_page;

        // Do the erase (this blocks) 
        // TODO_AC What is the error value? Log it or something.
        uint32_t error;
        result = HAL_FLASHEx_Erase(&info, &error);

        HAL_FLASH_Lock();
    }

    return (result == HAL_OK) ? Result::eOK : Result::eFlashFailed;
}


Result STM32U5FlashMemoryBase::write(uint32_t address, const uint8_t* data, uint32_t size)
{
    if (get_page_index(address) == kInvalidArgument) return Result::eInvalidAddress;
    if (((address % kPageSize) + size) > kPageSize)  return Result::eInvalidSize;
    if (size % kWriteSize != 0)    return Result::eUnalignedSize;
    if (address % kWriteSize != 0) return Result::eUnalignedAddress;

    uint32_t data_address = reinterpret_cast<uint32_t>(data);
    uint32_t data_written = 0;

    // Clear any previous errors
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    auto result  = HAL_FLASH_Unlock();
    while ((result == HAL_OK) && (data_written < size))
    {
        result         = HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, address, data_address);
        data_written  += kWriteSize; // 128 bits quad word
        data_address  += kWriteSize;
        address       += kWriteSize;
    }

    HAL_FLASH_Lock();

    return (result == HAL_OK) ? Result::eOK : Result::eFlashFailed;
}


Result STM32U5FlashMemoryBase::write(uint32_t page, uint32_t offset, const uint8_t* data, uint32_t size)
{
    auto page_address = get_page_address(page);
    if (page_address == kInvalidArgument) return Result::eInvalidPage; 
    return write(page_address + offset, data, size);
}


Result STM32U5FlashMemoryBase::read(uint32_t address, uint8_t* data, uint32_t size) const
{
    if (get_page_index(address) == kInvalidArgument) return Result::eInvalidAddress;
    if (((address % kPageSize) + size) > kPageSize)  return Result::eInvalidSize;
    //if (size % kWriteSize != 0)    return eUnalignedSize;
    //if (address % kWriteSize != 0) return eUnalignedAddress;

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    // No need for anything special to read the flash.
    const uint8_t* flash_data = get_data(address);
    std::memcpy(data, flash_data, size);

    return Result::eOK;
}


Result STM32U5FlashMemoryBase::read(uint32_t page, uint32_t offset, uint8_t* data, uint32_t size) const
{
    auto page_address = get_page_address(page);
    if (page_address == kInvalidArgument) return Result::eInvalidPage;
    return read(page_address + offset, data, size);
}


const uint8_t* STM32U5FlashMemoryBase::get_data(uint32_t address) const
{
    if (get_page_index(address) == kInvalidArgument) return nullptr;
    return reinterpret_cast<const uint8_t*>(address);
}


const uint8_t* STM32U5FlashMemoryBase::get_data(uint32_t page, uint32_t offset) const
{
    auto page_address = get_page_address(page);
    if (page_address == kInvalidArgument) return nullptr;
    return get_data(page_address + offset);
}


} // namespace eg {
