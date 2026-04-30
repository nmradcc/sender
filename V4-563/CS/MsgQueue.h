/*********************************************************************
*
* SOURCE FILENAME:	QUEUE.H
*
* DATE CREATED:		4-MAY-92
*
* PROGRAMMER:		K Kobel
*
* DESCRIPTION:  	Header file for the Queues
*
* COPYRIGHT (c) 1999 by Wangrow Electronics, Inc.  All Rights Reserved.
*
*********************************************************************/
#ifndef _MSG_QUEUE_H
#define _MSG_QUEUE_H

#include "cab.h"

/*********************************************************************
*
*							DEFINITIONS
*
*********************************************************************/

// Message types
#define MSG_CAB_KEY_MESSAGE		0x01
#define MSG_CAB_SPEED_MESSAGE		0x02


typedef struct
{
	unsigned char	bMessageType;
	int				nEvent;
	VIRTUAL_CAB*	pMessagePointer;
} EVENT_MESSAGE;


/*********************************************************************
*
*							FUNCTION PROTOTYPES
*
*********************************************************************/

extern unsigned char QueueMessage(unsigned char bMessageType, VIRTUAL_CAB* pMsgPointer, int nEvent);
extern VIRTUAL_CAB* GetMessage(unsigned char* bMessageType, int* nEvent);

extern void InitMessageQueue(void);

#endif


