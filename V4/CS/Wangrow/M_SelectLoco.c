/**********************************************************************
*
* SOURCE FILENAME:	M_SelectLoco.c
*
* DATE CREATED:		21-Dec-1999
*
* PROGRAMMER:		Karl Kobel
*
* DESCRIPTION:  	
*
* COPYRIGHT (c) 2000-2005 by K2 Engineering. All Rights Reserved.
*
*********************************************************************/
#include "Main.h"
#include <stdio.h>
#include <string.h>
#include "states.h"
#include "Events.h"
#include "editor.h"
#include "menu.h"
#include "Text.h"
#include "Cab.h"
#include "CabNCE.h"
#include "Loco.h"
#include "Expiration.h"
#include "TrakList.h"
#include "Shell.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/
extern void DisplaySpeed(VIRTUAL_CAB* pVirtualCab, unsigned int nSpeed);
extern void DisplayDirection(VIRTUAL_CAB* pVirtualCab, unsigned char Direction);
extern void DisplayFunction(VIRTUAL_CAB* pVirtualCab, unsigned int FunctionMap);
extern void DisplayAddress(VIRTUAL_CAB* pVirtualCab, unsigned int nAddress);

/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/
void DoSelectLoco(VIRTUAL_CAB* pVirtualCab, int nEvent);
void ExitSelectLoco(VIRTUAL_CAB* pVirtualCab, int nEvent);
void RepdisplayPrompt(VIRTUAL_CAB* pVirtualCab, int nEvent);

extern void RestoreOperateScreen(VIRTUAL_CAB* pVirtualCab, unsigned char Line1, unsigned char Line2);

extern void DeleteRecallLoco(VIRTUAL_CAB* pVirtualCab, unsigned int nLocoIndex);

extern void SetLocoDisconnect(VIRTUAL_CAB* pVirtualCab, Loco* pLoco);

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

VIRTUAL_CAB aVirtualCab[4];

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/

//#define NULL ((void*)0)

static const EVENT_MAP SelectLocoMap[] =
{
//	{EVENT_SELECT_LOCO,			STATE_SELECT_LOCO,				NULL},
	{EVENT_SELECT_ACCESSORY, 	STATE_SELECT_ACCESSORIES,		NULL},
//	{EVENT_SELECT_MACRO,		STATE_SELECT_MACROS,			NULL},
//	{EVENT_CONSIST_SETUP, 		STATE_SETUP_CONSIST,			NULL},
//	{EVENT_CONSIST_ADD,			STATE_ADD_CONSIST,				NULL},
//	{EVENT_CONSIST_DELETE,		STATE_DELETE_CONSIST,			NULL},
//	{EVENT_CONSIST_KILL,		STATE_CLEAR_CONSIST,			NULL},
	{EVENT_EXPANSION,			STATE_FUNCTIONS,				NULL},

	{EVENT_PROG_MODE,		0,			ExitSelectLoco},
	{EVENT_SELECT_LOCO,		0,			ExitSelectLoco},
	{EVENT_ENTER,			0,			HandleEnter},
//	{EVENT_TIMER_EXPIRED,	0,			RepdisplayPrompt},
	{EVENT_TIMER_EXPIRED,	0,			ExitSelectLoco},
	{0, 0, NULL}
};

static int temp;

const MENU_DEF  SelectLocoMenuItems[] =
{
	{tSelect_Loco,		"%4d",		&temp,		EditNumeric,	DoSelectLoco},
	{0,NULL}
};

