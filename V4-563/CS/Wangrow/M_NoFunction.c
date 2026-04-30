/**********************************************************************
*
* SOURCE FILENAME:	M_NoFunction.c
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

void ExitNoFunction(VIRTUAL_CAB* pVirtualCab, int nEvent);

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
	{EVENT_TIMER_EXPIRED,	0,			ExitNoFunction},
	{0, 0, NULL}
};


/**********************************************************************
*
*							CODE
*
**********************************************************************/

/**********************************************************************
*
* FUNCTION:	   StateNoFunction
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
void StateNoFunction(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	if(nEvent == EVENT_STATE_ENTER)
	{
												  //1234567890123456
		NCE_DisplayMessage(pVirtualCab->Cab, 0, 1, "NOT IMPLEMENTED ");
		pVirtualCab->nMenuShowing = MENU_SHOWING_LINE_2L | MENU_SHOWING_LINE_2R;
		SetExpiration(pVirtualCab, EXPIRATION_1_SECONDS);
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
* FUNCTION:	   ExitNoFunction
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
void ExitNoFunction(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	//ExitMenu(pVirtualCab);

	pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
	RestoreOperateScreen(pVirtualCab, 0, 1);

	ChangeState(pVirtualCab, STATE_IDLE);
}


