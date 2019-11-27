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
//#include "Schedule.h"
#include "ObjectNames.h"
#include "Shell.h"
#include "Settings.h"
#include "Variables.h"
#include "Track.h"
//#include "TrackProg.h"
#include "minini.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/


//#define true 1
//#define false 0
//const char TokenDelimitor[] = " \r\n\t";
//const char ReadString[] = "read";
//const char WriteString[] = "=";

extern char* ItoA(int n);


 /**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

extern char* strsep(char **stringp, const char *delim);

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

uint32_t lVersion;
uint32_t lSerialNumber;

uint32_t bTimeFmt;
uint32_t bDateFmt;

uint32_t CabPort;

uint32_t IpAddress;
uint32_t IpMask;
uint32_t GwAddress;
uint32_t DHCP;

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

const VAR_TABLE VarCmdTable[] =
{
	// name				variable			type									default				help string
	{szVersion,			&lVersion,			(VAR_TYPE_VER | VAR_TYPE_READ_ONLY),	"",					"Version" },
	{szSerialNumber,	&lSerialNumber,		(VAR_TYPE_INT | VAR_TYPE_READ_ONLY),	"",					"Serial Number" },

	{szTime,			NULL,				(VAR_TYPE_TIME),						"",					"Real Time hh:mm" },
	{szDate,			NULL,				(VAR_TYPE_DATE),						"",					"Real Time dd/mm/yyyy" },
	{szTimeFmt,			&bTimeFmt,			(VAR_TYPE_TIME_FMT | VAR_TYPE_PERSIST),	"12",				"Clock 12 | 24 hour mode" },
	{szDateFmt,			&bDateFmt,			(VAR_TYPE_DATE_FMT | VAR_TYPE_PERSIST),	"M/D/Y",			"Date M/D/Y | D/M/Y | Y/M/D" },

	{szIpAdr,			&IpAddress,			(VAR_TYPE_IP | VAR_TYPE_PERSIST),		"169.254.172,10",	"IP Address" },
	{szIpMsk,			&IpMask,			(VAR_TYPE_IP | VAR_TYPE_PERSIST),		"255.255.255.0",	"IP Mask" },
	{szGwAdr,			&GwAddress,			(VAR_TYPE_IP | VAR_TYPE_PERSIST),		"0.0.0.0",			"GW Address" },
	{szDHCP,			&DHCP,				(VAR_TYPE_ON_OFF | VAR_TYPE_PERSIST),	"0",				"DHCP on | off" },

	{szPort,			&CabPort,			(VAR_TYPE_PORT | VAR_TYPE_PERSIST),		"off",				"Port nce | xpressnet" },
	{szTrack,			NULL,				(VAR_TYPE_TRACK),						"off",				"Track on | off" },

	{szPath,			&szPathVar,			(VAR_TYPE_STRING | VAR_TYPE_PERSIST),	"\\",				"Script search path" },

//	{szSpeed,			NULL,				(VAR_TYPE_SPEED),						"",					"Current Train Speed" },
//	{szDir,				NULL,				(VAR_TYPE_DIR),							"",					"Current Train Direction fwd / rev" },
//	{szFunction,		NULL,				(VAR_TYPE_FUNCTION),					"",					"Current Train Functions" },
};



/**********************************************************************
*
* FUNCTION:		VarToString
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
char* VarToString(uint32_t idx)
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	unsigned int t1, t2;
	uint8_t ip_addr[4];
	//char szTemp[16];


    switch(VarCmdTable[idx].type & VAR_TYPE_MASK)
    {
    	case VAR_TYPE_INT:
    		sprintf(tempbuf, "%u", *(unsigned int*)(VarCmdTable[idx].var));
    	break;

    	case VAR_TYPE_TIME:
    		if(HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN) == HAL_OK)
    		{
    			//uint32_t bTimeFmt;
    			sprintf(tempbuf, "%2d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);
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
    	    			sprintf(tempbuf, "%2d/%02d/%04d", date.Month, date.Date, date.Year+2000);
    				break;
    				case 1:
    	    			sprintf(tempbuf, "%2d/%02d/%04d", date.Date, date.Month, date.Year+2000);
       				break;
    				case 2:
    	    			sprintf(tempbuf, "%04d/%2d/%02d", date.Year+2000, date.Month, date.Date);
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
    				strcpy(tempbuf, "M/D/Y");
    			break;

    			case 1:
    				strcpy(tempbuf, "D/M/Y");
    			break;

    			case 2:
    				strcpy(tempbuf, "Y/M/D");
    			break;
    		}
    	break;

    	case VAR_TYPE_VER:
    		t1 = *(unsigned int*)(VarCmdTable[idx].var);
    		t2 = t1 % 100;
    		t1 -= t2;
    		sprintf(tempbuf, "%u.%02u", t1 / 100, t2);
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

    	case VAR_TYPE_TRACK:
    		sprintf(tempbuf, "%s", GetTrackState() == 0 ? "off" : "on");
       	break;

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
    }
    return tempbuf;
}

/**********************************************************************
*
* FUNCTION:		ShowVariables
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
void ShowVariables(uint8_t bPort)
{
	int i;
	//uint32_t temp;
	//char TempStr[32];
	char* pStr;


    ShNL(bPort);
	for(i = 0; i < NUM_VARIABLES; i++)
	{
	    ShFieldOut(bPort, (char*)VarCmdTable[i].szCmdString, 0);
	    ShFieldOut(bPort, " = ", 0);

	    pStr = VarToString(i);
		ShFieldOut(bPort, pStr, 0);
	    ShNL(bPort);
	}
}


/**********************************************************************
*
* FUNCTION:		GetNumVariables
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
int GetNumVariables(void)
{

	return NUM_VARIABLES;
}

/**********************************************************************
*
* FUNCTION:		GetVariableName
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
char* GetVariableName(uint8_t idx)
{

	return (char*)VarCmdTable[idx].szCmdString;
}

/**********************************************************************
*
* FUNCTION:		GetVariableHelp
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
char* GetVariableHelp(uint8_t idx)
{

	return (char*)VarCmdTable[idx].pszHelp;
}


/**********************************************************************
*
* FUNCTION:		IsVariable
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



/**********************************************************************
*
* FUNCTION:		FindVariable
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



/**********************************************************************
*
* FUNCTION:		GetVariable
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



/**********************************************************************
*
* FUNCTION:		SetVariable
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
//CMD_BAD_NUMBER,

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
					temp = atoi(pStrValue);
					*(uint32_t*)(VarCmdTable[idx].var) = temp;

					(void)ini_putl("", szObject, (long)temp, SETTINGS_FILE);
				}
				else
				{
					return CMD_READ_ONLY;
				}
			break;

			case VAR_TYPE_TIME:
				pHr = strsep(&pStrValue, ":");
				pMn = strsep(&pStrValue, ":");
				if(pHr != NULL && pMn != NULL)
				{
					time.Hours = (uint8_t)atoi(pHr);
					time.Minutes = (uint8_t)atoi(pMn);

					pSc = strsep(&pStrValue, ":");
					if(pSc)
					{
						time.Seconds = (uint8_t)atoi(pSc);
					}
					else
					{
						time.Seconds = 0;
					}
					time.SecondFraction = 0;
					time.SubSeconds = 0;
					time.TimeFormat = RTC_HOURFORMAT12_AM;
					time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;

					HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
					// HAL_StatusTypeDef HAL_RTC_SetDate(RTC_HandleTypeDef *hrtc, RTC_DateTypeDef *sDate, uint32_t Format);
				}
			break;

			case VAR_TYPE_DATE:
				pMt = strsep(&pStrValue, "/-");
				pDa = strsep(&pStrValue, "/-");
				pYr = strsep(&pStrValue, "/-");
				if(pDa != NULL && pMt != NULL && pYr != NULL)
				{
					date.Date = (uint8_t)atoi(pDa);
					date.Month = (uint8_t)atoi(pMt);
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
				else
				{
					bTimeFmt = 1;
				}
				(void)ini_putl("", szObject, (long)bTimeFmt, SETTINGS_FILE);
			break;

			case VAR_TYPE_DATE_FMT:
				if(stricmp(pStrValue, "D/M/Y") == 0)
				{
					bDateFmt = 1;
				}
				else if(stricmp(pStrValue, "Y/M/D") == 0)
				{
					bDateFmt = 2;
				}
				else
				{
					bDateFmt = 0;
				}
				(void)ini_putl("", szObject, (long)bDateFmt, SETTINGS_FILE);
			break;

	    	case VAR_TYPE_VER:
				return CMD_READ_ONLY;
			break;

			case VAR_TYPE_ON_OFF:
				if((type & VAR_TYPE_READ_ONLY) == 0)
				{
					// ToDo - fail if the number is improperly formatted
					//temp = atoi(pStrValue);
					//*(uint32_t*)(VarCmdTable[idx].var) = temp;

					(void)ini_putl("", szObject, (long)temp, SETTINGS_FILE);
				}
				else
				{
					return CMD_READ_ONLY;
				}
			break;

			case VAR_TYPE_PORT:
				if(strcmp(pStrValue, "nce") == 0)
				{
					temp = 1;
				}
				else if(strcmp(pStrValue, "expressnet") == 0)
				{
					temp = 2;
				}
				else
				{
					temp = 0;
				}
				*(uint32_t*)(VarCmdTable[idx].var) = temp;
				(void)ini_putl("", szObject, (long)temp, SETTINGS_FILE);
			break;

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
	       	break;

	    	case VAR_TYPE_IP:
				(void)ini_puts("", szObject, pStrValue, SETTINGS_FILE);
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
					//strncpy((char*)VarCmdTable[idx].var, pStrValue, max_len);

					(void)ini_puts("", szObject, pStrValue, SETTINGS_FILE);
				}
				else
				{
					return CMD_READ_ONLY;
				}
			break;
		}
	}
	return CMD_OK;
}













//char szResponseString[64];

#ifdef NOT_USED
//typedef union
//{
//	float	f;
//	int		i;
//} USER_VARIABLE;
//
//#define NUM_USER_VARIABLES 4
//USER_VARIABLE UserVariables[NUM_USER_VARIABLES];

// form 1	/object/read
// form 2	/object/write argument


char* ParseRpcCommand(char* pBuffer)
{
	char* pToken;
	int i;

	szResponseString[0] ='\0';

	// the first character must be a '/'
	//if(*pBuffer++ == '/')
	{
		pToken = strsep(&pBuffer, TokenDelimitor);

		if(pToken != NULL)
		{
			for(i = 0; i < NUM_COMMANDS; i++)
			{
				if(strcmp(pToken, VarCmdTable[i].szCmdString) == 0)
				{
					// found command string
					strcpy(szResponseString ,pToken);
//					(*RpcCmdTable[i].pCmdFunction)(pBuffer);
					break;
				}
			}
		}
	}
	return szResponseString;
}


void BuildResponseString(const char* szSubObject, const char* szMethod, char* szValue)
{

	if(szSubObject)
	{
		//strcat(szResponseString, "/");
		strcat(szResponseString, szSubObject);
	}
	if(szMethod)
	{
		//strcat(szResponseString, "/");
		// swap "read" <--> "write"
		//if(strcmp(szMethod, ReadString) == 0)
		//{
		//	strcat(szResponseString, WriteString);
		//}
		//else if(strcmp(szMethod, WriteString) == 0)
		//{
		//	strcat(szResponseString, VerifyString);
		//}
		//else
		//{
			// not "read" or "write", used the original method
		//	strcat(szResponseString, szMethod);
		//}
	}

	strcat(szResponseString, " = ");
	strcat(szResponseString, szValue);
	strcat(szResponseString, "\r");
}

void BuildResponseStringNum(const char* szSubObject, const char* szMethod, int n)
{
	char szTemp[12];
	if(szSubObject)
	{
		//strcat(szResponseString, "/");
		strcat(szResponseString, szSubObject);
	}
	if(szMethod)
	{
		//strcat(szResponseString, "/");
		// swap "read" <--> "write"
		//if(strcmp(szMethod, ReadString) == 0)
		//{
		//	strcat(szResponseString, WriteString);
		//}
		//else if(strcmp(szMethod, WriteString) == 0)
		//{
		//	strcat(szResponseString, VerifyString);
		//}
		//else
		//{
		//	// not "read" or "write", used the original method
		//	strcat(szResponseString, szMethod);
		//}
	}

	strcat(szResponseString, " = ");

	sprintf(szTemp, "%d\r", n);
	strcat(szResponseString, szTemp);

//	strcat(szResponseString, szValue);
//	strcat(szResponseString, "\r");
}

//	/object[/sub-object]/error code [value]<cr>
void BuildErrorString(const char* szSubObject, const int iCode, char* szValue)
{
	char szTemp[10];

	if(szSubObject)
	{
		//strcat(szResponseString, "/");
		strcat(szResponseString, szSubObject);
	}

	strcat(szResponseString, "error ");

	sprintf(szTemp, "%d", iCode);
	strcat(szResponseString, szTemp);

	if(szValue)
	{
		strcat(szResponseString, " ");
		strcat(szResponseString, szValue);
	}

	strcat(szResponseString, "\r");
}
#endif

#ifdef NOT_USED
static int rpcFunctionMode(char* szBuffer)
{
	int iTemp;
	char* pToken;

	pToken = strsep(&szBuffer, TokenDelimitor);

	if(strcmp(pToken, WriteString) == 0)
	{
		iTemp = atoi(szBuffer);
		bfFMode = iTemp;
		//BuildResponseString(NULL, pToken, itoa(iTemp));
		BuildResponseStringNum(NULL, pToken, iTemp);
		return true;
	}
	else
	//else if(strcmp(pToken, ReadString) == 0)
	{
		iTemp = bfFMode;
		//BuildResponseString(NULL, pToken, itoa(iTemp));
		BuildResponseStringNum(NULL, pToken, iTemp);
		return true;
	}
	BuildErrorString(NULL, RPC_ERROR_BAD_METHOD, NULL);
	return false;
}


static int rpcUser(char* szBuffer)
{
	int iTemp;
	int iUserIndex;
	char* pIndex;
	char* pToken;

	pIndex = strsep(&szBuffer, TokenDelimitor);
	iUserIndex = *(char*)pIndex - 0x30;
	if(iUserIndex < 0 || iUserIndex >= NUM_USER_VARIABLES)
	{
		//BuildErrorString(NULL, RPC_ERROR_BAD_INDEX, itoa(iUserIndex, 10));
		BuildResponseStringNum(NULL, NULL, iUserIndex);
		return false;
	}

	pToken = strsep(&szBuffer, TokenDelimitor);

	if(strcmp(pToken, WriteString) == 0)
	{
		iTemp = atoi(szBuffer);
		UserVariables[iUserIndex].i = iTemp;
		//BuildResponseString(pIndex, pToken, itoa(iTemp, 10));
		BuildResponseStringNum(NULL, NULL, iTemp);
		return true;
	}
	else
	//else if(strcmp(pToken, ReadString) == 0)
	{
		iTemp = UserVariables[iUserIndex].i;
		//BuildResponseString(pIndex, pToken, itoa(iTemp, 10));
		BuildResponseStringNum(NULL, NULL, iTemp);
		return true;
	}
	BuildErrorString(NULL, RPC_ERROR_BAD_METHOD, NULL);
	return false;
}


static int rpcSchedule(char* szBuffer)
{
	char* pFilename;
	unsigned long lTime;
	SCH_DAY_MASK bDOW;
	SCH_ACTION bAction;
	SCH_FLAGS bFlags;
	//float fValue1;
	//float fValue2;
	unsigned long lValue1;
	unsigned long lValue2;
	char* pToken;

	pToken = strsep(&szBuffer, TokenDelimitor);
	if(strcmp(pToken, "add") == 0)
	{
		// get the file name from the buffer
		pFilename = strsep(&szBuffer, TokenDelimitor);
		if(*pFilename == '*')
		{
//			pFilename = Settings.ScheduleFilename;
		}

		// get time from buffer
		pToken = strsep(&szBuffer, TokenDelimitor);
		lTime = atoi(pToken);

		// get DOW from buffer
		pToken = strsep(&szBuffer, TokenDelimitor);
		bDOW = (SCH_DAY_MASK)atoi(pToken);

		// get ACTION from buffer
		pToken = strsep(&szBuffer, TokenDelimitor);
		bAction = (SCH_ACTION)atoi(pToken);

		// get FLAGS from buffer
		pToken = strsep(&szBuffer, TokenDelimitor);
		bFlags = (SCH_FLAGS)atoi(pToken);

		// get the first value from buffer
		pToken = strsep(&szBuffer, TokenDelimitor);
		if(bAction == SCH_SCRIPT || bAction == SCH_SCHEDULE)
		{
//			SetScheduleString(pFilename, lTime, bDOW, bAction, bFlags, pToken);
		}
		else
		{
			// get Values from buffer
			if(bFlags & SCH_VALUE_TYPE)
			{
				lValue1 = atoi(pToken);
				pToken = strsep(&szBuffer, TokenDelimitor);
				lValue2 = atoi(pToken);
//				SetScheduleLong(pFilename, lTime, bDOW, bAction, bFlags, lValue1, lValue2);
			}
			else
			{
//				fValue1 = atof(pToken);
				pToken = strsep(&szBuffer, TokenDelimitor);
//				fValue2 = atof(pToken);
//				SetScheduleFloat(pFilename, lTime, bDOW, bAction, bFlags, fValue1, fValue2);
			}
		}
		//BuildResponseString(NULL, NULL, itoa(1));
		BuildResponseStringNum(NULL, NULL, 1);
		return true;
	}
	else if(strcmp(pToken, "delete") == 0)
	{
		// get filename from buffer
		pToken = strsep(&szBuffer, TokenDelimitor);

//k		RemoveSchedule(pToken);
		//BuildResponseString(NULL, NULL, itoa(1));
		BuildResponseStringNum(NULL, NULL, 1);
		return true;
	}
	else if(strcmp(pToken, "remove") == 0)
	{
		// get key from buffer
		pToken = strsep(&szBuffer, TokenDelimitor);

		//RemoveFromSchedule(Schedule* pSchedule)
		//BuildResponseString(NULL, NULL, itoa(1));
		BuildResponseStringNum(NULL, NULL, 1);
		return true;
	}

	BuildErrorString(NULL, RPC_ERROR_BAD_METHOD, NULL);
	return false;
}

static int rpcSetRtc(char* szBuffer)
{
	//int iTemp;
	char* pToken;
//	RTC_t ts;
// 	time_t time;
// 	struct tm timeinfo;

	pToken = strsep(&szBuffer, TokenDelimitor);

	//if(strcmp(pToken, WriteString) == 0)
	{

//		time = (time_t)atoi(szBuffer);
//		gmtime_r(&time, &timeinfo);

//		ts.year = timeinfo.tm_year + 1900;
//		ts.month = timeinfo.tm_mon + 1;
//		ts.mday = timeinfo.tm_mday;
//		ts.wday = timeinfo.tm_wday;
//		ts.dst = 0;
//		if(timeinfo.tm_hour)
//		{
//			ts.hour = timeinfo.tm_hour - 1;
//		}
//		else
//		{
//			ts.hour = timeinfo.tm_hour;
//		}
//		ts.min = timeinfo.tm_min;
//		ts.sec = timeinfo.tm_sec;

//		rtc_settime(&ts);

//		LiveData.sShortTimeStamp = 0;
//		LiveData.lDataStartTime = 0;		// force the log code to re-initialize this
//		LiveData.StatusBits.bfPowerup = 0;	// time has been set
//	    LiveData.lSysTime = time;
//		WriteTimestamp();

//		WriteEventShort(EV_CLOCK_CHANGE, (unsigned short)time, 0);

		//BuildResponseString(NULL, pToken, itoa(LiveData.lSysTime));
//		BuildResponseStringNum(NULL, pToken, LiveData.lSysTime);
		return true;
	}
	//else if(strcmp(pToken, ReadString) == 0)
	{
//		rtc_gettime(&ts);

		//BuildResponseString(NULL, pToken, itoa(LiveData.lSysTime));
//		BuildResponseStringNum(NULL, pToken, LiveData.lSysTime);
		return true;
	}
	BuildErrorString(NULL, RPC_ERROR_BAD_METHOD, NULL);
	return false;
}
#endif




