/**********************************************************************
*
* SOURCE FILENAME:	M_Send.c
*
* DATE CREATED:		10-Dec-2020
*
* PROGRAMMER:		Karl Kobel
*
* DESCRIPTION:
*
* COPYRIGHT (c) 2000-2020 by K2 Engineering. All Rights Reserved.
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
#include "Text.h"
#include "Cab.h"
#include "CabNCE.h"
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

void ExitSendDisplay(VIRTUAL_CAB* pVirtualCab, int nEvent);

extern void RestoreOperateScreen(VIRTUAL_CAB* pVirtualCab, unsigned char Line1, unsigned char Line2);

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

extern int bfFMode;

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/
static const EVENT_MAP IdleMap[] =
{
//	{EVENT_TIMER_EXPIRED,	0,			ExitSendDisplay},
	{0, 0, NULL}
};


/**********************************************************************
*
*							CODE
*
**********************************************************************/

/**********************************************************************
*
* FUNCTION:	   StateSend
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
void StateSend(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	if(nEvent == EVENT_STATE_ENTER)
	{
												  //1234567890123456
		NCE_DisplayMessage(pVirtualCab->Cab, 0, 0, "SEND IN PROGRESS");
		pVirtualCab->nMenuShowing = MENU_SHOWING_LINE_1L | MENU_SHOWING_LINE_1R | MENU_SHOWING_LINE_2L | MENU_SHOWING_LINE_2R;
	}
	else if(nEvent == EVENT_STATE_EXIT)
	{
	}
	else if(!DispatchEvent(pVirtualCab, nEvent, IdleMap))
	{
		// special event processing not handled by DispatchEvent()
	}
}



/**********************************************************************
*
* FUNCTION:	   ExitSendDisplay
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
void ExitSendDisplay(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	//ExitMenu(pVirtualCab);

	pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
	RestoreOperateScreen(pVirtualCab, 0, 1);

	ChangeState(pVirtualCab, STATE_IDLE);
}


void SendCabMessage(VIRTUAL_CAB* pVirtualCab, char* msg)
{
	NCE_DisplayMessage(pVirtualCab->Cab, 0, 1, msg);
}
