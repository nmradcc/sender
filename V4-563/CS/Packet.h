
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

int BuildLocoPacket(unsigned char* pPacket, int nAddress, unsigned short nSpeed, unsigned char fDirection, int SpeedSteps);

int BuildFunction1Packet(unsigned char* pPacket, int nAddress, unsigned int nFunctions);
int BuildFunction2Packet(unsigned char* pPacket, int nAddress, unsigned int nFunctions);
int BuildFunction3Packet(unsigned char* pPacket, int nAddress, unsigned int nFunctions);
int BuildFunction4Packet(unsigned char* pPacket, int nAddress, unsigned int nFunctions);

int BuildAccessoryPacket(unsigned char* pPacket, int nAddress, unsigned char fState);

int BuildOpsWriteCVPacket(unsigned char* pPacket, int nAddress, unsigned short nCV, unsigned char bValue);

int BuildWriteCVPacket(unsigned char* pPacket, unsigned short nCV, unsigned char bValue, unsigned char Mode);

int BuildVerifyCVPacket(unsigned char* pPacket, unsigned short nCV, unsigned char bValue, unsigned char Mode);

int BuildPresetPagePacket(unsigned char* pPacket);

int BuildSetPagePacket(unsigned char* pPacket, unsigned char page);

int BuildResetPacket(unsigned char* pPacket);

int BuildIdlePacket(unsigned char* pPacket);

#endif
