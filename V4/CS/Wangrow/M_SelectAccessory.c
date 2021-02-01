/**********************************************************************
*
* SOURCE FILENAME:	M_SelectAccessory.c
*
* DATE CREATED:		21-Dec-1999
*
* PROGRAMMER:		Karl Kobel
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
//#include "kernel.h"
#include <stdio.h>

#include "Main.h"

#include "states.h"
#include "Events.h"
#include "editor.h"
#include "menu.h"
#include "Cab.h"
#include "Text.h"
//#include "Window.h"
#include "MsgQueue.h"
#include "Accessory.h"
#include "CabNCE.h"
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

void DoSelectAccessory(VIRTUAL_CAB* pVirtualCab, int nEvent);
void DoControlAccessory(VIRTUAL_CAB* pVirtualCab, int nEvent);
void ExitAccessory(VIRTUAL_CAB* pVirtualCab, int nEvent);
void ExitAccessory2(VIRTUAL_CAB* pVirtualCab, int nEvent);

void EditAccessory(VIRTUAL_CAB* pVirtualCab, int nEvent);

extern void RestoreOperateScreen(VIRTUAL_CAB* pVirtualCab, unsigned char Line1, unsigned char Line2);

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

extern int mpEvent;

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/

//#define NULL ((void*)0)

static const EVENT_MAP SelectAccessoryMap[] =
{
	{EVENT_SELECT_LOCO,			STATE_SELECT_LOCO,				NULL},
//	{EVENT_SELECT_ACCESSORY, 	STATE_SELECT_ACCESSORIES,		NULL},
//	{EVENT_SELECT_MACRO,		STATE_SELECT_MACROS,			NULL},
//	{EVENT_CONSIST_SETUP, 		STATE_SETUP_CONSIST,			NULL},
//	{EVENT_CONSIST_ADD,			STATE_ADD_CONSIST,				NULL},
//	{EVENT_CONSIST_DELETE,		STATE_DELETE_CONSIST,			NULL},
//	{EVENT_CONSIST_KILL,		STATE_CLEAR_CONSIST,			NULL},
	{EVENT_EXPANSION,			STATE_FUNCTIONS,				NULL},

	{EVENT_PROG_MODE,			0,			ExitAccessory},
	{EVENT_SELECT_ACCESSORY,	0,			ExitAccessory2},
	{EVENT_ENTER,				0,			HandleEnter},
	{EVENT_TIMER_EXPIRED,		0,			ExitAccessory},
	{0, 0, NULL}
};


// ToDo localize this

const char *AccessoryList[] =
{
//  1234567890123456
	"Normal  1=N 2=R ",
	"Reverse 1=N 2=R ",
	NULL
};


static int nAccessoryAddress = 1;
static int nAccessoryState = 1;


const MENU_DEF  SelectAccessoryMenuItems[] =
{
	// Prompt			format		variable					edit function	enter function
	{tSelect_Accy,		"%4d",		&nAccessoryAddress,	EditNumeric,	DoSelectAccessory},
	{0,NULL}
};

const MENU SelectAccessoryMenu =
{
	SelectAccessoryMenuItems,
	NULL,
	tNull,
	NULL
};

const MENU_DEF  ControlAccessoryMenuItems[] =
{
	// Prompt			format			variable				edit function		enter function
	{tNull,				AccessoryList,	&nAccessoryState,	EditAccessory,		DoControlAccessory},
	{0,NULL}
};

const MENU ControlAccessoryMenu =
{
	ControlAccessoryMenuItems,
	&nAccessoryAddress,
	tACC_9990,
	NULL
};

/**********************************************************************
*
*							CODE
*
**********************************************************************/

