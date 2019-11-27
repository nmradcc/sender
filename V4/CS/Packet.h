
 /**********************************************************************
*
* SOURCE FILENAME:	Packet.h
*
* DATE CREATED:		29/Aug/99
*
* PROGRAMMER:
*
* DESCRIPTION:
*
* COPYRIGHT (c) 1999-2019 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/

#ifndef PACKET_H
#define PACKET_H

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

enum
{
	MODE_DIRECT,
	MODE_PAGED,
};


#define FUTURE_EXPANSION	0xC0

/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

void BuildLocoPacket(unsigned char* pPacket, int nAddress, unsigned short nSpeed, unsigned char fDirection, int SpeedSteps);

void BuildFunction1Packet(unsigned char* pPacket, int nAddress, unsigned int nFunctions);
void BuildFunction2Packet(unsigned char* pPacket, int nAddress, unsigned int nFunctions);
void BuildFunction3Packet(unsigned char* pPacket, int nAddress, unsigned int nFunctions);
void BuildFunction4Packet(unsigned char* pPacket, int nAddress, unsigned int nFunctions);

void BuildAccessoryPacket(unsigned char* pPacket, int nAddress, unsigned char fState);

void BuildOpsWriteCVPacket(unsigned char* pPacket, int nAddress, unsigned short nCV, unsigned char bValue);

void BuildWriteCVPacket(unsigned char* pPacket, unsigned short nCV, unsigned char bValue, unsigned char Mode);

void BuildVerifyCVPacket(unsigned char* pPacket, unsigned short nCV, unsigned char bValue, unsigned char Mode);

void BuildPresetPagePacket(unsigned char* pPacket);

void BuildSetPagePacket(unsigned char* pPacket, unsigned char page);

void BuildResetPacket(unsigned char* pPacket);

void BuildIdlePacket(unsigned char* pPacket);

#endif
