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

#include "main.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

//#define LED_OFF		0
//#define LED_ON		1
//#define LED_BLINK		2

#define LED_OFF         0x00000000
#define LED_ON          0xffffffff

#define LED_POWERUP     0x33333333
#define LED_NORMAL      0x00010001
#define LED_NORMAL2     0x01000100
#define LED_BLINK   	0x00ff00ff
#define LED_FAST_BLINK	0x55555555

// Blink Codes
#define LED_EBLINK_1	0x0000000f
#define LED_EBLINK_2    0x00007007
#define LED_EBLINK_3    0x00030303
#define LED_EBLINK_4    0x00001111
#define LED_EBLINK_5    0x00011111
#define LED_EBLINK_6    0x00111111
#define LED_EBLINK_7    0x00049249
#define LED_EBLINK_8    0x00249249


#define RED_LED			LED3_Pin
#define RED_LED_PORT	LED3_GPIO_Port
#define YELLOW_LED		LED2_Pin
#define YELLOW_LED_PORT	LED2_GPIO_Port
#define GREEN_LED		LED1_Pin
#define GREEN_LED_PORT	LED1_GPIO_Port



/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

//extern void StatusLed(int s);

extern void GreenLed(uint32_t nPattern);
extern void RedLed(uint32_t nPattern);
extern void YellowLed(uint32_t nPattern);
extern uint32_t GetGreenLed(void);
extern uint32_t GetRedLed(void);
extern uint32_t GetYellowLed(void);

extern void LedTask(void *argument);

#endif
