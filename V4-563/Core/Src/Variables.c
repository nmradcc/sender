/*
 * Variables.c
 *
 *  Created on: May 1, 2018
 *      Author: Karl Kobel
*
* Copyright (c) Jun 29, 2017, K2 Engineering
*
*******************************************************************/
#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "ObjectNames.h"
#include "Shell.h"
#include "Settings.h"
#include "Variables.h"
//#include "Track.h"
#include "minini.h"
#include "led.h"
#include "input.h"
#include "Shell.h"

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

extern char* ItoA(int n);

extern char* strsep(char **stringp, const char *delim);

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

uint32_t lVersion;
uint32_t lSerialNumber;
char szBuildDateVar[20];

uint32_t bTimeFmt;
uint32_t bDateFmt;

uint32_t CabPort;

uint32_t IpAddress;
uint32_t IpMask;
uint32_t GwAddress;
uint32_t DHCP;
char szDnsName[24];

uint32_t Theme;

uint32_t TrackState;
uint32_t TrackIdle;
//uint32_t TrackLck;

extern uint32_t RedPattern;
extern uint32_t YellowPattern;
extern uint32_t GreenPattern;

float TrackCurrent;
float TrackVoltage;

char szPathVar[80];

extern RTC_HandleTypeDef hrtc;

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/

static char tempbuf[64];

/**********************************************************************
*
*							CODE
*
**********************************************************************/

const char* szMonth[] =
{
	"NON",
	"JAN",
	"FEB",
	"MAR",
	"APR",
	"MAY",
	"JUN",
	"JUL",
	"AUG",
	"SEP",
	"OCT",
	"NOV",
	"DEC",
};

const VAR_TABLE VarCmdTable[] =
{
	// name				variable			type									default				help string
	{szVersion,			&lVersion,			(VAR_TYPE_VER | VAR_TYPE_READ_ONLY),	"",					"Version" },
	{szSerialNumber,	&lSerialNumber,		(VAR_TYPE_INT | VAR_TYPE_READ_ONLY),	"",					"Serial Number" },
	{szBuildDate,		&szBuildDateVar,	(VAR_TYPE_STRING | VAR_TYPE_READ_ONLY),	"",					"Build Date" },

	{szTime,			NULL,				(VAR_TYPE_TIME),						"",					"Real Time hh:mm" },
	{szDate,			NULL,				(VAR_TYPE_DATE),						"",					"Real Time dd/mm/yyyy" },
	{szTimeFmt,			&bTimeFmt,			(VAR_TYPE_TIME_FMT | VAR_TYPE_PERSIST),	"12",				"Clock 12 | 24 hour mode" },
	{szDateFmt,			&bDateFmt,			(VAR_TYPE_DATE_FMT | VAR_TYPE_PERSIST),	"M/D/Y",			"Date M/D/Y|D/M/Y|Y/M/D MM=3 Char Month" },

	{szIpAdr,			&IpAddress,			(VAR_TYPE_IP | VAR_TYPE_PERSIST),		"169.254.172,10",	"IP Address" },
	{szIpMsk,			&IpMask,			(VAR_TYPE_IP | VAR_TYPE_PERSIST),		"255.255.255.0",	"IP Mask" },
	{szGwAdr,			&GwAddress,			(VAR_TYPE_IP | VAR_TYPE_PERSIST),		"0.0.0.0",			"GW Address" },
	{szDHCP,			&DHCP,				(VAR_TYPE_ON_OFF | VAR_TYPE_PERSIST),	"0",				"DHCP - on | off" },
	{szName,			&szDnsName,			(VAR_TYPE_STRING | VAR_TYPE_PERSIST),	"", 				"The Domain name" },

	{szPort,			&CabPort,			(VAR_TYPE_PORT | VAR_TYPE_PERSIST),		"off",				"Port - nce | xpressnet" },
#ifdef TRACK_LOCK_NOT_USED
	{szTrack,			&TrackState,		(VAR_TYPE_TRACK),						"off",				"Track - on | off" },
#endif
//	{szTrackIdle,		&TrackIdle,			(VAR_TYPE_TRACK_IDLE | VAR_TYPE_PERSIST), "idle",			"Track Idle - off | none | idle | reset | ones | zeros" },
//	{szTrackLock,		NULL,				(VAR_TYPE_TRACK_LOCK | CMD_READ_ONLY),	"",					"Track Lock -  None send | shell | cs" },

	{szPath,			&szPathVar,			(VAR_TYPE_STRING | VAR_TYPE_PERSIST),	"\\",				"Script search path" },

	{szRLed,			&RedPattern,		(VAR_TYPE_RLED),						"",					"Red LED - on | off | blink | flash | 32 bit pattern" },
	{szYLed,			&YellowPattern,		(VAR_TYPE_YLED),						"",					"Yellow LED - on | off | blink | flash | 32 bit pattern" },
	{szGLed,			&GreenPattern,		(VAR_TYPE_GLED),						"",					"Green LED - on | off | blink | flash | 32 bit pattern" },

	{szInputs,			&Inputs,			(VAR_TYPE_INPUTS | VAR_TYPE_READ_ONLY),	"",					"Inputs 1 - 4" },

	{szLoopCount,		NULL,				(VAR_TYPE_HIDDEN | VAR_TYPE_LOOP_CNT | VAR_TYPE_READ_ONLY),"",				"Loop Count" },

	{szTheme,			&Theme,				(VAR_TYPE_THEME | VAR_TYPE_PERSIST),	"0",				"Color Theme - light | dark" },

	{szTrackCurrent,	&TrackCurrent,		(VAR_TYPE_FLOAT | VAR_TYPE_READ_ONLY),	"",					"Track Current" },
	{szTrackVoltage,	&TrackVoltage,		(VAR_TYPE_FLOAT | VAR_TYPE_READ_ONLY),	"",					"Track Voltage" },

//	{szSpeed,			NULL,				(VAR_TYPE_SPEED),						"",					"Current Train Speed" },
//	{szDir,				NULL,				(VAR_TYPE_DIR),							"",					"Current Train Direction fwd / rev" },
//	{szFunction,		NULL,				(VAR_TYPE_FUNCTION),					"",					"Current Train Functions" },
};

