/**********************************************************************
*
* SOURCE FILENAME:	Loco.h
*
* DATE CREATED:		8/19/98
*
* PROGRAMMER:		Karl Kobel
*
* DESCRIPTION:  	
*
* COPYRIGHT (c) 2000-2005 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/
#ifndef LOCO_H
#define LOCO_H

#include "Cab.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define SPEED_PRIME_NUMBER	252
#define SPEED_14_DIVISOR	18
#define SPEED_28_DIVISOR	9
#define SPEED_128_DIVISOR	2

enum
{
	SPEED_MODE_NORMAL,
	SPEED_MODE_STOP,
	SPEED_MODE_GTZ,
} /* SPEED_FCN_MODE */;

enum
{
	SPEED_MODE_28,
	SPEED_MODE_28_PERCENT,
	SPEED_MODE_128,
	SPEED_MODE_128_PERCENT,
	SPEED_MODE_14,
	SPEED_MODE_14_PERCENT,
} /* SPEED_MODE */;

enum
{
	CH_SPEED,
	CH_FUNCTION_1,
	CH_FUNCTION_2,
	CH_FUNCTION_3,
	CH_FUNCTION_4,
} /* CHANGE */;

#define SPEED_UNITS_MASK	0x8000

#define ANALOG_LOCO			0xfffe

#define MAX_LOCOS	32
//#define MAX_LOCOS	10


typedef struct Loco
{
	// these elements need to be persistent
	unsigned int	Address;				// the address of the loco
	unsigned int	Train;					// the train number
	unsigned int	Locomotive;				// the locomotive number
	char			pTrainName[64];			// the train name
	unsigned int	Tons;					// the train weight
	unsigned int	Alias;					// the alias/consist address
	unsigned char	SpeedMode;				// the speed mode 14/28/128
	unsigned char	SpeedFcnMode;			// the speed mode Normal/Stop/GTZ
	unsigned short	MaxSpeed;				// the Max speed for MPH or KPH or Other
	char			pSpeedLabel[8];			// the speed label
	unsigned char	bHornFunction;			// Which function does this loco use for horn
	unsigned char	bBellFunction;			// Which function does this loco use for bell
	unsigned char	Direction;				// the direction
	unsigned long	FunctionMap;			// a bitmap of which functions are on
	unsigned long	FunctionOverrideMap;	// a bitmap of which functions are on
	struct Loco*	pConsistLink;			// link to the next loco in the consist chain

	// make these global
	unsigned char	bNumStopPackets;		// Number of times the stop packet should be sent
	unsigned char	bNumFunctionPackets;	// Number of times a function packet should be sent
	//unsigned char	bNumFunction2Packets;	// Number of times a function 2 packet should be sent

	// these elements do NOT need to be persistent
	unsigned short	Speed;					// the speed *

	// ToDo - make these bits
	struct
	{
		unsigned char	bfSendLoco:1;		// flag for indicating the loco message has been sent *
		unsigned char	bfStateDirty:1;		// flag to indicate that the loco information has changed and the state should be written *
	};
	unsigned char	bChange:1;				// a bit mask of what has been updated - speed, functions, ...

	unsigned char	bfWhichFunction;		// which function group is currently showing *
} Loco;

// * the state does not have to be saved
#define ESTOP	0xffff

enum
{
	DIR_FORWARD,
	DIR_REVERSE,
} /* LOCO_DIRECTION */;

typedef unsigned short word;

/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/
void InitLoco(void);

Loco* FindLoco(unsigned int nAddress);
Loco* FindAlias(unsigned int nAddress);
Loco* FindLeadLoco(unsigned int nAddress);

Loco* NewLoco(unsigned int nAddress);

//void DeleteRecallLoco(VIRTUAL_CAB* pVirtualCab, unsigned int nLocoIndex);

word GetLocoSpeed(Loco* pLoco);
void SetLocoSpeed(Loco* pLoco, word nSpeed);

word GetLocoMaxSpeed(Loco* pLoco);
void SetLocoMaxSpeed(Loco* pLoco, word nMaxSpeed);

unsigned char GetLocoSpeedUnits(Loco* pLoco);

word GetLocoSpeedMode(Loco* pLoco);
void SetLocoSpeedMode(Loco* pLoco, word nSpeedMode);

unsigned char GetLocoDirection(Loco* pLoco);
void SetLocoDirection(Loco* pLoco, unsigned char nDirection);

word GetLocoFunctionOverride(Loco* pLoco);
void SetLocoFunctionOverride(Loco* pLoco, word nFunctions);

unsigned long GetLocoFunctions(Loco* pLoco);
//void SetLocoFunctions1(Loco* pLoco, unsigned long nFunctionMap);
//void SetLocoFunctions2(Loco* pLoco, unsigned long nFunctionMap);
void SetLocoFunctions(Loco* pLoco, unsigned long nFunctionMap);

void SetLocoAlias(Loco* pLoco, word nAlias, word nDirection, unsigned char bLeadLoco);
//void SetLocoAlias(Loco* pLoco, unsigned int nAlias, word nDirection);
word GetLocoAlias(Loco* pLoco);
unsigned char GetLocoAliasDirection(Loco* pLoco);

unsigned char IsLeadLoco(Loco* pLoco);

word GetLocoAddress(Loco* pLoco);

word GetLocoTrainNumber(Loco* pLoco);
void SetLocoTrainNumber(Loco* pLoco, word nTrain);

word GetNumLocos(void);


word GetLocoLocomotiveNumber(Loco* pLoco);
void SetLocoLocomotiveNumber(Loco* pLoco, word wLocomotive);

word GetLocoTrainTons(Loco* pLoco);
void SetLocoTrainTons(Loco* pLoco, word wTons);

char* GetLocoTrainName(Loco* pLoco);
void SetLocoTrainName(Loco* pLoco, char* pTrainName);

char* GetLocoSpeedLabel(Loco* pLoco);
void SetLocoSpeedLabel(Loco* pLoco, char* pSpeedLabel);

extern void SaveLocoState(void);

#endif

