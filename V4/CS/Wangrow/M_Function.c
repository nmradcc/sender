/**********************************************************************
*
* SOURCE FILENAME:	M_Function.c
*
* DATE CREATED:		21-Dec-1999
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
#include <ctype.h>
#include <string.h>

#include "states.h"
#include "Events.h"
#include "editor.h"
#include "menu.h"
#include "Text.h"
#include "Cab.h"
//#include "Window.h"
#include "Loco.h"
#include "Expiration.h"
#include "BitMask.h"
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

void ExitFunction(VIRTUAL_CAB* pVirtualCab, int nEvent);
void NextFunction(VIRTUAL_CAB* pVirtualCab, int nEvent);
void DoNormalFunction(VIRTUAL_CAB* pVirtualCab, int nEvent);

void DoFunctionRefresh(VIRTUAL_CAB* pVirtualCab, int nEvent);
void DisplayFunction(VIRTUAL_CAB* pVirtualCab, unsigned long FunctionMap);

void EditFunction(VIRTUAL_CAB* pVirtualCab, int Event);


extern void RestoreOperateScreen(VIRTUAL_CAB* pVirtualCab, unsigned char Line1, unsigned char Line2);
extern void DisplayUpperFunction(VIRTUAL_CAB* pVirtualCab, unsigned long FunctionMap);


/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

//extern unsigned int nBitMask[];

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/
static const EVENT_MAP FunctionMap[] =
{
	{EVENT_SELECT_LOCO,			STATE_SELECT_LOCO,			NULL},
	{EVENT_SELECT_ACCESSORY, 	STATE_SELECT_ACCESSORIES,	NULL},
//	{EVENT_SELECT_MACRO,		STATE_SELECT_MACROS,		NULL},
//	{EVENT_CONSIST_SETUP, 		STATE_SETUP_CONSIST,		NULL},
//	{EVENT_CONSIST_ADD,			STATE_ADD_CONSIST,			NULL},
//	{EVENT_CONSIST_DELETE,		STATE_DELETE_CONSIST,		NULL},
//	{EVENT_CONSIST_KILL,		STATE_CLEAR_CONSIST,		NULL},

	{EVENT_PROG_MODE,			0,							ExitFunction},
	{EVENT_EXPANSION,			0,							ExitFunction},

	{EVENT_ZERO,				0,							DoNormalFunction},
	{EVENT_ONE,					0,							DoNormalFunction},
	{EVENT_TWO,					0,							DoNormalFunction},
	{EVENT_FOUR,				0,							DoNormalFunction},
	{EVENT_FIVE,				0,							DoNormalFunction},
	{EVENT_SIX,					0,							DoNormalFunction},
	{EVENT_SEVEN,				0,							DoNormalFunction},
	{EVENT_EIGHT,				0,							DoNormalFunction},
	{EVENT_NINE,				0,							DoNormalFunction},
	{EVENT_BELL,				0,							DoNormalFunction},
	{EVENT_HORN_DOWN,			0,							DoNormalFunction},
	{EVENT_HORN_UP,  			0,							DoNormalFunction},

//	{EVENT_SPEED_MODE,			0,							ExitFunction},
	{EVENT_TIMER_EXPIRED,		0,							DoFunctionRefresh},
	{0, 0, NULL}
};


const MENU_DEF  FunctionMenuItems[] =
{
	// Prompt			format		variable		edit function  enter function
	{tFnc7_12,			NULL,		NULL,	 		EditFunction,	NULL},
	{0,NULL}
};

const MENU FunctionMenu =
{
	FunctionMenuItems,
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
* FUNCTION:	   SetFunctionCursor
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
void SetFunctionCursor(int bCab, int pos)
{

	if(pos >= 20)
	{
		NCE_SetCursorPosition(bCab, pos-14, 1);
	}
	else if(pos >= 10)
	{
		NCE_SetCursorPosition(bCab, pos-4, 1);
	}
	else
	{
		NCE_SetCursorPosition(bCab, pos+6, 1);
	}
	NCE_CursorOn(bCab);
}


/**********************************************************************
*
* FUNCTION:	   StateFunction
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
void StateFunction(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	Loco* pLoco;


	pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];

	if(nEvent == EVENT_STATE_ENTER)
	{
		if(pVirtualCab->nCursor < 10)
		{
			pVirtualCab->nCursor = 10;
		}
		if(pLoco != NULL && GetLocoAddress(pLoco) != ANALOG_LOCO)
		{
			SetExpiration(pVirtualCab, 0);		// cancel the timeout if timeout is pending
			NewMenu(pVirtualCab, &FunctionMenu);
			pVirtualCab->nMenuShowing = MENU_SHOWING_LINE_1L | MENU_SHOWING_LINE_1R | MENU_SHOWING_LINE_2L | MENU_SHOWING_LINE_2R | MENU_SHOWING_FUNCTIONS;

			SetFunctionCursor(pVirtualCab->Cab, pVirtualCab->nCursor);
			pVirtualCab->CursorState = 1;
		}
		else
		{
			ChangeState(pVirtualCab, STATE_IDLE);
		}
	}
	else if(nEvent == EVENT_STATE_EXIT)
	{
		NCE_CursorOff(pVirtualCab->Cab);
	}
	else if(!DispatchEvent(pVirtualCab, nEvent, FunctionMap))
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
* FUNCTION:	   ExitFunction
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
void ExitFunction(VIRTUAL_CAB* pVirtualCab, int nEvent)
{

	//ExitMenu(pVirtualCab);

	pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
	//NCE_CursorOff(pVirtualCab->Cab);
	RestoreOperateScreen(pVirtualCab, 1, 1);

	ChangeState(pVirtualCab, STATE_IDLE);
}


/**********************************************************************
*
* FUNCTION:		DisplayFunctions
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:						// Function 13_18
*
*
* RESTRICTIONS:
*
**********************************************************************/
void DisplayFunctionsOld(VIRTUAL_CAB* pVirtualCab, unsigned long FunctionMap)
{
	char szFunctionList[17];
	int i;
	int MapIndex;
	int StrIndex;

	// 10-19:0123456789
	// 20-28:012345678

	if(pVirtualCab->nCursor < 10)
	{
		//if((pVirtualCab->nMenuShowing & MENU_SHOWING_LINE_2R) == 0)
		{
			strcpy(szFunctionList, "  0-9:H123456789");
			MapIndex = 0;
			StrIndex = 6;
			for(i = 0; i < 10; i++)
			{
				if((FunctionMap & lBitMask[MapIndex]) == 0)
				{
					szFunctionList[StrIndex] = '-';
				}
				MapIndex++;
				StrIndex++;
			}
			NCE_DisplayMessage(pVirtualCab->Cab, 0, 0, szFunctionList);

			strcpy(szFunctionList, "10-19:0123456789");
			MapIndex = 10;
			StrIndex = 6;

			for(i = 0; i < 10; i++)
			{
				if((FunctionMap & lBitMask[MapIndex]) == 0)
				{
					szFunctionList[StrIndex] = '-';
				}
				MapIndex++;
				StrIndex++;
			}
			NCE_DisplayMessage(pVirtualCab->Cab, 0, 1, szFunctionList);
		}
	}
	else
	{
		//if((pVirtualCab->nMenuShowing & MENU_SHOWING_LINE_2R) == 0)
		{
			strcpy(szFunctionList, "10-19:0123456789");
			MapIndex = 10;
			StrIndex = 6;

			for(i = 0; i < 10; i++)
			{
				if((FunctionMap & lBitMask[MapIndex]) == 0)
				{
					szFunctionList[StrIndex] = '-';
				}
				MapIndex++;
				StrIndex++;
			}
			NCE_DisplayMessage(pVirtualCab->Cab, 0, 0, szFunctionList);

			strcpy(szFunctionList, "20-28:012345678 ");
			MapIndex = 20;
			StrIndex = 6;
			for(i = 0; i < 9; i++)
			{
				if((FunctionMap & lBitMask[MapIndex]) == 0)
				{
					szFunctionList[StrIndex] = '-';
				}
				MapIndex++;
				StrIndex++;
			}
			NCE_DisplayMessage(pVirtualCab->Cab, 0, 1, szFunctionList);
		}
	}
}

