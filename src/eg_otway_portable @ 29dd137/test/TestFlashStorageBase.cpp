/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "gtest/gtest.h"
#include "drivers/FlashStorageBase.h"
#include "interfaces/IFlashStorage.h"
#include "utilities/CriticalSection.h"
#include "TestSingleThreadedUtils.h"
#include <stdint.h>


namespace {

int g_test_emit_count;

// Tracability: PRS-92 Event loop interface
class TestEventLoop : public eg::IEventLoop
{
public:
    // Just tick emit counter, dispatch the event.
    void post(const eg::Event& ev) override { ++g_test_emit_count; ev.dispatch();  }
    void run() override {}
#if defined(OTWAY_EVENT_LOOP_WATER_MARK)
    uint16_t get_high_water_mark() const { return 0; }
#endif
};

}

namespace eg 
{
    class FlashStorageBaseTest : public virtual FlashStorageBase
    {
    public:
        virtual bool Initialize() override { return true; }
        virtual uint32_t GetNumberOfSectors() override { return mNumSectors; }
        virtual bool IsValidSector(uint32_t sectorNumber) override { return sectorNumber < mNumSectors - 1; }
        virtual bool IsSectorReadOnly([[maybe_unused]] uint32_t sectorNumber) override { return false; }
        virtual uint8_t* GetSectorStartAddress([[maybe_unused]] uint32_t sectorNumber) override { return mStartAddress; }
        virtual uint32_t GetSectorSize([[maybe_unused]] uint32_t sectorNumber) override { return mSectorSize; }

    protected:
	    virtual FlashOperationStatus OnReadBytes([[maybe_unused]] uint32_t sector, 
            [[maybe_unused]] uint32_t offsetInSector, 
            [[maybe_unused]] uint8_t* destinationBuffer, 
            [[maybe_unused]] uint32_t numberOfBytesToRead) override 
            {
                // You must emit the status in your On... method.
                mLastOperationStatus = FlashOperationStatus::Success;
                mOnFlashReadComplete.emit(mLastOperationStatus);
                return FlashOperationStatus::Success;
            }

        virtual FlashOperationStatus OnWriteBytes([[maybe_unused]] uint32_t sector, 
            [[maybe_unused]] uint32_t offsetInSector, 
            [[maybe_unused]] const uint8_t* sourceBuffer,
		    [[maybe_unused]] uint32_t size) override 
            {
                mLastOperationStatus = FlashOperationStatus::Success;
                mOnFlashWriteComplete.emit(mLastOperationStatus);
                return FlashOperationStatus::Success;
            }

        virtual FlashOperationStatus OnEraseSector([[maybe_unused]] uint32_t sector) override 
        {
            mLastOperationStatus = FlashOperationStatus::Success;
            mOnFlashEraseComplete.emit(mLastOperationStatus);
            return FlashOperationStatus::Success;
        }

        virtual void OnAbort() override { }
        
    private:
        uint32_t mNumSectors = 256;
        uint32_t mSectorSize = 64;
        uint8_t* mStartAddress = nullptr;
    };

} // namespace eg


// Store operation result
eg::OperationCompletion_t operationResult;


// Callback for operation complete
static void complete(const eg::OperationCompletion_t & result) { operationResult = result; }


// Convenience to compare OperationCompletion_ts
void operationResultExpected(const eg::OperationCompletion_t expected) 
{
    EXPECT_EQ(operationResult.Status, expected.Status);
    EXPECT_EQ(operationResult.Op, expected.Op);
    EXPECT_EQ(operationResult.SectorNumber, expected.SectorNumber);
}


// Fixture for tests
class FlashTest : public testing::Test 
{
    protected:
    eg::IFlashStorage* flash;    
    TestEventLoop * m_loop;
    eg::OperationCompletion_t defaultOperationResult{ eg::FlashOperationStatus::None, eg::Operation::None, UINT32_MAX };
    void* connection;
    uint32_t bufLen = 8;
    uint8_t* buf;

    virtual void SetUp() 
    {
        buf = new uint8_t[bufLen];
        m_loop = new TestEventLoop(); 
        eg::CURRENT_EVENT_LOOP = m_loop;
        flash = new eg::FlashStorageBaseTest();
        connection = flash->OnFlashOperationComplete().connect<&complete>();
        g_test_emit_count = 0;
        operationResult = defaultOperationResult;
        EXPECT_EQ(flash->CheckAndClearStatus(), eg::FlashOperationStatus::None);
    }

    virtual void TearDown() 
    {
        delete [] buf;
        flash->OnFlashOperationComplete().disconnect(connection);
        delete flash;
        eg::CURRENT_EVENT_LOOP = nullptr;
        delete m_loop;
    }
};


// Read tests

TEST_F(FlashTest, ReadZeroLength)
{
    EXPECT_EQ(flash->Read(0, 0, buf, 0), eg::FlashOperationStatus::None);
    EXPECT_EQ(g_test_emit_count, 1);
    EXPECT_EQ(flash->CheckAndClearStatus(), eg::FlashOperationStatus::None);
    operationResultExpected(eg::OperationCompletion_t{eg::FlashOperationStatus::Success, eg::Operation::Read, 0});
}


TEST_F(FlashTest, ReadNonZeroLength)
{
    uint32_t bufLen = 8;
    uint8_t* buf = new uint8_t[bufLen];
    EXPECT_EQ(flash->Read(0, 0, buf, bufLen), eg::FlashOperationStatus::Success);
    EXPECT_EQ(g_test_emit_count, 2); 
    EXPECT_EQ(flash->CheckAndClearStatus(), eg::FlashOperationStatus::Success);
    // TODO: Should this really return Operation::None?
    operationResultExpected(eg::OperationCompletion_t{eg::FlashOperationStatus::Success, eg::Operation::None, 0});
}


