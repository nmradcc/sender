/**********************************************************************
*
* @file   Loco.c
* @Author Karl Kobel
* @date   4/2/2000
* @brief  Accessors for the locomotive database
*
* These functions access the data in the locomotive (Loco) database.
*
* COPYRIGHT (c) 1999-2017 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/

#include <stdio.h>
#include <string.h>

#include "Main.h"
#include "Loco.h"
#include "Events.h"
#include "Cab.h"
#include "TrakList.h"
#include "ff.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define true 1

/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

extern void SetZeroStretch(unsigned int nSpeed, unsigned char fDirection);

extern void SendWMIntMessage(int iMsgId, int iMsgData);

void SaveLocoState(void);
void ReadLocoState(void);

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

Loco ActiveLocos[MAX_LOCOS];
unsigned char abAnalogSpeedTable[29];

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
void InitLoco(void)
{
	int i;

	memset(ActiveLocos, 0, sizeof(ActiveLocos));

	//ReadLocoState();		// read loco data from a file

	// ToDo - this should come from a file
	for(i = 0; i <= 28; i++)
	{
		abAnalogSpeedTable[i] = i * 8;
	}
}



/**********************************************************************
*
* FUNCTION:		NewLoco
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
Loco* NewLoco(unsigned int nAddress)
{
	word i;
   
	for(i = 0; i < MAX_LOCOS; i++)
	{
		if(0 == ActiveLocos[i].Address)
		{
			memset(&ActiveLocos[i], 0, sizeof(ActiveLocos[i]));
			ActiveLocos[i].Address = nAddress;
			ActiveLocos[i].bfStateDirty = true;

			ActiveLocos[i].SpeedFcnMode = SPEED_MODE_NORMAL;
//			ActiveLocos[i].SpeedFcnMode = SPEED_MODE_STOP;
//			ActiveLocos[i].SpeedFcnMode = SPEED_MODE_GTZ;

			ActiveLocos[i].MaxSpeed = 100;
//			ActiveLocos[i].MaxSpeed = 80 | SPEED_UNITS_MASK;

//k			SendWMIntMessage(WM_USER_NEW_LOCO, 0);

			return &ActiveLocos[i];
		}
	}
	return NULL;
}


/**********************************************************************
*
* FUNCTION:		FindLoco
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
Loco* FindLoco(unsigned int nAddress)
{
	word i;
   
	for(i = 0; i < MAX_LOCOS; i++)
	{
		if(nAddress == ActiveLocos[i].Address)
		{
			return &ActiveLocos[i];
		}
	}
	return NULL;
}

/**********************************************************************
*
* FUNCTION:		FindAlias
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
Loco* FindAlias(unsigned int nAddress)
{
	word i;

	for(i = 0; i < MAX_LOCOS; i++)
	{
		if(nAddress == (ActiveLocos[i].Alias & 0x7f))
		{
			return &ActiveLocos[i];
		}
	}
	return NULL;
}


/**********************************************************************
*
* FUNCTION:		FindLeadLoco
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
Loco* FindLeadLoco(unsigned int nAddress)
{
	word i;

	for(i = 0; i < MAX_LOCOS; i++)
	{
		if(nAddress == (ActiveLocos[i].Alias & 0x7f) || nAddress == ActiveLocos[i].Address)
		{
			if(ActiveLocos[i].Alias & 0x8000)
         {
				return &ActiveLocos[i];
         }
		}
	}
	return NULL;
}


/**********************************************************************
*
* FUNCTION:	   DeleteRecallLoco
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
void DeleteRecallLoco(VIRTUAL_CAB* pVirtualCab, unsigned int nLocoIndex)
{
	word i;

	for(i = nLocoIndex; i < 7; i++)
	{
		if(pVirtualCab->pRecall[i] == NULL)
		{
			break;
		}
		pVirtualCab->pRecall[i] = pVirtualCab->pRecall[i + 1];
	}
	pVirtualCab->pRecall[i] = NULL;

	// make sure we are pointed to a good loco - if any (allow the queue to completly clear)
	if(pVirtualCab->pRecall[pVirtualCab->nWhichRecall] == NULL)
	{
		if(pVirtualCab->nWhichRecall != 0)
		{
			pVirtualCab->nWhichRecall--;
		}
	}

//	pVirtualCab->nActiveLoco = pVirtualCab->nRecall[pVirtualCab->nWhichRecall];
}


/**********************************************************************
*
* FUNCTION:		GetLocoSpeed
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
word GetLocoSpeed(Loco* pLoco)
{

	return pLoco->Speed;
}


/**********************************************************************
*
* FUNCTION:		SetLocoSpeed
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
void SetLocoSpeed(Loco* pLoco, word nSpeed)
{

	pLoco->Speed = nSpeed;

	if(pLoco->Address == ANALOG_LOCO)
	{
//k		SetZeroStretch(nSpeed, pLoco->Direction);
	}
	pLoco->bChange = CH_SPEED;
	MakeMostRecentLoco(pLoco);

	if(nSpeed)
	{
		pLoco->bNumStopPackets = 25;
	}

//k	SendWMIntMessage(WM_USER_SPEED_CHANGE, 0);
}


/**********************************************************************
*
* FUNCTION:		GetLocoMaxSpeed
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
word GetLocoMaxSpeed(Loco* pLoco)
{

	return pLoco->MaxSpeed & ~SPEED_UNITS_MASK;
}

/**********************************************************************
*
* FUNCTION:		SetLocoMaxSpeed
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
void SetLocoMaxSpeed(Loco* pLoco, word nMaxSpeed)
{

	pLoco->MaxSpeed = nMaxSpeed;
	pLoco->bfStateDirty = true;
}


/**********************************************************************
*
* FUNCTION:		GetLocoSpeedUnits
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
unsigned char GetLocoSpeedUnits(Loco* pLoco)
{

	return (pLoco->MaxSpeed & SPEED_UNITS_MASK) != 0;
}


/**********************************************************************
*
* FUNCTION:		GetLocoSpeedMode
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
word GetLocoSpeedMode(Loco* pLoco)
{

	return pLoco->SpeedMode;
}

/**********************************************************************
*
* FUNCTION:		SetLocoSpeedMode
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
void SetLocoSpeedMode(Loco* pLoco, word nSpeedMode)
{

	pLoco->SpeedMode = nSpeedMode;
	pLoco->bfStateDirty = true;
}

/**********************************************************************
*
* FUNCTION:		GetLocoDirection
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
unsigned char GetLocoDirection(Loco* pLoco)
{

	return pLoco->Direction;
}

/**********************************************************************
*
* FUNCTION:		SetLocoDirection
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
void SetLocoDirection(Loco* pLoco, unsigned char fDirection)
{

	pLoco->Direction = fDirection;

	if(pLoco->Address == ANALOG_LOCO)
	{
//k		SetZeroStretch(pLoco->Speed, fDirection);
	}
	pLoco->bChange = CH_SPEED;
	MakeMostRecentLoco(pLoco);
	pLoco->bfStateDirty = true;

//k	SendWMIntMessage(WM_USER_DIRECTION, 0);
}


/**********************************************************************
*
* FUNCTION:		GetLocoFunctions
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
word GetLocoFunctionOverride(Loco* pLoco)
{

	return pLoco->FunctionOverrideMap;
}

/**********************************************************************
*
* FUNCTION:		SetLocoFunctions
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
void SetLocoFunctionOverride(Loco* pLoco, word nFunctions)
{

	pLoco->FunctionOverrideMap = nFunctions;
	pLoco->bfStateDirty = true;
}

/**********************************************************************
*
* FUNCTION:		GetLocoFunctions
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
unsigned long GetLocoFunctions(Loco* pLoco)
{

	return pLoco->FunctionMap;
}

/**********************************************************************
*
* FUNCTION:		SetLocoFunctions
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
#ifdef REPLACED
void SetLocoFunctions1(Loco* pLoco, unsigned long nFunctionMap)
{

	pLoco->FunctionMap = nFunctionMap;
	//MakeMostRecentFunction1(pLoco);
	pLoco->bChange = CH_FUNCTION_1;
	MakeMostRecentLoco(pLoco);
	pLoco->bNumFunctionPackets = 10;
	pLoco->bfStateDirty = true;
}

void SetLocoFunctions2(Loco* pLoco, unsigned long nFunctionMap)
{

	pLoco->FunctionMap = nFunctionMap;
	//MakeMostRecentFunction2(pLoco);
	pLoco->bChange = CH_FUNCTION_2;
	MakeMostRecentLoco(pLoco);
	pLoco->bNumFunctionPackets = 10;
	pLoco->bfStateDirty = true;
}
#endif

void SetLocoFunctions(Loco* pLoco, unsigned long nFunctionMap)
{

	if((pLoco->FunctionMap & 0x000000FF) != (nFunctionMap & 0x000000FF))
	{
		pLoco->bChange |= CH_FUNCTION_1;
	}
	if((pLoco->FunctionMap & 0x0000FF00) != (nFunctionMap & 0x0000FF00))
	{
		pLoco->bChange |= CH_FUNCTION_2;
	}
	if((pLoco->FunctionMap & 0x00FF0000) != (nFunctionMap & 0x00FF0000))
	{
		pLoco->bChange |= CH_FUNCTION_3;
	}
	if((pLoco->FunctionMap & 0xFF000000) != (nFunctionMap & 0xFF000000))
	{
		pLoco->bChange |= CH_FUNCTION_4;
	}

	pLoco->FunctionMap = nFunctionMap;
	MakeMostRecentLoco(pLoco);
	pLoco->bNumFunctionPackets = 10;
	pLoco->bfStateDirty = true;
}

/**********************************************************************
*
* FUNCTION:		GetLocoAddress
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
word GetLocoAddress(Loco* pLoco)
{

	return pLoco->Address;
}

/**********************************************************************
*
* FUNCTION:		GetLocoTrainNumber
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
word GetLocoTrainNumber(Loco* pLoco)
{

	return pLoco->Train;
}

/**********************************************************************
*
* FUNCTION:		SetLocoTrainNumber
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
void SetLocoTrainNumber(Loco* pLoco, word nTrain)
{

	pLoco->Train = nTrain;
	pLoco->bfStateDirty = true;
}


/**********************************************************************
*
* FUNCTION:		GetLocoLocomotiveNumber
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
word GetLocoLocomotiveNumber(Loco* pLoco)
{

	return pLoco->Locomotive;
}

/**********************************************************************
*
* FUNCTION:		SetLocoLocomotiveNumber
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
void SetLocoLocomotiveNumber(Loco* pLoco, word wLocomotive)
{

	pLoco->Locomotive = wLocomotive;
	pLoco->bfStateDirty = true;
}


/**********************************************************************
*
* FUNCTION:		GetLocoTrainTons
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
word GetLocoTrainTons(Loco* pLoco)
{

	return pLoco->Tons;
}

/**********************************************************************
*
* FUNCTION:		SetLocoTrainTons
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
void SetLocoTrainTons(Loco* pLoco, word wTons)
{

	pLoco->Tons = wTons;
	pLoco->bfStateDirty = true;
}


/**********************************************************************
*
* FUNCTION:		GetLocoTrainName
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
char* GetLocoTrainName(Loco* pLoco)
{

	return pLoco->pTrainName;
}

/**********************************************************************
*
* FUNCTION:		SetLocoTrainName
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
void SetLocoTrainName(Loco* pLoco, char* pTrainName)
{

	strcpy(pLoco->pTrainName, pTrainName);
	pLoco->bfStateDirty = true;
}


/**********************************************************************
*
* FUNCTION:		GetLocoSpeedLabel
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
char* GetLocoSpeedLabel(Loco* pLoco)
{

	return pLoco->pSpeedLabel;
}

/**********************************************************************
*
* FUNCTION:		SetLocoSpeedLabel
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
void SetLocoSpeedLabel(Loco* pLoco, char* pSpeedLabel)
{

	strcpy(pLoco->pSpeedLabel, pSpeedLabel);
	pLoco->bfStateDirty = true;
}



/**********************************************************************
*
* FUNCTION:		SetLocoAlias
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
void SetLocoAlias(Loco* pLoco, unsigned short nAlias, unsigned short nDirection, unsigned char bLeadLoco)
{

	pLoco->Alias = nAlias;

	if(nDirection)
    {
		pLoco->Alias |= 0x80;
    }

    if(bLeadLoco)
    {
		pLoco->Alias |= 0x8000;
    }
	pLoco->bfStateDirty = true;
}

/**********************************************************************
*
* FUNCTION:		GetLocoAlias
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
word GetLocoAlias(Loco* pLoco)
{

	return pLoco->Alias & 0x7f;
}



/**********************************************************************
*
* FUNCTION:		GetLocoAliasDirection
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
unsigned char GetLocoAliasDirection(Loco* pLoco)
{

	return (unsigned char)pLoco->Alias & 0x80;
}



/**********************************************************************
*
* FUNCTION:		IsLeadLoco
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
unsigned char IsLeadLoco(Loco* pLoco)
{

	return (unsigned char)(pLoco->Alias & 0x8000);
}


/**********************************************************************
*
* FUNCTION:		StopAllLocos
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
void StopAllLocos(void)
{
   word i;

	for(i = 0; i < MAX_LOCOS; i++)
	{
		if(0 != ActiveLocos[i].Address)
		{
			//SetLocoSpeed(&ActiveLocos[i], 0); ToDo - should this be used instead
			ActiveLocos[i].Speed = 0;
		}
	}
}

/**********************************************************************
*
* FUNCTION:		StopCabLocos
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
void StopCabLocos(VIRTUAL_CAB* pVirtualCab)
{
	unsigned char i;
	Loco* pLoco;

	for(i = 0; i < MAX_RACALLS; i++)
	{
		pLoco =  pVirtualCab->pRecall[i];
		if(pLoco != NULL)
		{
			SetLocoSpeed(pLoco, 0);
		}
	}
}


/**********************************************************************
*
* FUNCTION:		GetNumLocos
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
word GetNumLocos(void)
{
   word i;
   word iCount = 0;

	for(i = 0; i < MAX_LOCOS; i++)
	{
		if(0 != ActiveLocos[i].Address)
		{
			iCount++;
		}
	}
	return iCount;
}


/**********************************************************************
*
* FUNCTION:		SaveLocoState
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
void SaveLocoState(void)
{
//k	int ret;
//k	FIL fp;
//k	unsigned int bw;

	// open file
//k	ret = f_open(&fp, "Locos", FA_CREATE_ALWAYS | FA_WRITE);
//k	if(ret == FR_OK)
//k	{
//k		ret = f_write(&fp, ActiveLocos, sizeof(ActiveLocos), &bw);
//k		(void)f_close (&fp);
//k	}
}

/**********************************************************************
*
* FUNCTION:		GetLocoState
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
void ReadLocoState(void)
{
	int ret;
	FIL fp;
	unsigned int br;

	// open file
	ret = f_open(&fp, "LocoState", FA_READ);
	if(ret == FR_OK)
	{
		ret = f_read (&fp, ActiveLocos, sizeof(ActiveLocos), &br);
		(void)f_close (&fp);
	}
}

