/**********************************************************************
*
* SOURCE FILENAME:	M_Track.c
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
#include "Track.h"

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

void ExitTrackDisplay(VIRTUAL_CAB* pVirtualCab, int nEvent);
void TrackStatus(VIRTUAL_CAB* pVirtualCab, int nEvent);

extern void RestoreOperateScreen(VIRTUAL_CAB* pVirtualCab, unsigned char Line1, unsigned char Line2);

void DisplayTrackStatus(VIRTUAL_CAB* pVirtualCab);

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
	{EVENT_ENTER,			0,			ExitTrackDisplay},
	{EVENT_UPDATE_TRACK,	0,			TrackStatus},
	{0, 0, NULL}
};


/**********************************************************************
*
*							CODE
*
**********************************************************************/


/**********************************************************************
*
* FUNCTION:	   StateTrack
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
void StateTrack(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	if(nEvent == EVENT_STATE_ENTER)
	{
		pVirtualCab->nMenuShowing = MENU_SHOWING_LINE_1L | MENU_SHOWING_LINE_1R | MENU_SHOWING_LINE_2L | MENU_SHOWING_LINE_2R;
		DisplayTrackStatus(pVirtualCab);
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
* FUNCTION:	   TrackStatus
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
void DisplayTrackStatus(VIRTUAL_CAB* pVirtualCab)
{

	if(IsTrackOpen(TR_NONE))
	{
												  //1234567890123456
		NCE_DisplayMessage(pVirtualCab->Cab, 0, 0, "TRACK FREE      ");
		NCE_DisplayMessage(pVirtualCab->Cab, 0, 1, "PRESS ENTER     ");
	}
	else
	{
												  //1234567890123456
		NCE_DisplayMessage(pVirtualCab->Cab, 0, 0, "TRACK IN USE BY ");
		if(IsTrackOpen(TR_TESTER))
		{
													  //1234567890123456
			NCE_DisplayMessage(pVirtualCab->Cab, 0, 1, "DECODER TEST    ");
		}
		else
		{
			NCE_DisplayMessage(pVirtualCab->Cab, 0, 1, "SHELL COMMANDS  ");
		}
	}
}

/**********************************************************************
*
* FUNCTION:	   TrackStatus
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
void TrackStatus(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	DisplayTrackStatus(pVirtualCab);
}

/**********************************************************************
*
* FUNCTION:	   ExitTrackDisplay
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
void ExitTrackDisplay(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	//ExitMenu(pVirtualCab);

	pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
	RestoreOperateScreen(pVirtualCab, 0, 1);

	ChangeState(pVirtualCab, STATE_IDLE);
}
