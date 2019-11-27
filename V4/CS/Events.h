/**********************************************************************
*
* SOURCE FILENAME:	Events.h
*
* DATE CREATED:		21-Dec-1999
*
* PROGRAMMER:		Karl Kobel
*
* DESCRIPTION:  	
*
* COPYRIGHT (c) 2000-2002 by K2 Engineering  Rights Reserved.
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

//#include "WM.h"

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
#define EVENT_MSG_TYPE	0x80

#define NO_EVENT			0

#define ACC_EMPTY			0xffff

#define EVENT_ENTER				(0x01 | EVENT_MSG_TYPE)
#define EVENT_PROG_MODE			(0x02 | EVENT_MSG_TYPE)
#define EVENT_RECALL			(0x03 | EVENT_MSG_TYPE)
#define EVENT_DIRECTION			(0x04 | EVENT_MSG_TYPE)
#define EVENT_CONSIST_SETUP		(0x05 | EVENT_MSG_TYPE)
#define EVENT_CONSIST_ADD		(0x06 | EVENT_MSG_TYPE)
#define EVENT_CONSIST_DELETE	(0x07 | EVENT_MSG_TYPE)
#define EVENT_CONSIST_KILL		(0x08 | EVENT_MSG_TYPE)
#define EVENT_SELECT_LOCO		(0x09 | EVENT_MSG_TYPE)
#define EVENT_HORN_DOWN			(0x0a | EVENT_MSG_TYPE)
#define EVENT_SPEED_INC			(0x0b | EVENT_MSG_TYPE)
#define EVENT_SPEED_DEC			(0x0c | EVENT_MSG_TYPE)
#define EVENT_EMERGENCY_STOP	(0x0d | EVENT_MSG_TYPE)
#define EVENT_BELL				(0x0e | EVENT_MSG_TYPE)
#define EVENT_SELECT_ACCESSORY	(0x0f | EVENT_MSG_TYPE)
#define EVENT_EXPANSION			(0x10 | EVENT_MSG_TYPE)
#define EVENT_ZERO				0x30
#define EVENT_ONE				0x31
#define EVENT_TWO				0x32
#define EVENT_THREE				0x33
#define EVENT_FOUR				0x34
#define EVENT_FIVE				0x35
#define EVENT_SIX				0x36
#define EVENT_SEVEN				0x37
#define EVENT_EIGHT				0x38
#define EVENT_NINE				0x39
#define EVENT_SPEED_INC_FAST	(0x1b | EVENT_MSG_TYPE)
#define EVENT_SPEED_DEC_FAST	(0x1c | EVENT_MSG_TYPE)
#define EVENT_SELECT_MACRO	  	(0x1d | EVENT_MSG_TYPE)
//#define EVENT_MACRO			(0x1d | EVENT_MSG_TYPE)
#define EVENT_SPEED_MODE		(0x1e | EVENT_MSG_TYPE)
#define EVENT_BRAKE				(0x1f | EVENT_MSG_TYPE)
#define EVENT_HORN_UP			(0x20 | EVENT_MSG_TYPE)
#define EVENT_ASSIGN_LOCO		(0x21 | EVENT_MSG_TYPE)
#define EVENT_PROG_ON_MAIN		(0x22 | EVENT_MSG_TYPE)
#define EVENT_SET_CLOCK			(0x23 | EVENT_MSG_TYPE)
#define EVENT_PROG_ON_TRACK		(0x24 | EVENT_MSG_TYPE)
#define EVENT_SETUP_CS			(0x25 | EVENT_MSG_TYPE)
#define EVENT_SETUP_CAB			(0x26 | EVENT_MSG_TYPE)
#define EVENT_DEFINE_MACRO		(0x27 | EVENT_MSG_TYPE)
#define EVENT_CONSIST_SETUP_OLD (0x28 | EVENT_MSG_TYPE)
#define EVENT_CONSIST_SETUP_ADV (0x29 | EVENT_MSG_TYPE)
#define EVENT_TOGGLE_LEFT_RIGHT (0x2a | EVENT_MSG_TYPE)
#define EVENT_DIR_FORWARD		(0x2b | EVENT_MSG_TYPE)
#define EVENT_DIR_REVERSE		(0x2c | EVENT_MSG_TYPE)
#define EVENT_KEY_32			(0x2d | EVENT_MSG_TYPE)
//#define EVENT_SELECT_MACRO	(0x2d | EVENT_MSG_TYPE)

//#define EVENT_SPEED			(0x30 | EVENT_MSG_TYPE)
#define EVENT_SPEED_TYPE		0x200

#define EVENT_STATE_ENTER		(0x40 | EVENT_MSG_TYPE)
#define EVENT_STATE_EXIT 		(0x41 | EVENT_MSG_TYPE)

#define EVENT_CLEAR_DATA		(0x50 | EVENT_MSG_TYPE)
#define EVENT_GET_DATA			(0x51 | EVENT_MSG_TYPE)
#define EVENT_STORE_DATA		(0x52 | EVENT_MSG_TYPE)
#define EVENT_TIMER_EXPIRED		(0x53 | EVENT_MSG_TYPE)

#define EVENT_HEADLIGHT			(0x54 | EVENT_MSG_TYPE)
#define EVENT_TOGGLE_ACCESSORY	(0x55 | EVENT_MSG_TYPE)

#define EVENT_SIMPLE_ACK		(0x56 | EVENT_MSG_TYPE)
#define EVENT_ADVANCED_ACK		(0x57 | EVENT_MSG_TYPE)

#define EVENT_UPDATE_CLOCK		(0x60 | EVENT_MSG_TYPE)

#define EVENT_FUNCTION_HL		0x40
#define EVENT_FUNCTION_1		0x41
#define EVENT_FUNCTION_2		0x42
#define EVENT_FUNCTION_3		0x43
#define EVENT_FUNCTION_4		0x44
#define EVENT_FUNCTION_5		0x45
#define EVENT_FUNCTION_6		0x46
#define EVENT_FUNCTION_7		0x47
#define EVENT_FUNCTION_8		0x48
#define EVENT_FUNCTION_9		0x49
#define EVENT_FUNCTION_10		0x4a
#define EVENT_FUNCTION_11		0x4b
#define EVENT_FUNCTION_12		0x4c


#ifdef NOT_USED
// Window Manager Messages

enum WM_USER_MESSAGES
{
	WM_USER_NEW_LOCO = WM_USER,
	WM_USER_SPEED_CHANGE,
	WM_USER_DIRECTION,
	WM_USER_FUNCTION_CHANGE,
	WM_USER_NEW_ACCESSORY,
	WM_USER_ACCESSORY_STATE_CHANGE,
	WM_USER_CAB_STATUS_CHANGE,
	WM_USER_1,
	WM_USER_2,
	WM_USER_3,
	WM_USER_4
};
#endif