const MENU SelectLocoMenu =
{
	SelectLocoMenuItems,
	NULL,
	tNull,
	NULL
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
void StateSelectLoco(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	Loco* pLoco;


	if(nEvent == EVENT_STATE_ENTER)
	{
		pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
		if(pLoco != NULL)
		{
			temp = GetLocoAddress(pLoco);
		}
		else
		{
			temp = 0;
		}

		NewMenu(pVirtualCab, &SelectLocoMenu);
		pVirtualCab->nMenuShowing = MENU_SHOWING_LINE_2L | MENU_SHOWING_LINE_2R;
		SetExpiration(pVirtualCab, EPIRATION_1_MINUTE);
	}
	else if(nEvent == EVENT_STATE_EXIT)
	{
	}
	else if(!DispatchEvent(pVirtualCab, nEvent, SelectLocoMap))
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
Loco* SelectLoco(VIRTUAL_CAB* pVirtualCab, unsigned int nLoco)
{
	Loco* pLoco;
	int i;
	int j;

	pLoco = FindLoco(nLoco);
	if(pLoco == NULL)
	{
		pLoco = NewLoco(nLoco);
		if(pLoco == NULL)
		{
			pLoco = OldestLoco();
		}
	}

	// find out if any other cab has this loco
	// ToDo - let user decide to take this loco or not
	for(i = 0; i < MAX_VIRTUAL_CABS; i++)
	{
		if(aVirtualCab[i].Cab != pVirtualCab->Cab)
		{
			for(j = 0; j < MAX_RACALLS; j++)
			{
				if(aVirtualCab[i].pRecall[j] == pLoco)
				{
					DeleteRecallLoco(&aVirtualCab[i], j);

					// select a loco we know is NULL so the "----" screen is displayed
					aVirtualCab[i].nWhichRecall = 8;

					RestoreOperateScreen(&aVirtualCab[i], 1, 1);
				}
			}
		}
	}

	for(i = 0; i < MAX_RACALLS; i++)
	{
		if(pVirtualCab->pRecall[i] == pLoco)
		{
			pVirtualCab->nWhichRecall = i;

			pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
			RestoreOperateScreen(pVirtualCab, 1, 1);

			ChangeState(pVirtualCab, STATE_IDLE);
			return pLoco;
		}
	}

	// ToDo - configure Recall type
//   if(1)
//   {
		for(i = 0; i < MAX_RACALLS; i++)
		{
			if(pVirtualCab->pRecall[i] == NULL)
			{
				pVirtualCab->pRecall[i] = pLoco;
				pVirtualCab->nWhichRecall = i;
				break;
			}
		}
		if(i == 8)
		{
			pVirtualCab->pRecall[pVirtualCab->nWhichRecall] = pVirtualCab->pRecall[7];
			pVirtualCab->nWhichRecall = 7;
		}
//	}
	return pLoco;
}


/**********************************************************************
*
* FUNCTION:	   DoSelectLoco
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
void DoSelectLoco(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	Loco* pLoco;
	unsigned int nSpeedMode;
	//char LocoFile[20];
	//char LocoNumber[8];


	ExitMenu(pVirtualCab);

	if(temp == 0)
	{
		temp = ANALOG_LOCO;
	}
	pLoco = SelectLoco(pVirtualCab, temp);

	//kstrcpy(LocoFile, "Loco");
	//ksprintf(LocoNumber, "%d", pLoco->Train);
	//kstrcat(LocoFile, LocoNumber);
	//kstrcat(LocoFile, ".txt");

	//kif(DoRun(ALL_PORTS, LocoFile) == CMD_NOT_FOUND)
	//k{
	//k}

	SetLocoDisconnect(pVirtualCab, pLoco);

	pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
	RestoreOperateScreen(pVirtualCab, 1, 1);


	nSpeedMode = GetLocoSpeedMode(pLoco);
	NCE_DisplayMessage(pVirtualCab->Cab, 8, 1, aText[tSpd_28 + nSpeedMode]);
	SetExpiration(pVirtualCab, EXPIRATION_2_SECONDS);


	ChangeState(pVirtualCab, STATE_IDLE);
}

/**********************************************************************
*
* FUNCTION:	   ExitSelectLoco
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
void ExitSelectLoco(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	ExitMenu(pVirtualCab);

	pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
	RestoreOperateScreen(pVirtualCab, 0, 1);

	ChangeState(pVirtualCab, STATE_IDLE);
}


/**********************************************************************
*
* FUNCTION:	   RepdisplayPrompt
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
void RepdisplayPrompt(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	DisplayPrompt(pVirtualCab, &SelectLocoMenu);
}


