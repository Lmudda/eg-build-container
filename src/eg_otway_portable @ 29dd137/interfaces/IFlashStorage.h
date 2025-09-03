//  $Id: IFlashStorage.h 542 2018-12-07 14:33:31Z davidg $
//
//  (c) eg technology ltd, 2018.  All rights reserved.
//
//  This software is the property of eg technology ltd. and may not be copied or reproduced
//  otherwise than on to a single hard disk for backup or archival purposes. The source code is
//  confidential information and must not be disclosed to third parties or used without the
//  express written permission of eg technology ltd.

#ifndef I_FLASH_STORAGE_H_
#define I_FLASH_STORAGE_H_

#include <cstdint>
#include "signals/Signal.h"
#include "utilities/NonCopyable.h"

namespace eg
{
    /**
    * @brief Indicates the status of a flash operation
    */
    enum class FlashOperationStatus
    {
        None,
        Success,
        InvalidSector,               ///< The sector number is not valid for the device.
        InvalidOffset,               ///< The specified offset is not a valid value in the sector
        InvalidLength,               ///< The specified length of data from the offset will not fit in the sector.
        DataBufferIsNull,            ///< The specified data buffer is null
        FailedToAllocateWriteBuffer, ///< The write buffer failed to dynamically allocate, so all write operations will fail.
        Busy,                        ///< The flash device was busy
        Failed,                      ///< The flash operation failed for any other reason
        Aborted                      ///< The flash operation was aborted
    };

    enum class Operation
    {
        None,
        Read,
        Write,
        Erase,
    };

    struct OperationCompletion_t
    {
//        OperationCompletion_t(FlashOperationStatus status, Operation op, uint32_t sectorNum)
//            : Status(status), operation(op), SectorNumber(sectorNum)
//        {
//        }
        FlashOperationStatus Status;
        Operation Op;
        uint32_t SectorNumber;
    };

    /**
     * @brief The interface for a Flash Device
     */
    class IFlashStorage : private NonCopyable
    {
    public:
        /**
        * @brief Initializes the flash
        * @returns True if initialized successfully, otherwise false
        */
        virtual bool Initialize() = 0;

        /**
        * @brief Gets the number of available flash sectors
        * @returns the number of flash sectors
        */
        virtual uint32_t GetNumberOfSectors() = 0;

        /**
        * @brief Determines if the supplied sector number is valid or not
        * @param[in] sectorNumber The sector number to check for validity
        * @returns True if the sector number is a valid sector number, otherwise false
        */
        virtual bool IsValidSector(uint32_t sectorNumber) = 0;

        /**
        * @brief Determines if the supplied sector number is read only or not
        * @param[in] sectorNumber The sector number to check
        * @returns True if the sector is read only, otherwise false
        */
        virtual bool IsSectorReadOnly(uint32_t sectorNumber) = 0;

        /**
        * @brief Gets the start address of the flash sector
        * @param[in] sectorNumber The number of the flash sector  
        * @returns start address
        */
        virtual uint8_t* GetSectorStartAddress(uint32_t sectorNumber) = 0;

        /**
        * @brief Gets the size of the flash sector
        * @param[in] sectorNumber The number of the flash sector  
        * @returns size in bytes
        */
        virtual uint32_t GetSectorSize(uint32_t sectorNumber) = 0;

	    /**
        * @brief Gets the write size (the minimum size of data that can be written in a single operation)
        * @returns write size in bytes.
        */
	    virtual uint32_t GetWriteSize() = 0;
	    
        /**
        * @brief Erases an entire flash sector. Please use with care.
        * @param[in] sectorNumber The number of the flash sector to be erased
        * @param[in] callback A callback that will be called upon completion, giving an indication of success or failure. Might be called in interrupt context.
        */
	    virtual FlashOperationStatus EraseSector(uint32_t sectorNumber) = 0;

        /**
        * @brief Reads from a flash sector
        * @param[in] sectorNumber The number of the flash sector  
        * @param[in] offset The offset of where to read within the flash sector
        * @param[in] buffer The buffer to read into.
        * @param[in] lengthToRead The number of bytes to read
        * @param[in] callback A callback that will be called upon completion, giving an indication of success or failure. Might be called in interrupt context.
        */
	    virtual FlashOperationStatus Read(uint32_t sectorNumber, uint32_t offset, uint8_t* buffer, uint32_t lengthToRead) = 0;

        /**
        * @brief Writes to a flash sector
        * @param[in] sectorNumber The number of the flash sector  
        * @param[in] offset The offset of where to write to within the flash sector
        * @param[in] buffer The buffer containing the data to write
        * @param[in] lengthToWrite The number of bytes to write
        * @param[in] callback A callback that will be called upon completion, giving an indication of success or failure. Might be called in interrupt context.
        * 
        * The assumption is that the calling application code is responsible for first erasing a sector, using EraseSector(), before 
        * writing to it with this method.  The application code may choose to write to the erased sector however it wishes, perhaps with
        * multiple calls to Write() addressing different parts of sector, and for this reason the Write() method needs to take into account that
        * locations adjacent to where it is writing may already have data in that needs preserving.
        */
	    virtual FlashOperationStatus Write(uint32_t sectorNumber, uint32_t offset, const uint8_t* buffer, uint32_t lengthToWrite) = 0;
            
        /**
            * @brief Signals that the read/write or erase currently in progress should be aborted.
            */
        virtual void Abort() = 0;

	    /**
        * @brief Check and the result of the last operation and clear the status
        * 
        * Returns the staus of the last operation
        */
	    virtual FlashOperationStatus CheckAndClearStatus() = 0;

        /**
         * @brief A signal that is emitted whenever a flash operation is completed.
         */
        virtual SignalProxy<OperationCompletion_t> OnFlashOperationComplete() = 0;

        /**
            * @brief Destructor (default)
            */
        virtual ~IFlashStorage() = default;

    protected:
        /**
            * @brief Required interface constructor (default)
            */
        IFlashStorage() = default;	    
    };
}

#endif // I_FLASH_STORAGE_H_
