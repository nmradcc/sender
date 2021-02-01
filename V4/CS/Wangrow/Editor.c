/**********************************************************************
*
* SOURCE FILENAME:	Editor.c
*
* DATE CREATED:		21-Dec-1999
*
* PROGRAMMER:		Karl Kobel
*
* DESCRIPTION:
*
* COPYRIGHT (c) 2000-2017 by K2 Engineering.  All Rights Reserved.
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
#include "Events.h"
#include "Cab.h"
#include "menu.h"
#include "MsgQueue.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
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

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

//extern int mpEvent;

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
* FUNCTION:	   	FormatLen
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
int FormatLen(char* szFormat)
{

	while(*szFormat && !isdigit(*szFormat))
	{
		szFormat++;
	}
	
	return atoi(szFormat);
}

/**********************************************************************
*
* FUNCTION:	   	EditNumeric
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
void EditNumeric(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	char* pEditBuffer;
	char szPrintBuffer[17];
	char *format;
	int *var_ptr;
	unsigned char i;
	unsigned char len;
	unsigned char fUpdate = 0;
	
	var_ptr = (int*)pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].variable;
	format = (char *)pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].format;
	pEditBuffer = pVirtualCab->szEditBuffer;
	
	if(var_ptr)
	{
		switch(nEvent)
		{
			case EVENT_CLEAR_DATA:
			case EVENT_EXPANSION:
				pEditBuffer[0] = 0;			// set variable to 0
				pVirtualCab->nCursor = 0;
				pVirtualCab->bFirstTime = 1;
				fUpdate = 1;
				break;
		
			case EVENT_GET_DATA:
				pVirtualCab->nCursor = 0;
				pVirtualCab->bFirstTime = 1;
				
				// get the data
				sprintf(pEditBuffer, "%d", *var_ptr);
				
				NCE_SetCursorPosition(pVirtualCab->Cab, 15, 1);
				NCE_CursorOn(pVirtualCab->Cab);
				fUpdate = 1;
				break;
		
			case EVENT_STORE_DATA:
				// convert string to number
				*var_ptr = atoi(pEditBuffer);
				//				NCE_CursorOff(pVirtualCab->Cab);
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
			if(atoi(pEditBuffer) == 0 && pVirtualCab->bFirstTime == 1)
			{
				NCE_DisplayMessage(pVirtualCab->Cab, 16 - 4, 1, "    ");
			}
			else
			{
				// display it
				sprintf(szPrintBuffer, format, atoi(pEditBuffer));
				len = strlen(szPrintBuffer);
				NCE_DisplayMessage(pVirtualCab->Cab, 16 - len, 1, szPrintBuffer);
			}
		}
	}
}


/**********************************************************************
*
* FUNCTION:	   	EditList
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
void EditList(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	// ToDo - move this to the cab array
	int *var_ptr;
	unsigned char fUpdate;
	char **List;
	int nTemp;
	
	
	fUpdate = 0;
	
	var_ptr = pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].variable;
	List = (char **)pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].format;
	
	switch(nEvent)
	{
		case EVENT_GET_DATA:
			pVirtualCab->nListCount = 0;
			while(List[pVirtualCab->nListCount])
			{
				pVirtualCab->nListCount++;
			}

			if(var_ptr == NULL)
			{
				pVirtualCab->nEditVar = 0;
			}
			else
			{
				pVirtualCab->nEditVar = (*var_ptr) % pVirtualCab->nListCount;
			}

			NCE_CursorOff(pVirtualCab->Cab);

			fUpdate = 1;
			break;

		case EVENT_STORE_DATA:
			if(var_ptr != NULL)
			{
				*var_ptr = pVirtualCab->nEditVar;
			}
			fUpdate = 1;
			break;

		case EVENT_PROG_MODE:
			pVirtualCab->nEditVar++;
			if(pVirtualCab->nEditVar == pVirtualCab->nListCount)
			{
				pVirtualCab->nEvent = EVENT_EXPANSION;
				QueueMessage(MSG_CAB_KEY_MESSAGE, pVirtualCab, EVENT_EXPANSION);
			}
			else
			{
				fUpdate = 1;
			}
			break;

		default:
			if(isdigit((char)nEvent))
			{
				nTemp = (nEvent - '1');
				if(nTemp < pVirtualCab->nListCount)
				{
					pVirtualCab->nEditVar = nTemp;
					pVirtualCab->nEvent = EVENT_ENTER;
					QueueMessage(MSG_CAB_KEY_MESSAGE, pVirtualCab, EVENT_ENTER);
				}
			}
			break;
	}

	if(fUpdate)
	{
		// display it
		NCE_DisplayMessage(pVirtualCab->Cab, 0, 1, List[pVirtualCab->nEditVar]);
	}
}


/**********************************************************************
*
* FUNCTION:	   EditSelection
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
void EditSelection(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	unsigned char fUpdate;
	char **List;
	unsigned char len;
	int *var_ptr;
	
	
	fUpdate = 0;
	
	var_ptr = pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].variable;
	List = (char **)pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].format;
	
	switch(nEvent)
	{
		case EVENT_GET_DATA:
			pVirtualCab->nListCount = 0;
			while(List[pVirtualCab->nListCount])
			{
				pVirtualCab->nListCount++;
			}

			if(var_ptr == NULL)
			{
				pVirtualCab->nEditVar = 0;
			}
			else
			{
				pVirtualCab->nEditVar = (*var_ptr) % pVirtualCab->nListCount;
			}

			NCE_CursorOff(pVirtualCab->Cab);

			fUpdate = 1;
			break;

		case EVENT_STORE_DATA:
			if(var_ptr != NULL)
			{
				*var_ptr = pVirtualCab->nEditVar;
			}
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
		NCE_DisplayMessage(pVirtualCab->Cab, 16 - len, 1, List[pVirtualCab->nEditVar]);
	}
}


/**********************************************************************
*
* FUNCTION:	   EditChoice
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
void EditChoice(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	int *var_ptr;


	var_ptr = pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].variable;

	switch(nEvent)
	{
		case EVENT_GET_DATA:
			NCE_CursorOff(pVirtualCab->Cab);
			break;

		case EVENT_STORE_DATA:
			break;

		case EVENT_ONE:
	         *var_ptr = 0;
			pVirtualCab->nEvent = EVENT_ENTER;
			QueueMessage(MSG_CAB_KEY_MESSAGE, pVirtualCab, EVENT_ENTER);
			break;

		case EVENT_TWO:
	         *var_ptr = 1;
			pVirtualCab->nEvent = EVENT_ENTER;
			QueueMessage(MSG_CAB_KEY_MESSAGE, pVirtualCab, EVENT_ENTER);
			break;

	}
}


/**********************************************************************
*
* FUNCTION:	   EditSelection
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
void EditSelection2(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	int *var_ptr;


	var_ptr = pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].variable;

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

			break;

		case EVENT_STORE_DATA:
			if(var_ptr != NULL)
			{
				*var_ptr = pVirtualCab->nEditVar;
			}
			break;

		case EVENT_ONE:
			pVirtualCab->nEditVar = 1;
			pVirtualCab->nEvent = EVENT_ENTER;
			QueueMessage(MSG_CAB_KEY_MESSAGE, pVirtualCab, EVENT_ENTER);
			break;

		case EVENT_TWO:
			pVirtualCab->nEditVar = 2;
			pVirtualCab->nEvent = EVENT_ENTER;
			QueueMessage(MSG_CAB_KEY_MESSAGE, pVirtualCab, EVENT_ENTER);
			break;
	}
}


