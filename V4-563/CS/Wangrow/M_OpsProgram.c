/**********************************************************************
*
* SOURCE FILENAME:	M_OpsProgram.c
*
* DATE CREATED:		12/Apr/2000
*
* PROGRAMMER:			Karl Kobel
*
* DESCRIPTION:
*
* COPYRIGHT (c) 2001-2002 by K2 Engineering. All Rights Reserved.
*
**********************************************************************
*
* VERSION CONTROL:
*
* $Revision: $
*
* $Log: $
*
**********************************************************************/

#include <stdio.h>

#include "Main.h"
#include "states.h"
#include "Events.h"
#include "Editor.h"
#include "Cab.h"
#include "CabNCE.h"
#include "Menu.h"
#include "Text.h"
#include "CV.h"
#include "Packet.h"
#include "Track.h"
#include "MsgQueue.h"
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

void ExitOpsProgram(VIRTUAL_CAB* pVirtualCab, int nEvent);

void EditOpsCV(VIRTUAL_CAB* pVirtualCab, int nEvent);

void ProgramOpsCV(VIRTUAL_CAB* pVirtualCab, int nEvent);

void EditOpsChoice(VIRTUAL_CAB* pVirtualCab, int nEvent);

void DoOpsMenu(VIRTUAL_CAB* pVirtualCab, int nEvent);

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

static const EVENT_MAP OpsProgramMap[] =
{
	{EVENT_PROG_MODE,		0,			ExitOpsProgram},
	{EVENT_ENTER,			0,			HandleEnter},
//	{EVENT_EXPANSION,		0,			ExitFunction},
//	{EVENT_SPEED_MODE,	0,			ExitFunction},
	{0, 0, NULL}
};


int nLocoAddressTemp;
int nCVAddressTemp;
int nCVValueTemp;
int nAccelAdj;
int nDecelAdj;

const MENU_DEF  OpsProgramConItems[] =
{
	// Prompt			format			variable					edit function  	enter function
	{tAccelAdj,			"%3d", 			&nAccelAdj,			 	EditNumeric,		NULL},
	{tDecalAdj,			"%3d", 			&nDecelAdj,	 			EditNumeric,		NULL},
	{0,NULL}
};


const MENU OpsProgramCon =
{
	OpsProgramConItems,
	&nLocoAddressTemp,
	tOpsLoco,
	ExitOpsProgram
};


const MENU_DEF  OpsProgramCVItems[] =
{
	// Prompt			format			variable					edit function  	enter function
	{tCVAddress,		"%3d", 			&nCVAddressTemp,	 	EditNumeric,		NULL},
	{tCVValue,			"%3d", 			&nCVValueTemp,	 		EditNumeric,		ProgramOpsCV},
	{0,NULL}
};


const MENU OpsProgramCV =
{
	OpsProgramCVItems,
	&nLocoAddressTemp,
	tOpsLoco,
	ExitOpsProgram
};


const MENU_DEF  OpsProgramItems[] =
{
	// Prompt			format			variable					edit function  	enter function
	{tOpsLocoAddress,	"%4d", 			&nLocoAddressTemp, 	EditNumeric,		NULL},
	{tOpsChoice,	 	NULL, 			NULL,				 		EditOpsChoice,		DoOpsMenu},
//	{tConsistDirection,	DirectionList,	&nDirection,  	EditChoice,			CheckAbort},
	{0,NULL}
};


const MENU OpsProgram =
{
	OpsProgramItems,
	NULL,
	tOpsPgm,
	ExitOpsProgram
};


/**********************************************************************
*
*							CODE
*
**********************************************************************/


