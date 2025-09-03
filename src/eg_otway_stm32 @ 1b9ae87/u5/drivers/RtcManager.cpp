/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "RtcManager.h"
#include "stm32u5xx_hal_rtc.h"
#include <cstdio>
#include <cstdint>

namespace eg
{
    RtcManager::RtcManager()
    {
        RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
        PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            // TODO
            //Error_Handler();
            while (1);
        }

        /* RTC clock enable */
        __HAL_RCC_RTC_ENABLE();
        __HAL_RCC_RTCAPB_CLK_ENABLE();
        __HAL_RCC_RTCAPB_CLKAM_ENABLE();


        RTC_PrivilegeStateTypeDef privilegeState = {0};

        m_rtc.Instance = RTC;
        m_rtc.Init.HourFormat = RTC_HOURFORMAT_24;
        m_rtc.Init.AsynchPrediv = 127;
        m_rtc.Init.SynchPrediv = 255;
        m_rtc.Init.OutPut = RTC_OUTPUT_DISABLE;
        m_rtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
        m_rtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
        m_rtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
        m_rtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
        m_rtc.Init.BinMode = RTC_BINARY_NONE;
        if (HAL_RTC_Init(&m_rtc) != HAL_OK)
        {
            // TODO
            //Error_Handler();
            while (1) ;
        }
        privilegeState.rtcPrivilegeFull = RTC_PRIVILEGE_FULL_NO;
        privilegeState.backupRegisterPrivZone = RTC_PRIVILEGE_BKUP_ZONE_NONE;
        privilegeState.backupRegisterStartZone2 = RTC_BKP_DR0;
        privilegeState.backupRegisterStartZone3 = RTC_BKP_DR0;
        if (HAL_RTCEx_PrivilegeModeSet(&m_rtc, &privilegeState) != HAL_OK)
        {
            // TODO
            //Error_Handler();
            while (1) ;
        }
    }

    TimeDate_t RtcManager::GetTimeDate() const
    {
        RTC_TimeTypeDef rtc_time;
        RTC_DateTypeDef rtc_date;
        HAL_RTC_GetTime(&m_rtc, &rtc_time, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&m_rtc, &rtc_date, RTC_FORMAT_BIN);

        TimeDate_t timeDate = {
            .DayInMonth = rtc_date.Date,
            .Month = rtc_date.Month,
            .Year = rtc_date.Year,
            .Hour = rtc_time.Hours,
            .Minute = rtc_time.Minutes,
            .Seconds = rtc_time.Seconds
        };
        timeDate.Year += 2000u; // year is only two digits so limited to 2000-2099

        return timeDate;
    }

    void RtcManager::SetTime(const TimeDate_t time)
    {
        m_rtc_time.TimeFormat = RTC_HOURFORMAT_24;
        m_rtc_time.Hours = time.Hour;
        m_rtc_time.Minutes = time.Minute;
        m_rtc_time.Seconds = time.Seconds;

        HAL_RTC_SetTime(&m_rtc, &m_rtc_time, RTC_FORMAT_BIN);
        mTimeChanged.emit();
    }

    void RtcManager::SetDate(const TimeDate_t date)
    {
        // TODO handle out of bound dates
        m_rtc_date.Date = date.DayInMonth;
        m_rtc_date.Month = date.Month;
        m_rtc_date.Year = date.Year % 100; // year is only two digits so from 2000 onwards

        HAL_RTC_SetDate(&m_rtc, &m_rtc_date, RTC_FORMAT_BIN);
    }

    void RtcManager::SetTimeDate(const TimeDate_t timeDate)
    {
        SetTime(timeDate);
        SetDate(timeDate);
    }
}
