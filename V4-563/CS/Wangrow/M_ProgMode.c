/**********************************************************************
*
* SOURCE FILENAME:	M_Idle.c
*
* DATE CREATED:		10-Feb-2000
*
* PROGRAMMER:			Karl Kobel
*
* DESCRIPTION:
*
* COPYRIGHT (c) 2000-2002 by K2 Engineering. All Rights Reserved.
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
#include "Main.h"
#include <stdio.h>
#include "states.h"
#include "Events.h"
#include "editor.h"
#include "menu.h"
#include "Cab.h"
#include "Text.h"
#include "States.h"
#include "Expiration.h"

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

void DoProgMode(VIRTUAL_CAB* pVirtualCab, int nEvent);

void ExitProgMode(VIRTUAL_CAB* pVirtualCab, int nEvent);

extern void RestoreOperateScreen(VIRTUAL_CAB* pVirtualCab, unsigned char Line1, unsigned char Line2);

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/
static const EVENT_MAP ProgModeMap[] =
{
	{EVENT_ENTER,			0,			HandleEnter},
	{EVENT_ZERO,			0,			ExitProgMode},
	{EVENT_EXPANSION,		0,			ExitProgMode},
	{EVENT_TIMER_EXPIRED,	0,			ExitProgMode},
	{0, 0, NULL}
};


static int temp;

const char *ProgModeList[] =
{

// ToDo localize this

//  1234567890123456
	"Program On Main ",
//	"Assign Loco->Cab",
//	"Set System Clock",
	"Program Decoder ",
//	"Set Cmd Station ",
//	"Set Cab         ",
//	"Prog Accessories",
//	"Program Macros  ",
	NULL
};


const MENU_DEF  ProgModeMenuItems[] =
{
	// Prompt		format			variable		edit function  enter function
	{0,			ProgModeList,	&temp,		EditList,		DoProgMode},
	{0,NULL}
};

const MENU ProgModeMenu =
{
	ProgModeMenuItems,
	NULL,
	tNull,
   ExitProgMode
};

/**********************************************************************
*
*							CODE
*
**********************************************************************/

/**********************************************************************
*
* FUNCTION:	   StateIdle
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
void StateProgMode(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	if(nEvent == EVENT_STATE_ENTER)
	{
	   temp = 0;
		NewMenu(pVirtualCab, &ProgModeMenu);
		pVirtualCab->nMenuShowing = MENU_SHOWING_LINE_2L | MENU_SHOWING_LINE_2R;
		SetExpiration(pVirtualCab, EXPIRATION_5_SECONDS);
	}
	else if(nEvent == EVENT_STATE_EXIT)
	{
	}
	else if(!DispatchEvent(pVirtualCab, nEvent, ProgModeMap))
	{
		// special event processing not handled by DispatchEvent()
		if(pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].EditFunc)
		{
			pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].EditFunc(pVirtualCab, nEvent);
		}
	}
}


/**********************************************************************
*
* FUNCTION:	   DoProgMode
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
void DoProgMode(VIRTUAL_CAB* pVirtualCab, int nEvent)
{


	//pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
   //RestoreOperateScreen(pVirtualCab, FALSE, TRUE);
   //ChangeState(pVirtualCab, STATE_IDLE);

	switch(temp)
   {
    	case 0:		// Ops Programming
			ExitMenu(pVirtualCab);
			ChangeState(pVirtualCab, STATE_PROGRAM_ON_MAIN);
      	break;

    	case 1:		// Assign Loco to Cab
//			ExitProgMode(pVirtualCab, nEvent);
//      	break;

//    	case 2:		// Set Clock
////			ChangeState(pVirtualCab, STATE_SET_SYSTEM_CLOCK);
//      	break;

//    	case 3:		// Program on Programming Track
			ExitMenu(pVirtualCab);
			ChangeState(pVirtualCab, STATE_PROGRAM_ON_TRACK);
      	break;

//    	case 4:		// Set Command Station
//			ExitProgMode(pVirtualCab, nEvent);
//      	break;

//    	case 5:		// Set Cab
//			ExitProgMode(pVirtualCab, nEvent);
//      	break;

//    	case 6:		// Program Accessories
//			ExitProgMode(pVirtualCab, nEvent);
//      	break;

//    	case 7:		// Program Macros
//			ExitProgMode(pVirtualCab, nEvent);
////			ChangeState(pVirtualCab, STATE_PROGRAM_MACROS);
//      	break;

   }

}


/**********************************************************************
*
* FUNCTION:	   ExitProgMode
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
void ExitProgMode(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	ExitMenu(pVirtualCab);

	pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
   RestoreOperateScreen(pVirtualCab, 0, 1);

	ChangeState(pVirtualCab, STATE_IDLE);
}


