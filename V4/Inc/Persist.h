/*********************************************************************
*
* SOURCE FILENAME:  Persist.h
*
* DATE CREATED:     12/Apr/2011
*
* PROGRAMMER:       K Kobel
*
* DESCRIPTION:      Persistant configuration data
*
* COPYRIGHT (c) 2011 by ZeusGrid LLC.  All Rights Reserved.
*
*********************************************************************/

#ifndef _PERSIST_H
#define _PERSIST_H

#include "ff.h"



#define DATA_FILE   "config.ini"
#define TEMP_FILE   "temp.ini"

/****************************************************************************************/
// FUNCTION PROTOTYPES
/****************************************************************************************/

extern int GetProfileInt(const char *szKey, const char *szSubkey, int* pInt);
extern int GetProfileString(const char *szKey, const char *szSubkey, char *buffer, int iMaxLength);
//extern int GetProfileFloat(const char *szKey, const char *szSubkey, float* pFloat);

extern void WriteProfileInt(FIL* file, const char *szKey, const char *szSubkey, int iValue);
extern void WriteProfileString(FIL* file, const char *szKey, const char *szSubkey, char* szString);
//extern void WriteProfileFloat(FIL* file, const char *szKey, const char *szSubkey, float fValue);

#endif
