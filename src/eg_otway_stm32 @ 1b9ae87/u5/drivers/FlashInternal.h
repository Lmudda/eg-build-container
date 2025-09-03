#ifndef FLASH_INTERNAL_H
#define FLASH_INTERNAL_H

#include "drivers/FlashStorageBase.h"

namespace eg
{    
    constexpr uint16_t kPageSize = 0x2000;

    enum class Bank
    {
        Bank_1,
        Bank_2
    };
    class FlashInternal : public virtual FlashStorageBase
    {
      public:
        FlashInternal(uint8_t *bank1StartAddress, uint8_t *bank2StartAddress, unsigned int numPages); // NB numPages *per* bank

        virtual bool Initialize() override;
        virtual uint32_t GetNumberOfSectors() override;
        virtual bool IsValidSector(uint32_t sectorNumber) override;
        virtual bool IsSectorReadOnly(uint32_t sectorNumber) override;
        void SetSectorReadOnly(uint32_t sectorNumber);
        virtual uint8_t *GetSectorStartAddress(uint32_t sectorNumber) override;
        virtual uint32_t GetSectorSize(uint32_t sectorNumber) override;

      protected:
        
	    virtual FlashOperationStatus OnReadBytes(uint32_t sector, 
            uint32_t offsetInSector, 
            uint8_t* destinationBuffer, 
            uint32_t numberOfBytesToRead) override;
        
	    virtual FlashOperationStatus OnWriteBytes(uint32_t sector, 
            uint32_t offsetInSector, 
            const uint8_t* sourceBuffer,
		    uint32_t size) override;
        
	    virtual FlashOperationStatus OnEraseSector(uint32_t sector) override;
        virtual void OnAbort() override;
        
      private:
        static constexpr uint32_t kWriteSize = 16u;
		    
        uint32_t mSectorSize;
        uint16_t mNumSectors;
        uint32_t mSectorReadOnly[8]; // Maximum 256 sectors todo this wont work for other u5 chips

	    uint8_t *mBank1StartAddress;
	    uint8_t *mBank2StartAddress;
	    
        Bank GetBank(uint32_t sectorNumber);
	    uint8_t *GetAddress(uint32_t sectorNumber);
    };
} // namespace eg

#endif