char m_szAscii[64];

char *F2A(float fValue)
{
	char szExp[10];
	char szMan[20];
	char bSign = 0;
	long lWhole, lFrac;

	if (fValue < 0.0)
	{
		bSign = 1;
		fValue = -fValue;
	}
	if (fValue < 0.00000000001)
	{
//		sprintf(szExp, "");
		strcpy(szExp, "");
		sprintf(szMan, "0.0");
	}
	else
	{
		if (fValue < 0.0000000001)
		{
			fValue *= 1000000000.0;
			sprintf(szExp, "e-10");
		}
		else if (fValue < 0.000000001)
		{
			fValue *= 100000000.0;
			sprintf(szExp, "e-09");
		}
		else if (fValue < 0.00000001)
		{
			fValue *= 100000000.0;
			sprintf(szExp, "e-08");
		}
		else if (fValue < 0.0000001)
		{
			fValue *= 10000000.0;
			sprintf(szExp, "e-07");
		}
		else if (fValue < 0.000001)
		{
			fValue *= 1000000.0;
			sprintf(szExp, "e-06");
		}
		else if (fValue < 0.00001)
		{
			fValue *= 100000.0;
			sprintf(szExp, "e-05");
		}
		else if (fValue < 0.0001)
		{
			fValue *= 10000.0;
			sprintf(szExp, "e-04");
		}
		else if (fValue < 0.001)
		{
			fValue *= 1000.0;
			sprintf(szExp, "e-03");
		}
		else if (fValue < 0.01)
		{
			fValue *= 100.0;
			sprintf(szExp, "e-02");
		}
		else if (fValue < 10000.0)
		{
			strcpy(szExp, "");
		}
		else if (fValue < 100000.0)
		{
			fValue /= 10000.0;
			sprintf(szExp, "e+04");
		}
		else if (fValue < 1000000.0)
		{
			fValue /= 100000.0;
			sprintf(szExp, "e+05");
		}
		else if (fValue < 10000000.0)
		{
			fValue /= 1000000.0;
			sprintf(szExp, "e+06");
		}
		else if (fValue < 100000000.0)
		{
			fValue /= 10000000.0;
			sprintf(szExp, "e+07");
		}
		else if (fValue < 1000000000.0)
		{
			fValue /= 100000000.0;
			sprintf(szExp, "e+08");
		}
		else if (fValue < 1.0e+10)
		{
			fValue /= 1.0e+9;
			sprintf(szExp, "e+09");
		}
		else if (fValue < 1.0e+11)
		{
			fValue /= 1.0e+10;
			sprintf(szExp, "e+10");
		}
		else if (fValue < 1.0e+12)
		{
			fValue /= 1.0e+11;
			sprintf(szExp, "e+11");
		}
		else if (fValue < 1.0e+13)
		{
			fValue /= 1.0e+12;
			sprintf(szExp, "e+12");
		}
		else if (fValue < 1.0e+14)
		{
			fValue /= 1.0e+13;
			sprintf(szExp, "e+13");
		}
		else if (fValue < 1.0e+15)
		{
			fValue /= 1.0e+14;
			sprintf(szExp, "e+14");
		}
		else if (fValue < 1.0e+16)
		{
			fValue /= 1.0e+15;
			sprintf(szExp, "e+15");
		}
		else if (fValue < 1.0e+17)
		{
			fValue /= 1.0e+16;
			sprintf(szExp, "e+16");
		}
		else if (fValue < 1.0e+18)
		{
			fValue /= 1.0e+17;
			sprintf(szExp, "e+17");
		}
		else if (fValue < 1.0e+19)
		{
			fValue /= 1.0e+18;
			sprintf(szExp, "e+18");
		}
		else
		{
			fValue /= 1.0e+19;
			sprintf(szExp, "e+19");
		}

		lWhole = (long)fValue;
		lFrac = (long)((fValue - (float)lWhole) * 10000.0);
		sprintf(szMan, "%ld.%04ld", lWhole, lFrac);
	}
	sprintf(m_szAscii, "%c%s%s", (bSign ? '-' : ' '), szMan, szExp);
	return (m_szAscii);
}

