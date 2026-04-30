/**********************************************************************
*
* SOURCE FILENAME:	Clock.c
*
* DATE CREATED:		29/Mar/00
*
* PROGRAMMER:			Karl Kobel
*
* DESCRIPTION:
*
* COPYRIGHT (c) 2000-2017 by K2 Engineering. All Rights Reserved.
*
**********************************************************************
*
* VERSION CONTROL:
*
* $Revision: $
*
* $Log: $
*
**********************************************************************/
#include "main.h"

#include "Main.h"
#include <stdio.h>
#include "Cab.h"
#include "CabNCE.h"
#include "States.h"
#include "Events.h"
//#include "Queue.h"
#include "Text.h"
#include "Settings.h"


/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define NO_OF_CLOCK_UPDATERS	4

/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/


/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

extern RTC_HandleTypeDef hrtc;

//uint8_t ClockChanged;

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/

//static RTC_TimeTypeDef sTime;
//static RTC_TimeTypeDef sFastTime;
//static RTC_DateTypeDef sDate;

// configuration settings
//byte nClockRatio;
//BOOL bClock12_24;


//static unsigned int nMinutes;

//int (*p[4]) (int x, int y);

static void (*ClockUpdate[NO_OF_CLOCK_UPDATERS])(void);

/**********************************************************************
*
*							CODE
*
**********************************************************************/

#ifdef NOT_USED
/**********************************************************************
*
* FUNCTION:
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
uint32_t TimeToDecimal(RTC_TimeTypeDef* sTime)
{
	uint32_t time;

	time = sTime->Seconds;
	time += sTime->Minutes * 60;
	time += sTime->Hours * (60 * 60);

	return time;
}


/**********************************************************************
*
* FUNCTION:
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void DecimalToTime(uint32_t time, RTC_TimeTypeDef* sTime)
{

	//sTime->Hours = time / (60 * 60);
	//sTime->Minutes = time / 60;
	//sTime->Seconds = time % 60;

	sTime->Seconds = time % 60;
	time /= 60;
	sTime->Minutes = time % 60;
	time /= 60;
	sTime->Hours = time % 24;
}


/**********************************************************************
*
* FUNCTION:
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void HandleClock(void)
{
	uint32_t time;
	static uint32_t old_time;

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_HOURFORMAT_24);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	time = TimeToDecimal(&sTime);
	time += HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0);
	time *= nClockRatio;

	DecimalToTime(time, &sFastTime);

	if(sFastTime.Minutes != old_time)
	{
		old_time = sFastTime.Minutes;
		ClockChanged = 1;
	}
}


/**********************************************************************
*
* FUNCTION:		GetClock
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
RTC_TimeTypeDef GetClock(void)
{

	return sTime;
}


/**********************************************************************
*
* FUNCTION:		GetFastClock
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
RTC_TimeTypeDef GetFastClock(void)
{

	return sFastTime;
}



/**********************************************************************
*
* FUNCTION:		MarkClockChanged
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void MarkClockChanged(void)
{

	ClockChanged = 1;
}


/**********************************************************************
*
* FUNCTION:		SetClock
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void SetClock(byte bHrs, byte bMins)
{

	sTime.Hours = bHrs;
	sTime.Minutes = bMins;
	sTime.Seconds = 0;

	HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0);

	HandleClock();		// update the fast clock

	ClockChanged = 1;
}


/**********************************************************************
*
* FUNCTION:		SetFastClock
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:	This sets the fast clock as an offset to the real time clock
*
* RESTRICTIONS:
*
**********************************************************************/
void SetFastClock(byte bHrs, byte bMins)
{
	uint32_t offset;
	uint32_t time;
	uint32_t fasttime;

	time = TimeToDecimal(&sTime);
	sFastTime.Hours = bHrs;
	sFastTime.Minutes = bMins;
	sFastTime.Seconds = 0;
	fasttime = TimeToDecimal(&sFastTime) / nClockRatio;

	offset = fasttime - time;
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, offset);

	//bClock12_24 = b12_24;
	//nClockRatio = bRatio;

	HandleClock();		// update the fast clock

	ClockChanged = 1;
}

/**********************************************************************
*
* FUNCTION:		GetClockString
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void GetClockString(char* szTime)
{
	RTC_TimeTypeDef sFastTime;


	sFastTime = GetFastClock();

	if(bClock12_24)
	{
		// 24 hour mode
		sprintf(szTime, " %2d:%02d  ", sFastTime.Hours, sFastTime.Minutes);
	}
	else
	{
		// 12 hour mode
		if(sFastTime.Hours > 12)
		{
			sFastTime.Hours -= 12;
			sprintf(szTime, " %2d:%02dAM", sFastTime.Hours, sFastTime.Minutes);
		}
		else
		{
			sprintf(szTime, " %2d:%02dPM", sFastTime.Hours, sFastTime.Minutes);
		}
	}
}

#endif

/**********************************************************************
*
* FUNCTION:		RegisterClockUpdate
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void RegisterClockUpdate(void (*ClockUpdateCallback)(void))
{
	int i;

	for(i = 0; i < NO_OF_CLOCK_UPDATERS; i++)
	{
		if(ClockUpdate[i] == NULL)
		{
			ClockUpdate[i] = ClockUpdateCallback;
			break;
		}
	}
}


/**********************************************************************
*
* FUNCTION:		CheckClockUpdate
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void CheckClockUpdate(void)
{
	int i;
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	static uint8_t OldMinute = 0;
	static int ClockCount = 100;

	ClockCount--;
	if(ClockCount == 0)
	{
		ClockCount = 100;

		if(HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN) == HAL_OK)
		{
			if(time.Minutes != OldMinute);
			OldMinute = time.Minutes;

			for(i = 0; i < NO_OF_CLOCK_UPDATERS; i++)
			{
				if(ClockUpdate[i] != NULL)
				{
					ClockUpdate[i]();
				}
			}

		}
		// call the date read to unlock the clock
		HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	}
}



