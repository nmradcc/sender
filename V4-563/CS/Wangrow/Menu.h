/**********************************************************************
*
* SOURCE FILENAME:	Menu.h
*
* DATE CREATED:		21-Dec-1999
*
* PROGRAMMER:		Karl Kobel
*
* DESCRIPTION:  	
*
* COPYRIGHT (c) 2000-2017 by K2 Engineering  All Rights Reserved.
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
#ifndef MENU_H
#define MENU_H

#include "Main.h"
#include "Cab.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define READ_ONLY	0x8000

#define MAX_VIRTUAL_CABS	4

#define MAX_RACALLS			8

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


/****************************************************************************************/
//function prototypes      
/****************************************************************************************/

typedef struct MENU_t MENU;

typedef struct VIRTUAL_CAB_t
{
	// Cab Stuff
	int				Cab;
	unsigned char	CabType;
	unsigned char	CabSubType;
	unsigned char	nState;
	void*			CabHandle;
	unsigned int	nSpeed;
	unsigned char	bSpeedDisconnect;

	// Menu stuff
	int				nEvent;
	const MENU*		pMenu;
	int				nMenuIndex;
	int				nCursor;
	unsigned char	CursorState;
	unsigned char	bFirstTime;
	char			szEditBuffer[6];
	int				nEditVar;
	int				nListCount;
	int				nMenuShowing;
	char			szTitleSave[9];

	unsigned int	nLastAccessory;
	unsigned int	nLastMacro;

	// Loco stuff
	unsigned int	nWhichRecall;
	struct Loco*	pRecall[MAX_RACALLS];
	//unsigned int	nPotSpeed;

	int				ExpirationTimer;
	int				nEStopState;
} VIRTUAL_CAB;

extern VIRTUAL_CAB aVirtualCab[MAX_VIRTUAL_CABS];

typedef struct MENU_DEF_t
{
	int		text;
	void 	*format;
	void	*variable;
	void	(*EditFunc) (VIRTUAL_CAB* pVirtualCab, int nEvent);
	void	(*EnterFunc) (VIRTUAL_CAB* pVirtualCab, int nEvent);
} MENU_DEF;


typedef struct MENU_t
{
	const MENU_DEF *def;
	void	*variable;
	int		title;
	void	(*ExitFunc) (VIRTUAL_CAB* pVirtualCab, int nEvent);
} MENU;

void DisplayPrompt(VIRTUAL_CAB* pVirtualCab, const MENU* pMenu);

void NewMenu(VIRTUAL_CAB* pVirtualCab, const MENU* pMenu);
void ExitMenu(VIRTUAL_CAB* pVirtualCab);
void HandleEnter(VIRTUAL_CAB* pVirtualCab, int nEvent);

void NextMenu(VIRTUAL_CAB* pVirtualCab);
void RestartMenu(VIRTUAL_CAB* pVirtualCab);

#endif

