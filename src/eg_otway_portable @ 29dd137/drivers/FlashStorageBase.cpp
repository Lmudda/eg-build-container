//  $Id: FlashStorageBase.cpp 542 2018-12-07 14:33:31Z davidg $
//
//  (c) eg technology ltd, 2018.  All rights reserved.
//
//  This software is the property of eg technology ltd. and may not be copied or reproduced
//  otherwise than on to a single hard disk for backup or archival purposes. The source code is
//  confidential information and must not be disclosed to third parties or used without the
//  express written permission of eg technology ltd.

#include "FlashStorageBase.h"
#include <cstring>

namespace eg
{
    FlashStorageBase::FlashStorageBase() : mOperation(Operation::None)
    {
        ClearToIdleState();

        // TODO EG ASSERT
        // EG_ASSERT(mWriteBuffer != nullptr, "Buffer pointer passed in is null");

        mOnFlashReadComplete.connect<&FlashStorageBase::FlashReadComplete>(this);
        mOnFlashWriteComplete.connect<&FlashStorageBase::FlashWriteComplete>(this);
        mOnFlashEraseComplete.connect<&FlashStorageBase::FlashEraseComplete>(this);
    }

    FlashOperationStatus FlashStorageBase::CheckFlashLocation(uint32_t sectorNumber, uint32_t offsetInSector, uint32_t length)
    {
        FlashOperationStatus flashOperationStatus = FlashOperationStatus::Success;

        if (!IsValidSector(sectorNumber))
        {
            flashOperationStatus = FlashOperationStatus::InvalidSector;
        }

        if (flashOperationStatus == FlashOperationStatus::Success)
        {
            if (offsetInSector >= GetSectorSize(sectorNumber))
            {
                flashOperationStatus = FlashOperationStatus::InvalidOffset;
            }
        }

        if (flashOperationStatus == FlashOperationStatus::Success)
        {
            if (offsetInSector + length > GetSectorSize(sectorNumber))
            {
                flashOperationStatus = FlashOperationStatus::InvalidLength;
            }
        }

        return flashOperationStatus;
    }

    FlashOperationStatus FlashStorageBase::EraseSector(uint32_t sectorNumber)
    {
        const auto checkFlashLocationResult = CheckFlashLocation(sectorNumber, 0, 0);

        if (checkFlashLocationResult != FlashOperationStatus::Success)
        {
            // Notify the caller immediately of the error and return.
            mOnFlashOperationComplete.emit(OperationCompletion_t{checkFlashLocationResult, Operation::Erase, sectorNumber});
            mLastOperationStatus = checkFlashLocationResult;
        }
        else if (mOperation != Operation::None)
        {
            // Notify the caller immediately of the error and return.
            mOnFlashOperationComplete.emit(OperationCompletion_t{FlashOperationStatus::Busy, Operation::Erase, sectorNumber});
            mLastOperationStatus = FlashOperationStatus::Busy;
        }
        else
        {
            mOperation = Operation::Erase;
            mSectorNumber = sectorNumber;
            mLastOperationStatus = OnEraseSector(sectorNumber);
        }

        return mLastOperationStatus;
    }

    FlashOperationStatus FlashStorageBase::Read(uint32_t sectorNumber, uint32_t offsetInSector, uint8_t *dataBuffer, uint32_t lengthToRead)
    {
        const auto checkFlashLocationResult = CheckFlashLocation(sectorNumber, offsetInSector, lengthToRead);

        if (checkFlashLocationResult != FlashOperationStatus::Success)
        {
            // Notify the caller immediately of the error and return.
            mOnFlashOperationComplete.emit(OperationCompletion_t{checkFlashLocationResult, Operation::Read, sectorNumber});
        }
        else if (lengthToRead == 0)
        {
            // Nothing to do
            mOnFlashOperationComplete.emit(OperationCompletion_t{FlashOperationStatus::Success, Operation::Read, sectorNumber});
        }
        else if (dataBuffer == nullptr)
        {
            // Notify the caller immediately of the error and return.
            mOnFlashOperationComplete.emit(OperationCompletion_t{FlashOperationStatus::DataBufferIsNull, Operation::Read, sectorNumber});
        }
        else if (mOperation != Operation::None)
        {
            // Notify the caller immediately of the error and return.
            mOnFlashOperationComplete.emit(OperationCompletion_t{FlashOperationStatus::Busy, Operation::Read, sectorNumber});
        }
        else
        {
            mOperation = Operation::Read;
            mSectorNumber = sectorNumber;
            return OnReadBytes(sectorNumber, offsetInSector, dataBuffer, lengthToRead);
        }

        return FlashOperationStatus::None;
    }