TEST_F(FlashTest, ReadInvalidSector)
{
    EXPECT_EQ(flash->Read(256, 0, buf, bufLen), eg::FlashOperationStatus::None);
    EXPECT_EQ(g_test_emit_count, 1); 
    EXPECT_EQ(flash->CheckAndClearStatus(), eg::FlashOperationStatus::None);
    operationResultExpected(eg::OperationCompletion_t{eg::FlashOperationStatus::InvalidSector, eg::Operation::Read, 256});
}


TEST_F(FlashTest, ReadWithNullBuffer)
{
    EXPECT_EQ(flash->Read(0, 0, nullptr, bufLen), eg::FlashOperationStatus::None);
    EXPECT_EQ(g_test_emit_count, 1); 
    EXPECT_EQ(flash->CheckAndClearStatus(), eg::FlashOperationStatus::None);
    operationResultExpected(eg::OperationCompletion_t{eg::FlashOperationStatus::DataBufferIsNull, eg::Operation::Read, 0});
}


// Write tests


TEST_F(FlashTest, WriteZeroLength)
{
    EXPECT_EQ(flash->Write(0, 0, buf, 0), eg::FlashOperationStatus::Success);
    EXPECT_EQ(g_test_emit_count, 1);
    EXPECT_EQ(flash->CheckAndClearStatus(), eg::FlashOperationStatus::None);
    operationResultExpected(eg::OperationCompletion_t{eg::FlashOperationStatus::Success, eg::Operation::Write, 0});
}


TEST_F(FlashTest, WriteNonZeroLength)
{
    EXPECT_EQ(flash->Write(0, 0, buf, bufLen), eg::FlashOperationStatus::Success);
    EXPECT_EQ(g_test_emit_count, 2); 
    EXPECT_EQ(flash->CheckAndClearStatus(), eg::FlashOperationStatus::Success);
    // TODO: Should this really return Operation::None?
    operationResultExpected(eg::OperationCompletion_t{eg::FlashOperationStatus::Success, eg::Operation::Write, 0});
}


TEST_F(FlashTest, WriteInvalidSector)
{
    EXPECT_EQ(flash->Write(256, 0, buf, bufLen), eg::FlashOperationStatus::InvalidSector);
    EXPECT_EQ(g_test_emit_count, 1); 
    EXPECT_EQ(flash->CheckAndClearStatus(), eg::FlashOperationStatus::None);
    operationResultExpected(eg::OperationCompletion_t{eg::FlashOperationStatus::InvalidSector, eg::Operation::Write, 256});
}


TEST_F(FlashTest, WriteWithNullBuffer)
{
    EXPECT_EQ(flash->Write(0, 0, nullptr, bufLen), eg::FlashOperationStatus::DataBufferIsNull);
    EXPECT_EQ(g_test_emit_count, 1); 
    EXPECT_EQ(flash->CheckAndClearStatus(), eg::FlashOperationStatus::None);
    operationResultExpected(eg::OperationCompletion_t{eg::FlashOperationStatus::DataBufferIsNull, eg::Operation::Write, 0});
}


TEST_F(FlashTest, WriteInvalidLength)
{
    uint32_t bufLenInvalid = 128;
    uint8_t* bufInvalid = new uint8_t[bufLen];
    EXPECT_EQ(flash->Write(0, 0, bufInvalid, bufLenInvalid), eg::FlashOperationStatus::InvalidLength);
    EXPECT_EQ(g_test_emit_count, 1); 
    EXPECT_EQ(flash->CheckAndClearStatus(), eg::FlashOperationStatus::None);
    // TODO: Should this really return Operation::None?
    operationResultExpected(eg::OperationCompletion_t{eg::FlashOperationStatus::InvalidLength, eg::Operation::Write, 0});
    delete [] bufInvalid;
}


TEST_F(FlashTest, WriteInvalidOffset)
{
    EXPECT_EQ(flash->Write(0, 64, buf, bufLen), eg::FlashOperationStatus::InvalidOffset);
    EXPECT_EQ(g_test_emit_count, 1); 
    EXPECT_EQ(flash->CheckAndClearStatus(), eg::FlashOperationStatus::None);
    // TODO: Should this really return Operation::None?
    operationResultExpected(eg::OperationCompletion_t{eg::FlashOperationStatus::InvalidOffset, eg::Operation::Write, 0});
}


// Erase tests


TEST_F(FlashTest, EraseGoodLocation)
{
    EXPECT_EQ(flash->EraseSector(0), eg::FlashOperationStatus::Success);
    EXPECT_EQ(g_test_emit_count, 2);
    EXPECT_EQ(flash->CheckAndClearStatus(), eg::FlashOperationStatus::Success);
    operationResultExpected(eg::OperationCompletion_t{eg::FlashOperationStatus::Success, eg::Operation::Erase, 0});
}


TEST_F(FlashTest, EraseBadLocation)
{
    EXPECT_EQ(flash->EraseSector(256), eg::FlashOperationStatus::InvalidSector);
    EXPECT_EQ(g_test_emit_count, 1);
    EXPECT_EQ(flash->CheckAndClearStatus(), eg::FlashOperationStatus::InvalidSector);
    operationResultExpected(eg::OperationCompletion_t{eg::FlashOperationStatus::InvalidSector, eg::Operation::Erase, 256});
}


// Other tests


TEST_F(FlashTest, WriteSize)
{
    EXPECT_EQ(flash->GetWriteSize(), 8u);
}