/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2025.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stm32h7xx_hal_rcc.h>
#include <stm32h7xx_hal_pwr_ex.h>
#include <stm32h7xx_hal_pwr.h>
#include "interfaces/IBackupRam.h"


namespace eg
{


template <uint32_t kBackupRamStart, uint32_t kBackupRamLength>
class BackupRam : public IBackupRam
{
public:
    BackupRam()
    {
        __HAL_RCC_BKPRAM_CLK_ENABLE();
        HAL_PWREx_EnableBkUpReg();
        HAL_PWR_EnableBkUpAccess();
    }

    uint8_t* back_ram_ptr() const override
    {
        return reinterpret_cast<uint8_t*>(kBackupRamStart);
    }

    uint32_t backup_ram_size() const override
    {
        return kBackupRamLength;
    }
};


} // namespace eg
