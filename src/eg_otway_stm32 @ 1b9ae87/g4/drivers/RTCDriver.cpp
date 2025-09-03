/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "RTCDriver.h"
#include "utilities/ErrorHandler.h"
#include "logging/Logger.h"
#include "logging/Assert.h"
#include <cstdio>
#include <cstdint>


// We have some dependencies on settings in stm32g4xx_hal_conf.h
#ifndef HAL_RTC_MODULE_ENABLED
#error HAL_RTC_MODULE_ENABLED must be defined to use this driver
#endif
#if (USE_HAL_RTC_REGISTER_CALLBACKS == 0)
#error USE_HAL_RTC_REGISTER_CALLBACKS must be set to use this driver
#endif


namespace eg {


RTCDriver::RTCDriver(const Config& conf)
{
    RCC_PeriphCLKInitTypeDef clock_init{};
    clock_init.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    switch (conf.clk_src)
    {
        case Clock::eLSI: 
            EG_LOG_INFO("RTC using LSI clock source");
            clock_init.RTCClockSelection = RCC_RTCCLKSOURCE_LSI; 
            break;

        // If this is not working, check that there is a crystal and that the LSE has been enabled. 
        // Ideally we would do that in this driver. See SystemClock_Config().
        case Clock::eLSE: 
            EG_LOG_INFO("RTC using LSE clock source");
            clock_init.RTCClockSelection = RCC_RTCCLKSOURCE_LSE; 
            break;
    }
    int status = HAL_RCCEx_PeriphCLKConfig(&clock_init);
    if (status != HAL_OK)
    {
        EG_LOG_ERROR("HAL_RCCEx_PeriphCLKConfig failed with code=%d", status);
        //Error_Handler();
    }
    else   
    {
        EG_LOG_INFO("HAL_RCCEx_PeriphCLKConfig succeeded");
    }

    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();

    m_rtc.Instance            = RTC;
    m_rtc.Init.HourFormat     = RTC_HOURFORMAT_24;
    m_rtc.Init.AsynchPrediv   = 127;
    m_rtc.Init.SynchPrediv    = 255;
    m_rtc.Init.OutPut         = RTC_OUTPUT_DISABLE;
    m_rtc.Init.OutPutRemap    = RTC_OUTPUT_REMAP_NONE;
    m_rtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    m_rtc.Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;
    m_rtc.Init.OutPutPullUp   = RTC_OUTPUT_PULLUP_NONE;

    status = HAL_RTC_Init(&m_rtc);
    if (status != HAL_OK)
    {
        EG_LOG_ERROR("HAL_RTC_Init failed with code=%d", status);
        //Error_Handler();
    }
    else   
    {
        EG_LOG_INFO("HAL_RTC_Init succeeded");
    }
}


RTCDriver::DateTime RTCDriver::get_datetime() const 
{
    RTC_TimeTypeDef rtc_time{};
    HAL_RTC_GetTime(&m_rtc, &rtc_time, RTC_FORMAT_BIN);

    RTC_DateTypeDef rtc_date{};
    HAL_RTC_GetDate(&m_rtc, &rtc_date, RTC_FORMAT_BIN);

    DateTime dt{};
    dt.date.year   = rtc_date.Year;
    dt.date.month  = rtc_date.Month;
    dt.date.day    = rtc_date.Date;
    dt.time.hour   = rtc_time.Hours;
    dt.time.minute = rtc_time.Minutes;
    dt.time.second = rtc_time.Seconds;                  

    // Year is only two digits so limited to 2000-2099
    dt.date.year += 2000U;
  
    return dt;
}


void RTCDriver::set_time(const Time& time) 
{
    // This function needs to be called to enable access to the backup domain register. 
    // It seems rather badly named as "backup" includes the RTC registers AND the RCC_LSE_ON bit
    // to enable the LSE clock source. This was not at all clear. The LSE enable failed silently.
    // It appears that the same condition does not apply to the LSI clock source. Confusing. 
    //
    // In order to enable the LSE HAL_PWR_EnableBkUpAccess() must be called in SystemClock_Config().
    // before HAL_RCC_OscConfig(). It seems fine to just leave it enabled after that but setting the 
    // date and time will not work if HAL_PWR_DisableBkUpAccess() has been called.
    HAL_PWR_EnableBkUpAccess();

    RTC_TimeTypeDef rtc_time{};
    rtc_time.TimeFormat = RTC_HOURFORMAT_24;
    rtc_time.Hours      = time.hour;
    rtc_time.Minutes    = time.minute;
    rtc_time.Seconds    = time.second;
    rtc_time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    rtc_time.StoreOperation = RTC_STOREOPERATION_SET;

    [[maybe_unused]] HAL_StatusTypeDef rc = HAL_RTC_SetTime(&m_rtc, &rtc_time, RTC_FORMAT_BIN);
    m_on_time_set.emit();

    EG_LOG_DEBUG("RTCDriver set time: %d-%d-%d, rc=%d",
                 rtc_time.Hours, rtc_time.Minutes, rtc_time.Seconds, rc);

    HAL_PWR_DisableBkUpAccess();
}


void RTCDriver::set_date(const Date& date) 
{
    HAL_PWR_EnableBkUpAccess();

    // Year is only two digits so from 2000 onwards
    RTC_DateTypeDef rtc_date{};
    rtc_date.Year    = date.year % 100;
    rtc_date.Month   = date.month;
    rtc_date.Date    = date.day;
    rtc_date.WeekDay = RTC_WEEKDAY_MONDAY; // Don't care about this, but it needs to be valid
    [[maybe_unused]] HAL_StatusTypeDef rc = HAL_RTC_SetDate(&m_rtc, &rtc_date, RTC_FORMAT_BIN);

    EG_LOG_DEBUG("RTCDriver set date: %d-%d-%d rc=%d",
                 rtc_date.Year, rtc_date.Month, rtc_date.Date, rc);

    HAL_PWR_DisableBkUpAccess();
}


void RTCDriver::set_datetime(const DateTime& datetime)
{
    set_time(datetime.time);
    set_date(datetime.date);
}


uint32_t RTCDriver::get_backup_register(uint32_t index) const
{
    EG_ASSERT(index < kNumBackupRegisters, "Invalid index for RTC backup register");
    return HAL_RTCEx_BKUPRead(&m_rtc, index);    
}


void RTCDriver::set_backup_register(uint32_t index, uint32_t value)
{
    EG_ASSERT(index < kNumBackupRegisters, "Invalid index for RTC backup register");
    HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&m_rtc, index, value);
    HAL_PWR_DisableBkUpAccess();
}


} // namespace eg