/*********************************************************************
*
* VarToString
*
* @brief	Return a string representing a variable value
*
* @param	idx - index of the variable table
*
* @return	char* string
*
*********************************************************************/
char* VarToString(uint32_t idx)
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	unsigned int t1, t2, t3;
	uint8_t ip_addr[4];
	//char szTemp[16];
	unsigned int lc;

    switch(VarCmdTable[idx].type & VAR_TYPE_MASK)
    {
    	case VAR_TYPE_INT:
    		sprintf(tempbuf, "%u", *(unsigned int*)(VarCmdTable[idx].var));
    	break;

    	case VAR_TYPE_FLOAT:
    		strcpy(tempbuf, F2A(*(float*)(VarCmdTable[idx].var)));
    	break;

    	case VAR_TYPE_TIME:
    		if(HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN) == HAL_OK)
    		{
				if(bTimeFmt == 0)
				{
					if(time.Hours > 12)
					{
						sprintf(tempbuf, "%2d:%02d:%02d PM", time.Hours-12, time.Minutes, time.Seconds);
					}
					else
					{
						sprintf(tempbuf, "%2d:%02d:%02d AM", time.Hours, time.Minutes, time.Seconds);
					}
				}
				else
				{
	    			sprintf(tempbuf, "%2d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);
				}
    		}
    		// call the date read to unlock the clock
    		HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
    	break;

    	case VAR_TYPE_DATE:
    		// call the time read to lock the clock
    		HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    		if(HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN) == HAL_OK)
    		{
    			switch(bDateFmt)
    			{
    				case 0:
    				default:
    	    			sprintf(tempbuf, "%2d/%02d/%04d", date.Month, date.Date, date.Year+2000);
    				break;
    				case 1:
    	    			sprintf(tempbuf, "%2d/%02d/%04d", date.Date, date.Month, date.Year+2000);
       				break;
    				case 2:
    	    			sprintf(tempbuf, "%04d/%2d/%02d", date.Year+2000, date.Month, date.Date);
       				break;
    				case 3:
    	    			sprintf(tempbuf, "%s/%02d/%04d", szMonth[date.Month], date.Date, date.Year+2000);
    				break;
    				case 4:
    	    			sprintf(tempbuf, "%2d/%s/%04d", date.Date, szMonth[date.Month], date.Year+2000);
       				break;
    				case 5:
    	    			sprintf(tempbuf, "%04d/%s/%02d", date.Year+2000, szMonth[date.Month], date.Date);
       				break;
    			}
    		}
    	break;

    	case VAR_TYPE_TIME_FMT:
    		sprintf(tempbuf, "%s", ((*(unsigned int*)VarCmdTable[idx].var) == 0) ? "12" : "24");
    	break;

    	case VAR_TYPE_DATE_FMT:
    		switch((*(unsigned int*)VarCmdTable[idx].var))
    		{
    			case 0:
				default:
    				strcpy(tempbuf, "M/D/Y");
    			break;

    			case 1:
    				strcpy(tempbuf, "D/M/Y");
    			break;

    			case 2:
    				strcpy(tempbuf, "Y/M/D");
    			break;

    			case 3:
    				strcpy(tempbuf, "MM/D/Y");
    			break;

    			case 4:
    				strcpy(tempbuf, "D/MM/Y");
    			break;

    			case 5:
    				strcpy(tempbuf, "Y/MM/D");
    			break;
    		}
    	break;

    	case VAR_TYPE_VER:
    		t1 = *(unsigned int*)(VarCmdTable[idx].var);
    		t3 = t1 % 100;
    		t1 -= t3;
    		t1 /= 100;
    		t2 = t1 % 100;
    		t1 -= t2;
    		t1 /= 100;
    		sprintf(tempbuf, "%u.%02u.%02u", t1, t2, t3);
    	break;

    	case VAR_TYPE_ON_OFF:
    		sprintf(tempbuf, "%s", ((*(unsigned int*)VarCmdTable[idx].var) == 0) ? "off" : "on");
    	break;

    	case VAR_TYPE_PORT:
    		t1 = *(unsigned int*)(VarCmdTable[idx].var);
    		if(t1 == 1)
    		{
				strcpy(tempbuf, "nce");
    		}
    		else if(t1 == 2)
    		{
				strcpy(tempbuf, "xpressnet");
    		}
    		else
    		{
				strcpy(tempbuf, "off");
    		}
    	break;

#ifdef NOT_YET
    	case VAR_TYPE_TRACK:
    		sprintf(tempbuf, "%s", GetTrackState() == 0 ? "off" : "on");
       	break;
#endif

