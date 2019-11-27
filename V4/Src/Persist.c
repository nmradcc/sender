/*********************************************************************
*
* SOURCE FILENAME:  Peersist.cpp
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
#include "main.h"
#include <string.h>
#include "ff.h"
#include "Persist.h"
#include "Settings.h"
#include <stdlib.h>
#include <stdio.h>

//*****************************************************************************
//Global Variables                                                        
//*****************************************************************************

//*****************************************************************************
//External Variables                                                      
//*****************************************************************************

//extern void F2A(char *szString, float fValue, int iWidth, int iPrecision);

static FIL fp;
//FIL tempFp;

/*********************************************************************
*
*                            DEFINITIONS
*
*********************************************************************/

//#define PERSIST_HOLDOFF	(30)		// time to wait to write after first change comes in
//#define PERSIST_DELAY	(10 * 60)	// minimum time between writes

/*********************************************************************
*
*                            STATIC DATA
*
*********************************************************************/

//#define DATA_FILE   "config.ini"
//#define TEMP_FILE   "temp.ini"

//static char buffer[12];

static char szFullKey[24];

/*********************************************************************
*
*                            GLOBAL DATA
*
*********************************************************************/

/*********************************************************************
*
*                            FUNCTION PROTOTYPES
*
*********************************************************************/

extern char* ItoA(int n);

extern int getLine(FIL *stream, char *buffer, int iBufLen);

/*********************************************************************
*
*                            CODE
*
*********************************************************************/

#ifdef DUPLICATE
/************************************************************************
* getLine
*
*  parameters:
*
*  returns:     the integer or zero
*
*  description: attempts to open the data file and search line-by-line for the key
*
*************************************************************************/
//uint8_t f_getc(FIL *stream)
//{
//	uint8_t c;
//	unsigned int rc;
//
//	f_read(stream, &c, 1, &rc);
//	if(rc)
//	{
//		return c;
//	}
//	else
//	{
//		return EOF;
//	}
//}

/************************************************************************
* getLine
*
*  parameters:    
*
*  returns:     the integer or zero
*
*  description: attempts to open the data file and search line-by-line for the key
*
*************************************************************************/
int getLine(FIL *stream, char *buffer, int iBufLen)
{
    //char bb[2];
    int i = 0;
    uint8_t j = 0;
    
    buffer[0] = '\0';

    j = f_getc(stream);

    while(j != (uint8_t)EOF)
    {
        if(j != '\n')
        {
            if(j != '\r')
            {
                buffer[i++] = j;
                buffer[i] = '\0';
                if(i >= iBufLen)                // truncate this at a screen-line's worth + hyperlinks
                {
                    return i;
                }
            }
            j = f_getc(stream);
        }
        else
        {
            return i;
        }
    }
    return j;
}
#endif



/************************************************************************
* MakeKey
*
*  parameters:    
*
*  returns:
*
*  description:
*
*************************************************************************/
void MakeKey(char* szFullKey, const char* szKey, const char* szSubkey)
{
	if(szSubkey)
	{
		strcpy(szFullKey, szKey);
		strcat(szFullKey, "/");
		strcat(szFullKey, szSubkey);
	}
	else
	{
		strcpy(szFullKey, szKey);
	}
}

/************************************************************************
* GetProfileInt
*
*  parameters:
*
*  returns:     the integer or zero
*
*  description: attempts to open the data file and search line-by-line for the key
*
*************************************************************************/
int GetProfileInt(const char *szKey, const char *szSubkey, int* pInt)
{
    char s[32];
    int result = 0;
    int ret;


    MakeKey(szFullKey, szKey, szSubkey);

    /* Open for read (will fail if file dataFile does not exist) */
    ret = f_open(&fp, DATA_FILE, FA_READ);
    if(ret == FR_OK)
    {
        /* Read data back from file: */      
//k        while (EOF != getLine(&fp, s, sizeof(s)))
//        while(getLine(cs->Scriptfp, cs->buffer, BUFFER_SIZE) == 0)
        while ((uint8_t)EOF != getLine(&fp, s, sizeof(s)))
        {
        	if(strlen(s) != 0)
        	{
				if(strncmp(s, szFullKey, strlen(szFullKey)) == 0 && s[strlen(szFullKey)] == '=' )
				{
					*pInt = atoi(strrchr(s, '=')+1);
					result = 1;
					break;
				}
			}
			else
			{
				break;
			}
        }

        /* Close stream */
        (void)f_close(&fp);
    }
    return result;        
}

