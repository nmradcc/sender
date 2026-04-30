/**********************************************************************
*
* SOURCE FILENAME:	Menu.c
*
* DATE CREATED:		21-Dec-1999
*
* PROGRAMMER:		Karl Kobel
*
* DESCRIPTION:  	
*
* COPYRIGHT (c) 2000-2002 by K2 Engineering.  All Rights Reserved.
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
#include <string.h>

#include "CabNCE.h"
#include "Main.h"
#include "Events.h"
#include "Text.h"
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

//void NewMenu(VIRTUAL_CAB* pVirtualCab, MENU* pMenu);
//void NextMenu(VIRTUAL_CAB* pVirtualCab);

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
* FUNCTION:		DisplayMenu
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
void DisplayPrompt(VIRTUAL_CAB* pVirtualCab, const MENU* pMenu)
{

   NCE_DisplayMessage(pVirtualCab->Cab, 0, 1, aText[pMenu->def[pVirtualCab->nMenuIndex].text]);
}



/**********************************************************************
*
* FUNCTION:		DisplayMenu
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
void DisplayMenu(VIRTUAL_CAB* pVirtualCab, const MENU* pMenu)
{

   // Display the Prompt
	DisplayPrompt(pVirtualCab, pMenu);

   // Display the data
	pMenu->def[pVirtualCab->nMenuIndex].EditFunc(pVirtualCab, EVENT_GET_DATA);
}



/**********************************************************************
*
* FUNCTION:		NewMenu
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
void NewMenu(VIRTUAL_CAB* pVirtualCab, const MENU* pMenu)
{
	char TitleBuf[20];
	int nTemp;
	
	
	pVirtualCab->nMenuIndex = 0;
	pVirtualCab->pMenu = pMenu;
	
	// Display the Title
	if(pMenu->title)
	{
		if(pMenu->variable)
		{
			nTemp = *(int*)pMenu->variable;
			//snprintf(TitleBuf, 20, aText[pMenu->title], nTemp);
			sprintf(TitleBuf, aText[pMenu->title], nTemp);
			NCE_DisplayMessage(pVirtualCab->Cab, 0, 0, TitleBuf);
			strcpy(pVirtualCab->szTitleSave, TitleBuf);
		}
		else
		{
			NCE_DisplayMessage(pVirtualCab->Cab, 0, 0, aText[pMenu->title]);
			strcpy(pVirtualCab->szTitleSave, aText[pMenu->title]);
		}
	}
	
	DisplayMenu(pVirtualCab, pMenu);
}



/**********************************************************************
*
* FUNCTION:		NextMenu
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
void NextMenu(VIRTUAL_CAB* pVirtualCab)
{

	pVirtualCab->nMenuIndex++;
	
	DisplayMenu(pVirtualCab, pVirtualCab->pMenu);
}


/**********************************************************************
*
* FUNCTION:		NextMenu
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
void RestartMenu(VIRTUAL_CAB* pVirtualCab)
{

   pVirtualCab->nMenuIndex = -1;
}


/**********************************************************************
*
* FUNCTION:		ExitMenu
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
void ExitMenu(VIRTUAL_CAB* pVirtualCab)
{

//	if(DirtyFlag)
//	{

//		if(pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].EditFunc)
		if(pVirtualCab->pMenu)
		{
			pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].EditFunc(pVirtualCab, EVENT_STORE_DATA);
		}
		pVirtualCab->pMenu = NULL;
//	}
	
	NCE_CursorOff(pVirtualCab->Cab);
}


/**********************************************************************
*
* FUNCTION:	   HandleEnter
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
void HandleEnter(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	const MENU* pMenu;

	pMenu = pVirtualCab->pMenu;

   // store the data
	if(pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].EditFunc)
	{
		pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].EditFunc(pVirtualCab, EVENT_STORE_DATA);
	}

   // perform the ENTER function
	if(pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].EnterFunc)
	{
		pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex].EnterFunc(pVirtualCab, nEvent);
	}

	// step to the next menu item if we are on the same menu
	if(pMenu == pVirtualCab->pMenu)
	{
		if(pVirtualCab->pMenu->def[pVirtualCab->nMenuIndex + 1].text == 0)
		{
			if(pVirtualCab->pMenu->ExitFunc)
			{
				pVirtualCab->pMenu->ExitFunc(pVirtualCab, NO_EVENT);
			}
		}
		else
		{
			NextMenu(pVirtualCab);
		}
	}
}



