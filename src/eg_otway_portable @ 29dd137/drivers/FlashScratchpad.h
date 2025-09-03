#ifndef FLASH_SCRATCHPAD_H
#define FLASH_SCRATCHPAD_H

#include "logging/Assert.h"
#include "interfaces/IFlashStorage.h"
#include "signals/Signal.h"
#include "utilities/CRC.h"
#include "utilities/ErrorHandler.h"

namespace eg
{
    constexpr uint32_t kBlankNumber = 0xFFFFFFFF;
	constexpr uint32_t kMagicNumber = 0xA55AA55A;

	// Flash page
	struct Page_t
	{
		uint32_t pageNumber;
		uint8_t *pageStartAddr;
	};
	    

    template <typename DATA_STRUCTURE>
    class FlashScratchpad
    {
      public:        
	    FlashScratchpad(eg::IFlashStorage& flashStorage, uint32_t pageOneOffset, uint32_t pageTwoOffset, const DATA_STRUCTURE &defaults);
        ~FlashScratchpad() = default;

        void UpdateData(const DATA_STRUCTURE &dataStructure);
	    const DATA_STRUCTURE& GetData();

        eg::SignalProxy<FlashOperationStatus> OnDataRead() { return SignalProxy(mOnDataRead); }
        eg::SignalProxy<FlashOperationStatus> OnDataWritten() { return SignalProxy(mOnDataWritten); }
	    eg::SignalProxy<> OnResetToDefaults() { return SignalProxy(mOnResetToDefaults); }

      private:
	    
	    // Struct to wrap the data to be stored in flash. Magic number at the start and CRC of the data at the end.
	    struct DataContainer_t
        {
          public:
	        DataContainer_t()
	        {
		        // Clear memory to 0xFF
				#pragma GCC diagnostic push
				#pragma GCC diagnostic ignored "-Wclass-memaccess"
		        memset(this, 0xff, sizeof(*this));
				#pragma GCC diagnostic pop
				magic   = kMagicNumber;
		        // counter = 0xFFFFFFFFu;
				crc     = 0u;	    
			}
	        
            DataContainer_t(const DATA_STRUCTURE &initialData)
            {
	            // Clear memory to 0xFF
				#pragma GCC diagnostic push
				#pragma GCC diagnostic ignored "-Wclass-memaccess"
				memset(this, 0xff, sizeof(*this));
				#pragma GCC diagnostic pop
	            
	            magic   = kMagicNumber;
	            // counter = 0xFFFFFFFFu;
	            memcpy(&data, &initialData, sizeof(data));
				CRC32 crc_calc;
	            // include the magic value and counter
	            crc = crc_calc.calculate(reinterpret_cast<const uint8_t*>(this), sizeof(DataContainer_t) - sizeof(uint32_t));
            }

	        uint32_t ComputeCrc()
	        {
		        CRC32 crc_calc;
		        return crc_calc.calculate(reinterpret_cast<const uint8_t*>(this), sizeof(DataContainer_t) - sizeof(uint32_t));
	        }

            uint32_t magic;
	        uint32_t counter;
            DATA_STRUCTURE data;
            uint32_t crc;
        } __attribute__((aligned(16)));
	    
	    // DataContainer_t must be a simple struct witout a vtable
	    static_assert(std::is_trivially_copyable<DataContainer_t>::value, "DataContainer_t must be trivially copyable");
	    static_assert(std::is_trivially_destructible<DataContainer_t>::value, "DataContainer_t must be trivially destructible");
	    static_assert(!std::has_virtual_destructor<DataContainer_t>::value, "DataContainer_t must not have a virtual destructor");

	    // DataContainer_t structure must be a multiple of 16 bytes (128 bits) for STM32 Flash API
	    // Structure padding etc should take care of this
	    static_assert((sizeof(DataContainer_t) & 15u) == 0u);

        eg::IFlashStorage &mFlashStorage;

        eg::Signal<FlashOperationStatus> mOnDataRead;
        eg::Signal<FlashOperationStatus> mOnDataWritten;