#ifdef TRACK_LOCK_NOT_USED
    	case VAR_TYPE_TRACK_LOCK:
    		switch(GetTrackLock())
    		{
    			case TR_NONE:
    			default:
    	    		sprintf(tempbuf, "None");
    			break;

    			case TR_TESTER:
    	    		sprintf(tempbuf, "Send");
    			break;

    			case TR_SHELL:
    	    		sprintf(tempbuf, "Shell");
    			break;

    			case TR_COMMAND_STATION:
    	    		sprintf(tempbuf, "CS");
    			break;
    		}
    	break;
#endif

#ifdef NOT_YET
    	case VAR_TYPE_TRACK_IDLE:
    		switch(TrackIdle)
    		{
    			case TI_NONE:
    	    		sprintf(tempbuf, "Off");
    			break;

    			case TI_IDLE:
    	    		sprintf(tempbuf, "Idle");
    			break;

    			case TI_RESET:
    	    		sprintf(tempbuf, "Reset");
    			break;

    			case TI_ONES:
    	    		sprintf(tempbuf, "ones");
    			break;

    			case TI_ZEROS:
    	    		sprintf(tempbuf, "zeros");
    			break;
    		}
       	break;
#endif

    	case VAR_TYPE_IP:
    		t1 = *(unsigned int*)(VarCmdTable[idx].var);
    		ip_addr[0] = t1 >> 24;
    		ip_addr[1] = (t1 >> 16) & 0xff;
    		ip_addr[2] = (t1 >> 8) & 0xff;
    		ip_addr[3] = t1 & 0xff;
    		//sprintf(tempbuf, "%02X.%02X.%02X.%02X", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
    		sprintf(tempbuf, "%03d.%03d.%03d.%03d", ip_addr[3], ip_addr[2], ip_addr[1], ip_addr[0]);
       	break;

    	case VAR_TYPE_STRING:
    		strcpy(tempbuf, (char*)VarCmdTable[idx].var);
		break;

    	case VAR_TYPE_RLED:
    		sprintf(tempbuf, "%08x", (int)GetRedLed());
   		break;

    	case VAR_TYPE_YLED:
    		sprintf(tempbuf, "%08x", (int)GetYellowLed());
   		break;

    	case VAR_TYPE_GLED:
    		sprintf(tempbuf, "%08x", (int)GetGreenLed());
   		break;

    	case VAR_TYPE_INPUTS:
    		//sprintf(tempbuf, "%x", (int)GetInputs());

    		if(GetInput1())
    		{
    			strcpy(tempbuf, "1=ON ");
    		}
    		else
    		{
    			strcpy(tempbuf, "1=OFF ");
    		}

    		if(GetInput2())
    		{
    			strcat(tempbuf, "2=ON ");
    		}
    		else
    		{
    			strcat(tempbuf, "2=OFF ");
    		}

    		if(GetInput3())
    		{
    			strcat(tempbuf, "3=ON ");
    		}
    		else
    		{
    			strcat(tempbuf, "3=OFF ");
    		}

    		if(GetInput4())
    		{
    			strcat(tempbuf, "4=ON ");
    		}
    		else
    		{
    			strcat(tempbuf, "4=OFF ");
    		}

    		if(GetInput5())
    		{
    			strcat(tempbuf, "5=ON ");
    		}
    		else
    		{
    			strcat(tempbuf, "5=OFF ");
    		}

    		if(GetInput6())
    		{
    			strcat(tempbuf, "Ack");
    		}
    		else
    		{
    			strcat(tempbuf, "Nack");
    		}
    		break;
    	case VAR_TYPE_LOOP_CNT:
    		lc = GetLoopCount();
    		if(lc == 0xffff)
    		{
    			sprintf(tempbuf, "None");
    		}
    		else
    		{
    			sprintf(tempbuf, "%u", lc);
    		}
    	break;

    	case VAR_TYPE_THEME:
    		sprintf(tempbuf, "%s", ((*(unsigned int*)VarCmdTable[idx].var) == 0) ? "dark" : "light");
    	break;
    }
    return tempbuf;
}

/*********************************************************************
*
* ShowVariables
*
* @brief	Shell command to dump all variables
*
* @param	bPort - I/O port
*
* @return	None
*
*********************************************************************/
void ShowVariables(uint8_t bPort)
{
	int i;
	//uint32_t temp;
	//char TempStr[32];
	char* pStr;


    ShNL(bPort);
	for(i = 0; i < NUM_VARIABLES; i++)
	{
		if(!(VarCmdTable[i].type & VAR_TYPE_HIDDEN))
		{
			ShFieldOut(bPort, (char*)VarCmdTable[i].szCmdString, 0);
			ShFieldOut(bPort, " = ", 0);

			pStr = VarToString(i);
			ShFieldOut(bPort, pStr, 0);
			ShNL(bPort);
		}
	}
}


/*********************************************************************
*
* GetNumVariables
*
* @brief	Get the number of variables in the variable table
*
* @param	None
*
* @return	number
*
*********************************************************************/
int GetNumVariables(void)
{

	return NUM_VARIABLES;
}


/*********************************************************************
*
* GetVariableName
*
* @brief	Return a string representing the name of a variable
*
* @param	idx - index of the variable table
*
* @return	char* name
*
*********************************************************************/
char* GetVariableName(uint8_t idx)
{

	return (char*)VarCmdTable[idx].szCmdString;
}

