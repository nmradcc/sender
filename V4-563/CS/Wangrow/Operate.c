/**********************************************************************
*
* SOURCE FILENAME:	Operate.c
*
* DATE CREATED:		8/3/99
*
* PROGRAMMER:		Karl Kobel
*
* DESCRIPTION:
*
* COPYRIGHT (c) 1999-2017 by K2 Engineering. All Rights Reserved.
*
**********************************************************************/
#include "main.h"

#include "Main.h"
#include <stdio.h>
#include <string.h>
#include "states.h"
#include "Events.h"
#include "Text.h"
#include "Cab.h"
#include "Loco.h"
//#include "Clock.h"
#include "Expiration.h"
#include "States.h"
#include "Track.h"
#include "CabNCE.h"
#include "BitMask.h"
#include "CabNCE.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

//#define SPEED_PRIME_NUMBER	252

#define SPEED_DIR_MAX		36

/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

void DoFunction(VIRTUAL_CAB* pVirtualCab, int nEvent);
void DoRunFunction(VIRTUAL_CAB* pVirtualCab, int nEvent);
void DoSpeedMode(VIRTUAL_CAB* pVirtualCab, int nEvent);
void DoBrake(VIRTUAL_CAB* pVirtualCab, int nEvent);
void DoEmergencyStop(VIRTUAL_CAB* pVirtualCab, int nEvent);

void SetLocoDisconnect(VIRTUAL_CAB* pVirtualCab, Loco* pLoco);

void DoDirection(VIRTUAL_CAB* pVirtualCab, int nEvent);
void DoSpeed(VIRTUAL_CAB* pVirtualCab, int nEvent);
void DoSpeedKnob(VIRTUAL_CAB* pVirtualCab);

void DoRecall(VIRTUAL_CAB* pVirtualCab, int nEvent);

//void DisplayClock(VIRTUAL_CAB* pVirtualCab);
void DisplaySpeed(VIRTUAL_CAB* pVirtualCab, unsigned int nSpeed);
void DisplayFunction(VIRTUAL_CAB* pVirtualCab, unsigned long FunctionMap);
//void DisplayUpperFunction(VIRTUAL_CAB* pVirtualCab, unsigned long FunctionMap);
void DisplayAddress(VIRTUAL_CAB* pVirtualCab, unsigned int nAddress);
void DisplayDirection(VIRTUAL_CAB* pVirtualCab, unsigned char Direction);

void RestoreOperateScreen(VIRTUAL_CAB* pVirtualCab, unsigned char Line1, unsigned char Line2);

void DoRefresh(VIRTUAL_CAB* pVirtualCab, int nEvent);

void DoEnter(VIRTUAL_CAB* pVirtualCab, int nEvent);


void DoUpdateClock(VIRTUAL_CAB* pVirtualCab, int nEvent);

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

#define SD_ENABLE		1
#define SD_DIRECTION	2

extern unsigned char eStopCab;

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/

//byte nMaxSpeed;

/**********************************************************************
*
*							CODE
*
**********************************************************************/

static const EVENT_MAP OperateMap[] =
{
	{EVENT_DIRECTION,			0,			DoDirection},
	{EVENT_DIR_FORWARD,			0,			DoDirection},
	{EVENT_DIR_REVERSE,			0,			DoDirection},
	{EVENT_SPEED_INC,			0,			DoSpeed},
	{EVENT_SPEED_DEC,			0,			DoSpeed},
	{EVENT_SPEED_INC_FAST,		0,			DoSpeed},
	{EVENT_SPEED_DEC_FAST,		0,			DoSpeed},

	{EVENT_ZERO,				0,			DoFunction},
	{EVENT_ONE,					0,			DoFunction},
	{EVENT_TWO,					0,			DoFunction},
	{EVENT_THREE,				0,			DoFunction},
	{EVENT_FOUR,				0,			DoFunction},
	{EVENT_FIVE,				0,			DoFunction},
	{EVENT_SIX,					0,			DoFunction},
	{EVENT_SEVEN,				0,			DoFunction},
	{EVENT_EIGHT,				0,			DoFunction},
	{EVENT_NINE,				0,			DoFunction},
	{EVENT_HEADLIGHT,			0,			DoFunction},

	{EVENT_BRAKE,				0,			DoBrake},
	{EVENT_EMERGENCY_STOP,		0,			DoEmergencyStop},
	{EVENT_SPEED_MODE,			0,			DoSpeedMode},
	{EVENT_BELL,				0,			DoRunFunction},
	{EVENT_HORN_DOWN,		  	0,			DoRunFunction},
	{EVENT_HORN_UP,				0,			DoRunFunction},

	{EVENT_RECALL,				0,			DoRecall},
	{EVENT_TIMER_EXPIRED,		0,			DoRefresh},
	{EVENT_ENTER,				0,			DoEnter},

//	{EVENT_UPDATE_CLOCK,		0,			DoUpdateClock},

	{0, 0, NULL}
};


