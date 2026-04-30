/**********************************************************************
*
* SOURCE FILENAME:	M_ProgramTrack.c
*
* DATE CREATED:		12/Apr/2000
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
**********************************************************************/
#include "Main.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "states.h"
#include "Events.h"
#include "Text.h"
#include "Cab.h"
#include "CabNCE.h"
#include "Menu.h"
#include "CV.h"
#include "Editor.h"


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

void ExitProgramTrack(VIRTUAL_CAB* pVirtualCab, int nEvent);

void EditCV(VIRTUAL_CAB* pVirtualCab, int nEvent);
void EditAddressCV(VIRTUAL_CAB* pVirtualCab, int nEvent);
void EditConfigCV(VIRTUAL_CAB* pVirtualCab, int nEvent);

void ProgramCV(VIRTUAL_CAB* pVirtualCab, int nEvent);
void ProgramAddressCV(VIRTUAL_CAB* pVirtualCab, int nEvent);
void ProgramConfigCV(VIRTUAL_CAB* pVirtualCab, int nEvent);

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

//#define NULL ((void*)0)

static const EVENT_MAP ProgramTrackMap[] =
{
	{EVENT_PROG_MODE,		0,			ExitProgramTrack},
	{EVENT_ENTER,			0,			HandleEnter},
//	{EVENT_EXPANSION,		0,			ExitFunction},
//	{EVENT_SPEED_MODE,	0,			ExitFunction},
	{0, 0, NULL}
};

//1234567890123456
//Direction:Normal

// ToDo localize these
const char *CVDirectionList[] =
{
	"Normal",
	"Revrse",
	NULL
};

const char *SpeedModeList[] =
{
	"28Stp",
	"14Stp",
	NULL
};

const char *EnabledList[] =
{
	" Enabled",
	"Disabled",
	NULL
};

const MENU_DEF  ProgramTrackItems[] =
{
	// Prompt			format			variable					edit function	enter function
	{tManufacturer,		"%3d", 			(void *)cvManufacturer,	 	EditCV,			NULL},
	{tVersion,			"%3d", 			(void *)cvVersion,	 		EditCV,			NULL},
	{tAddress,			"%4d", 			(void *)cvAddress,	 		EditAddressCV,	ProgramAddressCV},
	{tStartVoltage,		"%3d", 			(void *)cvStartVoltage,	 	EditCV,			ProgramCV},
	{tAcceleration,		"%3d", 			(void *)cvAcceleration,	 	EditCV,			ProgramCV},
	{tDeceleration,		"%3d", 			(void *)cvDeceleration,	 	EditCV,			ProgramCV},
	{tDirection,		CVDirectionList,(void *)cvDirection,	 	EditConfigCV,	ProgramConfigCV},
	{t28SpeedMode,		SpeedModeList,	(void *)cv28SpeedMode,	 	EditConfigCV,	ProgramConfigCV},
	{tDCMode,			EnabledList,	(void *)cvDCMode,	 		EditConfigCV,	ProgramConfigCV},
	{tAdvancedAck,		EnabledList,	(void *)cvAdvancedAck,	 	EditConfigCV,	ProgramConfigCV},
	{tSpeedTable,		EnabledList,	(void *)cvSpeedTable,	 	EditConfigCV,	ProgramConfigCV},

//	{tSpeedDisplay,	SpeedDispList,	(void *)nSpeedDisp,			EditList,		SetSpeedDisplay}.
//	"DISP 1-% 2-M 3-K"
//	{tMaxSpeed,			"%3d",			(void *)nMaxSpeed,			EditNumeric,	SetMaxSpeed},
//	"Max. Speed:  "

	{0,NULL}
};


const MENU ProgramTrackMenu =
{
	ProgramTrackItems,
	NULL,
	tPgmTrk,
	ExitProgramTrack
};

