#ifndef _RTC_H_
#define _RTC_H_

//#include "DataDefine.h"
typedef struct
{
    uint8_t Year;
    uint8_t Month;
    uint8_t Day;
    uint8_t Hour;
    uint8_t Minute;
    uint8_t Second;
} DateTime_t;
/**
  * @brief  Initiatlize the RTC module
  * @param  None
  * @retval None
  */
 void RTC_Init();
 /**
  * @brief  Set date time for RTC
  * @param  None
  * @retval None
  */
 void RTC_SetDateTime (DateTime_t Input, int32_t GMT_Adjust);
 /**
  * @brief  Get date time from RTC.
  * @param  None
  * @retval None
  */
/**
 DateTime_t RTC_GetDateTime (void);
  * @brief  Lay bien dem trong RTC
  * @param  None
  * @retval None
  */
 static uint32_t RTC_GetCounter(void);

 /*****************************************************************************/
/**
  * @brief  Cap nhat thoi gian vao bien tam.
  * @param  None
  * @retval None
  */
 void RTC_Tick(uint32_t diff_sec);
 /**
  * @brief  Kiem tra xem thoi gian truyen vao co hop le hay khong
  * @param  None
  * @retval 1 neu valid, 0 neu invalid
  */
 uint8_t ValidDateTime(DateTime_t ThoiGian);

 /**
  * @brief  Change the timestamp of current RTC
  * @param  None
  * @retval 
  */
 void RTC_UpdateTimeFromServer(uint32_t unixTimeStamp);
 /**
  * @brief  Get date time from RTC.
  * @param  None
  * @retval None
 */
DateTime_t RTC_GetDateTime (void);
/**
  * @brief  Lay bien dem trong RTC
  * @param  None
  * @retval None
  */
uint32_t RTC_getcounter(void);
#endif