/*********************************************************************
*
* GetVariableName
*
* @brief	Return a string representing the help string of a variable
*
* @param	idx - index of the variable table
*
* @return	char* help string
*
*********************************************************************/
char* GetVariableHelp(uint8_t idx)
{

	return (char*)VarCmdTable[idx].pszHelp;
}



/*********************************************************************
*
* IsVariable
*
* @brief	Check if a name is a variable
*
* @param	char* name
*
* @return	idx - index of the variable table
*
*********************************************************************/
int IsVariable(char* pBuffer)
{
	int i;

	for(i = 0; i < NUM_VARIABLES; i++)
	{
		if(strcmp(pBuffer, VarCmdTable[i].szCmdString) == 0)
		{
			// found command string
			return 1;
		}
	}
	return 0;
}


/*********************************************************************
*
* IsVariable
*
* @brief	Check if a variable is visible
*
* @param	index
*
* @return	true if visible
*
*********************************************************************/
int IsVariableVisible(uint8_t idx)
{

	return (int)!(VarCmdTable[idx].type & VAR_TYPE_HIDDEN);
}

/*********************************************************************
*
* IsVariable
*
* @brief	Check if a name is a variable
*
* @param	char* name
*
* @return	idx - index of the variable table
*
*********************************************************************/
int IsVariableHidden(char* pBuffer)
{
	int i;

	for(i = 0; i < NUM_VARIABLES; i++)
	{
		if(strcmp(pBuffer, VarCmdTable[i].szCmdString) == 0)
		{
			// found command string
			//return VarCmdTable[i].type & VAR_TYPE_HIDDEN;
			return 1;
		}
	}
	return 0;
}


/*********************************************************************
*
* FindVariable
*
* @brief	Return the index of a variable
*
* @param	char* name
*
* @return	idx - index of the variable table, -1 if not found
*
*********************************************************************/
int FindVariable(char* szObject)
{
	int i;

	for(i = 0; i < NUM_VARIABLES; i++)
	{
		if(strcmp(szObject, VarCmdTable[i].szCmdString) == 0)
		{
			// found command string
			return i;
		}
	}
	return -1;
}



/*********************************************************************
*
* GetVariable
*
* @brief	Return the string value of a variable
*
* @param	const char* szObject
* 			char* pStrValue
* 			int max_len
*
* @return	None
*
*********************************************************************/
void GetVariable(const char* szObject, char* pStrValue, int max_len)
{
	int idx;
	char* pStr;

	idx = FindVariable((char*)szObject);

	if(idx != -1)
	{
		pStr = VarToString(idx);
		strcpy(pStrValue , pStr);
	}
	else
	{
		strcpy(pStrValue, "");
	}
}


/*********************************************************************
*
* GetVariableValue
*
* @brief	Return the value of a variable
*
* @param	idx - index of the variable table
*
* @return	value
*
*********************************************************************/
uint32_t GetVariableValue(int idx)
{

    switch(VarCmdTable[idx].type & VAR_TYPE_MASK)
    {
    	case VAR_TYPE_INT:
    	case VAR_TYPE_VER:
    	case VAR_TYPE_ON_OFF:
    	case VAR_TYPE_PORT:
    	case VAR_TYPE_TRACK:
    	case VAR_TYPE_TRACK_IDLE:
    	case VAR_TYPE_RLED:
    	case VAR_TYPE_YLED:
    	case VAR_TYPE_GLED:
    	case VAR_TYPE_INPUTS:
    		return *(unsigned int*)(VarCmdTable[idx].var);
       	break;

       	case VAR_TYPE_TIME:
    	case VAR_TYPE_DATE:
    	case VAR_TYPE_TIME_FMT:
    	case VAR_TYPE_DATE_FMT:
    	case VAR_TYPE_IP:
    	case VAR_TYPE_STRING:
    	default:
    		return 0;
    	break;
    }
}


uint32_t hexadecimalToDecimal(const char hexVal[])
{
    int len = strlen(hexVal);

    // Initializing base value to 1, i.e 16^0
    uint32_t base = 1;

    uint32_t dec_val = 0;

    // Extracting characters as digits from last character
    for (int i=len-1; i>=0; i--)
    {
        // if character lies in '0'-'9', converting
        // it to integral 0-9 by subtracting 48 from
        // ASCII value.
        if (hexVal[i]>='0' && hexVal[i]<='9')
        {
            dec_val += (hexVal[i] - 48)*base;

            // incrementing base by power
            base = base * 16;
        }

        // if character lies in 'A'-'F' , converting
        // it to integral 10 - 15 by subtracting 55
        // from ASCII value
        else if (hexVal[i]>='A' && hexVal[i]<='F')
        {
            dec_val += (hexVal[i] - 55)*base;

            // incrementing base by power
            base = base*16;
        }
        // if character lies in 'a'-'f' , converting
        // it to integral 10 - 15 by subtracting 87
        // from ASCII value
        else if (hexVal[i]>='a' && hexVal[i]<='f')
        {
            dec_val += (hexVal[i] - 87)*base;

            // incrementing base by power
            base = base*16;
        }
    }

    return dec_val;
}