static unsigned char bReadCV;

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
void StateProgramTrack(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	if(nEvent == EVENT_STATE_ENTER)
	{
		NewMenu(pVirtualCab, &ProgramTrackMenu);
     	pVirtualCab->nMenuShowing = MENU_SHOWING_LINE_1L | MENU_SHOWING_LINE_2L | MENU_SHOWING_LINE_2R;
		bReadCV = 0;
	}
	else if(nEvent == EVENT_STATE_EXIT)
	{
	}
	else if(!DispatchEvent(pVirtualCab, nEvent, ProgramTrackMap))
	{
//		if(bReadCV)
//		{
//			
//		}
//		else
		{
			// special event processing not handled by DispatchEvent()
			if(pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].EditFunc)
			{
				pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].EditFunc(pVirtualCab, nEvent);
			}
		}
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
void ExitProgramTrack(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
	RestoreOperateScreen(pVirtualCab, 1, 1);

	ChangeState(pVirtualCab, STATE_IDLE);
}


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
void EditCV(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	char* pEditBuffer;
	char szPrintBuffer[17];
	char *format;
	//int *var_ptr;
	int i;
	int len;
	unsigned char fUpdate = 0;

//   var_ptr = pVirtualCab->pMenu->def->variable;
	format = (char *)pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].format;
	pEditBuffer = pVirtualCab->szEditBuffer;

//	if(var_ptr)
	{
		switch(nEvent)
		{
			case EVENT_GET_DATA:
				pVirtualCab->nCursor = 0;
				pVirtualCab->bFirstTime = 1;

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

					if(strlen(pEditBuffer) == len || pVirtualCab->bFirstTime == 1)
					{
						pVirtualCab->bFirstTime = 0;
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
         //snprintf(szPrintBuffer, 19, format, atoi(pEditBuffer));
         sprintf(szPrintBuffer, format, atoi(pEditBuffer));
         len = strlen(szPrintBuffer);
			//WM_PutMessage(pVirtualCab, 16 - len, 1, 0, 0, szPrintBuffer);
   		NCE_DisplayMessage(pVirtualCab->Cab, 16 - len, 1, szPrintBuffer);
   	}
	}
}

/**********************************************************************
*
* FUNCTION:	   EditAddressCV
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
void EditAddressCV(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	EditCV(pVirtualCab, nEvent);
}

/**********************************************************************
*
* FUNCTION:	   EditConfigCV
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
void EditConfigCV(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	unsigned char fUpdate;
	char **List;
   //int nTemp;
   int len;
   //char szTemp[17];


	fUpdate = 0;

	List = (char **)pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].format;

	switch(nEvent)
	{
		case EVENT_GET_DATA:
			pVirtualCab->nListCount = 0;
			while(List[pVirtualCab->nListCount])
			{
				pVirtualCab->nListCount++;
			}

         // ToDo - get the CV and proper bit
			pVirtualCab->nEditVar = 0;

			NCE_CursorOff(pVirtualCab->Cab);

			fUpdate = 1;
			break;

		case EVENT_STORE_DATA:
         // ToDo - save the CV
			fUpdate = 1;
			break;

		case EVENT_ONE:
			pVirtualCab->nEditVar = (pVirtualCab->nEditVar + 1) % pVirtualCab->nListCount;
//         editVar++;
//         if(editVar == ListCount)
//         {
//				//QueueMessage(MSG_CAB_KEY_MESSAGE, pVirtualCab, EVENT_ENTER);
//			}
			fUpdate = 1;
			break;

	}

	if(fUpdate)
   {
      // display it
      len = strlen(List[pVirtualCab->nEditVar]);
		//WM_PutMessage(pVirtualCab, 16 - len, 1, 0, 0, List[pVirtualCab->nEditVar]);
      NCE_DisplayMessage(pVirtualCab->Cab, 16 - len, 1, List[pVirtualCab->nEditVar]);
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
void ProgramCV(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

}

/**********************************************************************
*
* FUNCTION:	   ProgramAddressCV
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
void ProgramAddressCV(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

}

/**********************************************************************
*
* FUNCTION:	   ProgramConfigCV
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
void ProgramConfigCV(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

}