/**********************************************************************
*
* FUNCTION:		RunOperate
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
void RunOperate(VIRTUAL_CAB* pVirtualCab, int nEvent)
{


	if(nEvent & EVENT_SPEED_TYPE)
	{
		pVirtualCab->nSpeed = nEvent & 0xff;
		DoSpeedKnob(pVirtualCab);
	}
	else
	{
		DispatchEvent(pVirtualCab, nEvent, OperateMap);
	}
}


#ifdef MOVE_THIS
/**********************************************************************
*
* FUNCTION:		DoUpdateClock
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
void DoUpdateClock(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	//char szTime[8];

	//CalcClock(szTime);
	//fasttime = GetFastClock(time);
	//sprintf(szTime, "Fast Clock: %2d:%02d", fasttime.Hours, fasttime.Minutes);

	//NCE_DisplayMessage(0, 8, 0, szTime);

	DisplayClock(pVirtualCab);
}
#endif

/**********************************************************************
*
* FUNCTION:		DoDirection
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
void DoDirection(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	unsigned char bDir;
	Loco* pLoco;
	char szTemp[10];
	int nSpeed;
	
	pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
	
	if(pLoco != NULL && (pVirtualCab->nMenuShowing & MENU_SHOWING_ESTOP) == 0)
	{
		switch(pLoco->SpeedFcnMode)
		{
			case SPEED_MODE_NORMAL:
				// ToDo - make this programmable
				// Don't allow a direction change if speed is greater than a programmable amount
				if(GetLocoSpeed(pLoco) < SPEED_DIR_MAX)
				{
					bDir = GetLocoDirection(pLoco);
					if(bDir == DIR_FORWARD)
					{
						SetLocoDirection(pLoco, DIR_REVERSE);
						DisplayDirection(pVirtualCab, DIR_REVERSE);
					}
					else
					{
						SetLocoDirection(pLoco, DIR_FORWARD);
						DisplayDirection(pVirtualCab, DIR_FORWARD);
					}

					// ToDo - make this programmable
					// Auto stop when reversing
					DoBrake(pVirtualCab, nEvent);
				}
				else
				{

					switch(GetLocoSpeedMode(pLoco))
					{
						case SPEED_MODE_14:
							sprintf(szTemp, " SPED>%2d", SPEED_DIR_MAX / SPEED_14_DIVISOR);
						break;

						case SPEED_MODE_28:
							sprintf(szTemp, " SPED>%2d", SPEED_DIR_MAX / SPEED_28_DIVISOR);
						break;

						case SPEED_MODE_128:
							sprintf(szTemp, " SPED>%2d", SPEED_DIR_MAX / SPEED_128_DIVISOR);
						break;

						case SPEED_MODE_14_PERCENT:
						case SPEED_MODE_28_PERCENT:
						case SPEED_MODE_128_PERCENT:
							nSpeed = (SPEED_DIR_MAX * 100) / SPEED_PRIME_NUMBER;
							sprintf(szTemp, " SPED>%2d", nSpeed);
						break;
					}

					NCE_DisplayMessage(pVirtualCab->Cab, 8, 1, szTemp);
					SetExpiration(pVirtualCab, EXPIRATION_05_SECONDS);
				}
			break;

			case SPEED_MODE_STOP:
				if(GetLocoSpeed(pLoco) != 0)
				{
					DoBrake(pVirtualCab, nEvent);
				}
				else
				{
					bDir = GetLocoDirection(pLoco);
					if(bDir == DIR_FORWARD)
					{
						SetLocoDirection(pLoco, DIR_REVERSE);
						DisplayDirection(pVirtualCab, DIR_REVERSE);
					}
					else
					{
						SetLocoDirection(pLoco, DIR_FORWARD);
						DisplayDirection(pVirtualCab, DIR_FORWARD);
					}
				}
			break;
			
			case SPEED_MODE_GTZ:
				// the brake key is not used in this mode
			break;
		}
	}
}


/**********************************************************************
*
* FUNCTION:		IncSpeed/DecSpeed
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
void IncSpeed(unsigned int* nSpeed, unsigned int nIncAmount)
{
	*nSpeed += nIncAmount;
	if(*nSpeed > SPEED_PRIME_NUMBER)
	{
		*nSpeed = SPEED_PRIME_NUMBER;
	}
}

void DecSpeed(unsigned int* nSpeed, unsigned int nIncAmount)
{
	*nSpeed -= nIncAmount;
	if(*nSpeed > SPEED_PRIME_NUMBER)
	{
		*nSpeed = 0;
	}
}


/**********************************************************************
*
* FUNCTION:		DoSpeed
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
void DoSpeed(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	unsigned int nSpeed;
	unsigned int nIncAmount;
	Loco* pLoco;
	//static BOOL SpeedWasZero = FALSE;
	
	pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
	
	if((pLoco != NULL) &&
			((pVirtualCab->nMenuShowing & MENU_SHOWING_ESTOP) == 0) &&
			((pVirtualCab->nMenuShowing & MENU_SHOWING_FUNCTIONS) == 0))
	{
		switch(GetLocoSpeedMode(pLoco))
		{
			case SPEED_MODE_14:
				nIncAmount = SPEED_14_DIVISOR;
				break;
			
			case SPEED_MODE_28:
				nIncAmount = SPEED_28_DIVISOR;
				break;
			
			case SPEED_MODE_128:
				nIncAmount = SPEED_128_DIVISOR;
				break;
			
			case SPEED_MODE_14_PERCENT:
			case SPEED_MODE_28_PERCENT:
			case SPEED_MODE_128_PERCENT:
			default:
				nIncAmount = 3;
				break;
		}
		nSpeed = GetLocoSpeed(pLoco);
		//if(nSpeed == 0)
		//{
		//	SpeedWasZero = TRUE;
		//}

		switch(pLoco->SpeedFcnMode)
		{
			case SPEED_MODE_NORMAL:
			case SPEED_MODE_STOP:
				switch(nEvent)
				{
					case EVENT_SPEED_INC:
						IncSpeed(&nSpeed, nIncAmount);
					break;

					case EVENT_SPEED_DEC:
						DecSpeed(&nSpeed, nIncAmount);
					break;

					case EVENT_SPEED_INC_FAST:
						IncSpeed(&nSpeed, nIncAmount * 4);
					break;

					case EVENT_SPEED_DEC_FAST:
						DecSpeed(&nSpeed, nIncAmount * 4);
					break;
				}
			break;
			
			case SPEED_MODE_GTZ:
				switch(nEvent)
				{
					case EVENT_SPEED_INC:
						if(GetLocoDirection(pLoco) == DIR_FORWARD)
						{
							IncSpeed(&nSpeed, nIncAmount);
						}
						else
						{
							DecSpeed(&nSpeed, nIncAmount);
						}
					break;

					case EVENT_SPEED_DEC:
						if(GetLocoDirection(pLoco) == DIR_FORWARD)
						{
							DecSpeed(&nSpeed, nIncAmount);
						}
						else
						{
							IncSpeed(&nSpeed, nIncAmount);
						}
					break;

					case EVENT_SPEED_INC_FAST:
						if(GetLocoDirection(pLoco) == DIR_FORWARD)
						{
							IncSpeed(&nSpeed, nIncAmount * 4);
						}
						else
						{
							DecSpeed(&nSpeed, nIncAmount * 4);
						}
					break;

					case EVENT_SPEED_DEC_FAST:
						if(GetLocoDirection(pLoco) == DIR_FORWARD)
						{
							DecSpeed(&nSpeed, nIncAmount * 4);
						}
						else
						{
							IncSpeed(&nSpeed, nIncAmount * 4);
						}
					break;
				}

				//if(nSpeed == 0 && SpeedWasZero == FALSE)
				if(nSpeed == 0)
				{
					if(GetLocoDirection(pLoco) == DIR_FORWARD)
					{
						SetLocoDirection(pLoco, DIR_REVERSE);
						DisplayDirection(pVirtualCab, DIR_REVERSE);
					}
					else
					{
						SetLocoDirection(pLoco, DIR_FORWARD);
						DisplayDirection(pVirtualCab, DIR_FORWARD);
					}
				}
			break;
		}
		
		SetLocoSpeed(pLoco, nSpeed);
		
		SetLocoDisconnect(pVirtualCab, pLoco);
		
		DisplaySpeed(pVirtualCab, nSpeed);
	}
}


/**********************************************************************
*
* FUNCTION:		DoSpeedKnob
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
void DoSpeedKnob(VIRTUAL_CAB* pVirtualCab)
{
	Loco* pLoco;
	unsigned int nUpperSpeed;
	unsigned int nLowerSpeed;
	unsigned int nNewSpeed;
	char szTemp[16];
	unsigned int nTemp;

	
	pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
	nNewSpeed = pVirtualCab->nSpeed;
	
	if(pLoco != NULL && (pVirtualCab->nMenuShowing & MENU_SHOWING_ESTOP) == 0)
	{
		nNewSpeed *= 2;		// convert to the "speed prime number"

		if(pLoco->SpeedFcnMode == SPEED_MODE_GTZ)
		{
			if(nNewSpeed > (SPEED_PRIME_NUMBER / 2))
			{
				nNewSpeed -= (SPEED_PRIME_NUMBER / 2);
				nNewSpeed *= 2;		// compensate for the half range
				SetLocoDirection(pLoco, DIR_FORWARD);
				DisplayDirection(pVirtualCab, DIR_FORWARD);
			}
			else
			{
				nNewSpeed = ((SPEED_PRIME_NUMBER / 2) - nNewSpeed) * 2;		// compensate for the half range
				SetLocoDirection(pLoco, DIR_REVERSE);
				DisplayDirection(pVirtualCab, DIR_REVERSE);
			}
		}

		
		nUpperSpeed = GetLocoSpeed(pLoco) + 6;
		if(nUpperSpeed > SPEED_PRIME_NUMBER)
		{
			nUpperSpeed = SPEED_PRIME_NUMBER;
		}
		
		nLowerSpeed = GetLocoSpeed(pLoco) - 6;
		if(nLowerSpeed > SPEED_PRIME_NUMBER)
		{
			nLowerSpeed = 0;
		}
		
		if(nNewSpeed >= nLowerSpeed && nNewSpeed < nUpperSpeed)
		{
			if(pVirtualCab->bSpeedDisconnect != 0)
			{
				DisplayDirection(pVirtualCab, GetLocoDirection(pLoco));
			}
			pVirtualCab->bSpeedDisconnect = 0;
		}
		
		if(pVirtualCab->bSpeedDisconnect == 0)
		{
			SetLocoSpeed(pLoco, nNewSpeed);
			DisplaySpeed(pVirtualCab, nNewSpeed);
		}
		else
		{
			switch(GetLocoSpeedMode(pLoco))
			{
				case SPEED_MODE_14:
					nNewSpeed /= SPEED_14_DIVISOR;
					sprintf(szTemp, " %3d", nNewSpeed);
				break;

				case SPEED_MODE_28:
					nNewSpeed /= SPEED_28_DIVISOR;
					sprintf(szTemp, " %3d", nNewSpeed);
				break;

				case SPEED_MODE_128:
					nNewSpeed /= SPEED_128_DIVISOR;
					sprintf(szTemp, " %3d", nNewSpeed);
				break;

				case SPEED_MODE_14_PERCENT:
				case SPEED_MODE_28_PERCENT:
				case SPEED_MODE_128_PERCENT:
					nTemp = GetLocoMaxSpeed(pLoco);

					if(nTemp == 0)
					{
						nNewSpeed /= 2;
						if(nNewSpeed != 0)
						{
							nNewSpeed = (nNewSpeed * 100) / 126;
							if(nNewSpeed == 0)
							{
								nNewSpeed++;
							}
						}
						sprintf(szTemp, " %3d", nNewSpeed);
					}
					else
					{
						if(GetLocoSpeedUnits(pLoco))
						{
							nNewSpeed *= (nTemp & 0x7f);
							nNewSpeed /= SPEED_PRIME_NUMBER;
							sprintf(szTemp, "%3dk", nNewSpeed);
						}
						else
						{
							nNewSpeed *= nTemp;
							nNewSpeed /= SPEED_PRIME_NUMBER;
							sprintf(szTemp, "%3d%%", nNewSpeed);
						}
					}
				break;
			}

			for(nTemp = 0; nTemp < 3; nTemp++)
			{
				if(szTemp[nTemp] == ' ')
				{
					szTemp[nTemp] = '*';
				}
			}
			NCE_DisplayMessage(pVirtualCab->Cab, 2, 0, szTemp);
			SetExpiration(pVirtualCab, EXPIRATION_05_SECONDS);
		}
	}
}



/**********************************************************************
*
* FUNCTION:		GetCombinedFunctions
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
word GetCombinedFunctions(VIRTUAL_CAB* pVirtualCab, Loco* pLoco)
{
	unsigned int FunctionMapMask;
	unsigned int FunctionMapTemp;
	unsigned int FunctionMap;
	int i;

	FunctionMap = GetLocoFunctions(pLoco);
	
	if(IsLeadLoco(pLoco))
	{
		if(pLoco != NULL)
		{
			for(i = 0; i < MAX_LOCOS; i++)
			{
				if((pLoco = pLoco->pConsistLink) == NULL)
				{
					break;
				}
				else
				{
					FunctionMapMask = GetLocoFunctionOverride(pLoco);
					FunctionMapTemp = GetLocoFunctions(pLoco);
					
					if(GetLocoAliasDirection(pLoco))
					{
						if(FunctionMapTemp & 0x01)
						{
							FunctionMapTemp |= 0x02;
						}
					}
					
					FunctionMap |= FunctionMapTemp & FunctionMapMask;
				}
			}
		}
	}
	
	return FunctionMap;
}



/**********************************************************************
*
* FUNCTION:		GetFunctionLoco
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
Loco* GetFunctionLoco(VIRTUAL_CAB* pVirtualCab, int nEvent, unsigned char* bOverride)
{
	unsigned int Mask;
	unsigned int FunctionMap;
	Loco* pLoco;
	Loco* pConsistLoco;
	int i;
	
	*bOverride = 0;
	pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
	pConsistLoco = pLoco;
	
	if(pLoco != NULL && IsLeadLoco(pLoco))
	{
		for(i = 0; i < MAX_LOCOS; i++)
		{
			if((pConsistLoco = pConsistLoco->pConsistLink) == NULL)
			{
				return pLoco;
			}
			else
			{
				FunctionMap = GetLocoFunctionOverride(pConsistLoco);
				switch(nEvent)
				{
					case EVENT_BELL:
						// ToDo - Make this programmable
						nEvent = EVENT_THREE;
						break;
					
					case EVENT_HORN_DOWN:
						// ToDo - Make this programmable
						nEvent = EVENT_TWO;
						break;
					
					case EVENT_HORN_UP:
						// ToDo - Make this programmable
						nEvent = EVENT_TWO;
						break;
				}
					
				Mask = wBitMask[nEvent - EVENT_ZERO];
				if(FunctionMap & Mask)
				{
					*bOverride = 1;
					return pConsistLoco;
				}
			}
		}
	}
	return pLoco;
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
void DoFunction(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	unsigned int Mask;
	unsigned int FunctionMap;
	unsigned char bOverride;
	Loco* pLoco;
	
	
	pLoco = GetFunctionLoco(pVirtualCab, nEvent, &bOverride);
	
	if(pLoco != NULL && (pVirtualCab->nMenuShowing & MENU_SHOWING_ESTOP) == 0 && GetLocoAddress(pLoco) != ANALOG_LOCO)
	{
		if(pVirtualCab->nMenuShowing == NO_MENU_SHOWING || nEvent == EVENT_HEADLIGHT)
		//if(pVirtualCab->nMenuShowing == FALSE) 		// these keys are used in menus
		{
			// translate Headlight event into a zero key
			if(nEvent == EVENT_HEADLIGHT)
			{
				nEvent = EVENT_ZERO;
			}
			
			// if functions are overridden, swap F0 & F1
			if(bOverride && GetLocoAliasDirection(pLoco))
			{
				if(IsLeadLoco(pLoco))
				{
					if(nEvent == EVENT_ONE)
					{
						nEvent = EVENT_ZERO;
					}
				}
			}
			
			FunctionMap = GetLocoFunctions(pLoco);
			Mask = wBitMask[nEvent - EVENT_ZERO];
			if(FunctionMap & Mask)
			{
				FunctionMap &= ~Mask;
			}
			else
			{
				FunctionMap |= Mask;
			}
			
			SetLocoFunctions(pLoco, FunctionMap);
			
			pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
			FunctionMap = GetCombinedFunctions(pVirtualCab, pLoco);
			DisplayFunction(pVirtualCab, FunctionMap);
		}
	}
}


/**********************************************************************
*
* FUNCTION:		DoRefresh
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
void DoRefresh(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	Loco* pLoco;
	unsigned int FunctionMap;
	
	pVirtualCab->nEStopState = 0;
	pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
	
	if(pLoco != NULL)
	{
		if(pVirtualCab->bSpeedDisconnect != 0)
		{
			DisplayDirection(pVirtualCab, GetLocoDirection(pLoco));
			DisplaySpeed(pVirtualCab, GetLocoSpeed(pLoco));
		}

	
		if(GetLocoAddress(pLoco) == ANALOG_LOCO)
		{
			NCE_DisplayMessage(pVirtualCab->Cab, 8, 1, "        ");
		}
		else
		{
			FunctionMap = GetCombinedFunctions(pVirtualCab, pLoco);
			DisplayFunction(pVirtualCab, FunctionMap);
		}
	}
}


/**********************************************************************
*
* FUNCTION:		DoRunFunction
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
void DoRunFunction(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	unsigned int Mask;
	unsigned int FunctionMap;
	unsigned char bOverride;
	Loco* pLoco;
	
	
	pLoco = GetFunctionLoco(pVirtualCab, nEvent, &bOverride);
	
	if(pLoco != NULL && (pVirtualCab->nMenuShowing & MENU_SHOWING_ESTOP) == 0 && GetLocoAddress(pLoco) != ANALOG_LOCO)
	{
		FunctionMap = GetLocoFunctions(pLoco);
		switch(nEvent)
		{
			case EVENT_BELL:
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
				break;
			
			case EVENT_HORN_DOWN:
				// ToDo - Make this programmable
				FunctionMap |= wBitMask[3];
				break;
			
			case EVENT_HORN_UP:
				// ToDo - Make this programmable
				FunctionMap &= ~wBitMask[3];
				break;
		}
	
		SetLocoFunctions(pLoco, FunctionMap);
			
		pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
		FunctionMap = GetCombinedFunctions(pVirtualCab, pLoco);
		DisplayFunction(pVirtualCab, FunctionMap);
	}
}


/**********************************************************************
*
* FUNCTION:		DoSpeedMode
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
void DoSpeedMode(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	int nSpeedMode;
	int nWhichText;
	Loco* pLoco;
	
	
	pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
	
	if(pLoco != NULL && (pVirtualCab->nMenuShowing & MENU_SHOWING_ESTOP) == 0)
	{
		// ToDo - is this necessary?
		if(GetLocoAddress(pLoco) == ANALOG_LOCO)
		{
			nSpeedMode = (GetLocoSpeedMode(pLoco) + 1) % 2;
		}
		else
		{
			// ToDo - make programmable to allow 14 speed steps
			nSpeedMode = (GetLocoSpeedMode(pLoco) + 1) % 4;
		}
		
		switch(nSpeedMode)
		{
			case SPEED_MODE_14:
				nWhichText = tSpd_14;
				break;
			
			case SPEED_MODE_14_PERCENT:
				nWhichText = tSpd_14P;
				break;
			
			case SPEED_MODE_28:
			default:
				nWhichText = tSpd_28;
				break;
			
			case SPEED_MODE_28_PERCENT:
				nWhichText = tSpd_28P;
				break;
			
			case SPEED_MODE_128:
				nWhichText = tSpd128;
				break;
			
			case SPEED_MODE_128_PERCENT:
				nWhichText = tSpd128P;
				break;
		}
		
		if((pVirtualCab->nMenuShowing & MENU_SHOWING_LINE_2R) == 0)
		{
			if(GetLocoMaxSpeed(pLoco) != 0)
			{

			}
			NCE_DisplayMessage(pVirtualCab->Cab, 7, 1, aText[nWhichText]);
			SetExpiration(pVirtualCab, EXPIRATION_2_SECONDS);
		}
		SetLocoSpeedMode(pLoco, nSpeedMode);
		DisplaySpeed(pVirtualCab, GetLocoSpeed(pLoco));
	}
}


/**********************************************************************
*
* FUNCTION:		DoBrake
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
void DoBrake(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	Loco* pLoco;
	unsigned int nSpeed;
	
	
	pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
	
	if(pLoco != NULL && (pVirtualCab->nMenuShowing & MENU_SHOWING_ESTOP) == 0)
	{
		SetLocoSpeed(pLoco, 0);

		//SetLocoDisconnect(pVirtualCab, pLoco);

		nSpeed = pVirtualCab->nSpeed;
		if(nSpeed == 0)
		{
			pVirtualCab->bSpeedDisconnect = 0;
		}
		else
		{
			pVirtualCab->bSpeedDisconnect = SD_ENABLE;
		}

		DisplaySpeed(pVirtualCab, 0);
	}
}


/**********************************************************************
*
* FUNCTION:		DoEmergencyStop
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
void DoEmergencyStop(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	Loco* pLoco;
	
	pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];

#ifdef NO_WHOLE_LAYOUT_ESTOP
	int i;
	unsigned int nSpeed;
	if((pVirtualCab->nMenuShowing & MENU_SHOWING_ESTOP) == 0)
	{
		SetExpiration(pVirtualCab, EPIRATION_1_SECONDS);
		
		if(pVirtualCab->nEStopState == 0)
		{
			if(pLoco != NULL)
			{
				nSpeed = pVirtualCab->nSpeed;
				if(nSpeed == 0)
				{
					pVirtualCab->bSpeedDisconnect = 0;
				}
				else
				{
					pVirtualCab->bSpeedDisconnect = SD_ENABLE;
				}
				
				SetLocoSpeed(pLoco, ESTOP);
				DisplaySpeed(pVirtualCab, ESTOP);
				pVirtualCab->nMenuShowing = MENU_SHOWING_ESTOP;
			}
			pVirtualCab->nEStopState++;
		}
		else if(pVirtualCab->nEStopState == 2)
		{
			pVirtualCab->nEStopState = 0;
			SetExpiration(pVirtualCab, 0);
				
			eStopCab = pVirtualCab->Cab;
			for(i = 0; i < MAX_VIRTUAL_CABS; i++)
			{
				// ToDo - should this change ALL cabs
				if(aVirtualCab[i].Cab)
				{
					ExitMenu(&aVirtualCab[i]);
					ChangeState(&aVirtualCab[i], STATE_ESTOP);
				}
			}
			
//			NCE_DisplayMessage(0, 0, 0, aText[tE_Stop]);
//			NCE_DisplayMessage(0, 0, 1, aText[tLayoutStopped]);
//			pVirtualCab->nMenuShowing = MENU_SHOWING_ESTOP;
//
//			DisableTrack();
//			DisableProgrammingTrack();
		}
		else
		{
			pVirtualCab->nEStopState++;
		}
	}
	else
	{
		pVirtualCab->nMenuShowing = NO_MENU_SHOWING;
		SetLocoSpeed(pLoco, 0);
		DisplaySpeed(pVirtualCab, GetLocoSpeed(pLoco));
	}
#else
	SetLocoSpeed(pLoco, ESTOP);
	DisplaySpeed(pVirtualCab, ESTOP);
#endif
}


/**********************************************************************
*
* FUNCTION:		DoRecall
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
void DoRecall(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
	int i;
	Loco* pLoco;
	unsigned int nSpeedMode;
	
	
	if((pVirtualCab->nMenuShowing & MENU_SHOWING_ESTOP) == 0)
	{
		for(i = pVirtualCab->nWhichRecall + 1; i < 8; i++)
		{
			if(pVirtualCab->pRecall[i] != NULL)
			{
				pVirtualCab->nWhichRecall = i;
				break;
			}
		}
		if(i >= 8)
		{
			pVirtualCab->nWhichRecall = 0;
		}
	
		// display the speed mode
		pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
	
		SetLocoDisconnect(pVirtualCab, pLoco);
	
		RestoreOperateScreen(pVirtualCab, 1, 1);
	
		if(pLoco != NULL)
		{
			if((pVirtualCab->nMenuShowing & MENU_SHOWING_LINE_2R) == 0)
			{
				nSpeedMode = GetLocoSpeedMode(pLoco);
				NCE_DisplayMessage(pVirtualCab->Cab, 8, 1, aText[tSpd_28 + nSpeedMode]);
				SetExpiration(pVirtualCab, EXPIRATION_2_SECONDS);
			}
		}
	}
}



/**********************************************************************
*
* FUNCTION:		SetLocoDisconnect
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
void SetLocoDisconnect(VIRTUAL_CAB* pVirtualCab, Loco* pLoco)
{
	unsigned int nSpeed;
	
	
	nSpeed = pVirtualCab->nSpeed * 2;
	
	if(GetLocoSpeed(pLoco) == nSpeed)
	{
		pVirtualCab->bSpeedDisconnect = 0;
	}
	else if(GetLocoSpeed(pLoco) > nSpeed)
	{
		pVirtualCab->bSpeedDisconnect = SD_ENABLE | SD_DIRECTION;
	}
	else
	{
		pVirtualCab->bSpeedDisconnect = SD_ENABLE;
	}
}


/**********************************************************************
*
* FUNCTION:		DoEnter
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
void DoEnter(VIRTUAL_CAB* pVirtualCab, int nEvent)
{
//	char szTemp[9];
//	Loco* pLoco;
//	unsigned int nAlias;
//
//	pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
//	nAlias = GetLocoAlias(pLoco);
//
//	if(nAlias && pVirtualCab->nState == STATE_IDLE)
//	{
//		//snprintf(szTemp, 9, aText[tLoc], nAddress);
//		snprintf(szTemp, 9, "CON:%3d", nAlias);
//		NCE_DisplayMessage(pVirtualCab->Cab, 9, 1, szTemp);
//		SetExpiration(pVirtualCab, EPIRATION_2_SECONDS);
//	}
}


#ifdef MOVE_THIS
/**********************************************************************
*
* FUNCTION:		DisplayClock
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
void DisplayClock(VIRTUAL_CAB* pVirtualCab)
{
	char szTime[10];


	CalcClock(szTime);

	NCE_DisplayMessage(0, 8, 0, szTime);
}
#endif

/**********************************************************************
*
* FUNCTION:		DisplayAddress
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
void DisplayAddress(VIRTUAL_CAB* pVirtualCab, unsigned int nAddress)
{
	char szTemp[17];
	Loco* pLoco;

	if(nAddress == ANALOG_LOCO)
	{
		NCE_DisplayMessage(pVirtualCab->Cab, 0, 1, aText[tAnalog]);
	}
	else
	{
		pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
		
		if(GetLocoAlias(pLoco))
		{
			if(IsLeadLoco(pLoco))
			{
				sprintf(szTemp, aText[tConLead], nAddress);
			}
			else
			{
				sprintf(szTemp, aText[tCon], nAddress);
			}
		}
		else
		{
			sprintf(szTemp, aText[tLoc], nAddress);
		}
		NCE_DisplayMessage(pVirtualCab->Cab, 0, 1, szTemp);
	}
	
	
	if((pVirtualCab->nMenuShowing & MENU_SHOWING_LINE_2L) != 0)
	{
		NCE_DisplayMessage(pVirtualCab->Cab, 8, 1, " ");
		SetExpiration(pVirtualCab, EXPIRATION_2_SECONDS);
	}
//	}
}

/**********************************************************************
*
* FUNCTION:		DisplayDirection
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
void DisplayDirection(VIRTUAL_CAB* pVirtualCab, unsigned char Direction)
{

	if((pVirtualCab->nMenuShowing & MENU_SHOWING_ESTOP) == 0)
	{
		if(Direction == DIR_FORWARD)
		{
			NCE_DisplayMessage(pVirtualCab->Cab, 0, 0, aText[tFwd]);
		}
		else
		{
			NCE_DisplayMessage(pVirtualCab->Cab, 0, 0, aText[tRev]);
		}
		
		if(pVirtualCab->nMenuShowing & MENU_SHOWING_LINE_1L)
		{
			SetExpiration(pVirtualCab, EXPIRATION_1_SECONDS);
		}
	}
}

/**********************************************************************
*
* FUNCTION:		DisplaySpeed
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
void DisplaySpeed(VIRTUAL_CAB* pVirtualCab, unsigned int nSpeed)
{
	char szTemp[16];
	Loco* pLoco;
	unsigned int nTemp;
	
	if((pVirtualCab->nMenuShowing & MENU_SHOWING_ESTOP) == 0)
	{
		pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];
		
		if(nSpeed == ESTOP)
		{
			NCE_DisplayMessage(pVirtualCab->Cab, 2, 0, aText[tEStp]);
		}
		else
		{
			switch(GetLocoSpeedMode(pLoco))
			{
				case SPEED_MODE_14:
				nSpeed /= 18;
				sprintf(szTemp, " %3d", nSpeed);
				break;
			
			case SPEED_MODE_28:
				nSpeed /= 9;
				sprintf(szTemp, " %3d", nSpeed);
				break;
			
			case SPEED_MODE_128:
				nSpeed /= 2;
				sprintf(szTemp, " %3d", nSpeed);
				break;
			
			case SPEED_MODE_14_PERCENT:
			case SPEED_MODE_28_PERCENT:
			case SPEED_MODE_128_PERCENT:
			
				nTemp = GetLocoMaxSpeed(pLoco);
				
				if(nTemp == 0)
				{
					nSpeed /= 2;
					if(nSpeed != 0)
					{
						nSpeed = (nSpeed * 100) / 126;
						if(nSpeed == 0)
						{
							nSpeed++;
						}
					}
					sprintf(szTemp, "%3d%%", nSpeed);
				}
				else
				{
					// ToDo - make this a better, programmable, option
					if(GetLocoSpeedUnits(pLoco))
					{
						nSpeed *= (nTemp & 0x7f);
						nSpeed /= SPEED_PRIME_NUMBER;
						sprintf(szTemp, "%3dk", nSpeed);
					}
					else
					{
						nSpeed *= nTemp;
						nSpeed /= SPEED_PRIME_NUMBER;
						//sprintf(szTemp, "%3dm", nSpeed);
						sprintf(szTemp, "%3d%%", nSpeed);
					}
				}
				break;
			}
		
			if(nSpeed == 0)
			{
				NCE_DisplayMessage(pVirtualCab->Cab, 2, 0, aText[tStop]);
			}
			else
			{
				NCE_DisplayMessage(pVirtualCab->Cab, 2, 0, szTemp);
			}
		}
		
		if(pVirtualCab->bSpeedDisconnect)
		{
			if(pVirtualCab->bSpeedDisconnect & SD_DIRECTION)
			{
				NCE_DisplayMessage(pVirtualCab->Cab, 1, 0, "'");
			}
			else
			{
				NCE_DisplayMessage(pVirtualCab->Cab, 1, 0, ".");
			}
		}
		else
		{
			NCE_DisplayMessage(pVirtualCab->Cab, 1, 0, ":");
		}
		
		if(pVirtualCab->nMenuShowing & MENU_SHOWING_LINE_1L)
		{
			if(pVirtualCab->CursorState == 1)
			{
				pVirtualCab->CursorState = 0;
				NCE_CursorOff(pVirtualCab->Cab);
			}
			DisplayDirection(pVirtualCab, GetLocoDirection(pLoco));
			SetExpiration(pVirtualCab, EXPIRATION_1_SECONDS);
		}
	}
}

/**********************************************************************
*
* FUNCTION:		DisplayFunction
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
void DisplayFunction(VIRTUAL_CAB* pVirtualCab, unsigned long FunctionMap)
{
	char szFunctionList[9];
	int i;
	
	strcpy(szFunctionList, aText[tL123456]);

	// ToDo - make this programmable
	szFunctionList[2] = 'B';
	szFunctionList[3] = 'H';

	if((pVirtualCab->nMenuShowing & MENU_SHOWING_LINE_2R) == 0)
	{
		for(i = 0; i < 9; i++)
		{
			if((FunctionMap & lBitMask[i+1]) == 0)
			{
				szFunctionList[i + 1] = '-';
			}
		}
		NCE_DisplayMessage(pVirtualCab->Cab, 6, 1, szFunctionList);
	}

	if((pVirtualCab->nMenuShowing & MENU_SHOWING_LINE_1R) == 0)
	{
		if((FunctionMap & lBitMask[0]) != 0)
		{
			strcpy(szFunctionList, " L");
		}
		else
		{
			strcpy(szFunctionList, " -");
		}

		NCE_DisplayMessage(pVirtualCab->Cab, 6, 0, szFunctionList);
	}
}

#ifdef NOT_USED
/**********************************************************************
*
* FUNCTION:		DisplayUpperFunction
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
void DisplayUpperFunction(VIRTUAL_CAB* pVirtualCab, unsigned long FunctionMap)
{
	char szFunctionList[9];
	int i;
	
	strcpy(szFunctionList, aText[t789012]);
	if((pVirtualCab->nMenuShowing & MENU_SHOWING_LINE_2R) == 0)
	{
		for(i = 9; i < 15; i++)
		{
			if((FunctionMap & lBitMask[i]) == 0)
			{
				szFunctionList[i - 7] = '-';
			}
		}
		NCE_DisplayMessage(pVirtualCab->Cab, 6, 1, szFunctionList);
	}
}
#endif


/**********************************************************************
*
* FUNCTION:		RestoreOperateScreen
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
void RestoreOperateScreen(VIRTUAL_CAB* pVirtualCab, unsigned char Line1, unsigned char Line2)
{
	Loco* pLoco;
	unsigned int FunctionMap;
	
	
	//if(GetTrackLock() == TR_COMMAND_STATION)
	//{
		pLoco = pVirtualCab->pRecall[pVirtualCab->nWhichRecall];

		if(Line1)
		{
			if(pLoco == NULL)
			{
				NCE_DisplayMessage(pVirtualCab->Cab, 0, 0, aText[tFWD_Default]);
			}
			else
			{
				DisplayDirection(pVirtualCab, GetLocoDirection(pLoco));
				DisplaySpeed(pVirtualCab, GetLocoSpeed(pLoco));
			}

			UpdateWangrowClock();
		}

		if(Line2)
		{
			if(pLoco == NULL)
			{
				NCE_DisplayMessage(pVirtualCab->Cab, 0, 1, aText[tLOC_Default]);
			}
			else
			{
				DisplayAddress(pVirtualCab, GetLocoAddress(pLoco));

				if(GetLocoAddress(pLoco) == ANALOG_LOCO)
				{
					NCE_DisplayMessage(pVirtualCab->Cab, 8, 1, " xxxxxxx");
				}
				else
				{
					FunctionMap = GetCombinedFunctions(pVirtualCab, pLoco);
					DisplayFunction(pVirtualCab, FunctionMap);
				}
			}
		}
	//}
	//else
	//{
	//	ChangeState(pVirtualCab, STATE_TRACK);
	//}
}