/*********************************************************************
*
* GetAmPm
*
* @brief	See in the time string also contains AM or PM
* 			or am/pm, or A/P or a/p
*
* @param	const char* s
*
* @return	0 = am
* 			1 = pm
* 			-1 not found
*********************************************************************/
int GetAmPm(const char* ptm)
{

	while(*ptm)
	{
		if(*ptm == 'a' || *ptm == 'A')
		{
			return 0;
		}
		else if(*ptm == 'p' || *ptm == 'P')
		{
			return 1;
		}
		ptm++;
	}
	return -1;
}



/*********************************************************************
*
* GetLedPattern
*
* @brief	Return the LED pattern based on the pattern name
*
* @param	char* pStrValue
*
* @return	pattern
*
*********************************************************************/
unsigned long GetLedPattern(const char* pStrValue)
{

	if(stricmp(pStrValue, "on") == 0)
	{
		return LED_ON;
	}
	else if(stricmp(pStrValue, "off") == 0)
	{
		return LED_OFF;
	}
	else if(stricmp(pStrValue, "blink") == 0)
	{
		return LED_BLINK;
	}
	else if(stricmp(pStrValue, "flash") == 0)
	{
		return LED_FAST_BLINK;
	}
	else if(stricmp(pStrValue, "pulse") == 0)
	{
		return LED_NORMAL;
	}
	else
	{
		return hexadecimalToDecimal(pStrValue);
	}
}