	    eg::Signal<>                     mOnResetToDefaults;

	    Page_t          mPageOne;
	    Page_t          mPageTwo;
	    
	    Page_t          *mCurrentPage;
	    DataContainer_t *mCurrentDataContainer;	    

	    bool			mValidData;
	    
	    const size_t    mDataContainerSize = sizeof(DataContainer_t);
	    	    
	    
        [[nodiscard]] Page_t ConstructPageInfo(uint32_t pageNumber);
        void OnPageErased();
        void OnFlashOperationComplete(const OperationCompletion_t& operation);
	    
	    bool IsNextStructEmpty(const DataContainer_t &dC);
	    Page_t *FindLatestPage();
	    DataContainer_t *FindLatestDataContainer(Page_t &page);	    
	    uint32_t RoundUpToPowerOf2(uint32_t i);	    
    };

    template <typename DATA_STRUCTURE>
    FlashScratchpad<DATA_STRUCTURE>::FlashScratchpad(eg::IFlashStorage &flashStorage, uint32_t pageOneOffset, uint32_t pageTwoOffset, const DATA_STRUCTURE &defaults)
        : mFlashStorage(flashStorage), mPageOne(ConstructPageInfo(pageOneOffset)), mPageTwo(ConstructPageInfo(pageTwoOffset)), mValidData(false)
    {
	    mFlashStorage.OnFlashOperationComplete().connect<&FlashScratchpad<DATA_STRUCTURE>::OnFlashOperationComplete>(this);
	    
	    // data is stored in two "banks" of flash. First workout which is the active bank, then search in the bank for the latest data
	    // Or, if none, reset to defaults
	    DataContainer_t *pDC1 = reinterpret_cast<DataContainer_t *>(mPageOne.pageStartAddr);
	    DataContainer_t *pDC2 = reinterpret_cast<DataContainer_t *>(mPageTwo.pageStartAddr);
		    
	    if (kMagicNumber == pDC1->magic && kBlankNumber == pDC2->magic)
	    {
		    mCurrentPage = &mPageOne;
		    mCurrentDataContainer = FindLatestDataContainer(*mCurrentPage);
		    if (nullptr != mCurrentDataContainer)
		    {
			    mValidData = true;
		    }
	    }
	    else if (kMagicNumber == pDC2->magic && kBlankNumber == pDC1->magic)
	    {
		    mCurrentPage = &mPageTwo;
		    mCurrentDataContainer = FindLatestDataContainer(*mCurrentPage);
		    if (nullptr != mCurrentDataContainer)
		    {
			    mValidData = true;
		    }
	    }
	    else if (kBlankNumber == pDC1->magic && kBlankNumber == pDC2->magic)
	    {
		    // flash is blank, so set to defaults
		    mCurrentPage = &mPageOne;
		    mCurrentDataContainer = pDC1;

		    DataContainer_t writeBuffer(defaults);
		    writeBuffer.counter = 0;		    
		    mFlashStorage.Write(mCurrentPage->pageNumber, 0u, reinterpret_cast<uint8_t *>(&writeBuffer), sizeof(DataContainer_t));
		    if (FlashOperationStatus::Success != mFlashStorage.CheckAndClearStatus())
			{
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				// There needs to be a reset here to workaround an issue where the
				// Flash programming API returns an error code (0x82) from the CPU
				// peripheral on the second write to flash after an erase. After a
				// CPU reset, a write to the exact same address works fine.
				Error_Handler(); // LCOV_EXCL_LINE
			}
	    }
	    else
	    {
		    // both flash pages appear to be active, so could be a failed erase
		    // Find the most recent data container over both pages and erase the
		    // other page
		    DataContainer_t *pLatestDCP1 = FindLatestDataContainer(mPageOne);
		    DataContainer_t *pLatestDCP2 = FindLatestDataContainer(mPageTwo);
		    
		    if (pLatestDCP1->counter > pLatestDCP2->counter)
		    {
			    mCurrentPage = &mPageOne;
			    mCurrentDataContainer = pLatestDCP1;
			    mFlashStorage.EraseSector(mPageTwo.pageNumber);
		    }
		    else
		    {
			    mCurrentPage = &mPageTwo;
			    mCurrentDataContainer = pLatestDCP2;
			    mFlashStorage.EraseSector(mPageOne.pageNumber);
		    }
		    
		    if (nullptr != mCurrentDataContainer)
		    {
			    mValidData = true;
		    }			    
	    }
    }


