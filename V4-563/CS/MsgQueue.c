/*********************************************************************
*
* SOURCE FILENAME:	QUEUE.C
*
* DATE CREATED:		
*
* PROGRAMMER:		K Kobel
*
* DESCRIPTION:  	Queue Management
*
*
* COPYRIGHT (c) 2003 by K2 Engineering, Inc.  All Rights Reserved.
*
*********************************************************************/
#include "Main.h"
#include <string.h>
#include <stdio.h>
#include "MsgQueue.h"


/*********************************************************************
*
*							DEFINITIONS
*
*********************************************************************/

#define MESSAGE_QUEUE_DEPTH 20

//#pragma section NV_RAM
EVENT_MESSAGE MessageQueue[MESSAGE_QUEUE_DEPTH];	// Queue
unsigned char message_queue_in;		   				// queue input index
unsigned char message_queue_out;	   				// queue output index
//#pragma section

/*********************************************************************
*
*							FUNCTION PROTOTYPES
*
*********************************************************************/


/*********************************************************************
*
*							CODE
*
*********************************************************************/


/************************************************************************
* QueueMessage
*
*  parameters:	
*
*  returns:	TRUE if sucessfull
*		FALSE if queue is full
*
*  description:	queues up the event pointer (string_ptr)
*
*	psuedo code:
*		if in_ptr = out_ptr & q_ptr(in_ptr) <> 0
*			return queue full
*		if high priority
*			move down other pionters in queue
*			index = out_ptr
*		else
*			index = in_ptr
*		q_ptr(in_ptr) = messege_to_be_queued
*		inc in_ptr (accounting for wrap)
*
*************************************************************************/
unsigned char QueueMessage(unsigned char bMessageType, VIRTUAL_CAB* pMessagePointer, int nEvent)
{

	if((message_queue_in == message_queue_out) && (MessageQueue[message_queue_in].bMessageType != '\0'))
	{
		/* queue is full */
		return 0;
	}
	else
	{
		/* queue is not full */
		MessageQueue[message_queue_in].pMessagePointer = pMessagePointer;
		MessageQueue[message_queue_in].bMessageType = bMessageType;
		MessageQueue[message_queue_in].nEvent = nEvent;

		++message_queue_in;
		if(message_queue_in == MESSAGE_QUEUE_DEPTH)
		{
			message_queue_in = 0;
		}

		return 1;
	}
}


/*********************************************************************
*
* FUNCTION:		GetMessage
*
* ARGUMENTS:	None
*
* RETURNS:		Event or 0
*
* DESCRIPTION:	get the next message from the queue
*				does not take the message off the queue
*
*	psuedo code:
*		if in_ptr = out_ptr & queue(out_ptr) = 0
*			return queue empty
*		else
*			save queue(out_ptr)
*			zero queue(out_ptr)
*			inc out_ptr (accounting for wrap)
*			return saved pointer
*
*
* RESTRICTIONS:	*
*
*********************************************************************/
VIRTUAL_CAB* GetMessage(unsigned char* bMessageType, int* nEvent)
{
	VIRTUAL_CAB* pMessagePointer;

	if((message_queue_in == message_queue_out) && (MessageQueue[message_queue_out].bMessageType == '\0'))
	{
		/* queue is empty */
		return NULL;
	}
	else
	{
		/* queue is not empty */
		pMessagePointer = MessageQueue[message_queue_out].pMessagePointer;
		*bMessageType = MessageQueue[message_queue_out].bMessageType;
		*nEvent = MessageQueue[message_queue_out].nEvent;
		MessageQueue[message_queue_out].bMessageType = '\0';

		++message_queue_out;
		if(message_queue_out == MESSAGE_QUEUE_DEPTH)
		{
			message_queue_out = 0;
		}
		return pMessagePointer;
	}
}


/*********************************************************************
*
* FUNCTION:		InitMessageQueue
*
* ARGUMENTS:	none
*
* RETURNS:		none
*
* GLOBAL VARS REFERENCED:	none
*
* GLOBAL VARS MODIFIED:		none
*
* DESCRIPTION:	initialize the rt queue
*
* RESTRICTIONS:	none
*
*********************************************************************/
void InitMessageQueue(void)
{
	int index;


	message_queue_in = 0;
	message_queue_out = 0;

	for(index = 0; index < MESSAGE_QUEUE_DEPTH; ++index)
	{
		MessageQueue[index].bMessageType = 0;
	}
}

