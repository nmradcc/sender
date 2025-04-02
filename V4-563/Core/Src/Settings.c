/*
 * Settings.c
 *
 *  Created on: Aug 9, 2011
 *      Author: Karl
 */

#include "main.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "Settings.h"
#include "ObjectNames.h"
#include "Variables.h"
//#include "Track.h"

#include "minini.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define PERSIST_HOLDOFF	(30)		// time to wait to write after first change comes in
#define PERSIST_DELAY	(10 * 60)	// minimum time between writes

/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

extern char* strsep(char **stringp, const char *delim);

int VarUpdate(const mTCHAR *Section, const mTCHAR *Key, const mTCHAR *Value, const void *UserData);

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

//static int bfPersistDirty;
//static int iPersistDelayCtr;
//static FIL fp;

/**********************************************************************
*
*							CODE
*
**********************************************************************/

/************************************************************************
* GetSettings
*
*  parameters:
*
*  returns:
*
*  description:
*
*************************************************************************/
int GetSettings(void)
{
	int i;
	int j;
	long lTemp;
	char szTemp[64];
	char* pStr;
	char* pOct;


#ifdef USE_MININI
	for(i = 0; i < NUM_VARIABLES; i++)
	{

		if(VarCmdTable[i].type & VAR_TYPE_PERSIST)
		{

			switch(VarCmdTable[i].type & VAR_TYPE_MASK)
			{
				case VAR_TYPE_INT:
				case VAR_TYPE_12_24:
				case VAR_TYPE_ON_OFF:
				case VAR_TYPE_PORT:
					if(GetProfileInt(VarCmdTable[i].szCmdString, NULL, &iTemp))
					{
						sprintf(szTemp, "%d", iTemp);
						SetVariable(VarCmdTable[i].szCmdString, szTemp);
					}
					else
					{
						SetVariable(VarCmdTable[i].szCmdString, VarCmdTable[i].pszDefault);
					}
				break;

				case VAR_TYPE_IP:
				case VAR_TYPE_STRING:
					if(GetProfileString(VarCmdTable[i].szCmdString, NULL, szTemp, sizeof(szTemp)))
					{
						SetVariable(VarCmdTable[i].szCmdString, szTemp);
					}
					else
					{
						SetVariable(VarCmdTable[i].szCmdString, (char*)VarCmdTable[i].pszDefault);
					}
				break;
			}
		}
	}
#endif

	for(i = 0; i < NUM_VARIABLES; i++)
	{

		if(VarCmdTable[i].type & VAR_TYPE_PERSIST)
		{

			switch(VarCmdTable[i].type & VAR_TYPE_MASK)
			{
				case VAR_TYPE_INT:
				case VAR_TYPE_TIME_FMT:
				case VAR_TYPE_DATE_FMT:
				case VAR_TYPE_ON_OFF:
				case VAR_TYPE_PORT:
				case VAR_TYPE_THEME:
					lTemp = atoi(VarCmdTable[i].pszDefault);
					lTemp = ini_getl("", VarCmdTable[i].szCmdString, lTemp, SETTINGS_FILE);

					*(uint32_t*)(VarCmdTable[i].var) = lTemp;
				break;

		    	case VAR_TYPE_TRACK_IDLE:
					lTemp = atoi(VarCmdTable[i].pszDefault);
					lTemp = ini_getl("", VarCmdTable[i].szCmdString, lTemp, SETTINGS_FILE);

					*(uint32_t*)(VarCmdTable[i].var) = lTemp;
//k					SetTrackIdle(lTemp);
				break;

				case VAR_TYPE_IP:
					(void)ini_gets("", VarCmdTable[i].szCmdString, VarCmdTable[i].pszDefault, szTemp, sizeof(szTemp), SETTINGS_FILE);

					lTemp = 0;
					pStr = szTemp;
				    for(j = 0; j < 4; j++)
				    {
				    	pOct = strsep(&pStr, ".");
				    	if(pOct != NULL)
				    	{
				    		lTemp |= atoi(pOct) << j * 8;
				    	}
				    }

					*(uint32_t*)(VarCmdTable[i].var) = lTemp;
				break;

				case VAR_TYPE_STRING:
					(void)ini_gets("", VarCmdTable[i].szCmdString, VarCmdTable[i].pszDefault, szTemp, sizeof(szTemp), SETTINGS_FILE);
					strcpy((char*)VarCmdTable[i].var, szTemp);
				break;
			}
		}
	}

    return 1;
}






#ifdef NOT_USED
int VarUpdate(const mTCHAR *Section, const mTCHAR *Key, const mTCHAR *Value, const void *UserData)
{

	(void)SetVariable(Key, (char*)UserData);
}


/************************************************************************
* SavePersistantData
*
*  parameters:
*
*  returns:
*
*  description:
*
*************************************************************************/
void SaveSettings(void)
{
    int ret;
	int i;


    if(GetPersistDirty())
    {
		ClrPersistDirty();

		// Create if dataFile does not exist
		ret = f_open(&fp, DATA_FILE, FA_WRITE | FA_CREATE_ALWAYS);
		if(ret == FR_OK)
		{
			for(i = 0; i < NUM_VARIABLES; i++)
			{
				if(VarCmdTable[i].type & VAR_TYPE_PERSIST)
				{
					switch(VarCmdTable[i].type & VAR_TYPE_MASK)
					{
						case VAR_TYPE_INT:
						case VAR_TYPE_12_24:
						case VAR_TYPE_ON_OFF:
						case VAR_TYPE_PORT:
							WriteProfileInt(&fp, VarCmdTable[i].szCmdString, NULL, *(unsigned int*)(VarCmdTable[i].var));
						break;

						case VAR_TYPE_IP:
						case VAR_TYPE_STRING:
							WriteProfileString(&fp, VarCmdTable[i].szCmdString, NULL, VarToString(i));
						break;
					}
				}
			}
			(void)f_close(&fp);
		}
	}
}


void SettingsTask(void *argument)
{

	while(1)
	{
		SaveSettings();
		osDelay(1000);
	}
}


/**********************************************************************
*
* FUNCTION:		SetPersistDirty
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
void SetPersistDirty(void)
{
	bfPersistDirty = 1;
}

/**********************************************************************
*
* FUNCTION:		ClrPersistDirty
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
void ClrPersistDirty(void)
{
	bfPersistDirty = 0;
}

/**********************************************************************
*
* FUNCTION:		GetPersistDirty
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
int GetPersistDirty(void)
{
	return bfPersistDirty;
}
#endif