/*********************************************************************
*
* SetVariable
*
* @brief	Set a variable from the string value
*
* @param	const char* szObject
* 			char* pStrValue
*
* @return	Shell Error value
*
*********************************************************************/
int SetVariable(const char* szObject, char* pStrValue)
{
	int idx;
	int i;
	int type;
	int temp;
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	char* pHr;
	char* pMn;
	char* pSc;
	char* pYr;
	char* pMt;
	char* pDa;
	char* pOct;
	int ampm;

	idx = FindVariable((char*)szObject);

	if(idx != -1)
	{
		type = VarCmdTable[idx].type;
		switch(type & VAR_TYPE_MASK)
		{
			case VAR_TYPE_INT:
				if((type & VAR_TYPE_READ_ONLY) == 0)
				{
					// ToDo - fail if the number is improperly formatted
					i = FindVariable(pStrValue);
					if(i != -1)
					{
						temp = GetVariableValue(i);
					}
					else
					{
						temp = atoi(pStrValue);
					}

					*(uint32_t*)(VarCmdTable[idx].var) = temp;

					if((type & VAR_TYPE_PERSIST) != 0)
					{
						(void)ini_putl("", szObject, (long)temp, SETTINGS_FILE);
					}
				}
				else
				{
					return CMD_READ_ONLY;
				}
			break;

			case VAR_TYPE_TIME:
				pHr = strsep(&pStrValue, ":");
				pMn = strsep(&pStrValue, ": ");
				if(pHr != NULL && pMn != NULL)
				{
					time.Hours = (uint8_t)atoi(pHr);
					time.Minutes = (uint8_t)atoi(pMn);

					pSc = strsep(&pStrValue, ":");
					if(pSc)
					{
						time.Seconds = (uint8_t)atoi(pSc);
						ampm = GetAmPm(pSc);
					}
					else
					{
						time.Seconds = 0;
						ampm = GetAmPm(pMn);
					}

					if(ampm == 1)
					{
						time.Hours += 12;
					}

					time.SecondFraction = 0;
					time.SubSeconds = 0;
					time.TimeFormat = RTC_HOURFORMAT_24;	// RTC_HOURFORMAT12_AM;
					time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;

					HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
				}
			break;

			case VAR_TYPE_DATE:

    			switch(bDateFmt)
    			{
    				case 0:
    				case 3:
    				default:
    	    			// M/D/Y
    					pMt = strsep(&pStrValue, "/-");
    					pDa = strsep(&pStrValue, "/-");
    					pYr = strsep(&pStrValue, "/-");
    				break;
    				case 1:
    				case 4:
    	    			// D/M/Y
    					pDa = strsep(&pStrValue, "/-");
    					pMt = strsep(&pStrValue, "/-");
    					pYr = strsep(&pStrValue, "/-");
       				break;
    				case 2:
    				case 5:
    	    			// Y/M/D
    					pYr = strsep(&pStrValue, "/-");
    					pMt = strsep(&pStrValue, "/-");
    					pDa = strsep(&pStrValue, "/-");
       				break;
    			}

				if(pDa != NULL && pMt != NULL && pYr != NULL)
				{
					if(!isdigit((int)pMt[0]))
					{
						for(i = 1; i < 12; i++)
						{
							if(stricmp(szMonth[i], pMt) == 0)
							{
								date.Month = (uint8_t)i;
								break;
							}
						}
						if(i > 12)
						{
							return CMD_BAD_PARAMS;
						}
					}
					else
					{
						date.Month = (uint8_t)atoi(pMt);
					}
					date.Date = (uint8_t)atoi(pDa);
					date.Year = (uint8_t)(atoi(pYr) - 2000);
					date.WeekDay = 0;

					HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);
				}
			break;

			case VAR_TYPE_TIME_FMT:
				if(strcmp(pStrValue, "12") == 0)
				{
					bTimeFmt = 0;
				}
				else if(strcmp(pStrValue, "24") == 0)
				{
					bTimeFmt = 1;
				}
				else
				{
					return CMD_BAD_PARAMS;
				}
				if((type & VAR_TYPE_PERSIST) != 0)
				{
					(void)ini_putl("", szObject, (long)bTimeFmt, SETTINGS_FILE);
				}
			break;

			case VAR_TYPE_DATE_FMT:
				if(stricmp(pStrValue, "M/D/Y") == 0)
				{
					bDateFmt = 0;
				}
				if(stricmp(pStrValue, "D/M/Y") == 0)
				{
					bDateFmt = 1;
				}
				else if(stricmp(pStrValue, "Y/M/D") == 0)
				{
					bDateFmt = 2;
				}
				if(stricmp(pStrValue, "MM/D/Y") == 0)
				{
					bDateFmt = 3;
				}
				else if(stricmp(pStrValue, "D/MM/Y") == 0)
				{
					bDateFmt = 4;
				}
				else if(stricmp(pStrValue, "Y/MM/D") == 0)
				{
					bDateFmt = 5;
				}
				else
				{
					return CMD_BAD_PARAMS;
				}

				if((type & VAR_TYPE_PERSIST) != 0)
				{
					(void)ini_putl("", szObject, (long)bDateFmt, SETTINGS_FILE);
				}
			break;

	    	//case VAR_TYPE_TRACK_LOCK:
	    	case VAR_TYPE_VER:
	    	case VAR_TYPE_INPUTS:
				return CMD_READ_ONLY;
			break;

			case VAR_TYPE_ON_OFF:
				if((type & VAR_TYPE_READ_ONLY) == 0)
				{
					temp = 0;
					if(stricmp(pStrValue, "on") == 0)
					{
						temp = 1;
					}
					if(stricmp(pStrValue, "true") == 0)
					{
						temp = 1;
					}
					if(stricmp(pStrValue, "yes") == 0)
					{
						temp = 1;
					}
					if(stricmp(pStrValue, "1") == 0)
					{
						temp = 1;
					}

					*(uint32_t*)(VarCmdTable[idx].var) = temp;
					if((type & VAR_TYPE_PERSIST) != 0)
					{
						(void)ini_putl("", szObject, (long)temp, SETTINGS_FILE);
					}
				}
				else
				{
					return CMD_READ_ONLY;
				}
			break;

			case VAR_TYPE_PORT:
				if(stricmp(pStrValue, "nce") == 0)
				{
					temp = 1;
				}
				else if(stricmp(pStrValue, "expressnet") == 0)
				{
					temp = 2;
				}
				else
				{
					temp = 0;
				}
				*(uint32_t*)(VarCmdTable[idx].var) = temp;
				if((type & VAR_TYPE_PERSIST) != 0)
				{
					(void)ini_putl("", szObject, (long)temp, SETTINGS_FILE);
				}
			break;

#ifdef NOT_YET
	    	case VAR_TYPE_TRACK:
	    		temp = (uint8_t)atoi(pStrValue);
	    		if(temp)
	    		{
	    			EnableTrack();
	    		}
	    		else
	    		{
	    			DisableTrack();
	    		}
				*(uint32_t*)(VarCmdTable[idx].var) = temp;
				if((type & VAR_TYPE_PERSIST) != 0)
				{
				}
	       	break;

	    	case VAR_TYPE_TRACK_IDLE:
				if(stricmp(pStrValue, "off") == 0 || stricmp(pStrValue, "none") == 0)
				{
					TrackIdle = TI_NONE;
					SetTrackIdle(TI_NONE);
				}
				else if(stricmp(pStrValue, "reset") == 0)
				{
					TrackIdle = TI_RESET;
					SetTrackIdle(TI_RESET);
				}
				else if(stricmp(pStrValue, "ones") == 0)
				{
					TrackIdle = TI_ONES;
					SetTrackIdle(TI_ONES);
				}
				else if(stricmp(pStrValue, "zeros") == 0)
				{
					TrackIdle = TI_ZEROS;
					SetTrackIdle(TI_ZEROS);
				}
				else if(stricmp(pStrValue, "idle") == 0)
				{
					TrackIdle = TI_IDLE;
					SetTrackIdle(TI_IDLE);
					//return CMD_BAD_PARAMS;
				}
				if((type & VAR_TYPE_PERSIST) != 0)
				{
					(void)ini_putl("", szObject, (long)TrackIdle, SETTINGS_FILE);
				}
			break;
#endif

	    	case VAR_TYPE_IP:
				if((type & VAR_TYPE_PERSIST) != 0)
				{
					(void)ini_puts("", szObject, pStrValue, SETTINGS_FILE);
				}
	    		temp = 0;
	    	    for(i = 0; i < 4; i++)
	    	    {
	    	    	pOct = strsep(&pStrValue, ".");
	    	    	if(pOct != NULL)
	    	    	{
	    	    		//temp |= atoi(pOct) << (3 - i) * 8;
	    	    		temp |= atoi(pOct) << i * 8;
	    	    	}
	    	    }

				*(uint32_t*)(VarCmdTable[idx].var) = temp;
	    	break;

			case VAR_TYPE_SPEED:
			break;

			case VAR_TYPE_DIR:
			break;

			case VAR_TYPE_FUNCTION:
			break;

			case VAR_TYPE_STRING:
				if((type & VAR_TYPE_READ_ONLY) == 0)
				{
					// ToDo - put the string length in the table
					strncpy((char*)VarCmdTable[idx].var, pStrValue, 64);
					if((type & VAR_TYPE_PERSIST) != 0)
					{
						(void)ini_puts("", szObject, pStrValue, SETTINGS_FILE);
					}
				}
				else
				{
					return CMD_READ_ONLY;
				}
			break;

	    	case VAR_TYPE_RLED:
    			RedLed(GetLedPattern(pStrValue));
			break;

	    	case VAR_TYPE_YLED:
	    		YellowLed(GetLedPattern(pStrValue));
			break;

	    	case VAR_TYPE_GLED:
	    		GreenLed(GetLedPattern(pStrValue));
			break;

	    	case VAR_TYPE_THEME:
				if((type & VAR_TYPE_READ_ONLY) == 0)
				{
					temp = 0;
					if(stricmp(pStrValue, "light") == 0)
					{
						temp = 1;
					}

					*(uint32_t*)(VarCmdTable[idx].var) = temp;
					if((type & VAR_TYPE_PERSIST) != 0)
					{
						(void)ini_putl("", szObject, (long)temp, SETTINGS_FILE);
					}
				}
	    	break;
		}
	}
	return CMD_OK;
}



