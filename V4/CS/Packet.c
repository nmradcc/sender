/**********************************************************************
*
* SOURCE FILENAME:	Packet.c
*
* DATE CREATED:		3/27/00
*
* PROGRAMMER:		Karl Kobel
*
* DESCRIPTION:  	
*
* COPYRIGHT (c) 2000-2002 by K2 Engineering. All Rights Reserved.
*
**********************************************************************
*
* VERSION CONTROL:
*
* $Revision: $
*
* $Log: $
*
**********************************************************************/
#include "Main.h"
#include "loco.h"
#include "Packet.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define BYTE0(Var)   (*((uint8_t*)&Var + 0))
#define BYTE1(Var)   (*((uint8_t*)&Var + 1))
#define BYTE2(Var)   (*((uint8_t*)&Var + 2))
#define BYTE3(Var)   (*((uint8_t*)&Var + 3))
#define BYTE4(Var)   (*((uint8_t*)&Var + 4))
#define BYTE5(Var)   (*((uint8_t*)&Var + 5))
#define BYTE6(Var)   (*((uint8_t*)&Var + 6))
#define BYTE7(Var)   (*((uint8_t*)&Var + 7))

/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

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

/**********************************************************************
*
*							CODE
*
**********************************************************************/


/**********************************************************************
*
* FUNCTION:	   	BuildAddress
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
unsigned char* BuildAddress(unsigned char* pPacket, int nAddress, unsigned char* bChecksum, unsigned char* bLength)
{
	unsigned char bTemp;
	
	if(nAddress > 127)
	{
		bTemp = BYTE1(nAddress) | 0xc0;
		*bChecksum = bTemp;
		*pPacket++ = bTemp;
		
		*pPacket++ = BYTE0(nAddress);
		*bChecksum = *bChecksum ^ BYTE0(nAddress);
		*bLength = 2;
	}
	else
	{
		*bChecksum = (unsigned char)(nAddress);
		*pPacket++ = (unsigned char)(nAddress);
		*bLength = 1;
	}
	return pPacket;
}	


/**********************************************************************
*
* FUNCTION:	   	BuildLocoPacket
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
const unsigned char Speed28Table[] =
{
	0x00,		// 0
	0x02,		// 1
	0x12,		// 2
	0x03,		// 3
	0x13,		// 4
	0x04,		// 5
	0x14,		// 6
	0x05,		// 7
	0x15,		// 8
	0x06,		// 9
	0x16,		// 10
	0x07,		// 11
	0x17,		// 12
	0x08,		// 13
	0x18,		// 14
	0x09,		// 15
	0x19,		// 16
	0x0a,		// 17
	0x1a,		// 18
	0x0b,		// 19
	0x1b,		// 20
	0x0c,		// 21
	0x1c,		// 22
	0x0d,		// 23
	0x1d,		// 24
	0x0e,		// 25
	0x1e,		// 26
	0x0f,		// 27
	0x1f,		// 28
};

void BuildLocoPacket(unsigned char* pPacket, int nAddress, unsigned short nSpeed, unsigned char fDirection, int SpeedSteps)
{
	unsigned char bChecksum;
	unsigned char bTemp;
	unsigned char bLength;
	unsigned char* pTemp;
	
	pTemp = pPacket++;				// leave room for the length byte
	
	pPacket = BuildAddress(pPacket, nAddress, &bChecksum, &bLength);
	
	if(SpeedSteps == SPEED_MODE_128 || SpeedSteps == SPEED_MODE_128_PERCENT)
	{
		*pPacket++ = 0x3f;
		bChecksum = bChecksum ^ 0x3f;
		
		bTemp = nSpeed / 2;
		if(nSpeed == ESTOP)
		{
			bTemp = 1;
		}
		else
		{
			bTemp += 1;
		}
		
		if(!fDirection)
		{
			bTemp |= 0x80;
		}
		bChecksum = bChecksum ^ bTemp;
		*pPacket++ = bTemp;
		bLength += 2;
	}
	else
	{
		if(SpeedSteps == SPEED_MODE_14 || SpeedSteps == SPEED_MODE_14_PERCENT)
		{
			if(nSpeed == ESTOP)
			{
				bTemp = 1;
			}
			else
			{
				bTemp = nSpeed / 18;
				if(bTemp != 0)
				{
					bTemp += 1;
				}
			}
		}
		else
		{
			if(nSpeed == ESTOP)
			{
				bTemp = 1;
			}
			else
			{
				nSpeed /= 9;
				bTemp = Speed28Table[nSpeed];
			}
		}
	
		if(!fDirection)
		{
			bTemp |= 0x20;
		}
		bTemp |= 0x40;
		bChecksum = bChecksum ^ bTemp;
		*pPacket++ = bTemp;
		bLength += 1;
	}
	
	*pPacket++ = bChecksum;
	*pPacket = '\0';
	*pTemp = bLength + 1;
}


/**********************************************************************
*
* FUNCTION:	   	BuildLocoEStopPacket
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
void BuildLocoEStopPacket(unsigned char* pPacket, int nAddress, unsigned char fDirection, int SpeedSteps)
{
	unsigned char bChecksum;
	unsigned char bTemp;
	unsigned char bLength;
	unsigned char* pTemp;
	
	pTemp = pPacket++;				// leave room for the length byte
	
	pPacket = BuildAddress(pPacket, nAddress, &bChecksum, &bLength);
	
	if(SpeedSteps == SPEED_MODE_128 || SpeedSteps == SPEED_MODE_128_PERCENT)
	{
		*pPacket++ = 0x3f;
		bChecksum = bChecksum ^ 0x3f;
		
		bTemp = 1;
		if(!fDirection)
		{
			bTemp |= 0x80;
		}
		bChecksum = bChecksum ^ bTemp;
		*pPacket++ = bTemp;
		bLength += 2;
	}
	else
	{
		bTemp = 1;
		if(!fDirection)
		{
			bTemp |= 0x20;
		}
		bTemp |= 0x40;
		bChecksum = bChecksum ^ bTemp;
		*pPacket++ = bTemp;
		bLength += 1;
	}

	*pPacket++ = bChecksum;
	*pPacket = '\0';
	*pTemp = bLength + 1;
}





/**********************************************************************
*
* FUNCTION:	   	BuildFunction1Packet
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
void BuildFunction1Packet(unsigned char* pPacket, int nAddress, unsigned int nFunctions)
{
	unsigned char bChecksum;
	unsigned char bTemp;
	unsigned char bLength;
	unsigned char* pTemp;
	
	pTemp = pPacket++;				// leave room for the length byte
	
	pPacket = BuildAddress(pPacket, nAddress, &bChecksum, &bLength);
	
	// build function instruction
	bTemp = ((nFunctions >> 1) & 0x1F) | 0x80;
	if(nFunctions & 0x01)
	{
		bTemp |= 0x10;
	}
	*pPacket++ = bTemp;
	bChecksum = bChecksum ^ bTemp;
	
	*pPacket++ = bChecksum;
	*pPacket = '\0';
	*pTemp = bLength + 2;
}


/**********************************************************************
*
* FUNCTION:	   	BuildFunction2Packet
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
void BuildFunction2Packet(unsigned char* pPacket, int nAddress, unsigned int nFunctions)
{
	unsigned char bChecksum;
	unsigned char bTemp;
	unsigned char bLength;
	unsigned char* pTemp;
	
	pTemp = pPacket++;				// leave room for the length byte
	
	pPacket = BuildAddress(pPacket, nAddress, &bChecksum, &bLength);
	
	// send function instruction
	bTemp = ((nFunctions / 0x20) & 0x1F) | 0xa0;
	*pPacket++ = bTemp;
	bChecksum = bChecksum ^ bTemp;
	
	*pPacket++ = bChecksum;
	*pPacket = '\0';
	*pTemp = bLength + 2;
}



// Fcn	13-20			110 1 1110	2
/**********************************************************************
*
* FUNCTION:	   	BuildFunction3Packet
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
void BuildFunction3Packet(unsigned char* pPacket, int nAddress, unsigned int nFunctions)
{
	unsigned char bChecksum;
	unsigned char bTemp;
	unsigned char bLength;
	unsigned char* pTemp;

	pTemp = pPacket++;				// leave room for the length byte

	pPacket = BuildAddress(pPacket, nAddress, &bChecksum, &bLength);

	// send function instruction
	bTemp = FUTURE_EXPANSION | 0x1E;
	*pPacket++ = bTemp;
	bChecksum = bChecksum ^ bTemp;

	bTemp = ((nFunctions / 0x100) & 0xFF);
	*pPacket++ = bTemp;
	bChecksum = bChecksum ^ bTemp;

	*pPacket++ = bChecksum;
	*pPacket = '\0';
	*pTemp = bLength + 2;
}

// Fcn	21-28			110 11111	2
/**********************************************************************
*
* FUNCTION:	   	BuildFunction4Packet
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
void BuildFunction4Packet(unsigned char* pPacket, int nAddress, unsigned int nFunctions)
{
	unsigned char bChecksum;
	unsigned char bTemp;
	unsigned char bLength;
	unsigned char* pTemp;

	pTemp = pPacket++;				// leave room for the length byte

	pPacket = BuildAddress(pPacket, nAddress, &bChecksum, &bLength);

	// send function instruction
	bTemp = FUTURE_EXPANSION | 0x1F;
	*pPacket++ = bTemp;
	bChecksum = bChecksum ^ bTemp;

	bTemp = ((nFunctions / 0x1000) & 0xFF);
	*pPacket++ = bTemp;
	bChecksum = bChecksum ^ bTemp;

	*pPacket++ = bChecksum;
	*pPacket = '\0';
	*pTemp = bLength + 2;
}


/**********************************************************************
*
* FUNCTION:	   	BuildAccessoryPacket
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
void BuildAccessoryPacket(unsigned char* pPacket, int nAddress, unsigned char fState)
{
	unsigned char bChecksum;
	unsigned char bTemp;
	unsigned char bLowAddress;
	unsigned char bHighAddress;
	unsigned char Accessory;
	unsigned char* pTemp;
	
	pTemp = pPacket++;				// leave room for the length byte
	
	//	nAddress += 3;
	bTemp = (nAddress - 1) / 4;
	bTemp++;
	
	bLowAddress = (bTemp  & 0x3f) | 0x80;
	*pPacket++ = bLowAddress;
	bChecksum = bLowAddress;
	
	nAddress--;
	
	bHighAddress = (~BYTE1(nAddress)) & 0x07;
	bHighAddress = (~(nAddress / 0x100)) & 0x07;
	Accessory = nAddress & 0x03;
	bTemp = (bHighAddress * 0x10) | (Accessory * 0x02) | 0x88;
	
	if(fState)
	{
		bTemp |= 1;
	}
	
	*pPacket++ = bTemp;
	bChecksum = bChecksum ^ bTemp;
	
	*pPacket++ = bChecksum;
	*pPacket = '\0';
	*pTemp = 3;
}


/**********************************************************************
*
* FUNCTION:	   	BuildOpsWriteCVPacket
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
void BuildOpsWriteCVPacket(unsigned char* pPacket, int nAddress, unsigned short nCV, unsigned char bValue)
{
	unsigned char bChecksum;
	unsigned char bTemp;
	unsigned char bLength;
	unsigned char* pTemp;
	
	pTemp = pPacket++;				// leave room for the length byte
	pPacket = BuildAddress(pPacket, nAddress, &bChecksum, &bLength);
	
	if(nCV != 0)
	{
		nCV--;
	}
	
	bTemp = (BYTE1(nCV) & 0x03) | 0xec;
	*pPacket++ = bTemp;
	bChecksum = bChecksum ^ bTemp;
	
	bTemp = BYTE0(nCV);
	*pPacket++ = bTemp;
	bChecksum = bChecksum ^ bTemp;
	
	*pPacket++ = bValue;
	bChecksum = bChecksum ^ bValue;
	
	*pPacket++ = bChecksum;
	*pPacket = '\0';
	*pTemp = bLength + 4;

}


/**********************************************************************
*
* FUNCTION:	   	BuildSetPagePacket / BuildPresetPagePacket
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
void BuildPresetPagePacket(unsigned char* pPacket)
{
	*pPacket++ = 0x03;
	*pPacket++ = 0xfd;
	*pPacket++ = 0x01;
	*pPacket++ = 0x7c;
	*pPacket = 0x00;
}

void BuildSetPagePacket(unsigned char* pPacket, unsigned char page)
{

	BuildWriteCVPacket(pPacket, 6, page, MODE_DIRECT);
}


/**********************************************************************
*
* FUNCTION:	   	BuildWriteCVPacket
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
void BuildWriteCVPacket(unsigned char* pPacket, unsigned short nCV, unsigned char bValue, unsigned char Mode)
{
	unsigned char bChecksum = 0;
	unsigned char bTemp;
	unsigned char bLength;
	unsigned char* pTemp;

	bLength = 0;

	if(Mode == MODE_DIRECT)
	{
		pTemp = pPacket++;				// leave room for the length byte

		if(nCV != 0)
		{
			nCV--;
		}
		
		bTemp = (BYTE1(nCV) & 0x03) | 0x7c;
		*pPacket++ = bTemp;
		bChecksum = bChecksum ^ bTemp;
		
		bTemp = BYTE0(nCV);
		*pPacket++ = bTemp;
		bChecksum = bChecksum ^ bTemp;
		
		*pPacket++ = bValue;
		bChecksum = bChecksum ^ bValue;
		
		*pPacket++ = bChecksum;
		*pPacket = '\0';
		*pTemp = bLength + 4;
	}
	else
	{
		pTemp = pPacket++;				// leave room for the length byte
		
		if(nCV != 0)
		{
			nCV--;
		}

		bTemp = ((nCV % 4) & 0x07) | 0x78;
		*pPacket++ = bTemp;
		bChecksum = bChecksum ^ bTemp;
		
		*pPacket++ = bValue;
		bChecksum = bChecksum ^ bValue;
		
		*pPacket++ = bChecksum;
		*pPacket = '\0';
		*pTemp = bLength + 4;
	}
}


/**********************************************************************
*
* FUNCTION:	   	BuildVerifyCVPacket
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
void BuildVerifyCVPacket(unsigned char* pPacket, unsigned short nCV, unsigned char bValue, unsigned char Mode)
{
	unsigned char bChecksum = 0;
	unsigned char bTemp;
	unsigned char bLength;
	unsigned char* pTemp;

	bLength = 0;

	if(Mode == MODE_DIRECT)
	{
		pTemp = pPacket++;				// leave room for the length byte

		if(nCV != 0)
		{
			nCV--;
		}

		bTemp = (BYTE1(nCV) & 0x03) | 0x74;
		*pPacket++ = bTemp;
		bChecksum = bChecksum ^ bTemp;

		bTemp = BYTE0(nCV);
		*pPacket++ = bTemp;
		bChecksum = bChecksum ^ bTemp;

		*pPacket++ = bValue;
		bChecksum = bChecksum ^ bValue;

		*pPacket++ = bChecksum;
		*pPacket = '\0';
		*pTemp = bLength + 4;
	}
	else
	{
		pTemp = pPacket++;				// leave room for the length byte

		if(nCV != 0)
		{
			nCV--;
		}

		bTemp = ((nCV % 4) & 0x07) | 0x78;
		*pPacket++ = bTemp;
		bChecksum = bChecksum ^ bTemp;

		//*pPacket++ = bValue;
		//bChecksum = bChecksum ^ bValue;

		*pPacket++ = bChecksum;
		*pPacket = '\0';
		*pTemp = bLength + 4;
	}
}


/**********************************************************************
*
* FUNCTION:	   	BuildWriteBitPacket
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
void BuildWriteBitPacket(unsigned char* pPacket, unsigned short nCV, unsigned char bBit, unsigned char bValue)
{
	unsigned char bChecksum = 0;
	unsigned char bTemp;
	unsigned char bLength;
	unsigned char* pTemp;

	bLength = 0;

	pTemp = pPacket++;				// leave room for the length byte

	if(nCV != 0)
	{
		nCV--;
	}

	bTemp = (BYTE1(nCV) & 0x03) | 0x78;
	*pPacket++ = bTemp;
	bChecksum = bChecksum ^ bTemp;

	bTemp = BYTE0(nCV);
	*pPacket++ = bTemp;
	bChecksum = bChecksum ^ bTemp;

	// 111KDBBB 0 EEEEEEEE 1
	// Where BBB represents the bit position within the CV (000 being defined as bit 0), and D contains the value of the
	// bit to be verified or written, K=1 signifies a "Write Bit" operation and K=0 signifies a "Bit Verify" operation

	bTemp = 0xF0 | ((bValue & 0x01) << 4) | bBit;
	*pPacket++ = bTemp;
	bChecksum = bChecksum ^ bTemp;

	*pPacket++ = bChecksum;
	*pPacket = '\0';
	*pTemp = bLength + 4;
}


/**********************************************************************
*
* FUNCTION:	   	BuildVerifyBitPacket
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
void BuildVerifyBitPacket(unsigned char* pPacket, unsigned short nCV, unsigned char bBit, unsigned char bValue)
{
	unsigned char bChecksum = 0;
	unsigned char bTemp;
	unsigned char bLength;
	unsigned char* pTemp;

	bLength = 0;

	pTemp = pPacket++;				// leave room for the length byte

	if(nCV != 0)
	{
		nCV--;
	}

	bTemp = (BYTE1(nCV) & 0x03) | 0x78;
	*pPacket++ = bTemp;
	bChecksum = bChecksum ^ bTemp;

	bTemp = BYTE0(nCV);
	*pPacket++ = bTemp;
	bChecksum = bChecksum ^ bTemp;

	// 111KDBBB 0 EEEEEEEE 1
	// Where BBB represents the bit position within the CV (000 being defined as bit 0), and D contains the value of the
	// bit to be verified or written, K=1 signifies a "Write Bit" operation and K=0 signifies a "Bit Verify" operation

	bTemp = 0xE0 | ((bValue & 0x01) << 4) | bBit;
	*pPacket++ = bTemp;
	bChecksum = bChecksum ^ bTemp;

	*pPacket++ = bChecksum;
	*pPacket = '\0';
	*pTemp = bLength + 4;
}




// Binary State LF		110 00000	3
// Binary State SF		110 11101	2
// CV Access Verify		111001vv	3
// CV Access Write		111011vv	3
// CV Access Bit		111010vv	3



/**********************************************************************
*
* FUNCTION:	   	BuildResetPacket
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
void BuildResetPacket(unsigned char* pPacket)
{

	*pPacket++ = 0x03;
	*pPacket++ = 0x00;
	*pPacket++ = 0x00;
	*pPacket++ = 0x00;
	*pPacket = 0x00;
}

/**********************************************************************
*
* FUNCTION:	   	BuildIdlePacket
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
//kvoid BuildIdlePacket(unsigned char* pPacket)
//k{
//k
//k	*pPacket++ = 0x03;
//k	*pPacket++ = 0xff;
//k	*pPacket++ = 0x00;
//k	*pPacket++ = 0xff;
//k	*pPacket = 0x00;
//k}


