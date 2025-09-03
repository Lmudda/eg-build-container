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
#include <cstdint>


namespace eg
{


// Simple API to abstract flash memory. An implementation could represent some or all of the 
// pages in a flash memory on a microcontroller, or might be a mock to simulate a flash. 
// Different types of non-volatile storage can be built on top of an IFlashMemory.
//
// TODO_AC Perhaps abstract out IFlashDriver to do the actual writes and erases. Then the flash
// memory (a bunch of pages) is hardware agnostic and can be moved to portable repo.
class IFlashMemory : public NonCopyable
{
public:
    enum class Result
    {
        eOK, 
        eInvalidAddress, 
        eInvalidPage, 
        eInvalidOffset, 
        eInvalidSize,     
        eUnalignedAddress, 
        eUnalignedSize,
        eFlashFailed,
    };
    static constexpr uint32_t kInvalidArgument = 0xFFFF'FFFF;
    static constexpr uint32_t kInvalidAddress  = 0xFFFF'FFFF;

public:
    virtual ~IFlashMemory() = default;

    // get_page_count() returns the numbers of flash pages (aka sectors) that are
    // managed by the IFlashMemory instance. The IFlashMemory instance may manage a
    // subset of the flash pages available in the microcontroller.
    //
    // Note that the pages need not all be the same size, nor even contiguous. 
    virtual uint32_t get_page_count() const = 0;

    // Given a memory address, get_page_index() returns the index of the flash sector
    // that the address is located in. The page index is relative to the first page
    // that the IFlashMemory instance is managing - so page 0 returned by this method
    // may not refer to page 0 of the physial flash bank.
    //
    // If the address is outside the range managed by the IFlashMemory instance then this
    // method returns kInvalidArgument. This is the case even if the memory address is
    // within the microcontroller's flash address space.
    virtual uint32_t get_page_index(uint32_t address) const = 0;

    // Given a page index, get_page_address() returns the memory address of the start of
    // that page of flash memory. If the page is outside the range managed by the
    // IFlashMemory instance then this method returns kInvalidArgument.
    //
    // The page index is relative to the first page of flash that the IFlashMemory instance
    // is managing - not to the first page of physical flash in the bank. The IFlashMemory
    // instance may manage a subset of the flash pages available in the microcontroller
    virtual uint32_t get_page_address(uint32_t page) const = 0;

    // Return the size in bytes of the page index specified. If the page index is outside
    // the range managed by this IFlashMemory instance the method returns kInvalidArgument.
    //
    // The page index is relative to the first page of flash that the IFlashMemory instance
    // is managing - not to the first page of physical flash in the bank. The IFlashMemory
    // instance may manage a subset of the flash pages available in the microcontroller
    virtual uint32_t get_page_size(uint32_t page) const = 0;

    // Query the minimum write size of the flash memory, counted in bytes. It is not possible
    // write smaller blocks of data to the Flash memory than this size.
    virtual uint32_t get_write_size() const = 0;

    // Erase the flash page that contains the address passed in. If the address is outside
    // the range managed by this IFlashMemory instance the method returns eInvalidAddress.
    //
    // The IFlashMemory instance may manage a subset of the flash pages available in the
    // microcontroller, so an error may be returned even if the address is itself valid.
    virtual Result erase_address(uint32_t address) = 0;

    // Erase the flash page specified. If the page is outside the range managed by this
    // IFlashMemory instance the method returns eInvalidPage.
    //
    // The page index is relative to the first page of flash that the IFlashMemory instance
    // is managing - not to the first page of physical flash in the bank. The IFlashMemory
    // instance may manage a subset of the flash pages available in the microcontroller
    virtual Result erase_page(uint32_t page) = 0;

    // Write a chunk of data to flash.
    //
    // The IFlashMemory instance may manage a subset of the flash pages available in the
    // microcontroller, so an error may be returned even if the address is itself valid.
    //
    // The size of the write must be constrained to be a multiple of get_write_size().
    // Also the offset within a page must be aligned to get_write_size(). That is a detail of
    // the particular implementation.
    //
    // If the address is invalid the method returns eInvalidAddress.
    // If the region to be written crosses a page boundary, the method returns eInvalidSize
    // If the size is not aligned to get_write_size() the method returns eUnalignedSize
    // If the address is not aligned to get_write_size() the method returns eUnalignedAddress
    virtual Result write(uint32_t address, const uint8_t* data, uint32_t size) = 0;

    // Write a chunk of data to flash.
    //
    // The size of the write must be constrained to be a multiple of get_write_size().
    // Also the offset within a page must be aligned to get_write_size(). That is a detail of
    // the particular implementation.
    //
    // The page index is relative to the first page of flash that the IFlashMemory instance
    // is managing - not to the first page of physical flash in the bank. The IFlashMemory
    // instance may manage a subset of the flash pages available in the microcontroller
    //
    // If the address is invalid the method returns eInvalidAddress.
    // If the region to be written crosses a page boundary, the method returns eInvalidSize
    // If the size is not aligned to get_write_size() the method returns eUnalignedSize
    // If the address is not aligned to get_write_size() the method returns eUnalignedAddress
    virtual Result write(uint32_t page, uint32_t offset, const uint8_t* data, uint32_t size) = 0;

    // Read a chunk of data from a flash page.
    //
    // Virtually all microcontrollers allow byte-aligned reads. If a device does not allow
    // this, then this API will need to be updated to add a get_read_size() method to return
    // the minimum read size in bytes. Then if a call to read() is not aligned to the necessary
    // size an error eUnalignedAddress or eUnalignedSize will be returned, as required.
    //
    // The IFlashMemory instance may manage a subset of the flash pages available in the
    // microcontroller, so an error may be returned even if the address is itself valid.
    //
    // If the address is invalid the method returns eInvalidAddress.
    // If the region to be written crosses a page boundary, the method returns eInvalidSize
    virtual Result read(uint32_t address, uint8_t* data, uint32_t size) const = 0;

    // Read a chunk of data from a flash page.
    //
    // The page index is relative to the first page of flash that the IFlashMemory instance
    // is managing - not to the first page of physical flash in the bank. The IFlashMemory
    // instance may manage a subset of the flash pages available in the microcontroller
    virtual Result read(uint32_t page, uint32_t offset, uint8_t* data, uint32_t size) const = 0;

    // Direct access to the flash as a pointer to data. If the address is invalid nullptr
    // is returned.
    //
    // The IFlashMemory instance may manage a subset of the flash pages available in the
    // microcontroller, so an error may be returned even if the address is itself valid.
    virtual const uint8_t* get_data(uint32_t address) const = 0;

    // Direct access to the flash as a pointer to data. If the address is invalid nullptr
    // is returned.
    //
    // The page index is relative to the first page of flash that the IFlashMemory instance
    // is managing - not to the first page of physical flash in the bank. The IFlashMemory
    // instance may manage a subset of the flash pages available in the microcontroller
    virtual const uint8_t* get_data(uint32_t page, uint32_t offset) const = 0;
};


} // namespace eg {

