/*********************************************************************
*
* SOURCE FILENAME:	PacketQueue.h
*
* DATE CREATED:		4-MAY-92
*
* PROGRAMMER:		K Kobel
*
* DESCRIPTION:  	Header file for the Packet Queue
*
* COPYRIGHT (c) 2019 by K2 Engineering, Inc.  All Rights Reserved.
*
*********************************************************************/
#ifndef _PACKET_QUEUE_H
#define _PACKET_QUEUE_H


/*********************************************************************
*
*							DEFINITIONS
*
*********************************************************************/


/*********************************************************************
*
*							FUNCTION PROTOTYPES
*
*********************************************************************/

extern void InitProgPacketQueue(void);

extern unsigned char QueueProgPacket(unsigned char count, unsigned char* pPacket);

extern unsigned char GetProgPacket(unsigned char* count, unsigned char* pPacket);

#endif
