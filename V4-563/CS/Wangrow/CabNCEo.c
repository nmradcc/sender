/**********************************************************************
*
* SOURCE FILENAME:	CabBus.c
*
* DATE CREATED:		9/9/98
*
* PROGRAMMER:		
*
* DESCRIPTION:  	
*
* COPYRIGHT (c) 1999-2017 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/
#include "stm32l4xx_hal.h"
#include "CabNCE.h"
#include "Main.h"
#include <string.h>
#include <ctype.h>
#include "Events.h"
#include "MsgQueue.h"
#include "Uart2.h"
#include "Uart3.h"
#include "BitMask.h"


/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define CAB_POLL_TIME				5

#define CAB_NO_KEY					0x7d
#define CAB_REFRESH_DISPLAY			0x7e
#define CAB_NO_SPEED				0x7f

/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

byte HandleCabOutput(byte bCab);
void SendCabMessage(byte CommandCode, char *Message);
void HandleCabResponse(byte Cab);
void SendCabText(byte bCab);
byte IsAnyCabText(byte bCab);

void NCE_PutQueue(byte bCab, int nMsg);

void RxCabResponse(byte* Response, byte* ResponseLength);
void TxCabResponse(void);

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

unsigned long NCE_KeyFilter;

byte abCabActiveQueue[MAX_CABS];
byte abCabInactiveQueue[MAX_CABS];
byte bCabIndex;
byte bInactiveCabIndex;

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/

static CAB_BUS CabBus[MAX_CABS];

static byte iCabBusState;

static byte CabResponse[5];
static byte GotCabResponse;

static byte	TxComplete;
static byte	TxDelayTime;

static byte Packet[9];
static byte PacketLength;


/**********************************************************************
*
*							CODE
*
**********************************************************************/
unsigned char cBitMask[] =
{
	0x01, 0x02,	0x04,	0x08,	0x10,	0x20,	0x40,	0x80
};

#define CAB_KEY_OFFSET	0x40

int abCabKeyTranslate[] =
{
		EVENT_ENTER,					// CK_ENTER
		EVENT_PROG_MODE,				// CK_PROG_MODE
		EVENT_RECALL,					// CK_RECALL
		EVENT_DIRECTION,				// CK_DIRECTION
		EVENT_CONSIST_SETUP,			// CK_CONSIST_SETUP
		EVENT_CONSIST_ADD,				// CK_CONSIST_ADD
		EVENT_CONSIST_DELETE,			// CK_CONSIST_DELETE
		EVENT_CONSIST_KILL,				// CK_CONSIST_KILL
		EVENT_SELECT_LOCO,				// CK_SELECT_LOCO
		EVENT_HORN_DOWN,				// CK_HORN_DOWN
		EVENT_SPEED_INC,				// CK_SPEED_INC
		EVENT_SPEED_DEC,				// CK_SPEED_DEC
		EVENT_EMERGENCY_STOP,			// CK_EMERGENCY_STOP
		EVENT_BELL,						// CK_BELL
		EVENT_SELECT_ACCESSORY,			// CK_SELECT_ACCESSORY
		EVENT_EXPANSION,				// CK_EXPANSION
		EVENT_ZERO,						// CK_ZERO
		EVENT_ONE,						// CK_ONE
		EVENT_TWO,						// CK_TWO
		EVENT_THREE,					// CK_THREE
		EVENT_FOUR,			  			// CK_FOUR
		EVENT_FIVE,			  			// CK_FIVE
		EVENT_SIX,			  			// CK_SIX
		EVENT_SEVEN,		  			// CK_SEVEN
		EVENT_EIGHT,		  			// CK_EIGHT
		EVENT_NINE,			  			// CK_NINE
		EVENT_SPEED_INC_FAST,			// CK_SPEED_INC_FAST
		EVENT_SPEED_DEC_FAST,			// CK_SPEED_DEC_FAST
		EVENT_SELECT_MACRO,				// CK_MACRO
		EVENT_SPEED_MODE,				// CK_SPEED_MODE
		EVENT_BRAKE,					// CK_BRAKE
		EVENT_HORN_UP,					// CK_HORN_UP
		EVENT_ASSIGN_LOCO,				// CK_ASSIGN_LOCO
		EVENT_PROG_ON_MAIN,				// CK_PROG_ON_MAIN
		EVENT_SET_CLOCK,				// CK_SET_CLOCK
		EVENT_PROG_ON_TRACK,			// CK_PROG_ON_TRACK
		EVENT_SETUP_CS,					// CK_SETUP_CS
		EVENT_SETUP_CAB,				// CK_SETUP_CAB
		EVENT_DEFINE_MACRO,				// CK_DEFINE_MACRO
		EVENT_CONSIST_SETUP_OLD,		// CK_CONSIST_SETUP_OLD
		EVENT_CONSIST_SETUP_ADV,		// CK_CONSIST_SETUP_ADV
		EVENT_TOGGLE_LEFT_RIGHT,		// CK_TOGGLE_LEFT_RIGHT
		EVENT_DIR_FORWARD,				// CK_DIR_FORWARD
		EVENT_DIR_REVERSE,				// CK_DIR_REVERSE
		EVENT_KEY_32,					// CK_KEY_32
};