	template <typename DATA_STRUCTURE>
	bool FlashScratchpad<DATA_STRUCTURE>::IsNextStructEmpty(const DataContainer_t &dC)
    {	    
	    bool isBlank = true;
	    
		for (unsigned int i = 0 ; i < sizeof(dC.data) ; i++)
	    {
		    if (dC.data[i] != 0xFF)
		    {
			    isBlank = true;
			    break;
		    }
	    }
	    
        return isBlank;
    }

    template <typename DATA_STRUCTURE>
    Page_t *FlashScratchpad<DATA_STRUCTURE>::FindLatestPage()
    {
	    DataContainer_t *pDC1 = static_cast<DataContainer_t *>(mFlashStorage.GetSectorStartAddress(mPageOne.pageNumber));
	    DataContainer_t *pDC2 = static_cast<DataContainer_t *>(mFlashStorage.GetSectorStartAddress(mPageTwo.pageNumber));
	    Page_t *pPage = nullptr;
		    
	    if (kMagicNumber == pDC1->magic && kBlankNumber == pDC2->magic)
	    {
		    pPage = &mPageOne;
	    }
	    else if (kMagicNumber == pDC2->magic && kBlankNumber == pDC1->magic)
	    {
		    pPage = &mPageTwo;		    
	    }
	    else 
	    {
		    // Neither page contains any recognisable data
	    }
	    
	    return pPage;
    }

	template <typename DATA_STRUCTURE>
	uint32_t FlashScratchpad<DATA_STRUCTURE>::RoundUpToPowerOf2(uint32_t i)
	{			
		if (i == 0u) {
			return 1u;
		}
			
		i--;
		i |= i >> 1u;
		i |= i >> 2u;
		i |= i >> 4u;
		i |= i >> 8u;
		i |= i >> 16u; // Maximum shift needed for 32-bit integers
			
		return i + 1u;
	}
	
	template <typename DATA_STRUCTURE>
	FlashScratchpad<DATA_STRUCTURE>::DataContainer_t *FlashScratchpad<DATA_STRUCTURE>::FindLatestDataContainer(Page_t &page)
	{
		// binary search for most recent data which is always at the end of the written data
		DataContainer_t *pDC        = reinterpret_cast<DataContainer_t *>(page.pageStartAddr);
		unsigned int searchArea     = RoundUpToPowerOf2(mFlashStorage.GetSectorSize(0) / sizeof(DataContainer_t));
		unsigned int searchStartIdx = 0u;
		while (searchArea > 1u)
		{
			searchArea /= 2;
			unsigned int topHalfIdx = searchStartIdx + searchArea;
			if (kMagicNumber == pDC[topHalfIdx].magic)
			{
				searchStartIdx = topHalfIdx;
			}
			else
			{
				EG_ASSERT(kBlankNumber != pDC[searchStartIdx].magic, "Binary search error");			    
			}
		}
		
		// The next entry after the most recent entry should be blank, unless this is the last entry
		EG_ASSERT(kBlankNumber == pDC[searchStartIdx + 1u].magic, "Binary search error");
	    
		return &pDC[searchStartIdx];		
	}

    /**
     * @brief Update the data stored in flash
     * @params Data to overwrite that stored in flash with
     */

