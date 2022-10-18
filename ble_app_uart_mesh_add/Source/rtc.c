#include "nrfx_rtc.h"
#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"
#include "DataDefine.h"
#include "rtc.h"
#include <stdint.h>
#include <stdbool.h>

/*****************************************************************************
 * Private Macros
 *****************************************************************************/
#define FIRSTYEAR   2000		// start year
#define FIRSTDAY    6			// 0 = Sunday


#define COMPARE_COUNTERTIME  (3)  /**< Get Compare event COMPARE_TIME seconds after the counter starts from 0. */=
const static nrf_drv_rtc_t RtcInstance = NRF_DRV_RTC_INSTANCE(2); /**< Declaring an instance of nrf_drv_rtc for RTC0. */

static DateTime_t LastRtcTime;
static const uint8_t DaysInMonth[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static uint32_t CurrentCounter = 0;
/*****************************************************************************
 * Forward Decleration function
 *****************************************************************************/
static void RtcHandler(nrf_drv_rtc_int_type_t int_type);
static void CounterToStruct(uint32_t sec, DateTime_t *t, uint8_t CalYear);
static uint32_t StructToCounter(DateTime_t *t );
/*****************************************************************************
 * Implementation function
 *****************************************************************************/
 void RTC_Init()
{
  nrf_drv_rtc_config_t RtcConfig = NRF_DRV_RTC_DEFAULT_CONFIG;
	
  RtcConfig.prescaler = 32767;
	
  uint32_t err = nrf_drv_clock_init();
  if (err != NRF_ERROR_MODULE_ALREADY_INITIALIZED)
    APP_ERROR_CHECK(err);
  nrf_drv_clock_lfclk_request(NULL);
  
  err = nrf_drv_rtc_init(&RtcInstance, &RtcConfig, RtcHandler);
  APP_ERROR_CHECK(err);
  nrf_drv_rtc_tick_enable(&RtcInstance ,true);

  err = nrf_drv_rtc_cc_set(&RtcInstance, 0, (uint32_t)24, true);
  APP_ERROR_CHECK(err);

  nrf_drv_rtc_enable(&RtcInstance);
}


/**
  * @brief  Set date time for RTC
  * @param  None
  * @retval None
  */
void RTC_SetDateTime (DateTime_t Input, int32_t GMT_Adjust)
{	
	CurrentCounter = StructToCounter(&Input) + GMT_Adjust;
}
///*****************************************************************************/
///**
//  * @brief  Set date time for RTC
//  * @param  2008-12-16 10:00:00
//  * @retval None
//  */
//static void UpdateTimeFromServer (uint8_t *Buffer)
//{
//	DateTime_t DateTime;
//	
//	DateTime.Second	= GetNumberFromString(17, (char *)Buffer);
//	DateTime.Minute = GetNumberFromString(14, (char *)Buffer);
//	DateTime.Hour = GetNumberFromString(11, (char *)Buffer);
//	
//	DateTime.Day = GetNumberFromString(8, (char *)Buffer);
//	DateTime.Month = GetNumberFromString(5, (char *)Buffer);
//	DateTime.Year = GetNumberFromString(0, (char *)Buffer) - 2000;
//	    
//	SetDateTime(DateTime);
//}

static void UpdateTimeFromServer (uint32_t unixTimeStamp)
{
    //DateTime_t DateTime;

    //if(!unixTimeStamp) 
    //    return;
    
    //// Convert to GMT +7. 25200 = 7 * 3600
    //int64_t rawtime = unixTimeStamp + 25200;
    //struct tm ts;
    
    //// Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
    //ts = *localtime(&rawtime);

    //NRF_LOG_INFO("Time now: %02d:%02d:%02d  %02d-%02d-%d\r\n", ts.tm_hour, ts.tm_min, ts.tm_sec, ts.tm_mday, ts.tm_mon + 1, ts.tm_year + 1900);

    //DateTime.Year = (ts.tm_year + 1900) % 2000;		//Year - 1900
    //DateTime.Month = ts.tm_mon + 1;		// Month, where 0 = jan
    //DateTime.Day = ts.tm_mday;		// Day of the month
    //DateTime.Hour = ts.tm_hour;
    //DateTime.Minute = ts.tm_min;
    //DateTime.Second = ts.tm_sec;

    //// Convert to GMT +7. 25200 = 7 * 3600
    //SetDateTime(DateTime, 0);
}

/*****************************************************************************/
/**
  * @brief  Get date time from RTC.
  * @param  None
  * @retval None
  */
DateTime_t RTC_GetDateTime (void)
{	
    CounterToStruct(CurrentCounter, &LastRtcTime, 1);
    return LastRtcTime;
}
/*****************************************************************************/
/**
  * @brief  Lay bien dem trong RTC
  * @param  None
  * @retval None
  */
uint32_t RTC_GetCounter(void)
{        
    return CurrentCounter;
}
/*****************************************************************************/
/**
  * @brief  Cap nhat thoi gian vao bien tam.
  * @param  None
  * @retval None
  */
 void RTC_Tick(uint32_t diff_sec)
{
    CurrentCounter += diff_sec;
//    CounterToStruct(CurrentCounter, &LastRtcTime, 1);
}
/*****************************************************************************/
/**
  * @brief  Kiem tra xem thoi gian truyen vao co hop le hay khong
  * @param  None
  * @retval 1 neu valid, 0 neu invalid
  */
uint8_t ValidDateTime(DateTime_t ThoiGian)
{
    if(ThoiGian.Year < 14 || ThoiGian.Year == 0) return 0;
    if(ThoiGian.Month > 12 || ThoiGian.Month == 0) return 0;
    if(ThoiGian.Day > 31 || ThoiGian.Day == 0) return 0;
    if(ThoiGian.Hour > 23) return 0;
    if(ThoiGian.Minute > 59) return 0;
    if(ThoiGian.Second > 59) return 0;
    
    return 1;
}




/*****************************************************************************/
/**
  * @brief  Set date time for RTC
  * @param  None
  * @retval None
  */
static void RtcHandler(nrf_drv_rtc_int_type_t int_type)
{
    if (int_type == NRF_DRV_RTC_INT_COMPARE0)
    {
        
    }
    else if (int_type == NRF_DRV_RTC_INT_TICK)
    {

    }
}
/*******************************************************************************
**
	Function Name  : counter_to_struct
	* Description    : populates time-struct based on counter-value
	* Input          : - counter-value (unit seconds, 0 -> 1.1.2000 00:00:00),
	*                  - Pointer to time-struct
	* Output         : time-struct gets populated, DST not taken into account here
	* Return         : none
	* Based on code from Peter Dannegger found in the mikrocontroller.net forum.
*/
static void CounterToStruct(uint32_t sec, DateTime_t *t, uint8_t CalYear)
{
	uint16_t day;
	uint8_t year;
	uint16_t dayofyear;
	uint8_t leap400;
	uint8_t month;
	t->Second = sec % 60;
	sec /= 60;
	t->Minute = sec % 60;
	sec /= 60;
	t->Hour = sec % 24;
	if(CalYear == 0) 
          return;
	day = (uint16_t)(sec / 24);
	year = FIRSTYEAR % 100;                         // 0..99
	leap400 = 4 - ((FIRSTYEAR - 1) / 100 & 3);      // 4, 3, 2, 1
	for(;;)
        {
          dayofyear = 365;
          if( (year & 3) == 0 ) 
          {
            dayofyear = 366;                                        // leap year
            if( year == 0 || year == 100 || year == 200 )
            { // 100 year exception
              if( --leap400 )
              {                       // 400 year exception
                dayofyear = 365;
               }
            }
          }
          if( day < dayofyear ) 
          {
            break;
          }
          day -= dayofyear;
          year++;                                 // 00..136 / 99..235
	}
	t->Year = year + FIRSTYEAR / 100 * 100 - 2000; // + century
	if( dayofyear & 1 && day > 58 )
        {       // no leap year and after 28.2.
          day++;                                  // skip 29.2.
	}
	for(month = 1; day >= DaysInMonth[month-1]; month++ ) 
        {
          day -= DaysInMonth[month-1];
	}
	t->Month = month;                               // 1..12
	t->Day = day + 1;                              // 1..31
}
/*******************************************************************************
** 
	* Function Name  : struct_to_counter
	* Description    : calculates second-counter from populated time-struct
	* Input          : Pointer to time-struct
	* Output         : none
	* Return         : counter-value (unit seconds, 0 -> 1.1.2000 00:00:00),
	*  Based on code from "LalaDumm" found in the mikrocontroller.net forum.
	*/
static uint32_t StructToCounter(DateTime_t *t )
{
	uint16_t i;
	uint32_t result = 0;
	uint16_t idx, year;
	year = t->Year + 2000;
	/* Calculate days of years before */
	result = (uint32_t)year * 365;
	if (t->Year >= 1) 
        {
          result += (year + 3) / 4;
          result -= (year - 1) / 100;
          result += (year - 1) / 400;
	}
	/* Start with 2000 a.d. */
	result -= 730485UL;
	/* Make month an array index */
	idx = t->Month - 1;
	/* Loop thru each month, adding the days */
	for (i = 0; i < idx; i++) 
        {
          result += DaysInMonth[i];
	}
	/* Leap year? adjust February */
	if (year%400 == 0 || (year%4 == 0 && year%100 !=0))
        {
			;
	} 
        else
        {
          if (t->Month > 2) 
          {
            result--;
          }
	}

	/* Add remaining days */
	result += t->Day;
	/* Convert to seconds, add all the other stuff */
	result = (result - 1) * 86400L + (uint32_t)t->Hour * 3600 +
			(uint32_t)t->Minute * 60 + t->Second;

	return result;
}