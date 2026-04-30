/**********************************************************************
*
* SOURCE FILENAME:	CabBus.h
*
* DATE CREATED:		8/19/98
*
* PROGRAMMER:		Karl Kobel
*
* DESCRIPTION:  	
*
* COPYRIGHT (c) 1999-2002 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/
#ifndef CABBUS_H
#define CABBUS_H

#include "Cab.h"
#include "Main.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

//#define MAX_CABS			16
#define MAX_CABS			10

#define NCE_TEXT_FLAG_1		0x01
#define NCE_TEXT_FLAG_2		0x02
#define NCE_TEXT_FLAG_3		0x04
#define NCE_TEXT_FLAG_4		0x08
#define NCE_NEW_CAB_FLAG	0x10


//#define CABBUS_PORT			SERIAL1


typedef struct
{
	char				TextQuadrant[4][8];
	//char				cTTYChar;
	uint16_t			nCabTimeout;
	unsigned char		nCursor;
	//BOOL				bCursorOn;
	unsigned char		OldSpeed;
	unsigned char		UpLimit;
	unsigned char		DownLimit;
	//BOOL				bVersionReturn;
	uint16_t				nQueue1;
	uint16_t				nQueue2;
	uint16_t				nQueue3;
	uint16_t				nQueue4;
	//unsigned int	  	Flags;
	char				bCabType;
	VIRTUAL_CAB*		pVirtualCab;

	unsigned int		CursonOn :1;
	unsigned int		TextQuadrant1 :1;
	unsigned int		TextQuadrant2 :1;
	unsigned int		TextQuadrant3 :1;
	unsigned int		TextQuadrant4 :1;
	unsigned int		NewCab :1;
	unsigned int		VersionReturn :1;

	// Menu stuff
#ifdef MOVE_THIS_HERE
	int				nEvent;
	const MENU*		pMenu;
	int				nMenuIndex;
	//int				nCursor;
	BOOL			CursorState;
	BOOL			bFirstTime;
	char			szEditBuffer[6];
	int				nEditVar;
	int				nListCount;
	int				nMenuShowing;
	char			szTitleSave[9];
#endif
} CAB_BUS;


extern unsigned char abCabActiveQueue[MAX_CABS];
extern unsigned char abCabInactiveQueue[MAX_CABS];
                                     

// NCE Cab Commands
#define CAB_TEXT_1			0xc0
#define CAB_TEXT_2			0xc1
#define CAB_TEXT_3			0xc2
#define CAB_TEXT_4			0xc3
#define CAB_TEXT_5			0xc4
#define CAB_TEXT_6			0xc5
#define CAB_TEXT_7			0xc6
#define CAB_TEXT_8			0xc7
#define MOVE_CURSOR			0xc8
#define TTY_BACK			0xc9
#define TTY_NEXT			0xca
#define GRAPHIC_UPLOAD		0xcb
#define GRAPHIC_CHARACTER	0xcc
#define CLEAR_DISPLAY		0xcd
#define CURSOR_OFF			0xce
#define CURSOR_ON			0xcf
#define SHIFT_DISPLAY_LEFT	0xd0
#define SHIFT_DISPLAY_RIGHT	0xd1
#define GET_VERSION			0xd2
#define SETUP				0xd3


// NCE Cab Keys
#define CK_ENTER			0x40
#define CK_PROG_MODE		0x41
#define CK_RECALL			0x42
#define CK_DIRECTION		0x43
#define CK_CONSIST_SETUP	0x44
#define CK_CONSIST_ADD		0x45
#define CK_CONSIST_DELETE	0x46
#define CK_CONSIST_KILL		0x47
#define CK_SELECT_LOCO		0x48
#define CK_HORN_DOWN		0x49
#define CK_SPEED_INC		0x4a
#define CK_SPEED_DEC		0x4b
#define CK_EMERGENCY_STOP	0x4c
#define CK_BELL				0x4d
#define CK_SELECT_ACCESSORY	0x4e
#define CK_EXPANSION		0x4f
#define CK_ZERO				0x50
#define CK_ONE				0x51
#define CK_TWO				0x52
#define CK_THREE			0x53
#define CK_FOUR				0x54
#define CK_FIVE				0x55
#define CK_SIX				0x56
#define CK_SEVEN			0x57
#define CK_EIGHT			0x58
#define CK_NINE				0x59
#define CK_SPEED_INC_FAST	0x5a
#define CK_SPEED_DEC_FAST	0x5b
#define CK_MACRO			0x5c
#define CK_SPEED_MODE		0x5d
#define CK_BRAKE			0x5e
#define CK_HORN_UP			0x5f
#define CK_ASSIGN_LOCO		0x60
#define CK_PROG_ON_MAIN		0x61
#define CK_SET_CLOCK		0x62
#define CK_PROG_ON_TRACK	0x63
#define CK_SETUP_CS			0x64
#define CK_SETUP_CAB		0x65
#define CK_DEFINE_MACRO		0x66
#define CK_CONSIST_SETUP_OLD 0x67
#define CK_CONSIST_SETUP_ADV 0x68
#define CK_TOGGLE_LEFT_RIGHT 0x69
#define CK_DIR_FORWARD		0x6a
#define CK_DIR_REVERSE		0x6b
#define CK_KEY_32			0x6c


/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

extern void HandleNCECabCommunication(void);

extern void InitCabCommunication(unsigned char Ports);

extern void NCE_DisplayMessage(unsigned char bCab, unsigned char x, unsigned char y, char* pcMessage);
extern void NCE_DisplayChar(unsigned char bCab, unsigned char c);

extern void NCE_RefreshDisplay(unsigned char bCab);

extern void NCE_SetCursorPosition(unsigned char bCab, unsigned char x, unsigned char y);
extern void NCE_CursorOn(unsigned char bCab);
extern void NCE_CursorOff(unsigned char bCab);


extern void UpdateWangrowClock(void);
extern void NCE_SuppressClock(unsigned char bClock);


#endif