    template <typename DATA_STRUCTURE>
    void FlashScratchpad<DATA_STRUCTURE>::UpdateData(const DATA_STRUCTURE &dataStructure)
    {	    
	    if (0 != memcmp(reinterpret_cast<const void *>(&mCurrentDataContainer->data), reinterpret_cast<const void *>(&dataStructure), sizeof(DATA_STRUCTURE)))
	    {	    
		    uint32_t nextWriteOffset = reinterpret_cast<uint8_t *>(mCurrentDataContainer + 1) - mCurrentPage->pageStartAddr;
		    DataContainer_t writeBuffer(dataStructure);
		    writeBuffer.counter = mCurrentDataContainer->counter + 1u;
	
		    // round to next write boundary
		    EG_ASSERT(0u == (nextWriteOffset & (mFlashStorage.GetWriteSize() - 1u)), "Badly aligned write offset");
	    
			if (nextWriteOffset <= (mFlashStorage.GetSectorSize(0) - sizeof(DataContainer_t))) // if there is space for a new load of data
		    {
			    mFlashStorage.Write(mCurrentPage->pageNumber, nextWriteOffset, reinterpret_cast<uint8_t *>(&writeBuffer), sizeof(DataContainer_t));
			    mCurrentDataContainer++;
		    }
		    else
		    {
			    uint32_t oldPageNumber = mCurrentPage->pageNumber;

			    // write data to new blank page
			    mCurrentPage = (mCurrentPage->pageNumber == mPageOne.pageNumber) ? &mPageTwo : &mPageOne;
			    mFlashStorage.Write(mCurrentPage->pageNumber, 0u, reinterpret_cast<uint8_t *>(&writeBuffer), sizeof(DataContainer_t));
			    mCurrentDataContainer = reinterpret_cast<DataContainer_t *>(mCurrentPage->pageStartAddr);
	        
			    // now erase the old page
			    // TODO_AC This was not actually erasing the page because the internal state of mFlashStorage was 
			    // not Operation::None. It would instead emit an error. Abort() just resets this state so that the 
			    // erase will proceed. This whole asynchronous design is fundamentally flawed since all the flash 
			    // operations are synchronous. What was the point? The code is more complicated, and broken. I do not
			    // feel comfortable that this change is sufficient to repair the flash storage. I would prefer to 
			    // replace the whole thing. 
			    mFlashStorage.Abort();
			    mFlashStorage.EraseSector(oldPageNumber);
		    }   
	    }
	    // else data is the same so no need to write a new one
    }

    /**
     * @brief Retrieve the latest saved copy of the data
     * @params a reference to the data structure where the read data should be put
     */
    template <typename DATA_STRUCTURE>
    const DATA_STRUCTURE &FlashScratchpad<DATA_STRUCTURE>::GetData(void)
    {
	    EG_ASSERT(mValidData, "No valid data available in the flash");
	    
		return mCurrentDataContainer->data;
    }

    template <typename DATA_STRUCTURE>
    Page_t FlashScratchpad<DATA_STRUCTURE>::ConstructPageInfo(uint32_t pageNumber)
    {	    
        return Page_t{pageNumber, mFlashStorage.GetSectorStartAddress(pageNumber)};
    }

    template <typename DATA_STRUCTURE>
    void FlashScratchpad<DATA_STRUCTURE>::OnFlashOperationComplete(const OperationCompletion_t& operation)
    {
        if (operation.SectorNumber == mCurrentPage->pageNumber)
        {
            // what happens if it fails
            switch (operation.Op)
            {
            case Operation::Erase:
	            // an erase only happens on the write operation after a page is full
	            // confirm the write is complete once the erase has completed
				mOnDataWritten.emit(operation.Status);			            
                break;
	            
            case Operation::Write:
			    mOnDataWritten.emit(operation.Status);			            
                break;
	            
            case Operation::Read:
	            if (mCurrentDataContainer->crc == mCurrentDataContainer->ComputeCrc())
	            {
		            mOnDataRead.emit(operation.Status);		            
	            }
	            else
	            {
		            mOnDataRead.emit(FlashOperationStatus::Failed);		            
				}
			break;
            case Operation::None:
            default:
                break;
            }
        }
    }
}

#endif