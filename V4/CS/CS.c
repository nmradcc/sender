/**********************************************************************
*
* SOURCE FILENAME:	CS.c
*
* DATE CREATED:		2/Apr/2019
*
* PROGRAMMER:
*
* DESCRIPTION:
*
* COPYRIGHT (c) 2000-2019 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/
#include "main.h"
#include "cmsis_os.h"
#include "CS.h"
#include "Track.h"
#include "Packet.h"

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

void HandlePackets(void);

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

static unsigned char PacketBuf[8];

/**********************************************************************
*
*							CODE
*
**********************************************************************/

/**********************************************************************
*
* FUNCTION:		CommandStationTask
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
void CommandStationTask(void* argument)
{
	VIRTUAL_CAB* pVirtualCab;
	int	nEvent;
	uint8_t bMessageType;
	//uint8_t work[_MAX_SS];
	//FRESULT res;
	uint8_t ExpirationCount = 10;
	//TRACK_RESOURCE tl;

	InitMessageQueue();

	InitLoco();
	InitVirtualCab();
	InitLocoList();
	StopAllLocos();

	InitCabCommunication(0);
	//k	InitXpressNet();

	// set the clock update callback function
	RegisterClockUpdate(UpdateWangrowClock);
	//RegisterClockUpdate(UpdateXpressnetClock);

	// ************ Wangrow / NCE state machine
	InitState(STATE_IDLE);
	// ************ Wangrow / NCE state machine

	// temporary - open the track for the Command Station
//	(void)OpenTrack(TR_COMMAND_STATION, TI_IDLE, 0);
	// temporary - open the track for the Command Station

	while(1)
	{
		HandleCabCommunication();
		HandlePackets();
		Acknowledge();
		ServiceMode();

		pVirtualCab = GetMessage(&bMessageType, &nEvent);
		if(pVirtualCab != NULL)
		{
			nEvent = pVirtualCab->nEvent;
			if(nEvent & EVENT_SPEED_TYPE)
			{
				RunOperate(pVirtualCab, nEvent);
			}
			else
			{
				RunState(pVirtualCab, nEvent);
				RunOperate(pVirtualCab, nEvent);
			}
		}

		ExpirationCount--;
		if(ExpirationCount == 0)
		{
			ExpirationCount = 10;
			HandleExpiration();

			CheckClockUpdate();
		}

		//if(GetTrackLock() != tl)
		//{
		//	QueueMessage(0, &aVirtualCab[i], EVENT_TIMER_EXPIRED);
		//}

		osDelay(pdMS_TO_TICKS(1));
	}
}


/**********************************************************************
*
* FUNCTION:		HandlePackets
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
void HandlePackets(void)
{
	uint8_t* pPacket = PacketBuf;
	//static BOOL fWhich;
	//static BOOL fWhichFunction;
	uint32_t len = 0;
	static Loco* pLoco;
	unsigned int nAlias;
	static uint8_t bfInsertIdle;


	if(IsTrackOpen(TR_COMMAND_STATION))
	{

//		pPacket = abPacket;


		if(bfInsertIdle)
		{
			bfInsertIdle = 0;
//			BuildIdlePacket(pPacket);
		}
		else
		{
			bfInsertIdle = 1;

//			if(!GetProgrammingPacket(pPacket))
			{
				pLoco = NextLocoMsg();
				if(pLoco != NULL && pLoco->Address != 0)
				{
					if(pLoco->bChange == CH_FUNCTION_1)
					{
						len = BuildFunction1Packet(pPacket, pLoco->Address, pLoco->FunctionMap);
					}
					else if(pLoco->bChange == CH_FUNCTION_2)
					{
						len = BuildFunction2Packet(pPacket, pLoco->Address, pLoco->FunctionMap);
					}
					else if(pLoco->bChange == CH_FUNCTION_3)
					{
						len = BuildFunction3Packet(pPacket, pLoco->Address, pLoco->FunctionMap);
					}
					else if(pLoco->bChange == CH_FUNCTION_4)
					{
						len = BuildFunction4Packet(pPacket, pLoco->Address, pLoco->FunctionMap);
					}
					else
					{
						nAlias = GetLocoAlias(pLoco);
						if(nAlias)
						{
							len = BuildLocoPacket(pPacket, nAlias, pLoco->Speed, pLoco->Direction, pLoco->SpeedMode);
						}
						else
						{
							len = BuildLocoPacket(pPacket, pLoco->Address, pLoco->Speed, pLoco->Direction, pLoco->SpeedMode);
						}
					}
				}
			}
//			else
//			{
//
//			}
		}

		if(len)
		{
			BuildPacketCS(pPacket, len, 116, 100, 50, 18);
		}
	}
}

