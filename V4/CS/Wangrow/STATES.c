/**********************************************************************
*
* SOURCE FILENAME:	States.c
*
* DATE CREATED:		21-Dec-1999
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

#include <stdio.h>
#include "states.h"

#include "CabNCE.h"
#include "Events.h"
#include "Loco.h"

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

/**********************************************************************
*
*							CODE
*
**********************************************************************/

/**********************************************************************
*
* FUNCTION:		RunState
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
void RunState(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	int nState;
	
	if(nEvent == EVENT_TIMER_EXPIRED)
	{
		if(pVirtualCab->nMenuShowing & MENU_SHOWING_LINE_1L)
		{
			NCE_DisplayMessage(pVirtualCab->Cab, 0, 0, pVirtualCab->szTitleSave);
		}
	}
	
	if(pVirtualCab == NULL)
	{
		nState = 0;
	}
	else
	{
		nState = pVirtualCab->nState;
	}

	switch(nState)
	{
		case STATE_IDLE:
			StateIdle(pVirtualCab, nEvent);
			break;

		case STATE_SELECT_LOCO:
			StateSelectLoco(pVirtualCab, nEvent);
			break;

		case STATE_SELECT_ACCESSORIES:
			StateSelectAccessory(pVirtualCab, nEvent);
			break;

//		case STATE_SELECT_MACROS:
//k			StateSelectMacro(pVirtualCab, nEvent);
//			break;

//		case STATE_SETUP_CONSIST:
//k			StateSetupConsist(pVirtualCab, nEvent);
//			break;

//		case STATE_ADD_CONSIST:
//k			StateAddConsist(pVirtualCab, nEvent);
//			break;

//		case STATE_DELETE_CONSIST:
//k			StateDeleteConsist(pVirtualCab, nEvent);
//			break;

//		case STATE_CLEAR_CONSIST:
//k			StateClearConsist(pVirtualCab, nEvent);
//			break;

		case STATE_FUNCTIONS:
			StateFunction(pVirtualCab, nEvent);
			break;

//		case STATE_ESTOP:
//			StateEStop(pVirtualCab, nEvent);
//			break;

		case STATE_PROGRAM_ON_MAIN:
			StateProgramOnMain(pVirtualCab, nEvent);
			break;

//		case STATE_ASSIGN_LOCO_CAB:
//			StateAssignLocoCab(pVirtualCab, nEvent);
//			pVirtualCab->nState = STATE_IDLE;
//			break;

//		case STATE_SET_SYSTEM_CLOCK:
//k			StateSetSystemClock(pVirtualCab, nEvent);
//			break;

		case STATE_PROGRAM_ON_TRACK:
			StateProgramTrack(pVirtualCab, nEvent);
			break;

//		case STATE_SET_COMMAND_STATION:
//			StateSetCommandStation(pVirtualCab, nEvent);
//			pVirtualCab->nState = STATE_IDLE;
//			break;

		case STATE_SET_CAB:
//			StateSetCab(pVirtualCab, nEvent);
			pVirtualCab->nState = STATE_IDLE;
			break;

//		case STATE_PROGRAM_ACCESSORIES:
//			StateProgramAccessories(pVirtualCab, nEvent);
//			pVirtualCab->nState = STATE_IDLE;
//			break;

//		case STATE_PROGRAM_MACROS:
//k			StateProgramMacros(pVirtualCab, nEvent);
//			break;

		case STATE_PROG_MODE:
			StateProgMode(pVirtualCab, nEvent);
			break;

		case STATE_NO_FUNCTION:
			StateNoFunction(pVirtualCab, nEvent);
			break;

		default:
			pVirtualCab->nState = STATE_IDLE;
			break;
	}
}

/**********************************************************************
*
* FUNCTION:		InitState
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
void InitState(int nState)
{

	RunState(NULL, EVENT_STATE_ENTER);
}


/**********************************************************************
*
* FUNCTION:		ChangeState
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
void ChangeState(VIRTUAL_CAB* pVirtualCab, int nState)
{

	RunState(pVirtualCab, EVENT_STATE_EXIT);
	pVirtualCab->nState = nState;
	RunState(pVirtualCab, EVENT_STATE_ENTER);
}


#ifdef NOT_USED
/**********************************************************************
*
* FUNCTION:		ReturnState
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
void ReturnState(VIRTUAL_CAB* pVirtualCab)
{

	RunState(pVirtualCab, EVENT_STATE_EXIT);
	pVirtualCab->nState = m_nLastState;
	RunState(pVirtualCab, EVENT_STATE_ENTER);
}
#endif


/**********************************************************************
*
* FUNCTION:		DispatchEvent
*
* ARGUMENTS:	int nEvent - the event to be processed
*				EVENT_MAP Map - the event map
*
* RETURNS:		TRUE if event was successfully mapped and processed
*
* DESCRIPTION:	
*
* RESTRICTIONS:	
*
**********************************************************************/
unsigned char DispatchEvent(VIRTUAL_CAB* pVirtualCab, int nEvent, const EVENT_MAP *pMap)
{
	unsigned char bRet = 0;

	while(pMap->nEvent)
	{
		if(nEvent == pMap->nEvent) 
		{
			if(pMap->EventFunction)
			{
				pMap->EventFunction(pVirtualCab, nEvent);
				bRet = 1;
			}
			else
			{
				ChangeState(pVirtualCab, pMap->nNewState);
			}
			break;
		}
		pMap++;
	}
	return bRet;
}