    FlashOperationStatus FlashStorageBase::Write(uint32_t sectorNumber, uint32_t offsetInSector, const uint8_t *dataBuffer, uint32_t lengthToWrite)
    {
        const auto checkFlashLocationResult = CheckFlashLocation(sectorNumber, offsetInSector, lengthToWrite);

        if (checkFlashLocationResult != FlashOperationStatus::Success)
        {
            // Notify the caller immediately of the error and return.
            mOnFlashOperationComplete.emit(OperationCompletion_t{checkFlashLocationResult, Operation::Write, sectorNumber});
            return checkFlashLocationResult;
        }
        else if (lengthToWrite == 0)
        {
            // Nothing to do
            mOnFlashOperationComplete.emit(OperationCompletion_t{FlashOperationStatus::Success, Operation::Write, sectorNumber});
            return FlashOperationStatus::Success;
        }
        else if (dataBuffer == nullptr)
        {
            // Notify the caller immediately of the error and return.
            mOnFlashOperationComplete.emit(OperationCompletion_t{FlashOperationStatus::DataBufferIsNull, Operation::Write, sectorNumber});
            return FlashOperationStatus::DataBufferIsNull;
        }
        else if (mOperation != Operation::None)
        {
            // Notify the caller immediately of the error and return.
            mOnFlashOperationComplete.emit(OperationCompletion_t{FlashOperationStatus::Busy, Operation::Write, sectorNumber});
            return FlashOperationStatus::Busy;
        }
        else
        {
            mOperation = Operation::Write;
            mSectorNumber = sectorNumber;
            return OnWriteBytes(sectorNumber, offsetInSector, dataBuffer, lengthToWrite);
        }
        return FlashOperationStatus::None;
    }

    void FlashStorageBase::Abort()
    {
        ClearToIdleState();
    }

    FlashOperationStatus FlashStorageBase::CheckAndClearStatus()
    {
        ClearToIdleState();
        return mLastOperationStatus;
    }

    uint32_t FlashStorageBase::GetWriteSize()
    {
        return 8u; // 128 bits
    }

    SignalProxy<OperationCompletion_t> FlashStorageBase::OnFlashOperationComplete()
    {
        return SignalProxy(mOnFlashOperationComplete);
    }

    void FlashStorageBase::ClearToIdleState()
    {
        mOperation = Operation::None;
    }

    void FlashStorageBase::FlashReadComplete(const FlashOperationStatus& flashOperationStatus)
    {
        ClearToIdleState();
        mLastOperationStatus = flashOperationStatus;
        mOnFlashOperationComplete.emit(OperationCompletion_t{flashOperationStatus, mOperation, mSectorNumber});
    }

    void FlashStorageBase::FlashWriteComplete(const FlashOperationStatus& flashOperationStatus)
    {
        ClearToIdleState();
        mLastOperationStatus = flashOperationStatus;
        mOnFlashOperationComplete.emit(OperationCompletion_t{flashOperationStatus, Operation::Write, mSectorNumber});
    }

    void FlashStorageBase::FlashEraseComplete(const FlashOperationStatus& flashOperationStatus)
    {
        ClearToIdleState();
        mLastOperationStatus = flashOperationStatus;
        mOnFlashOperationComplete.emit(OperationCompletion_t{flashOperationStatus, Operation::Erase, mSectorNumber});
    }
} // namespace eg::Flash
