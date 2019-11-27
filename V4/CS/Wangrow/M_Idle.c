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

void RefreshDisplay(VIRTUAL_CAB* pVirtualCab, int nEvent);
void Expansion(VIRTUAL_CAB* pVirtualCab, int nEvent);

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

	{EVENT_SELECT_LOCO,			STATE_SELECT_LOCO,				NULL},
	{EVENT_SELECT_ACCESSORY, 	STATE_SELECT_ACCESSORIES,		NULL},
	{EVENT_SELECT_MACRO,		STATE_NO_FUNCTION,				NULL},
	{EVENT_CONSIST_SETUP, 		STATE_NO_FUNCTION,				NULL},
	{EVENT_CONSIST_ADD,			STATE_NO_FUNCTION,				NULL},
	{EVENT_CONSIST_DELETE,		STATE_NO_FUNCTION,				NULL},
	{EVENT_CONSIST_KILL,		STATE_NO_FUNCTION,				NULL},
	{EVENT_EXPANSION,			STATE_FUNCTIONS,				NULL},

	{EVENT_ASSIGN_LOCO,			STATE_NO_FUNCTION,				NULL},
	{EVENT_PROG_ON_MAIN,		STATE_PROGRAM_ON_MAIN,			NULL},
	{EVENT_SET_CLOCK,			STATE_NO_FUNCTION,				NULL},
	{EVENT_PROG_ON_TRACK,		STATE_PROGRAM_ON_TRACK,			NULL},
	{EVENT_SETUP_CS,			STATE_NO_FUNCTION,				NULL},
	{EVENT_SETUP_CAB,			STATE_SET_CAB,					NULL},
	{EVENT_DEFINE_MACRO,		STATE_NO_FUNCTION,				NULL},
	{EVENT_CONSIST_SETUP_OLD,	STATE_NO_FUNCTION,				NULL},
	{EVENT_CONSIST_SETUP_ADV,	STATE_NO_FUNCTION,				NULL},
	{EVENT_PROG_MODE,			STATE_PROG_MODE,				NULL},
    {EVENT_ENTER,				0,								RefreshDisplay},
	{0, 0, NULL}
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
void StateIdle(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	if(nEvent == EVENT_STATE_ENTER)
	{
	}
	else if(nEvent == EVENT_STATE_EXIT)
	{
	}
	else if(!DispatchEvent(pVirtualCab, nEvent, IdleMap))
	{
		// special event processing not handled by DispatchEvent()
//		if(menu.def[menu.index].func)
//		{
//			menu.def[menu.index].func(nEvent);
//		}
	}
}


/**********************************************************************
*
* FUNCTION:	   RefreshDisplay
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
void RefreshDisplay(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	char szTemp[9];
	Loco* pLoco;
	unsigned int nAlias;

  	pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
	nAlias = GetLocoAlias(pLoco);

   if(nAlias && pVirtualCab->nState == STATE_IDLE)
   {
// ToDo - this seems wrong		
  		//snprintf(szTemp, 9, aText[tLoc], nAddress);
	  	//snprintf(szTemp, 9, "CON:%3d", nAlias);
	  	sprintf(szTemp, "CON:%3d", nAlias);
// ToDo - this seems wrong		

		NCE_DisplayMessage(pVirtualCab->Cab, 9, 1, szTemp);
		SetExpiration(pVirtualCab, EXPIRATION_2_SECONDS);
   }

	//NCE_RefreshDisplay(pVirtualCab->Cab);
   //RestoreOperateScreen(pVirtualCab, TRUE, TRUE);
}

