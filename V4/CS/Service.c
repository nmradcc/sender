/**********************************************************************
*
* SOURCE FILENAME:	Service.c
*
* DATE CREATED:		4/24/19
*
* PROGRAMMER:
*
* DESCRIPTION:
*
* COPYRIGHT (c) 1999-2019 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/
//#include <stdio.h>
//#include <string.h>
#include "Main.h"
//#include "GPIO.h"
#include "Track.h"
//#include "Loco.h"
//#include "TrakList.h"
#include "Packet.h"
#include "CV.h"
#include "PacketQueue.h"
#include "Acknowledge.h"


/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

enum
{
	SM_IDLE,
	SM_WAIT_FOR_ACK,
	SM_WRITE,
	SM_VERIFY,
};


/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

//void AckStart(void);
//int GetAck(void);

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

static int SmState = SM_IDLE;
//static int SmNextState = SM_IDLE;
static unsigned short SmCV;
static unsigned char SmValue;

/**********************************************************************
*
*							CODE
*
**********************************************************************/

/**********************************************************************
*
* FUNCTION:		ServiceMode
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:	Run the service mode state machine
*
* RESTRICTIONS:
*
**********************************************************************/

enum
{
	PT_NO_MESSAGE,
	PT_ALL_PACKETS_SENT,
	PT_ACK,
};


void ServiceMode(void)
{
	unsigned char packet[8];
	int status;
	ACK_STATUS ack_status;

	// get the ACK status, overcurrent status, and the state of the Prog Track state machine and set status accordingly
	//if()
	//status = GetProgTrackStatus();

	status = PT_NO_MESSAGE;



	switch(SmState)
	{
		case SM_IDLE:
		break;

		case SM_WAIT_FOR_ACK:
			if(status == PT_ALL_PACKETS_SENT)
			{
				// EQ = error
				SmState = SM_IDLE;
			}
			else if(status == PT_ACK)
			{
				// EQ =  operation OK
				//SmState = SmNextState;
				SmState = SM_IDLE;
			}
		break;

		case SM_WRITE:


		break;

		case SM_VERIFY:
			BuildWriteCVPacket(packet, SmCV, SmValue, MODE_DIRECT);
//k			if(QueueProgPacket(11, packet) == 0)
//k			{
//k				// error
//k			}
			SmState = SM_WAIT_FOR_ACK;
		break;
	}
}


/**********************************************************************
*
* FUNCTION:		WaitForAck
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:	Run the service mode state machine
*
* RESTRICTIONS:
*
**********************************************************************/
void WaitForAck(void)
{
	while(SmState != SM_IDLE)
	{
		;
	}
}


#ifdef NOT_USED
void PushProgPacket(byte* pPacket, byte n)
{
}


/**********************************************************************
*
* FUNCTION:		EnterServiceMode
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:	Run the service mode state machine
*
* RESTRICTIONS:
*
**********************************************************************/
void EnterServiceMode(void)
{
	byte Packet[8];

	BuildResetPacket(Packet);
	PushProgPacket(Packet, 3);

	// force this to direct mode
	BuildVerifyCVPacket(Packet, cvManufacturer, 0, MODE_DIRECT);	// ToDo - is thia waht we want to do?
	PushProgPacket(Packet, 0);

	SmState = SM_VERIFY;
//	BitMask = 0x80;
//	SmNextState = UNKNOWN;
}
#endif

/**********************************************************************
*
* FUNCTION:		ServiceModeWriteCV
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:	Run the service mode state machine
*
* RESTRICTIONS:
*
**********************************************************************/
void ServiceModeWriteCV(unsigned short nCV, unsigned char bValue)
{

	SmCV = nCV;
	SmValue = bValue;

	SmState = SM_WRITE;
}


/**********************************************************************
*
* FUNCTION:		ServiceModeWriteCV
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:	Run the service mode state machine
*
* RESTRICTIONS:
*
**********************************************************************/
ACK_STATUS ServiceModeReadCV(unsigned short nCV, unsigned char bValue)
{

	SmCV = nCV;
//	SmValue = bValue;

	SmState = SM_VERIFY;

	WaitForAck();
	return GetAck();

}