/**********************************************************************
*
* FUNCTION:		DisplayFunctions
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:						// Function 13_18
*
*
* RESTRICTIONS:
*
**********************************************************************/
void DisplayFunctions(VIRTUAL_CAB* pVirtualCab, unsigned long FunctionMap)
{
	char szFunctionList[17];
	int i;
	int MapIndex;
	int StrIndex;

	// 10-19:0123456789
	// 20-28:012345678

	if(pVirtualCab->nCursor < 10)
	{
		strcpy(szFunctionList, "  0-9:H123456789");
		MapIndex = 0;
		StrIndex = 6;
		for(i = 0; i < 10; i++)
		{
			if((FunctionMap & lBitMask[MapIndex]) == 0)
			{
				szFunctionList[StrIndex] = '-';
			}
			MapIndex++;
			StrIndex++;
		}
		NCE_DisplayMessage(pVirtualCab->Cab, 0, 1, szFunctionList);
	}
	else if(pVirtualCab->nCursor < 20)
	{
		strcpy(szFunctionList, "10-19:0123456789");
		MapIndex = 10;
		StrIndex = 6;

		for(i = 0; i < 10; i++)
		{
			if((FunctionMap & lBitMask[MapIndex]) == 0)
			{
				szFunctionList[StrIndex] = '-';
			}
			MapIndex++;
			StrIndex++;
		}
		NCE_DisplayMessage(pVirtualCab->Cab, 0, 1, szFunctionList);
	}
	else
	{
		strcpy(szFunctionList, "20-28:012345678 ");
		MapIndex = 20;
		StrIndex = 6;
		for(i = 0; i < 9; i++)
		{
			if((FunctionMap & lBitMask[MapIndex]) == 0)
			{
				szFunctionList[StrIndex] = '-';
			}
			MapIndex++;
			StrIndex++;
		}
		NCE_DisplayMessage(pVirtualCab->Cab, 0, 1, szFunctionList);
	}
}


