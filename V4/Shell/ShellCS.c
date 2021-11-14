/*******************************************************************************
* @file ShellScript.c
* @brief The command station commands for the shell. The command table is in Shell.c
*
* @author K. Kobel
* @date 9/15/2019
* @Revision: 24 $
* @Modtime: 10/31/2019
*
* @copyright	(c) 2019  all Rights Reserved.
*******************************************************************************/
#include "main.h"
#include <string.h>
#include <stdlib.h>
#include "Track.h"
#include "CabNCE.h"
#include "Shell.h"
#include "Loco.h"
#include "Text.h"
//#include "TrackProg.h"
#include "Service.h"
#include "GetLine.h"
#include "Bitmask.h"

//*******************************************************************************
// Definitions
//*******************************************************************************

//*******************************************************************************
// Static Variables
//*******************************************************************************

static PACKET_BITS apShellPacket[80];

//*******************************************************************************
// Global Variables
//*******************************************************************************

extern Loco ActiveLocos[];

//*******************************************************************************
// Function prototypes
//*******************************************************************************

//*******************************************************************************
// Source
//*******************************************************************************

/*********************************************************************
*
* ShCabStat
* @catagory	Shell Command
*
* @brief	Wangrow Cab status
*
* @param	bPort - port that issued this command
*			argc - argument country
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShCabStat(uint8_t bPort, int argc, char *argv[])
{
	int i;
	//int j;
	//int index;
	//char CabBuf[64];
	char LabelBuf[64];
	//int cab;

	if(argc == 2)
	{
		//cab = atoi(argv[1]);


	}
	else
	{
		ShNL(bPort);
		strcpy(LabelBuf, "Active Cabs:");
		ShFieldOut(bPort, LabelBuf, 16);
		for(i = 0; i < MAX_CABS; i++)
		{
			if(abCabActiveQueue[i])
			{
				//for(j = 0; j < 8; j++)
				//{
				//	if(abCabActiveQueue[(i * 8) + j])
				//	{
						ShFieldNumberOut(bPort, "", abCabActiveQueue[i], 3);
				//	}
				//}
				//ShNL(bPort);
				strcpy(LabelBuf, "");
			}
			else
			{
				break;
			}
		}
		ShNL(bPort);

		strcpy(LabelBuf, "Inactive Cabs:");
		ShFieldOut(bPort, LabelBuf, 16);
		for(i = 0; i < MAX_CABS; i++)
		{
			//index = i * 8;
			if(abCabInactiveQueue[i])
			{
				//for(j = 0; j < 8; j++)
				//{
				//	if(abCabInactiveQueue[(i * 8) + j])
				//	{
						ShFieldNumberOut(bPort, "", abCabInactiveQueue[i], 3);
				//	}
				//}
				//ShNL(bPort);
				strcpy(LabelBuf, "");
			}
			else
			{
				break;
			}
		}
	}
	return CMD_OK;
}


//Loco* FindAlias(unsigned int nAddress);
//Loco* FindLeadLoco(unsigned int nAddress);

//word GetLocoFunctionOverride(Loco* pLoco);
//word GetLocoAlias(Loco* pLoco);
//BOOL GetLocoAliasDirection(Loco* pLoco);
//BOOL IsLeadLoco(Loco* pLoco);
//word GetLocoAddress(Loco* pLoco);
//word GetLocoTrainNumber(Loco* pLoco);
//word GetLocoLocomotiveNumber(Loco* pLoco);
//word GetLocoTrainTons(Loco* pLoco);
//char* GetLocoTrainName(Loco* pLoco);
//char* GetLocoSpeedLabel(Loco* pLoco);

/*********************************************************************
*
* ShLocoStat
* @catagory	Shell Command
*
* @brief	Command Station Locomotive status 
*
* @details	Loco - list all active locos
*			Loco <number> - details on the specific loco
*
* @param	bPort - port that issued this command
*			argc - argument country
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
#define SPEED_PRIME_NUMBER	252
//#define SPEED_DIR_MAX		36
CMD_RETURN ShLocoStat(uint8_t bPort, int argc, char *argv[])
{
	int i;
	int address;
	word temp;
	word sm;
	Loco* pLoco;

	if(argc == 2)
	{
		address = atoi(argv[1]);
		pLoco = FindLoco(address);

		if(pLoco)
		{
			ShNL(bPort);
			ShFieldOut(bPort, "Loco", 15);
			ShFieldNumberOut(bPort, "", address, 0);
			ShNL(bPort);
			//ShFieldNumberOut(bPort, "Address", ActiveLocos[nIndex].Address, 0);
			//ShNL(bPort);
			//ShFieldNumberOut(bPort, "Alias", ActiveLocos[nIndex].Alias, 0);
			//ShNL(bPort);

			temp = GetLocoSpeed(pLoco);
			sm = GetLocoSpeedMode(pLoco);
			ShFieldOut(bPort, "Speed", 15);
			//ShFieldNumberOut(bPort, "", temp, 0);

			char szTemp[10];
			if(temp == ESTOP)
			{
				strcpy(szTemp, aText[tEStp]);
			}
			else if(temp == 0)
			{
				strcpy(szTemp, aText[tStop]);
			}
			else
			{
				switch(sm)
				{
					case SPEED_MODE_14:
						sprintf(szTemp, "%d", temp / 18);
					break;

					case SPEED_MODE_28:
						sprintf(szTemp, "%d", temp / 9);
					break;

					case SPEED_MODE_128:
						sprintf(szTemp, "%d", temp / 2);
					break;

					case SPEED_MODE_14_PERCENT:
					case SPEED_MODE_28_PERCENT:
					case SPEED_MODE_128_PERCENT:
						sprintf(szTemp, "%d%%", (temp * 100) / SPEED_PRIME_NUMBER);
					break;
				}
			}
			ShFieldOut(bPort, szTemp, 0);
			ShNL(bPort);

			ShFieldOut(bPort, aText[t28SpeedMode], 14);
			ShFieldOut(bPort, aText[tSpd_28 + sm], 0);
			ShNL(bPort);

			temp = GetLocoMaxSpeed(pLoco);
			ShFieldOut(bPort, "Max Speed", 15);
			ShFieldNumberOut(bPort, "", temp, 0);
			ShNL(bPort);

			ShFieldOut(bPort, aText[tDirection], 15);
			if(GetLocoDirection(pLoco))
			{
				ShFieldOut(bPort,  "Rev", 0);
			}
			else
			{
				ShFieldOut(bPort,  "Fwd", 0);
			}
			ShNL(bPort);

			temp = GetLocoFunctions(pLoco);
			char szFunctionList[32];
			strcpy(szFunctionList, "L1BH4567890123456789012345678");

			for(i = 0; i < 29; i++)
			{
				if((temp & lBitMask[i]) == 0)
				{
					szFunctionList[i] = '-';
				}
			}
			ShFieldOut(bPort, "Function Map", 15);
			ShFieldOut(bPort, szFunctionList, 0);


			ShNL(bPort);
			//ShFieldNumberOut(bPort, "FunctionOverrideMap", ActiveLocos[nIndex].FunctionOverrideMap, 0);
			//ShNL(bPort);
			//ShFieldNumberOut(bPort, "ConsistLink", ActiveLocos[nIndex].pConsistLink, 0);
			//ShNL(bPort);
			//ShFieldNumberOut(bPort, "NumStopPackets", ActiveLocos[nIndex].bNumStopPackets, 0);
			//ShNL(bPort);
			//ShFieldNumberOut(bPort, "NumFunctionPackets", ActiveLocos[nIndex].bNumFunctionPackets, 0);
			//ShNL(bPort);
		}
	}
	else
	{
		temp = GetNumLocos();
		ShNL(bPort);
		ShFieldNumberOut(bPort, "Number of Active Locos: ", temp, 0);
	    ShNL(bPort);
	    for(i = 0; i < MAX_LOCOS; i++)
	    {
	    	if(ActiveLocos[i].Address != 0)
	    	{
	    	    ShFieldNumberOut(bPort, "Loco", ActiveLocos[i].Address, 9);
	    	    ShNL(bPort);
	    	}
	    }
	}
	return CMD_OK;
}


/*********************************************************************
*
* ShAssign
* @catagory	Shell Command
*
* @brief	Assign Loco to Cab 
*
* @param	bPort - port that issued this command
*			argc - argument country
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShAssign(uint8_t bPort, int argc, char *argv[])
{
	int nIndex;
	int i, j;
	char szTemp[16];

	if(argc == 2)
	{
		nIndex = atoi(argv[1]);

		for(i = 0; i < MAX_VIRTUAL_CABS; i++)
		{
			if(aVirtualCab[i].Cab == nIndex)
			{
				//kprintf("Loco Assigments for Cab %d:\n", nIndex);

				for(j = 0; j < MAX_RACALLS; j++)
				{
					if(aVirtualCab[i].pRecall[j] != NULL)
					{
						//kprintf("Loco: %4d ", aVirtualCab[i].pRecall[j]->Address);
						sprintf(szTemp, "Loco: %4d ", aVirtualCab[i].pRecall[j]->Address);
						ShFieldOut(bPort, szTemp, 0);
						if(aVirtualCab[i].pRecall[j]->Alias)
						{
							//kprintf("Alias: %3d ", aVirtualCab[i].pRecall[j]->Alias);
							sprintf(szTemp, "Alias: %3d ", aVirtualCab[i].pRecall[j]->Alias);
							ShFieldOut(bPort, szTemp, 0);
						}
						//kprintf("Speed: %3d", ActiveLocos[nIndex].Speed);
						//kprintf("SpeedMode: %s\n", aText[tSpd_28 + ActiveLocos[nIndex].SpeedMode]);
						//kprintf("Direction: %s", ActiveLocos[nIndex].Direction ? "Forward" : "Reverse");
						//kprintf("\n");
			    	    ShNL(bPort);
						return CMD_OK;
					}
				}
			}
		}
		//kprintf("Cab %d is Not Active\n", nIndex);
		ShFieldOut(bPort, "Cab is Not Active", 0);
		return CMD_OK;
	}
	else
	{
	    ShNL(bPort);
		ShFieldOut(bPort, "Usage: assign <cab> <address>", 0);
	    ShNL(bPort);
	}

	return CMD_OK;
}

/*********************************************************************
*
* ShSystemStatus
* @catagory	Shell Command
*
* @brief	Command Station status 
*
* @param	bPort - port that issued this command
*			argc - argument country
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShSystemStatus(uint8_t bPort, int argc, char *argv[])
{
    // command station status
    // Number of cabs
    // number of locos
    GetNumLocos();
    // number of accessories
    // voltages, ect.
	return CMD_OK;
    
}

/*********************************************************************
*
* ShSendPacket
* @catagory	Shell Command
*
* @brief	Build and send an arbitrary packet from bit descriptions in a file
*
* @details
*			File Format:
*				Preambles = nn
*				<period>, <pulse>, <count>
*				<period>, <pulse>, <count>
*				<period>, <pulse>, <count>
*				<period>, <pulse>, <count>
*				<period>, <pulse>, <count>
*				0, 0, 0
*					
*			Variants:
*				packet <file> - send one packet
*				packet <file> <count>
*				packet <count> - send the previously built packet 'count' times
*				packet - send the previously built packet
*
* @param	bPort - port that issued this command
*			argc - argument country
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
#ifdef NEW_PACKET
CMD_RETURN ShSendPacket(uint8_t bPort, int argc, char *argv[])
{
	static PACKET_BITS* pPacket = apShellPacket;
    FRESULT ret;
    char szTypeBuf[80];
	char* pBuf = szTypeBuf;
	char* pp;
    uint32_t bc;
	uint32_t preambles;
	int i;
	int period;
	int pulse;
	int count;
    FIL fp;
	
	
    ShNL(bPort);

	if(argc == 1)
	{
		if(apShellPacket[0].period != 0)
		{
			bc = atoi(argv[1]);
			BuildPacketBits(apShellPacket, bc);
		}
		else
		{
			return CMD_BAD_PACKET;
		}
	}
	else if(argc == 2 || argc == 3)
	{
		// open file
		ret = f_open(&fp, argv[1], FA_READ);
		if(ret != FR_OK)
		{
			// append '.pkt' and try again
			strcat(argv[1], ".pkt");
			ret = f_open(&fp, argv[1], FA_READ);
		}

		if(ret == FR_OK)
		{
			bc = getLine(&fp, szTypeBuf, sizeof(szTypeBuf));
			// the first line is the header and number of preambles
			if(strnicmp(szTypeBuf, "PREAMBLES", 9) == 0)
			{
				// ToDo - verify the '=' and number format
				
				preambles = atoi(&szTypeBuf[12]);
				
				for(i = 0; i < preambles; i++)
				{
					pPacket->period = ONE_PERIOD;
					pPacket->pulse = ONE_PULSE;
					pPacket++;
				}

				while(1)
				{
					bc = getLine(&fp, szTypeBuf, sizeof(szTypeBuf));
					if(bc == 0)
					{
						break;
					}
					
					pBuf = szTypeBuf;
					pp = strsep(&pBuf, ",");

					period = atoi(pp) * TICKS_PER_MICROSECOND;
					pp = strsep(&pBuf, ",");
					pulse = atoi(pp) * TICKS_PER_MICROSECOND;
					pp = strsep(&pBuf, ",");
					count = atoi(pp);
					if(count == 0)
					{
						count++;
					}

					for(i = 0; i < count; i++)
					{
						pPacket->period = period;
						pPacket->pulse = pulse;
						pPacket++;
					}
				}
				f_close(&fp);

				// add the terminator
				pPacket->period = 0;
				pPacket->pulse = 0;
				
				if(argc == 3)
				{
					bc = atoi(argv[1]);
					BuildPacketBits(apShellPacket, bc);
				}
				else
				{
					BuildPacketBits(apShellPacket, 1);
				}
			}
			else
			{
				return CMD_BAD_BIT_FILE;
			}
		}
		else
		{
			// if no file, try to convert arg to a number,
			// if less than 100 send that many packets
			bc = atoi(argv[1]);
			if(apShellPacket[0].period != 0 && bc != 0 && bc <= 100)
			{
				BuildPacketBits(apShellPacket, bc);
			}
			else
			{
				ShFieldOut(bPort, "File not found.\r\n", 0);
				// return file not found
				return CMD_NOT_FOUND;
			}
		}
	}
	else
	{
		return CMD_BAD_PARAMS;
	}
	return CMD_OK;
    
}
#else
CMD_RETURN ShSendPacketOrig(uint8_t bPort, int argc, char *argv[])
{
	static PACKET_BITS* pPacket = apShellPacket;
    FRESULT ret;
    char szTypeBuf[80];
	char* pBuf = szTypeBuf;
	char* pp;
    uint32_t bc;
	uint32_t preambles;
	int i;
	int period;
	int pulse;
	int count;
    FIL fp;


    ShNL(bPort);

	if(argc == 1)
	{
		if(apShellPacket[0].period != 0)
		{
			bc = atoi(argv[1]);
			BuildPacketBits(apShellPacket, bc);
		}
		else
		{
			return CMD_BAD_PACKET;
		}
	}
	else if(argc == 2 || argc == 3)
	{
		// open file
		ret = f_open(&fp, argv[1], FA_READ);
		if(ret != FR_OK)
		{
			// append '.pkt' and try again
			strcat(argv[1], ".pkt");
			ret = f_open(&fp, argv[1], FA_READ);
		}

		if(ret == FR_OK)
		{
			while(1)
			{
				bc = getLine(&fp, szTypeBuf, sizeof(szTypeBuf));
				if(bc == 0)
				{
					return CMD_BAD_BIT_FILE;
				}

				// the first line is the header and number of preambles
				if(strnicmp(szTypeBuf, "PREAMBLES", 9) == 0)
				{
					// ToDo - verify the '=' and number format

					preambles = atoi(&szTypeBuf[12]);

					for(i = 0; i < preambles; i++)
					{
						pPacket->period = ONE_PERIOD;
						pPacket->pulse = ONE_PULSE;
						pPacket++;
					}
					break;
				}

				while(1)
				{
					bc = getLine(&fp, szTypeBuf, sizeof(szTypeBuf));
					if(bc == 0)
					{
						break;
					}

					pBuf = szTypeBuf;
					pp = strsep(&pBuf, ",");

					period = atoi(pp) * TICKS_PER_MICROSECOND;
					pp = strsep(&pBuf, ",");
					pulse = atoi(pp) * TICKS_PER_MICROSECOND;
					pp = strsep(&pBuf, ",");
					count = atoi(pp);
					if(count == 0)
					{
						count++;
					}

					for(i = 0; i < count; i++)
					{
						pPacket->period = period;
						pPacket->pulse = pulse;
						pPacket++;
					}
				}
				f_close(&fp);

				// add the terminator
				pPacket->period = 0;
				pPacket->pulse = 0;

				if(argc == 3)
				{
					bc = atoi(argv[1]);
					BuildPacketBits(apShellPacket, bc);
				}
				else
				{
					BuildPacketBits(apShellPacket, 1);
				}
			}
		}
		else
		{
			// if no file, try to convert arg to a number,
			// if less than 100 send that many packets
			bc = atoi(argv[1]);
			if(apShellPacket[0].period != 0 && bc != 0 && bc <= 100)
			{
				BuildPacketBits(apShellPacket, bc);
			}
			else
			{
				ShFieldOut(bPort, "File not found.\r\n", 0);
				// return file not found
				return CMD_NOT_FOUND;
			}
		}
	}
	else
	{
		return CMD_BAD_PARAMS;
	}
	return CMD_OK;

}
#endif

CMD_RETURN ShSendPacket(uint8_t bPort, int argc, char *argv[])
{
	static PACKET_BITS* pPacket = apShellPacket;
    FRESULT ret;
    char szTypeBuf[80];
	char* pBuf = szTypeBuf;
	char* pp;
    uint32_t bc;
	uint32_t preambles = 16;
	uint32_t scope = 0;
	int i;
	int period;
	int pulse;
	int count;
    FIL fp;

    ShNL(bPort);

	if(argc == 1)
	{
		if(apShellPacket[0].period != 0)
		{
			bc = atoi(argv[1]);
			BuildPacketBits(apShellPacket, bc);
		}
		else
		{
			return CMD_BAD_PACKET;
		}
	}
	else if(argc == 2 || argc == 3)
	{
		// open file
		ret = f_open(&fp, argv[1], FA_READ);
		if(ret != FR_OK)
		{
			// append '.pkt' and try again
			strcat(argv[1], ".pkt");
			ret = f_open(&fp, argv[1], FA_READ);
		}

		if(ret == FR_OK)
		{
			// start a new packet
			pPacket = apShellPacket;

			while(1)
			{
				bc = getLine(&fp, szTypeBuf, sizeof(szTypeBuf));
				if(bc == 0)
				{
					//return CMD_BAD_BIT_FILE;
					break;
				}

				// the first lines are optional modifiers:
				//	- REM - for shell help
				//	- number of preambles
				//	- override of the scope position
				if(strnicmp(szTypeBuf, "REM", 3) == 0)
				{
					;	// just skip this
				}
				else if(strnicmp(szTypeBuf, "PREAMBLES", 9) == 0)
				{
					// ToDo - verify the '=' and number format
					for(i = 9; i < bc; i++)
					{
						if(szTypeBuf[i] != ' ' && szTypeBuf[i] != '=')
						{
							break;
						}
					}
					preambles = atoi(&szTypeBuf[i]);
				}
				else if(strnicmp(szTypeBuf, "SCOPE", 5) == 0)
				{
					// ToDo - verify the '=' and number format
					for(i = 5; i < bc; i++)
					{
						if(szTypeBuf[i] != ' ' && szTypeBuf[i] != '=')
						{
							break;
						}
					}

					scope = atoi(&szTypeBuf[i]);
				}
				else
				{
					if(preambles)
					{
						// build the preamble before any defined bits
						for(i = 0; i < preambles; i++)
						{
							pPacket->period = ONE_PERIOD;
							pPacket->pulse = ONE_PULSE;
							if(scope)
							{
								scope--;
								if(scope == 0)
								{
									pPacket->scope = 1;
								}
								else
								{
									pPacket->scope = 0;
								}
							}
							else
							{
								pPacket->scope = 0;
							}
							pPacket++;
						}
						preambles = 0;		// done, don't do it again
					}

					pBuf = szTypeBuf;
					pp = strsep(&pBuf, ",");

					period = atoi(pp) * TICKS_PER_MICROSECOND;
					pp = strsep(&pBuf, ",");
					pulse = atoi(pp) * TICKS_PER_MICROSECOND;
					pp = strsep(&pBuf, ",");
					count = atoi(pp);
					if(count == 0)
					{
						count++;
					}

					for(i = 0; i < count; i++)
					{
						pPacket->period = period;
						pPacket->pulse = pulse;
						if(scope)
						{
							scope--;
							if(scope == 0)
							{
								pPacket->scope = 1;
							}
							else
							{
								pPacket->scope = 0;
							}
						}
						else
						{
							pPacket->scope = 0;
						}
						pPacket++;
					}
				}
			}

			f_close(&fp);

			// add the terminator
			pPacket->period = 0;
			pPacket->pulse = 0;
			pPacket->scope = 0;

			if(argc == 3)
			{
				bc = atoi(argv[1]);
				BuildPacketBits(apShellPacket, bc);
			}
			else
			{
				BuildPacketBits(apShellPacket, 1);
			}
		}
		else
		{
			// if no file, try to convert arg to a number,
			// if less than 100 send that many packets
			bc = atoi(argv[1]);
			if(apShellPacket[0].period != 0 && bc != 0 && bc <= 100)
			{
				BuildPacketBits(apShellPacket, bc);
			}
			else
			{
				ShFieldOut(bPort, "File not found.\r\n", 0);
				// return file not found
				return CMD_NOT_FOUND;
			}
		}
	}
	else
	{
		return CMD_BAD_PARAMS;
	}
	return CMD_OK;

}


#ifdef FUTURE
/**********************************************************************
*
* FUNCTION:		ShSystemStatus
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
CMD_RETURN ShSelectLoco(uint8_t bPort, int argc, char *argv[])
{
	int nLoco;
    Loco* pLoco;

	if(argc == 2)
	{
		nLoco = atoi(argv[1]);
    
        pLoco = SelectLoco(pVirtualCab, nLoco);

        // ??????
        SetLocoDisconnect(pVirtualCab, pLoco);
    }
	else
	{
		return CMD_NEED_HELP;
	}
}

CMD_RETURN ShSelectAccessory(uint8_t bPort, int argc, char *argv[])
#endif


/**********************************************************************
*
* FUNCTION:		ShSetLoco
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
CMD_RETURN ShSetLoco(uint8_t bPort, int argc, char *argv[])
{
    Loco* pLoco;
    unsigned int nAddress;
    word nSpeed;
    uint8_t nDirection;
    unsigned long nFunctionMap;
    
	if(argc >= 2)
	{
		nAddress = atoi(argv[1]);
		pLoco = FindLoco(nAddress);
		//Loco* FindAlias(unsigned int nAddress);
		//Loco* FindLeadLoco(unsigned int nAddress);
		if(pLoco == NULL)
		{
			pLoco = NewLoco(nAddress);
		}

		if(pLoco != NULL)
		{
			if(argc >= 3)
			{
				nSpeed = atoi(argv[2]);
				SetLocoSpeed(pLoco, nSpeed);
			}
			if(argc >= 4)
			{
				nDirection = atoi(argv[3]);
				SetLocoDirection(pLoco, nDirection);
			}
#ifdef COMBINED_FUNCTIONS
			if(argc >= 5)
			{
				nFunctionMap = atoi(argv[4]);
				SetLocoFunctions1(pLoco, nFunctionMap);
			}
			if(argc >= 6)
			{
				nFunctionMap = atoi(argv[5]);
				SetLocoFunctions2(pLoco, nFunctionMap);
			}
#endif
			if(argc >= 5)
			{
				nFunctionMap = atoi(argv[4]);
				SetLocoFunctions(pLoco, nFunctionMap);
			}
			//    SetLocoFunctions3(pLoco, nFunctionMap);
		}
		else
		{
			return CMD_FAILED;
		}
	}
	else
	{
		return CMD_BAD_PARAMS;
	}
	return CMD_OK;
}


CMD_RETURN ShCabDisplay(uint8_t bPort, int argc, char *argv[])
{
	int Cab;

	if(argc == 3)
	{
		Cab = atoi(argv[1]);

		//aVirtualCab[Cab];

		NCE_DisplayMessage(Cab, 0, 1, argv[2]);

//		SetExpiration(VIRTUAL_CAB* pVirtualCab, EPIRATION_2_SECONDS);

	}
	else
	{
		return CMD_BAD_PARAMS;
	}
	return CMD_OK;
}


void ServiceModeCallback(int status)
{
//	uint8_t Value;

//	if(status == SM_OK)
	{
//		ShFieldNumberOut(bPort, "Read Successful: ", Value, 0);
//		ShFieldOut(bPort, "Write Successful", 0);
	}
//	else
	{
//		ShFieldOut(bPort, "Failed", 0);
	}
//   ShNL(bPort);
}

CMD_RETURN ShProgTrackWriteCV(uint8_t bPort, int argc, char *argv[])
{
//	int CV;
//	int Value;

	if(argc == 3)
	{
//		CV = atoi(argv[1]);
//		Value = atoi(argv[2]);

//		ServiceModeWriteCV(CV, Value, ServiceModeCallback, NULL);
	}
	else
	{
//		return CMD_NEED_HELP;
	}

	return CMD_OK;
}


CMD_RETURN ShProgTrackReadCV(uint8_t bPort, int argc, char *argv[])
{
//	int CV;

	if(argc == 2)
	{
//		CV = atoi(argv[1]);

//		ServiceModeReadCV(CV, ServiceModeCallback, NULL);
	}
	else
	{
//		return CMD_NEED_HELP;
	}

	return CMD_OK;
}


#ifdef NOT_USED
CMD_RETURN ShTrack(uint8_t bPort, int argc, char *argv[])
{
	if(argc == 2)
	{
		if(atoi(argv[1]) == 1)
		{
			EnableTrack();
		}
		else
		{
			DisableTrack();
		}
	}
	else
	{
		return CMD_BAD_PARAMS;
	}
	return CMD_OK;
}


CMD_RETURN ShProgTrack(uint8_t bPort, int argc, char *argv[])
{
	if(argc == 2)
	{
		if(atoi(argv[1]) == 1)
		{
//k			EnableProgrammingTrack();
		}
		else
		{
//k			DisableProgrammingTrack();
		}
	}
	else
	{
		return CMD_BAD_PARAMS;
	}
	return CMD_OK;
}


CMD_RETURN ShFindLoco(uint8_t bPort, int argc, char *argv[])
{

//	Loco* FindLoco(unsigned int nAddress);
	return CMD_OK;
}
#endif