/*********************************************************************
*
* MathVariable
*
* @brief	Set a variable from the string value
*
* @param	const char* szObject
* 			char* pStrValue
* 			int op
*
* @return	Shell Error value
*
*********************************************************************/
int MathVariable(const char* szObject, char* pStrValue, const int op)
{
	int idx;
	int i;
	int type;
	int temp;

	idx = FindVariable((char*)szObject);

	if(idx != -1)
	{
		type = VarCmdTable[idx].type;
		switch(type & VAR_TYPE_MASK)
		{
			case VAR_TYPE_INT:
			case VAR_TYPE_ON_OFF:
	    	case VAR_TYPE_TRACK:
	    	case VAR_TYPE_SPEED:
			case VAR_TYPE_DIR:
			case VAR_TYPE_FUNCTION:
				if((type & VAR_TYPE_READ_ONLY) == 0)
				{
					// ToDo - fail if the number is improperly formatted
					i = FindVariable(pStrValue);
					if(i != -1)
					{
						temp = GetVariableValue(i);
					}
					else
					{
						temp = atoi(pStrValue);
					}

					switch(op)
					{
						case MV_ADD:
							*(uint32_t*)(VarCmdTable[idx].var) += temp;
						break;

						case MV_SUBTRACT:
							*(uint32_t*)(VarCmdTable[idx].var) -= temp;
						break;

						case MV_MULTIPLY:
							*(uint32_t*)(VarCmdTable[idx].var) *= temp;
						break;

						case MV_DIVIDE:
							if(temp)
							{
								*(uint32_t*)(VarCmdTable[idx].var) /= temp;
							}
						break;

						default:
							return CMD_BAD_PARAMS;
						break;
					}

					(void)ini_putl("", szObject, (long)temp, SETTINGS_FILE);
					return CMD_OK;
				}
				else
				{
					return CMD_READ_ONLY;
				}
			break;

			case VAR_TYPE_TIME:
			case VAR_TYPE_DATE:
			case VAR_TYPE_TIME_FMT:
			case VAR_TYPE_DATE_FMT:
	    	case VAR_TYPE_VER:
			case VAR_TYPE_PORT:
	    	case VAR_TYPE_IP:
			case VAR_TYPE_STRING:
				return CMD_READ_ONLY;
			break;
		}
	}
	return CMD_BAD_PARAMS;
}



/*********************************************************************
*
* GetNextPath
*
* @brief	Get the next piece of the path string
*
* @param	pStr - a pointer to the current path
* 			path - pointer to the string pointer of the next piece
*
* @return	length of he piece returned, 0 if no more pieces
*
* @detail	move the pStr pointer by the length for each repeated call
*
*********************************************************************/
int GetNextPath(char* pStr, char* path)
{
	char* pBeginning = pStr;

	int len = 0;
	while((*pStr != ';' && *pStr != ',') && *pStr != 0)
	{
		len++;
		pStr++;
	}
	if(len)
	{
		memcpy(path, pBeginning, len);
		path[len] = 0;
	}
	return len;
}