/**********************************************************************
*
* FUNCTION:	   	EditFunction
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
void EditFunction(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	unsigned long Mask;
	unsigned long FunctionMap;
	unsigned char fUpdate = 0;
	Loco* pLoco;


	pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];

	switch(nEvent)
	{
		case EVENT_GET_DATA:
			FunctionMap = GetLocoFunctions(pLoco);
			//NCE_CursorOff(pVirtualCab->Cab);
			fUpdate = 1;
		break;

		default:
			//if(isdigit((char)nEvent))
			{
				FunctionMap = GetLocoFunctions(pLoco);

				switch(nEvent)
				{
					case EVENT_SPEED_INC:
						pVirtualCab->nCursor++;
						if(pVirtualCab->nCursor > 28)
						{
							pVirtualCab->nCursor = 0;
						}
					break;

					case EVENT_SPEED_DEC:
						if(pVirtualCab->nCursor == 0)
						{
							pVirtualCab->nCursor = 28;
						}
						else
						{
							pVirtualCab->nCursor--;
						}
					break;

					case EVENT_SPEED_INC_FAST:
						pVirtualCab->nCursor += 10;
						if(pVirtualCab->nCursor > 28)
						{
							pVirtualCab->nCursor = 0;
						}
//						if(pVirtualCab->nCursor < 10)
//						{
//							pVirtualCab->nCursor += 10;
//						}
//						else
//						{
//							pVirtualCab->nCursor -= 10;
//						}
					break;

					case EVENT_SPEED_DEC_FAST:
						if(pVirtualCab->nCursor < 10)
						{
							pVirtualCab->nCursor = 20;
						}
						else
						{
							pVirtualCab->nCursor -= 10;
						}
//						if(pVirtualCab->nCursor < 20)
//						{
//							pVirtualCab->nCursor += 10;
//						}
//						else
//						{
//							pVirtualCab->nCursor -= 10;
//						}
					break;

					case EVENT_ENTER:
						Mask = lBitMask[pVirtualCab->nCursor];
						if(FunctionMap & Mask)
						{
							FunctionMap &= ~Mask;
						}
						else
						{
							FunctionMap |= Mask;
						}
						SetLocoFunctions(pLoco, FunctionMap);
					break;
				}

				if(pVirtualCab->nCursor > 28)
				{
					SetFunctionCursor(pVirtualCab->Cab, 28);
				}
				else
				{
					SetFunctionCursor(pVirtualCab->Cab, pVirtualCab->nCursor);
				}

				fUpdate = 1;
			}
		break;
	}

	if(fUpdate)
	{
		// display it
		pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
		DisplayFunctions(pVirtualCab, FunctionMap);
		pVirtualCab->nMenuShowing = MENU_SHOWING_LINE_1L | MENU_SHOWING_LINE_1R | MENU_SHOWING_LINE_2L | MENU_SHOWING_LINE_2R | MENU_SHOWING_FUNCTIONS;
	}
}



/**********************************************************************
*
* FUNCTION:		DoFunctionRefresh3
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
void DoFunctionRefresh(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	unsigned long FunctionMap;
	int nMenuShowingSave;
	Loco* pLoco;

	pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
	FunctionMap = GetLocoFunctions(pLoco);

	nMenuShowingSave = pVirtualCab->nMenuShowing;
	pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
	DisplayFunctions(pVirtualCab, FunctionMap);
	pVirtualCab->nMenuShowing = nMenuShowingSave;
	NCE_CursorOn(pVirtualCab->Cab);
	pVirtualCab->CursorState = 1;
}


/**********************************************************************
*
* FUNCTION:		DoFunction
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
void DoNormalFunction(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	unsigned int Mask;
	unsigned int FunctionMap;
	int nMenuShowingSave;
	Loco* pLoco;

	pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
	FunctionMap = GetLocoFunctions(pLoco);

	if(nEvent == EVENT_BELL)
	{
		// ToDo - Make this programmable
		Mask = wBitMask[2];
		if(FunctionMap & Mask)
		{
			FunctionMap &= ~Mask;
		}
		else
		{
			FunctionMap |= Mask;
		}
		// don't save this - the event will get processed by operate.c
	}
	else if(nEvent == EVENT_HORN_DOWN)
	{
		FunctionMap |= wBitMask[3];
		// don't save this - the event will get processed by operate.c
	}
	else if(nEvent == EVENT_HORN_UP)
	{
		FunctionMap &= ~wBitMask[3];
		// don't save this - the event will get processed by operate.c
	}
	else
	{
		// translate Headlight event into a zero key
		if(nEvent == EVENT_HEADLIGHT)
		{
			nEvent = EVENT_ZERO;
		}

		Mask = wBitMask[nEvent - 0x30];
		if(FunctionMap & Mask)
		{
			FunctionMap &= ~Mask;
		}
		else
		{
			FunctionMap |= Mask;
		}
		SetLocoFunctions(pLoco, FunctionMap);
		//if(nEvent < EVENT_FIVE)
		//{
		//	SetLocoFunctions1(pLoco, FunctionMap);
		//}
		//else
		//{
		//	SetLocoFunctions2(pLoco, FunctionMap);
		//}
	}

	nMenuShowingSave = pVirtualCab->nMenuShowing;
	pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
	if(nEvent == EVENT_SEVEN || nEvent == EVENT_EIGHT || nEvent == EVENT_NINE)
	{
		DisplayUpperFunction(pVirtualCab, FunctionMap);
	}
	else
	{
		DisplayFunction(pVirtualCab, FunctionMap);
	}
	pVirtualCab->nMenuShowing = nMenuShowingSave;;
	NCE_CursorOff(pVirtualCab->Cab);

	SetExpiration(pVirtualCab, EXPIRATION_2_SECONDS);
}