#ifdef NO_FLOATS
/************************************************************************
* GetProfileFloat
*
*  parameters:    
*
*  returns:     the integer or zero
*
*  description: attempts to open the data file and search line-by-line for the key
*
*************************************************************************/
int GetProfileFloat(const char *szKey, const char *szSubkey, float* pFloat)
{
    char s[32];
    int result = 0;
    int ret;


    MakeKey(szFullKey, szKey, szSubkey);

    /* Open for read (will fail if file dataFile does not exist) */
    ret = f_open(&fp, DATA_FILE, FA_READ);
    if(ret == FR_OK)
    {
        /* Read data back from file: */      
        while ((uint8_t)EOF != getLine(&fp, s, sizeof(s)))
        {
            if(strncmp(s, szFullKey, strlen(szFullKey)) == 0 && s[strlen(szFullKey)] == '=' )
            {
                *pFloat = atof(strrchr(s, '=')+1);
                result = 1;
                break;    
            }
        }
        /* Close stream */
        (void)f_close(&fp);
    }
    return result;        
}
#endif

/************************************************************************
* GetProfileInt
*
*  parameters:    
*
*  returns:     the integer or zero
*
*  description: attempts to open the data file and search line-by-line for the key
*               copies the string into the supplied buffer or a NULL, returns 0 if failed
*
*************************************************************************/
int GetProfileString(const char *szKey, const char *szSubkey, char *buffer, int iMaxLength)
{
    char s[64];
    int result = 0;
    int ret;
    unsigned int bc;


    MakeKey(szFullKey, szKey, szSubkey);

    buffer[0] = '\0';
    /* Open for read (will fail if file dataFile does not exist) */
    ret = f_open(&fp, DATA_FILE, FA_READ);
    if(ret == FR_OK)
    {
		while(1)
		{
			bc = getLine(&fp, s, sizeof(s));
			if(bc == 0)
			{
				break;
			}

            if(strncmp(s, szFullKey, strlen(szFullKey)) == 0 && s[strlen(szFullKey)] == '=' )
            {
                strncpy(buffer, strrchr(s, '=')+1, iMaxLength);
                buffer[iMaxLength-1] = 0;  // force terminate
                result = strlen(buffer);
                break;    
            }
		}

        /* Read data back from file: */
        //while ((uint8_t)EOF != getLine(&fp, s, sizeof(s)))
        //{
        //    if(strncmp(s, szFullKey, strlen(szFullKey)) == 0 && s[strlen(szFullKey)] == '=' )
        //    {
        //        strncpy(buffer, strrchr(s, '=')+1, iMaxLength);
        //        buffer[iMaxLength-1] = 0;  // force terminate
        //        result = strlen(buffer);
        //        break;
        //    }
        //}
        /* Close stream */
        (void)f_close(&fp);
    }
    return result;        
}


void WriteProfileString(FIL* file, const char *szKey, const char *szSubkey, char* szString)
{
	//char s[64];

	MakeKey(szFullKey, szKey, szSubkey);

	f_puts(szFullKey, file);
	f_putc('=', file);
	f_puts(szString, file);
	f_puts("\r\n", file);
}

void WriteProfileInt(FIL* file, const char *szKey, const char *szSubkey, int iValue)
{
	char szTemp[64];

	sprintf(szTemp, "%d", iValue);
	WriteProfileString(file, szKey, szSubkey, szTemp);
}


#ifdef NO_FLOATS
void WriteProfileFloat(FIL* file, const char *szKey, const char *szSubkey, float fValue)
{
	char s[24];

	F2A(s, fValue, 6, 2);
	WriteProfileString(file, szKey, szSubkey, s);
}
#endif






