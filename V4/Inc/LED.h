 /**********************************************************************
*
* SOURCE FILENAME:	Track.h
*
* DATE CREATED:		29/Aug/99
*
* PROGRAMMER:
*
* DESCRIPTION:
*
* COPYRIGHT (c) 1999-2017 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/

#ifndef LED_H
#define LED_H

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define LED_OFF			0
#define LED_ON			1
#define LED_BLINK		2



 /**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

extern void StatusLed(int s);

extern void LedTask(void* argument);

#endif
