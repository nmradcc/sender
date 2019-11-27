/**********************************************************************
*
* SOURCE FILENAME:	M_EStop.c
*
* DATE CREATED:		11-Apr-2000
*
* PROGRAMMER:		Karl Kobel
*
* DESCRIPTION:
*
* COPYRIGHT (c) 2000-2017 by K2 Engineering. All Rights Reserved.
*
*********************************************************************/
#include "Main.h"
#include <stdio.h>
#include <ctype.h>

#include "states.h"
#include "Events.h"
#include "Track.h"
#include "Text.h"
#include "CabNCE.h"

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

void ExitEStop(VIRTUAL_CAB* pVirtualCab, int nEvent);
extern void RestoreOperateScreen(VIRTUAL_CAB* pVirtualCab, unsigned char Line1, unsigned char Line2);

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

unsigned char eStopCab;

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/
static const EVENT_MAP FunctionMap[] =
{
	{EVENT_PROG_MODE,		STATE_IDLE,			NULL},
	{EVENT_ENTER,			STATE_IDLE,			NULL},
//	{EVENT_EXPANSION,		STATE_IDLE,			NULL},
//	{EVENT_SPEED_MODE,		STATE_IDLE,			NULL},
	{0, 0, NULL}
};


/**********************************************************************
*
*							CODE
*
**********************************************************************/

/**********************************************************************
*
* FUNCTION:	   StateSelectLoco
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
void StateEStop(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	char szTemp[17];


	if(nEvent == EVENT_STATE_ENTER)
	{
		sprintf(szTemp, aText[tE_Stop], eStopCab);
		NCE_DisplayMessage(0, 0, 0, szTemp);
		NCE_DisplayMessage(0, 0, 1, aText[tLayoutStopped]);

		DisableTrack();
//k		DisableProgrammingTrack();

		pVirtualCab->nMenuShowing = MENU_SHOWING_LINE_1L | MENU_SHOWING_LINE_2L | MENU_SHOWING_LINE_2R | MENU_SHOWING_ESTOP;
	}
	else if(nEvent == EVENT_STATE_EXIT)
	{
		pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
		RestoreOperateScreen(pVirtualCab, 1, 1);

		ChangeState(pVirtualCab, STATE_IDLE);

		EnableTrack();
//k		EnableProgrammingTrack();
	}
	else if(!DispatchEvent(pVirtualCab, nEvent, FunctionMap))
	{
		// special event processing not handled by DispatchEvent()
	}
}