#ifdef NOT_USED
/************************************************************************
* WriteProfileString
*
*  parameters:
*
*  returns:     the integer or zero
*
*  description: attempts to open the data file and search line-by-line for the key
*               copies the string into the supplied buffer or a NULL, returns 0 if failed
*
*************************************************************************/
int WriteProfileString(const char *szKey, const char *szSubkey, char *string)
{
    char s[64];
    int iKeyLen;
    int result = 0;
    int ret;
    int found = 0;
    int iReadPos;
    int iWritePos;

    MakeKey(szFullKey, szKey, szSubkey);

    /* Open for read (will fail if file dataFile does not exist) */
    ret = f_open(&fp, DATA_FILE, FA_READ);
    if(ret != FR_OK)
    {
        // Create if dataFile does not exist
		ret = f_open(&fp, DATA_FILE, FA_WRITE | FA_CREATE_NEW);
		if(ret == FR_OK)
        {
            // it's a new file, store the new key and exit
            //f_printf(&fp, "%s=%s\r\n", szFullKey, string);
            f_puts(szFullKey, &fp);
            f_putc('=', &fp);
            f_puts(string, &fp);
            f_puts("\r\n", &fp);
            (void)f_close(&fp);
            result = 1;
        }
    }
    (void)f_close(&fp);

    if(result == 0)			// key not written yet
    {
		iKeyLen = strlen(szFullKey);

	    iReadPos = 0;
	    iWritePos = 0;

		while (1)
		{
			ret = f_open(&fp, DATA_FILE, FA_READ);
			if(ret != FR_OK)
			{
				break;
			}
			f_lseek(&fp, iReadPos);
			ret = getLine(&fp, s, sizeof(s));
			//f_pos
		    iReadPos = fp.fptr;
			(void)f_close(&fp);
			if(ret == (uint8_t)EOF)
			{
				break;
			}

			if(strncmp(s, szFullKey, iKeyLen) == 0 && s[iKeyLen] == '=' )
			{
				*(strrchr(s, '=')+1) = '\0';
				strcat(s, string);
				strcat(s, "\r\n");
	            result = 1;
				found = 1;
			}
			else
			{
				strcat(s, "\r\n");
			}

			ret = f_open(&fp, TEMP_FILE, FA_WRITE);
			if(ret != FR_OK)
			{
				ret = f_open(&fp, TEMP_FILE, FA_WRITE | FA_CREATE_NEW);
				if(ret != FR_OK)
				{
					break;
				}
			}
			f_lseek(&fp, iWritePos);
			f_puts(s, &fp);
			//f_pos
			iWritePos = fp.fptr;
			(void)f_close(&fp);
		}

		if(found == 0)
		{
			// key not found, add it
			strcpy(s, szFullKey);
			strcat(s, "=");
			strcat(s, string);
			strcat(s, "\r\n");

            result = 1;

			ret = f_open(&fp, TEMP_FILE, FA_WRITE);
			if(ret == FR_OK)
			{
				f_lseek(&fp, iWritePos);
				f_puts(s, &fp);
				(void)f_close(&fp);
			}
		}

		/* Attempt to old file: */
		ret = f_unlink(DATA_FILE);
		if( ret != 0 )
		{
			result = 0;
		}

		ret = f_rename(TEMP_FILE, DATA_FILE);
		if( ret != 0 )
		{
			result = 0;
		}
    }
    return result;
}

/************************************************************************
* WriteProfileInt
*
*  parameters:
*
*  returns:     the integer or zero
*
*  description: attempts to open the data file and search line-by-line for
*               the key, when it finds it it replaces the value
*
*************************************************************************/
int WriteProfileInt(const char *szKey, const char *szSubkey, int value)
{
    return WriteProfileString(szKey, szSubkey, ItoA(value));
}


/************************************************************************
* WriteProfileFloat
*
*  parameters:
*
*  returns:     the integer or zero
*
*  description: attempts to open the data file and search line-by-line for
*               the key, when it finds it it replaces the value
*
*************************************************************************/
int WriteProfileFloat(const char *szKey, const char *szSubkey, float value)
{
    char s[24];

    F2A(s, value, 6, 2);
    return WriteProfileString(szKey, szSubkey, s);
}
#endif

