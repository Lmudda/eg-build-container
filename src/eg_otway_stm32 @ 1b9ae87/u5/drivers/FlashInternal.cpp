#include "FlashInternal.h"
#include "stm32u5xx_hal_flash.h"
#include "logging/Assert.h"

#include <cstdio>

namespace eg
{
	FlashInternal::FlashInternal(uint8_t *bank1StartAddress, uint8_t *bank2StartAddress, unsigned int numPages)
        // TODO_AC Why has this changed?
        //: FlashStorageBase(kWriteSize), mSectorSize(8192u), mBank1StartAddress(bank1StartAddress), mBank2StartAddress(bank2StartAddress)
        : FlashStorageBase(), mSectorSize(8192u), mBank1StartAddress(bank1StartAddress), mBank2StartAddress(bank2StartAddress)
    {
        mNumSectors = numPages * 2;

        memset(mSectorReadOnly, 0, sizeof(mSectorReadOnly)); // All sectors are unlocked by default
    }


    bool FlashInternal::Initialize()
    {
        // TODO handle fault conditions and take corrective action if possible
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

        return true;
    }

    uint32_t FlashInternal::GetNumberOfSectors()
    {
        return mNumSectors;
    }

    bool FlashInternal::IsValidSector(uint32_t sectorNumber)
    {
        return sectorNumber < mNumSectors;
    }

    Bank FlashInternal::GetBank(uint32_t sectorNumber)
    {
        return (sectorNumber < (mNumSectors / 2)) ? Bank::Bank_1 : Bank::Bank_2;
    }

    uint8_t *FlashInternal::GetAddress(uint32_t sectorNumber)
    {
        Bank bank = GetBank(sectorNumber);
        unsigned int bankSectorNumber = bank == Bank::Bank_1 ? sectorNumber : sectorNumber - (mNumSectors / 2);

        // All sectors are the same size
        uint8_t *address = (bank == Bank::Bank_1 ? mBank1StartAddress : mBank2StartAddress);
        address += kPageSize * bankSectorNumber;

        return address;
    }

    bool FlashInternal::IsSectorReadOnly(uint32_t sectorNumber)
    {
        bool isReadOnly = false;

        if (IsValidSector(sectorNumber))
        {
            const auto wordIndex = sectorNumber / 32u;
            const auto bitIndex = sectorNumber % 32u;
            const auto mask = 1u << bitIndex;
            isReadOnly = (mSectorReadOnly[wordIndex] & mask) == mask;
        }

        return isReadOnly;
    }

    void FlashInternal::SetSectorReadOnly(const uint32_t sectorNumber)
    {
        if (IsValidSector(sectorNumber))
        {
            auto wordIndex = sectorNumber / 32;
            auto bitIndex = sectorNumber % 32;
            auto mask = 1 << bitIndex;
            mSectorReadOnly[wordIndex] |= mask;
        }
    }

    uint8_t* FlashInternal::GetSectorStartAddress(uint32_t sectorNumber)
    {
        uint8_t *address = nullptr;

        if (IsValidSector(sectorNumber))
        {
            address = GetAddress(sectorNumber);
        }

        return address;
    }
    uint32_t FlashInternal::GetSectorSize(uint32_t sectorNumber)
    {
        return IsValidSector(sectorNumber) ? mSectorSize : 0;
    }

    FlashOperationStatus FlashInternal::OnReadBytes(uint32_t sector,
                     uint32_t offsetInSector,
                     uint8_t *destinationBuffer,
                     uint32_t numberOfBytesToRead)
    {
        EG_ASSERT(sector < GetNumberOfSectors(), "Invalid Flash Sector");

        // Clear any previous errors
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

        uint8_t* sectorAddress = GetAddress(sector);

        // All sectors are the same size
        uint8_t *readAddress = sectorAddress + offsetInSector;
        memcpy(destinationBuffer, readAddress, numberOfBytesToRead);

        mLastOperationStatus = FlashOperationStatus::Success;
        mOnFlashReadComplete.emit(mLastOperationStatus);

        return mLastOperationStatus;
    }


	FlashOperationStatus FlashInternal::OnWriteBytes(uint32_t sector,
                      uint32_t offsetInSector,
                      const uint8_t *sourceBuffer,
                      uint32_t size)
    {
        EG_ASSERT(sector < GetNumberOfSectors(), "Invalid Flash Sector");
        EG_ASSERT((offsetInSector % kWriteSize) == 0, "Offset not aligned with write size");

        // Clear any previous errors
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

        // All sectors are the same size
        uint8_t* sectorAddress = GetAddress(sector);

        auto result = HAL_FLASH_Unlock();
        uint32_t written = 0;

        do
        {
            result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, reinterpret_cast<uint32_t>(sectorAddress + offsetInSector + written), reinterpret_cast<uint32_t>(sourceBuffer) + written);
            written += kWriteSize; // 128bits quad word
        } while (HAL_OK == result && written < size);

        HAL_FLASH_Lock();

        mLastOperationStatus = result == HAL_OK ? FlashOperationStatus::Success : FlashOperationStatus::Failed;

        mOnFlashWriteComplete.emit(mLastOperationStatus);

        return mLastOperationStatus;
    }

    FlashOperationStatus FlashInternal::OnEraseSector(uint32_t sector)
    {
        auto result = HAL_FLASH_Unlock();

        if (HAL_OK == result)
        {
            __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

            // Configure the erase. All pages are located in physical bank 2
            FLASH_EraseInitTypeDef eraseInfo = {};
            eraseInfo.TypeErase = FLASH_TYPEERASE_PAGES;
            eraseInfo.NbPages = 1;
            eraseInfo.Banks = FLASH_BANK_2;

            // TODO_AC This could never have worked properly. There is a confusion of nomenclature between physical "banks" 
            // in the flash (each 1MB) and notional "banks" in the block of flash managed by this object (each N pages at 
            // some offset from the start of physical bank #2). I think it would be wise to replace this class entirely with 
            // FlashMemory (added to support the system logs).  
            //eraseInfo.Page = (GetSectorStartAddress(sector) - mBank2StartAddress) / kPageSize;
            // Base address of physical bank #2
            constexpr uint32_t kBank2BaseAddress = 0x08100000;
            eraseInfo.Page = (reinterpret_cast<uint32_t>(GetSectorStartAddress(sector)) - kBank2BaseAddress) / kPageSize;

            // Do the erase (this blocks)
            uint32_t sectorError;
            result = HAL_FLASHEx_Erase(&eraseInfo, &sectorError);

            HAL_FLASH_Lock();
        }

        mLastOperationStatus = HAL_OK == result ? FlashOperationStatus::Success : FlashOperationStatus::Failed;

        mOnFlashEraseComplete.emit(mLastOperationStatus);

        return mLastOperationStatus;
    }

    void FlashInternal::OnAbort()
    {
        // todo nothing to do here, its a blocking implementation
    }

} // namespace eg