void TaskCabPoll(void);
void TaskCabResponse(void);
void TaskCabTimeout(void);
void SelectNextCab(void);
void MakeCabActive(byte bCab);


byte bCab;
byte nRecvState;

#define RSTATE_NOTHING	0
#define RSTATE_KEY		1
#define	RSTATE_SPEED	2
#define	RSTATE_CAB_TYPE	3

#define CAB_TIMEOUT		300


enum
{
 	CAB_BUS_STARTUP,
 	CAB_BUS_POLLING,
	CAB_BUS_TRANSMIT,
	CAB_BUS_TRANSMIT_DELAY,
	CAB_BUS_TRANSMIT_WAIT,
    CAB_BUS_EXSISTING_CAB,
    CAB_BUS_NEW_CAB,

// 	CAB_BUS_BROADCAST,
// 	CAB_SEND_NEW_DATA,
// 	CAB_SEND_EXISTING_DATA,
};



/**********************************************************************
*
* FUNCTION:		InitCabCommunication
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
void InitCabCommunication(void)
{
	byte i;
	UART_DEF uart_def;

	if(CabPortProtocol1 == CAB_PROTOCOL_NCE)
	{
		uart_def.mode = U_IDLE;
		uart_def.bits = U_DATAWIDTH_8B;
		uart_def.parity = U_PARITY_NONE;
		uart_def.stop_bits = U_STOPBITS_2;
		uart_def.baud = 9600;
		uart_def.UartRxCallback = RxCabResponse;
		uart_def.UartTxCallback = TxCabResponse;
		if(Uart2_Init(&uart_def) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
	}

	if(CabPortProtocol2 == CAB_PROTOCOL_NCE)
	{
		uart_def.mode = U_IDLE;
		uart_def.bits = U_DATAWIDTH_8B;
		uart_def.parity = U_PARITY_NONE;
		uart_def.stop_bits = U_STOPBITS_2;
		uart_def.baud = 9600;
		uart_def.UartRxCallback = RxCabResponse;
		uart_def.UartTxCallback = TxCabResponse;
		if(Uart3_Init(&uart_def) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
	}

	memset(abCabActiveQueue, 0, MAX_CABS);

	for(i = 0; i < MAX_CABS - 1; i++)
	{
		abCabInactiveQueue[i] = i + 1;

		memset(CabBus, 0, sizeof(CabBus));

		CabBus[i].Flags = 0;

		CabBus[i].nQueue1 = 0;
		CabBus[i].nQueue2 = 0;
		CabBus[i].nQueue3 = 0;
		CabBus[i].nQueue4 = 0;

		CabBus[i].bVersionReturn = FALSE;
	}

	bCabIndex = MAX_CABS;
	bInactiveCabIndex = 0;
	bCab = 1;					// anything non-zero
}


void SelectNextCab(void)
{

	if(bCab == 0)
	{
		bCab = abCabInactiveQueue[bInactiveCabIndex];
		if(bCab == 0)
		{
			bInactiveCabIndex = 0;
			bCab = abCabInactiveQueue[bInactiveCabIndex];
		}
		bInactiveCabIndex++;
	}
	else
	{
		// select the next Cab
		bCabIndex++;
		if(bCabIndex >= MAX_CABS)
		{
			bCabIndex = 0;
		}

		bCab = abCabActiveQueue[bCabIndex];
	}
}


void MakeCabActive(byte bCab)
{
	byte i;

	// take it out of the inactive list
	for(i = bInactiveCabIndex; i < MAX_CABS - 1; i++)
	{
		abCabInactiveQueue[i] = abCabInactiveQueue[i + 1];
	}
	abCabInactiveQueue[i] = 0;

	// put it in the active list
	abCabActiveQueue[bCabIndex] = bCab;

	// reset to beginning of list
	bCabIndex = 0;

	bCab = abCabActiveQueue[bCabIndex];
}


/**********************************************************************
*
* FUNCTION:		HandleCabOutput
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
byte HandleCabOutput(byte bCab)
{
	byte bQuadrant;
	byte bTemp;

	// Handle the single/two byte commands from the "queue"
	if(CabBus[bCab].nQueue1)
	{
		bTemp = CabBus[bCab].nQueue1 & 0xff;
		Packet[0] = bTemp;
		PacketLength = 1;

		if(bTemp == MOVE_CURSOR || bTemp == TTY_BACK || bTemp == TTY_NEXT)
		{
			bTemp = CabBus[bCab].nQueue1 >> 8;
			Packet[1] = bTemp;
			PacketLength++;
		}

		// pop the "queue"
		CabBus[bCab].nQueue1 = CabBus[bCab].nQueue2;
		CabBus[bCab].nQueue2 = CabBus[bCab].nQueue3;
		CabBus[bCab].nQueue3 = CabBus[bCab].nQueue4;
		CabBus[bCab].nQueue4 = 0;
		return 1;
	}
	else
	{
		bQuadrant = IsAnyCabText(bCab);
		if(bQuadrant)
		{
			bQuadrant--;

			switch(bQuadrant)
			{
				case 0:
					bTemp = CAB_TEXT_1;
				break;
				case 1:
					bTemp = CAB_TEXT_2;
				break;
				case 2:
					bTemp = CAB_TEXT_3;
				break;
				case 3:
					bTemp = CAB_TEXT_4;
				break;
				default:
					bTemp = CAB_TEXT_1;
				break;
			}
			Packet[0] = bTemp;
			memcpy(&Packet[1], CabBus[bCab].TextQuadrant[bQuadrant], 8);
			PacketLength = 9;

//			Uart2_SendPacket(Packet, 9);
//			bByteCount = 8;
//			bCommandChar = bTemp;
//			pcText = CabBus[bCab].TextQuadrant[bQuadrant];

			if(CabBus[bCab].bCursorOn)
			{
				NCE_PutQueue(bCab, MOVE_CURSOR | (CabBus[bCab].nCursor << 8));
			}
			return 1;
		}
	}
	return 0;
}



/**********************************************************************
*
* FUNCTION:		IsKeyRegistered
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
unsigned char IsKeyRegistered(byte bCab, short nEvent)
{

	if(nEvent < 32)
	{
		if((NCE_KeyFilter & lBitMask[bCab]) != 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**********************************************************************
*
* FUNCTION:		SendNCECabKey
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
void SendNCECabKey(byte bCab, short nEvent)
{

//k	if(IsKeyRegistered(bCab, nEvent))
//k	{
//k		if(DoRun(ALL_PORTS, "NCE_Key.txt") != CMD_CANCEL_KEY)
//k		{
//k			QueueMessage(MSG_CAB_KEY_MESSAGE, CabBus[bCab].pVirtualCab, nEvent);
//k		}
//k	}
//k	else
	{
		QueueMessage(MSG_CAB_KEY_MESSAGE, CabBus[bCab].pVirtualCab, nEvent);
	}
}


/**********************************************************************
*
* FUNCTION:		IsAnyCabText
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
byte IsAnyCabText(byte bCab)
{
	int i;

	// send any new text
   for(i = 0; i < 4; i++)
   {
		if(CabBus[bCab].Flags & cBitMask[i])
		{
			CabBus[bCab].Flags &= ~cBitMask[i];
			return i + 1;
		}
   }
   return 0;
}


/**********************************************************************
*
* FUNCTION:		SetLimits
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
void SetLimits(byte bCab, byte upper, byte lower)
{

	CabBus[bCab].UpLimit = upper;
	CabBus[bCab].DownLimit = lower;
}


/**********************************************************************
*
* FUNCTION:		NCE_RefreshDisplay
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
void NCE_RefreshDisplay(byte bCab)
{

	CabBus[bCab].Flags = NCE_TEXT_FLAG_1 | NCE_TEXT_FLAG_2 | NCE_TEXT_FLAG_3 | NCE_TEXT_FLAG_4;
}


/**********************************************************************
*
* FUNCTION:		NCE_DisplayMessage
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
void NCE_DisplayMessage(byte bCab, byte x, byte y, char* pcMessage)
{
	byte c;
	byte i;

	//while(*pcMessage)
	for(i = 0; i < 32; i++)
	{
		if(*pcMessage == 0)
		{
			break;
		}

		c = *pcMessage++;
		if(c == 0x0a)
		{
			y++;
			x = 0;
			c = *pcMessage++;
		}

		// convert to uppercase
		c = (char)toupper((int)c);

		CabBus[bCab].TextQuadrant[y * 2][x] = c | 0xc0;
		CabBus[bCab].Flags |= bBitMask[(x / 8) + (y * 2)];
		x++;
		if(x > 15)
		{
			y++;
			x = 0;
		}
	}
}


/**********************************************************************
*
* FUNCTION:		NCE_PutQueue
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
void NCE_PutQueue(byte bCab, int nMsg)
{

	CabBus[bCab].nQueue4 = CabBus[bCab].nQueue3;
	CabBus[bCab].nQueue3 = CabBus[bCab].nQueue2;
	CabBus[bCab].nQueue2 = CabBus[bCab].nQueue1;
  	CabBus[bCab].nQueue1 = nMsg;
}


/**********************************************************************
*
* FUNCTION:		NCE_DisplayChar
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
void NCE_DisplayChar(byte bCab, unsigned char c)
{

	// convert to uppercase
	c = (char)toupper((int)c);
	NCE_PutQueue(bCab, TTY_NEXT | (c << 8));
}


/**********************************************************************
*
* FUNCTION:		NCE_SetCursorPosition
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
void NCE_SetCursorPosition(byte bCab, byte x, byte y)
{

	CabBus[bCab].nCursor = ((y * CAB_KEY_OFFSET) + x) | 0x80;
}


/**********************************************************************
*
* FUNCTION:		NCE_CursorOn
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
void NCE_CursorOn(byte bCab)
{

	NCE_PutQueue(bCab, CURSOR_ON);
	CabBus[bCab].bCursorOn = TRUE;
}


/**********************************************************************
*
* FUNCTION:		NCE_CursorOff
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
void NCE_CursorOff(byte bCab)
{

	NCE_PutQueue(bCab, CURSOR_OFF);
	CabBus[bCab].bCursorOn = FALSE;
}


/**********************************************************************
*
* FUNCTION:		NCE_ClearDisplay
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
void NCE_ClearDisplay(byte bCab)
{

	NCE_PutQueue(bCab, CLEAR_DISPLAY);
}


/**********************************************************************
*
* FUNCTION:		HandleCabCommunication
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
void HandleNCECabCommunication(void)
{
	byte i;
	static byte bCab;
	static int iPollTime;
	static byte bNewCab;
	byte poll;

	switch(iCabBusState)
	{
	 	case CAB_BUS_STARTUP:
			bCabIndex = 0;
			bInactiveCabIndex = 0;
			iPollTime = 1;
			bCab = 0;
			iCabBusState = CAB_BUS_POLLING;
	 	break;

	 	case CAB_BUS_POLLING:
			if(--iPollTime == 0)
			{
				poll = bCab + 0x80;
				//Uart2_SendToken(poll);
				//Uart2_SendPacket(&poll, 1);
				Uart3_SendToken(poll);
				//Uart3_SendPacket(&poll, 1);

				GotCabResponse = 0;

				iPollTime = CAB_POLL_TIME;

				if(bCab == 0)
				{
					// Cab is zero
					if(HandleCabOutput(bCab))
					{
						TxDelayTime = 1;
						iCabBusState = CAB_BUS_TRANSMIT_DELAY;
						//iCabBusState = CAB_BUS_TRANSMIT;
					}
					else
					{
						bCab = abCabInactiveQueue[bInactiveCabIndex];
						if(bCab == 0)
						{
							bInactiveCabIndex = 0;
							bCab = abCabInactiveQueue[bInactiveCabIndex];
						}
						bNewCab = TRUE;
						iCabBusState = CAB_BUS_POLLING;
					}
				}
				else
				{
					// Cab is not zero
					if(bNewCab)
					{
						bNewCab = FALSE;
						iCabBusState = CAB_BUS_NEW_CAB;
					}
					else
					{
						iCabBusState = CAB_BUS_EXSISTING_CAB;
					}
				}
			}
	 	break;

	 	case CAB_BUS_TRANSMIT_DELAY:
			TxDelayTime--;
			if(TxDelayTime == 0)
			{
				iCabBusState = CAB_BUS_TRANSMIT;
			}
	 	break;

	 	case CAB_BUS_TRANSMIT:
			//Uart2_SendPacket(Packet, PacketLength);
			Uart3_SendPacket(Packet, PacketLength);
			iCabBusState = CAB_BUS_TRANSMIT_WAIT;
	 	break;

	 	case CAB_BUS_TRANSMIT_WAIT:
			if(TxComplete)
			{
				TxComplete = FALSE;
				bCab = abCabInactiveQueue[bInactiveCabIndex];
				iPollTime = CAB_POLL_TIME;
				iCabBusState = CAB_BUS_POLLING;
			}
	 	break;

	    case CAB_BUS_EXSISTING_CAB:

			if(GotCabResponse)
			{
				GotCabResponse = 0 ;

		    	// this cab responded - queue a 'cab' message
				HandleCabResponse(bCab);

				// select the next Cab
				bCabIndex++;
				if(bCabIndex >= MAX_CABS)
				{
					bCabIndex = 0;
				}

				if(HandleCabOutput(bCab))
				{
					TxDelayTime = 1;
					iCabBusState = CAB_BUS_TRANSMIT_DELAY;
					//iCabBusState = CAB_BUS_TRANSMIT;
				}
				else
				{
					bCab = abCabActiveQueue[bCabIndex];
					iPollTime = CAB_POLL_TIME;
					iCabBusState = CAB_BUS_POLLING;
				}
			}
			else
			{
				if(--iPollTime == 0)
				{
					// no response

					// cab has timed-out, remove from active list
					for(i = bCabIndex; i < MAX_CABS - 1; i++)
					{
						abCabActiveQueue[i] = abCabActiveQueue[i + 1];
					}
					abCabActiveQueue[i] = 0;

					// put it in the inactive list
					for(i = 0; i < MAX_CABS; i++)
					{
						if(abCabInactiveQueue[i] == 0)
						{
							break;
						}
					}
					abCabInactiveQueue[i] = bCab;

					// select the next Cab
					bCabIndex++;
					if(bCabIndex >= MAX_CABS)
					{
						bCabIndex = 0;
					}

					bCab = abCabActiveQueue[bCabIndex];
					iPollTime = CAB_POLL_TIME;
					iCabBusState = CAB_BUS_POLLING;
				}
			}
	    break;

	    case CAB_BUS_NEW_CAB:
		    if(GotCabResponse)
			{
		    	GotCabResponse = 0 ;

				// this cab responded - queue a 'cab' message
				HandleCabResponse(bCab);

				// handle any cab list management

				// take it out of the inactive list
				for(i = bInactiveCabIndex; i < MAX_CABS - 1; i++)
				{
            		abCabInactiveQueue[i] = abCabInactiveQueue[i + 1];
            	}
            	abCabInactiveQueue[i] = 0;

				// put it in the active list
				abCabActiveQueue[bCabIndex] = bCab;

				// reset to beginning of list
				bCabIndex = 0;

				if(HandleCabOutput(bCab))
				{
					TxDelayTime = 1;
					//iCabBusState = CAB_BUS_TRANSMIT_DELAY;
					iCabBusState = CAB_BUS_TRANSMIT;
				}
				else
				{
					bCab = abCabActiveQueue[bCabIndex];
					iPollTime = CAB_POLL_TIME;
					iCabBusState = CAB_BUS_POLLING;
				}
			}
			else
			{
				if(--iPollTime == 0)
				{
					// no response

					// reset to beginning of list
					bCabIndex = 0;

					// select next inactive Can
					bInactiveCabIndex++;
					if(abCabInactiveQueue[bInactiveCabIndex] == 0)
        			{
						bInactiveCabIndex = 0;
					}

					bCab = abCabActiveQueue[bCabIndex];
					iPollTime = CAB_POLL_TIME;
					iCabBusState = CAB_BUS_POLLING;
				}
			}
	    	break;

	    default:
	    	iCabBusState = CAB_BUS_STARTUP;
	    	break;
	}


#ifdef NOT_USED
	case CAB_BUS_BROADCAST:
		// send the data
		if(bByteCount--)
		{
			if(bCommandChar)
			{
				c = bCommandChar;
				bCommandChar = 0;
			}
			else
			{
				c = *pcText++;
			}
			Uart2_SendPacket(&c, 1);
		}
		else
		{
			bCab = abCabInactiveQueue[bInactiveCabIndex];
			iPollTime = CAB_POLL_TIME;
			iCabBusState = CAB_BUS_POLLING;
		}
	break;
#endif

#ifdef NOT_USED
	 	case CAB_SEND_NEW_DATA:
			// send the data
			if(bByteCount--)
			{
				if(bCommandChar)
				{
					c = bCommandChar;
					bCommandChar = 0;
				}
				else
				{
					c = *pcText++;
				}
				Uart2_SendPacket(&c, 1);
			}
			else
			{
				bCab = abCabActiveQueue[bCabIndex];
				iPollTime = CAB_POLL_TIME;
				iCabBusState = CAB_BUS_POLLING;
			}
	 		break;

	 	case CAB_SEND_EXISTING_DATA:
			// send the data
			if(bByteCount--)
			{
				if(bCommandChar)
				{
					c = bCommandChar;
					bCommandChar = 0;
				}
				else
				{
					c = *pcText++;
				}
				Uart2_SendPacket(&c, 1);
			}
			else
			{
				bCab = abCabActiveQueue[bCabIndex];
				iPollTime = CAB_POLL_TIME;
				iCabBusState = CAB_BUS_POLLING;
			}
	 		break;
#endif

}

/**********************************************************************
*
* FUNCTION:		HandleCabResponse
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
void HandleCabResponse(byte bCab)
{
	byte Speed;
	byte bCabType;
	short nEvent;

	// get response data
	if(CabBus[bCab].bVersionReturn == TRUE)
	{
		// This is a new cab, get a Virtual Cab (Context)
		bCabType = CabResponse[0];
		if(bCabType != CabBus[bCab].bCabType)
		{
			CabBus[bCab].bCabType = bCabType;
			CabBus[bCab].pVirtualCab = WM_RegisterCab((int)bCab, CAB_TYPE_NCE, bCabType, (void *)&CabBus[bCab]);
		}
		CabBus[bCab].bVersionReturn = FALSE;

	}
	else if(CabResponse[0] == CAB_REFRESH_DISPLAY)
	{
		CabBus[bCab].Flags |= cBitMask[0] | cBitMask[1] | cBitMask[2] | cBitMask[3];
		NCE_DisplayChar(bCab, GET_VERSION);
		NCE_CursorOff(bCab);
		CabBus[bCab].bVersionReturn = TRUE;
	}
	else if(CabResponse[0] != CAB_NO_KEY)
	{
		// if we don't have a virtual cab at this point - get one (error condition)
		if(CabBus[bCab].pVirtualCab == 0)
		{
			CabBus[bCab].Flags |= bBitMask[0] | bBitMask[1] | bBitMask[2] | bBitMask[3];
			NCE_CursorOff(bCab);
			NCE_DisplayChar(bCab, GET_VERSION);
			CabBus[bCab].bVersionReturn = TRUE;
		}
		else
		{
			// send Cab message to Cab Handler
			nEvent = abCabKeyTranslate[CabResponse[0] - CAB_KEY_OFFSET];
			if(nEvent != -1)
			{
				CabBus[bCab].pVirtualCab->nEvent = nEvent;
				//x CabBus[bCab].nEvent = nEvent;
				//SendNCECabKey(bCab, nEvent);
				QueueMessage(MSG_CAB_KEY_MESSAGE, CabBus[bCab].pVirtualCab, nEvent);
			}
		}
	}

	// Send the speed, if any, and if changed, to the foreground
	Speed = CabResponse[1];

	if(Speed > (CabBus[bCab].OldSpeed + CabBus[bCab].UpLimit) || Speed < (CabBus[bCab].OldSpeed - CabBus[bCab].DownLimit))
	{
		if(Speed > CabBus[bCab].OldSpeed)
		{
			CabBus[bCab].UpLimit = 0;
			CabBus[bCab].DownLimit = 1;
		}
		else
		{
			CabBus[bCab].UpLimit = 1;
			CabBus[bCab].DownLimit = 0;
		}
		CabBus[bCab].OldSpeed = Speed;

		// if we don't have a virtual cab at this point - get one (error condition)
		if(CabBus[bCab].pVirtualCab == 0)
		{
			CabBus[bCab].Flags |= bBitMask[0] | bBitMask[1] | bBitMask[2] | bBitMask[3];
			NCE_CursorOff(bCab);
			NCE_DisplayChar(bCab, GET_VERSION);
			CabBus[bCab].bVersionReturn = TRUE;
		}
		else
		{
			// send Cab message to Cab Handler
			nEvent = Speed | EVENT_SPEED_TYPE;
			CabBus[bCab].pVirtualCab->nEvent = nEvent;
			//x CabBus[bCab].nEvent = nEvent;
			//QueueMessage(MSG_CAB_SPEED_MESSAGE, CabBus[bCab].pVirtualCab, nEvent);
			QueueMessage(MSG_CAB_KEY_MESSAGE, CabBus[bCab].pVirtualCab, nEvent);
		}
	}

//	abCabTimeout[bCab] = CAB_TIMEOUT;
}


void RxCabResponse(byte* Response, byte* ResponseLength)
{

	CabResponse[0] = Response[0];
	CabResponse[1] = Response[1];
	GotCabResponse = 1;
}


void TxCabResponse(void)
{
	TxComplete = TRUE;
}

