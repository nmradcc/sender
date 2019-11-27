/**********************************************************************
*
* SOURCE FILENAME:	Cab.c
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
#include <stdio.h>
#include <string.h>

#include "Main.h"
#include "Cab.h"

#include "CabNCE.h"
//#include "CabXpressNet.h"
#include "Events.h"
//#include "Clock.h"
#include "MsgQueue.h"
#include "Settings.h"

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

extern void CabDisplay(unsigned char bCab, unsigned char x, unsigned char y, char* pcMessage);

void RestoreOperateScreen(VIRTUAL_CAB* pVirtualCab, unsigned char Line1, unsigned char Line2);


extern void XpressNetUpdate(void);

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

extern VIRTUAL_CAB aVirtualCab[MAX_VIRTUAL_CABS];

//byte CabPortProtocol1;
//byte CabPortProtocol2;

//byte NCE_Ports;
//byte XpressNet_Ports;

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

/*********************************************************************
*
* FUNCTION:
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:	none
*
*********************************************************************/
void HandleCabCommunication(void)
{

#ifdef USE_PORTS
	if(CabPortProtocol1 == CAB_PROTOCOL_NCE)
	{
		HandleNCECabCommunication();	// pass in port
	}
	else if(CabPortProtocol1 == CAB_PROTOCOL_XPRESSNET)
	{
		//HandleXPressNetCabCommunication();	// pass in port
		XpressNetUpdate();
	}

	if(CabPortProtocol2 == CAB_PROTOCOL_NCE)
	{
		HandleNCECabCommunication();	// pass in port
	}
	else if(CabPortProtocol2 == CAB_PROTOCOL_XPRESSNET)
	{
		//HandleXPressNetCabCommunication();	// pass in port
		XpressNetUpdate();
	}

	// WiThrottleTask();

#else

	HandleNCECabCommunication(2);
//k	XpressNetUpdate();

#endif
}

/*********************************************************************
*
* FUNCTION:		WM_RegisterCab
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:	none
*
*********************************************************************/
VIRTUAL_CAB* WM_RegisterCab(int Cab, int CabType, int CabSubType, void* CabHandle)
{
	unsigned char i;
	
	// Iterate to find an empty cab
	for(i = 0; i < MAX_VIRTUAL_CABS; i++)
	{
		if(aVirtualCab[i].Cab == 0)
		{
			break;
		}
	}
	
	if(i == MAX_VIRTUAL_CABS)
	{
		// no more virtual cabs
		return NULL;
	}
	else
	{
		// setup defaults
		aVirtualCab[Cab].Cab = Cab;
		aVirtualCab[Cab].CabType = CabType;
		aVirtualCab[Cab].CabSubType = CabSubType;
		aVirtualCab[Cab].CabHandle = CabHandle;

		aVirtualCab[Cab].nState = 0;

		aVirtualCab[Cab].nCursor = 0;

		// Display the default screen
		for(i = 0; i < 8; i++)
		{
			aVirtualCab[Cab].pRecall[i] = (Loco*)NULL;
		}
		aVirtualCab[Cab].nWhichRecall = 0;

		RestoreOperateScreen(&aVirtualCab[Cab], 1, 1);
	
		return &aVirtualCab[Cab];
	}
}


/*********************************************************************
*
* FUNCTION:		WM_UnRegisterCab
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:	none
*
*********************************************************************/
void WM_UnRegisterCab(VIRTUAL_CAB* pVC)
{
	pVC->Cab = 0;
}


/**********************************************************************
*
* FUNCTION:		SetExpiration
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
void SetExpiration(VIRTUAL_CAB* pVirtualCab, int nTime)
{

	pVirtualCab->ExpirationTimer = nTime;
}


/**********************************************************************
*
* FUNCTION:		SetExpiration
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
void InitExpiration(void)
{

}


/**********************************************************************
*
* FUNCTION:		HandleExpiration
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
void HandleExpiration(void)
{
	int i;

	for(i = 0; i < MAX_VIRTUAL_CABS; i++)
	{
		if(aVirtualCab[i].Cab != 0)
		{
			if(aVirtualCab[i].ExpirationTimer)
			{
				if(--aVirtualCab[i].ExpirationTimer == 0)
				{
					aVirtualCab[i].nEvent = EVENT_TIMER_EXPIRED;
					QueueMessage(0, &aVirtualCab[i], EVENT_TIMER_EXPIRED);
				}
			}
		}
	}
}


/**********************************************************************
*
* FUNCTION:		InitVirtualCab
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
void InitVirtualCab(void)
{

	//if(InitializedVirtualCab != 0x5a5a)
	//{
	//	InitializedVirtualCab = 0x5a5a;
		memset(aVirtualCab, 0, sizeof(aVirtualCab));
	//}
}

