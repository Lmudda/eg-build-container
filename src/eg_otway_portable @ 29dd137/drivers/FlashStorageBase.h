//  $Id: FlashStorageBase.h 622 2019-05-08 07:26:55Z davidg $
//
//  (c) eg technology ltd, 2018.  All rights reserved.
//
//  This software is the property of eg technology ltd. and may not be copied or reproduced
//  otherwise than on to a single hard disk for backup or archival purposes. The source code is
//  confidential information and must not be disclosed to third parties or used without the
//  express written permission of eg technology ltd.

#ifndef FLASH_STORAGE_BASE_H_
#define FLASH_STORAGE_BASE_H_

#include "interfaces/IFlashStorage.h"


// TODO_AC Remove this - warning from GCC ignoring unknown pragma
// #ifndef __IAR_SYSTEMS_ICC__
// #pragma warning( push )
// #pragma warning( disable : 4121) // alignment of a member was sensitive to packing
// #endif


namespace eg
{
    /**
    * @brief The Base class for a Flash Device. Implements IFlashStorage
    * This operates under a signals framework
    */
    class FlashStorageBase : public virtual IFlashStorage
    {
    public:
        /**
        * @brief Constructor
        */
        FlashStorageBase();
            
        /**
        * @brief Reads from a flash sector
        * @param[in] sectorNumber The number of the flash sector  
        * @param[in] offsetInSector The offset of where to read within the flash sector
        * @param[in] dataBuffer The buffer to read into.
        * @param[in] lengthToRead The number of bytes to read
        */
	    FlashOperationStatus Read(uint32_t sectorNumber, uint32_t offsetInSector, uint8_t* dataBuffer, uint32_t lengthToRead) override;
            
        /**
        * @brief Writes to a flash sector
        * @param[in] sectorNumber The number of the flash sector  
        * @param[in] offsetInSector The offset of where to write to within the flash sector
        * @param[in] dataBuffer The buffer containing the data to write
        * @param[in] lengthToWrite The number of bytes to write
        */
	    FlashOperationStatus Write(uint32_t sectorNumber, uint32_t offsetInSector, const uint8_t* dataBuffer, uint32_t lengthToWrite) override;

        /**
        * @brief Erases an entire flash sector. Please use with care.
        * @param[in] sectorNumber The number of the flash sector to be erased
        */
	    FlashOperationStatus EraseSector(uint32_t sectorNumber) override;
            
        /**
        * @brief Signals that the read/write or erase currently in progress should be aborted.
        */
        void Abort() override;

	    /**
	    * @brief Checks and clears the last operation status.
	    * 
	    * returns the status of the last operation
	    */
	    virtual FlashOperationStatus CheckAndClearStatus() override;

        SignalProxy<OperationCompletion_t> OnFlashOperationComplete() override;

    protected:
        // OnReadBytes, OnWriteBytes and OnEraseSector are called from the base class, ensuring parameter values are safe.
        /**
        * @brief Handles device-specific reading of the bytes. 
        * @param[in] sector The sector to read from
        * @param[in] offsetInSector The offset in bytes within the selected sector. This must be on a mWriteSize boundary.
        * @param[in] destinationBuffer The buffer to hold the result of the read
        * @param[in] numberOfBytesToRead The number of bytes to read.
        */
	    virtual FlashOperationStatus OnReadBytes(
            uint32_t sector,
            uint32_t offsetInSector, 
            uint8_t* destinationBuffer,
            uint32_t numberOfBytesToRead) = 0;

        /**
        * @brief Handles device-specific writing of a chunk of bytes of length mWriteSize.
        * @param[in] sector The sector to write to
        * @param[in] offsetInSector The offset in bytes within the selected sector. This must be on a mWriteSize boundary.
        * @param[in] sourceBuffer The buffer holding the bytes to be written
        */
	    virtual FlashOperationStatus OnWriteBytes(
            uint32_t sector,
            uint32_t offsetInSector,
            const uint8_t* sourceBuffer,
		    uint32_t size) = 0;
            
        /**
        * @brief Handles device-specific erasing of a sector
        * @param[in] sector The sector to erase
        */
	    virtual FlashOperationStatus OnEraseSector(
            uint32_t sector) = 0;
            
        /**
        * @brief Handles device-specific request to abort the current operation.
        */
        virtual void OnAbort() = 0;

	    /**
        * @brief Gets the write size (the minimum size of data that can be written in a single operation)
        * @returns write size in bytes.
        */
	    uint32_t GetWriteSize() override;


        // these are emitted by the derived class
        Signal<FlashOperationStatus> mOnFlashReadComplete; // For internal use
        Signal<FlashOperationStatus> mOnFlashWriteComplete; // For internal use
        Signal<FlashOperationStatus> mOnFlashEraseComplete; // For internal use

	    FlashOperationStatus mLastOperationStatus = FlashOperationStatus::None;

    private:
            
        FlashOperationStatus CheckFlashLocation(uint32_t sectorNumber, uint32_t offsetInSector, uint32_t length);
        void ClearToIdleState();
            
        // signal handlers
        void FlashReadComplete(const FlashOperationStatus& flashOperationStatus);
        void FlashWriteComplete(const FlashOperationStatus& flashOperationStatus);
        void FlashEraseComplete(const FlashOperationStatus& flashOperationStatus);

        // the size and pointer to the start of the write buffer are passed in by the derived implementation of this base class
        Signal<OperationCompletion_t> mOnFlashOperationComplete;             // To call back when whole operation has completed
        
	    Operation mOperation;
		    
	    uint32_t mSectorNumber = 0;
    };
}

#endif // FLASH_STORAGE_BASE_H_