/**********************************************************************
*
* FUNCTION:	   StateSelectAccessory
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
void StateSelectAccessory(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	if(nEvent == EVENT_STATE_ENTER)
	{
		nAccessoryAddress = pVirtualCab->nLastAccessory;
		if(nAccessoryAddress == 0)
		{
			nAccessoryAddress = 1;
		}
		
		NewMenu(pVirtualCab, &SelectAccessoryMenu);
		pVirtualCab->nMenuShowing = MENU_SHOWING_LINE_2L | MENU_SHOWING_LINE_2R;
		SetExpiration(pVirtualCab, EPIRATION_1_MINUTE);
	}
	else if(nEvent == EVENT_STATE_EXIT)
	{
	}
	else if(!DispatchEvent(pVirtualCab, nEvent, SelectAccessoryMap))
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
* FUNCTION:	   DoSelectAccessory
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
void DoSelectAccessory(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	unsigned int nAccessory;
	
	
	ExitMenu(pVirtualCab);
	
	pVirtualCab->nLastAccessory = nAccessoryAddress;
	nAccessory = FindAccessory(nAccessoryAddress);
	if(nAccessory == ACC_EMPTY)
	{
		nAccessory = NewAccessory(nAccessoryAddress);
	}
	
	nAccessoryState = GetAccessoryState(nAccessory);
	
	NewMenu(pVirtualCab, &ControlAccessoryMenu);
	pVirtualCab->nMenuShowing |= MENU_SHOWING_LINE_1L;
}


/**********************************************************************
*
* FUNCTION:	   DoControlAccessory
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
void DoControlAccessory(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	unsigned int nAccessory;
	
	nAccessory = FindAccessory(nAccessoryAddress);
	SetAccessoryState(nAccessory, nAccessoryState);
	
	ExitMenu(pVirtualCab);
	
	pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
	RestoreOperateScreen(pVirtualCab, 1, 1);
	
	ChangeState(pVirtualCab, STATE_IDLE);
}


/**********************************************************************
*
* FUNCTION:	   ExitAccessory
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
void ExitAccessory(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	ExitMenu(pVirtualCab);
	
	pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
	RestoreOperateScreen(pVirtualCab, 1, 1);
	
	ChangeState(pVirtualCab, STATE_IDLE);
}


/**********************************************************************
*
* FUNCTION:	   ExitAccessory2
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
void ExitAccessory2(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

// 	ExitMenu(pVirtualCab);

	if(pVirtualCab->pMenu == &SelectAccessoryMenu)
	{
		ExitMenu(pVirtualCab);
		
		pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
		RestoreOperateScreen(pVirtualCab, 1, 1);
		
		ChangeState(pVirtualCab, STATE_IDLE);
	}
	else
	{
		QueueMessage(MSG_CAB_KEY_MESSAGE, pVirtualCab, EVENT_TOGGLE_ACCESSORY);
//		pVirtualCab->nEvent = EVENT_TOGGLE_ACCESSORY;
//		psend(mpEvent, (WORD)pVirtualCab);
	}
}


/**********************************************************************
*
* FUNCTION:	   	EditAccessory
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
void EditAccessory(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	int *var_ptr;
	unsigned char fUpdate;
	char **List;
	unsigned int nAccessory;
	
	
	fUpdate = 0;
	
	var_ptr = (int*)pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].variable;
	List = (char **)pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].format;
	
	switch(nEvent)
	{
		case EVENT_GET_DATA:
			if(var_ptr == NULL)
			{
				pVirtualCab->nEditVar = 0;
			}
			else
			{
				pVirtualCab->nEditVar = (*var_ptr) % 2;
			}
			
			NCE_CursorOff(pVirtualCab->Cab);
			
			fUpdate = 1;
			break;
		
		case EVENT_STORE_DATA:
			if(var_ptr != NULL)
			{
				*var_ptr = pVirtualCab->nEditVar;
			}
			break;
		
		case EVENT_ZERO:
			pVirtualCab->nEvent = EVENT_HEADLIGHT;
			QueueMessage(MSG_CAB_KEY_MESSAGE, pVirtualCab, EVENT_HEADLIGHT);
//			psend(mpEvent, (WORD)pVirtualCab);
			break;
		
		case EVENT_ONE:
			pVirtualCab->nEditVar = 0;
			nAccessory = FindAccessory(nAccessoryAddress);
			SetAccessoryState(nAccessoryAddress, 0);
			pVirtualCab->nEvent = EVENT_ENTER;
			QueueMessage(MSG_CAB_KEY_MESSAGE, pVirtualCab, EVENT_ENTER);
//			psend(mpEvent, (WORD)pVirtualCab);
			break;
		
		case EVENT_TWO:
			pVirtualCab->nEditVar = 1;
			nAccessory = FindAccessory(nAccessoryAddress);
			SetAccessoryState(nAccessory, 1);
			pVirtualCab->nEvent = EVENT_ENTER;
			QueueMessage(MSG_CAB_KEY_MESSAGE, pVirtualCab, EVENT_ENTER);
//			psend(mpEvent, (WORD)pVirtualCab);
			break;
		
		case EVENT_SELECT_ACCESSORY:
		case EVENT_TOGGLE_ACCESSORY:
			nAccessory = FindAccessory(nAccessoryAddress);
			if(pVirtualCab->nEditVar)
			{
				pVirtualCab->nEditVar = 0;
				SetAccessoryState(nAccessory, 0);
			}
			else
			{
				pVirtualCab->nEditVar = 1;
				SetAccessoryState(nAccessory, 1);
			}
		
			fUpdate = 1;
			break;
	}
	
	if(fUpdate)
	{
		NCE_DisplayMessage(pVirtualCab->Cab, 0, 1, List[pVirtualCab->nEditVar]);
	}
}