/**********************************************************************
*
* FUNCTION:	   StateProgramTrack
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
void StateProgramOnMain(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
   Loco* pLoco;

	if(nEvent == EVENT_STATE_ENTER)
	{
		pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
		if(pLoco != NULL)
		{
			nLocoAddressTemp = GetLocoAddress(pLoco);
		}
		else
		{
			nLocoAddressTemp = 0;
		}

		//NewMenu(pVirtualCab, &OpsProgramCV);

		NewMenu(pVirtualCab, &OpsProgram);
		pVirtualCab->nMenuShowing = MENU_SHOWING_LINE_1L | MENU_SHOWING_LINE_2L | MENU_SHOWING_LINE_2R;
	}
	else if(nEvent == EVENT_STATE_EXIT)
	{
	}
	else if(!DispatchEvent(pVirtualCab, nEvent, OpsProgramMap))
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
* FUNCTION:			DoOpsMenu
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
void DoOpsMenu(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	int nTemp;


	nTemp = pVirtualCab->nEditVar;

	if(nTemp == 1)
	{
		NewMenu(pVirtualCab, &OpsProgramCV);
	}
	else if(nTemp == 2)
	{
		NewMenu(pVirtualCab, &OpsProgramCV);
	}
	else if(nTemp == 3)
	{
		NewMenu(pVirtualCab, &OpsProgramCon);
	}
	else
	{
		ExitOpsProgram(pVirtualCab, nEvent);
	}
}

/**********************************************************************
*
* FUNCTION:			ExitProgramTrack
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
void ExitOpsProgram(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	ExitMenu(pVirtualCab);

	pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
	RestoreOperateScreen(pVirtualCab, 1, 1);

	ChangeState(pVirtualCab, STATE_IDLE);
}


#ifdef NOT_USED
/**********************************************************************
*
* FUNCTION:	   EditCV
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
void EditOpsCV(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	char* pEditBuffer;
	char szPrintBuffer[17];
	char *format;
	int *var_ptr;
	int i;
	int len;
	BOOL fUpdate = 0;

//   var_ptr = pVirtualCab->pMenu->def->variable;
	format = (char *)pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].format;
	pEditBuffer = pVirtualCab->szEditBuffer;

//	if(var_ptr)
	{
		switch(nEvent)
		{
			case EVENT_GET_DATA:
				pVirtualCab->nCursor = 0;
				pVirtualCab->bFirstTime = TRUE;

            // get the data
				//snprintf(pEditBuffer, 9, "%d", *var_ptr);
				//snprintf(pEditBuffer, 9, "%d", 0);
				sprintf(pEditBuffer, "%d", 0);

				NCE_SetCursorPosition(pVirtualCab->Cab, 15, 1);
				NCE_CursorOn(pVirtualCab->Cab);

			 	fUpdate = 1;
				break;

			case EVENT_STORE_DATA:
				// convert string to number
				//*var_ptr = atoi(pEditBuffer);
				//WM_CursorOff(pVirtualCab);
				NCE_CursorOff(pVirtualCab->Cab);
				break;

			default:
				if(isdigit((char)nEvent))
				{
               // ToDo - try to find a way to do this
					// let leading zero (Headlight key) pass through
               //if(nEvent == EVENT_ZERO && pVirtualCab->bFirstTime == TRUE)
               //{
					//	QueueMessage(MSG_CAB_KEY_MESSAGE, pVirtualCab, EVENT_HEADLIGHT);
					//	return;
               //}

	            len = FormatLen(format);

					if(strlen(pEditBuffer) == len || pVirtualCab->bFirstTime == TRUE)
					{
						pVirtualCab->bFirstTime = FALSE;
						pVirtualCab->nCursor = 0;
						pEditBuffer[0] = '\0';
					}

					// push remaining characters
					i = strlen(pEditBuffer) + 1;		// include the '\0'

					while(i != pVirtualCab->nCursor)
					{
						pEditBuffer[i] = pEditBuffer[i - 1];
						--i;
					}

					pEditBuffer[pVirtualCab->nCursor++] = (char)nEvent;
				 	fUpdate = 1;
				}
				break;
		}

		if(fUpdate)
      {
         // display it
         //snprintf(szPrintBuffer, 17, format, atoi(pEditBuffer));
         sprintf(szPrintBuffer, format, atoi(pEditBuffer));
         len = strlen(szPrintBuffer);
			//WM_PutMessage(pVirtualCab, 16 - len, 1, 0, 0, szPrintBuffer);
   		NCE_DisplayMessage(pVirtualCab->Cab, 16 - len, 1, szPrintBuffer);
   	}
	}
}
#endif

/**********************************************************************
*
* FUNCTION:	   EditOpsChoice
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
void EditOpsChoice(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

//	switch(nEvent)
//	{
//		case EVENT_ONE:
//			pVirtualCab->nEditVar = 1;
//			QueueMessage(MSG_CAB_KEY_MESSAGE, pVirtualCab, EVENT_ENTER);
////			fUpdate = TRUE;
//			break;
//
//		case EVENT_TWO:
//			pVirtualCab->nEditVar = 2;
//			QueueMessage(MSG_CAB_KEY_MESSAGE, pVirtualCab, EVENT_ENTER);
////			fUpdate = TRUE;
//			break;
//
//		case EVENT_THREE:
//			pVirtualCab->nEditVar = 3;
//			QueueMessage(MSG_CAB_KEY_MESSAGE, pVirtualCab, EVENT_ENTER);
////			fUpdate = TRUE;
//			break;
//
//	}

   if(nEvent >= EVENT_ONE && nEvent <= EVENT_NINE)
   {
		pVirtualCab->nEditVar = nEvent - EVENT_ZERO;
		pVirtualCab->nEvent = EVENT_ENTER;
		QueueMessage(MSG_CAB_KEY_MESSAGE, pVirtualCab, EVENT_ENTER);
   }
}


/**********************************************************************
*
* FUNCTION:	   ProgramCV
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
void ProgramOpsCV(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
//k	unsigned char baPacket[8];


//k	BuildOpsWriteCVPacket((byte*)baPacket, nLocoAddressTemp, nCVAddressTemp, nCVValueTemp);
//k	PutProgrammingPacket((byte*)baPacket);
//k	PutProgrammingPacket((byte*)baPacket);
//k	PutProgrammingPacket((byte*)baPacket);
//k	PutProgrammingPacket((byte*)baPacket);
//k	PutProgrammingPacket((byte*)baPacket);
}


