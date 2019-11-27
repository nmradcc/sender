/**********************************************************************
*
*
* @file
* @Author Karl Kobel
* @date   4/2/2000
* @brief  DCC Accessory Decoder Functions
*
* These functions create new accessory decoder addresses in the 
* the accessory queue.  It can be searched with the find() function,
* and the status can be read via get() or writen via set().  New 
* entries are created with the new() fuction.
*
* SOURCE FILENAME:	Accessory.c
*
* DATE CREATED:		2/Apr/2000
*
* PROGRAMMER:		
*
* DESCRIPTION:  	
*
* COPYRIGHT (c) 2000-2002 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/
#include "Main.h"
#include "Accessory.h"
#include "Events.h"
#include "Packet.h"
#include "Track.h"

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

/* extern */ ACCESSORY aAccessory[MAX_ACCESSORIES];

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

#ifdef NOT_USED
/**********************************************************************
*
* FUNCTION:		InitLoco
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
void InitAccessory(void)
{
	unsigned int LowTimeStamp;
	word i;

   LowTimeStamp = 0;

	// find lowest non-zero TimeStamp
	for(i = 0; i < MAX_LOCOS; i++)
	{
		if(ActiveLocos[i].TimeStamp)
		{
			if(ActiveLocos[i].TimeStamp < LowTimeStamp)
			{
				LowTimeStamp = ActiveLocos[i].TimeStamp;
			}
		}
	}

	// subtract low TimeStamp from all non-zero TimeStamps
	for(i = 0; i < MAX_LOCOS; i++)
	{
		if(ActiveLocos[i].TimeStamp)
		{
			ActiveLocos[i].TimeStamp -= LowTimeStamp;
		}
	}
}
#endif

/**********************************************************************
*
* FUNCTION:		NewAccessory
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
unsigned int NewAccessory(unsigned int nAddress)
{
	unsigned short i;
	
	for(i = 0; i < MAX_ACCESSORIES; i++)
	{
		if(0 == aAccessory[i].Address)
		{
			aAccessory[i].Address = nAddress;
			return i;
		}
	}
	return ACC_EMPTY;
}


/**********************************************************************
*
* FUNCTION:		FindAccessory
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
unsigned int FindAccessory(unsigned int nAddress)
{
	unsigned short i;
	
	for(i = 0; i < MAX_ACCESSORIES; i++)
	{
		if(nAddress == aAccessory[i].Address)
		{
			return i;
		}
	}
	return ACC_EMPTY;
}


/**********************************************************************
*
* FUNCTION:		GetAccessoryState
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
unsigned char GetAccessoryState(unsigned int nAccessory)
{

	return aAccessory[nAccessory].State;
}

/**********************************************************************
*
* FUNCTION:		SetAccessoryState
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
void SetAccessoryState(unsigned int nAccessory, unsigned char nState)
{
//k	unsigned char baPacket[8];


	aAccessory[nAccessory].State = nState;

	// ToDo - build accessory packet
//k	BuildAccessoryPacket((unsigned char*)baPacket, nAccessory, nState);
//k	PutProgrammingPacket((unsigned char*)baPacket);
//k	PutProgrammingPacket((unsigned char*)baPacket);
//k	PutProgrammingPacket((unsigned char*)baPacket);
//k	PutProgrammingPacket((unsigned char*)baPacket);
//k	PutProgrammingPacket((unsigned char*)baPacket);
}

/**********************************************************************
*
* FUNCTION:		GetAccessoryFunctions
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
unsigned int GetAccessoryAddress(unsigned int nAccessory)
{

	return aAccessory[nAccessory].Address;
}

