/* Martin Thomas 4/2009 */

#include "integer.h"
#include "main.h"
#include <stdio.h>

extern RTC_HandleTypeDef hrtc;


DWORD get_fattime (void)
{
	DWORD res;
	//RTC_t rtc;
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	//rtc_gettime( &rtc );
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	if(HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN) == HAL_OK)
	{
		res =  (((DWORD)date.Year + 20) << 25)	// normalize to 1980
				| ((DWORD)date.Month << 21)
				| ((DWORD)date.Date << 16)
				| (WORD)(time.Hours << 11)
				| (WORD)(time.Minutes << 5)
				| (WORD)(time.Seconds >> 1);
	}
	else
	{
		res =  0;
	}
	return res;
}

void fat_to_time(WORD fattime, char* szTime)
{
	int hours;
	int minutes;
	int seconds;

	hours = (fattime >> 11) & 0x1f;
	minutes = (fattime >> 5) & 0x3f;
	seconds = (fattime & 0x1f) * 2;
	
	sprintf(szTime, "%2d:%02d:%02d", hours, minutes, seconds);

}

void fat_to_date(WORD fatdate, char* szDate)
{
	int date;
	int month;
	int year;

	year = (fatdate >> 9) & 0x7f;
	month = (fatdate >> 5) & 0xf;
	date = fatdate & 0x1f;
	
	sprintf(szDate, "%2d/%02d/%04d", date, month, year+1980);
}
