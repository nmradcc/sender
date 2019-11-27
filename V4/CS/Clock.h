/**********************************************************************
*
* SOURCE FILENAME:	Cab.h
*
* DATE CREATED:		5-Jan-1999
*
* PROGRAMMER:		Karl Kobel
*
* DESCRIPTION:  	
*
* COPYRIGHT (c) 2000-2002 by K2 Engineering  All Rights Reserved.
*
**********************************************************************
*
* VERSION CONTROL:
*
* $Revision: $
*
* $Log: $
*
*********************************************************************/
#ifndef CLOCK_H
#define CLOCK_H

#include "main.h"
//#include "stm32l4xx_hal.h"


/**********************************************************************
*
*							Usage
*
**********************************************************************/

// time    displays the RTC and the fast time
//
// time hh:mm    sets the RTC
//
// time fast hh:mm    sets the fast clock (the word "fast" is just a place holder to differentiate the fast vs RTC)
//
//
//
// If you cab needs a fast clock, there is a clock update register function.
//
//    RegisterClockUpdate(<your update function>);
//
// There are currently four slots.
//
// The function will get called whenever the fast clock changes.
//

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/
#ifdef NOT_USED
extern RTC_TimeTypeDef GetClock(void);
extern RTC_TimeTypeDef GetFastClock(void);

extern void SetClock(byte bHrs, byte bMins);
extern void SetFastClock(byte bHrs, byte bMins);

extern void HandleClock(void);

extern void MarkClockChanged(void);

extern void GetClockString(char* szTime);

#endif

extern void RegisterClockUpdate(void (*ClockUpdateCallback)(void));

extern void CheckClockUpdate(void);

#endif
