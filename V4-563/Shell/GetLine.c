/*******************************************************************************
* @file GetLine.c
* @brief Read a character of a line from an open file
*
* @author K. Kobel
* @date 9/15/2019
* @Revision: 24 $
* @Modtime: 10/31/2019
*
* @copyright	(c) 2019  all Rights Reserved.
*******************************************************************************/
#include "main.h"
#include "fatfs.h"



/*********************************************************************
*
* f_getc
*
* @brief read a character from an open file 
*
* @param	stream - open file context
*
* @return	character
*
*********************************************************************/
uint8_t f_getc(FIL *stream)
{
	uint8_t c;
	unsigned int rc;

	f_read(stream, &c, 1, &rc);
	if(rc)
	{
		return c;
	}
	else
	{
		return EOF;
	}
}


/*********************************************************************
*
* getLine
*
* @brief read a line (terminated by newline (\n) from an open file 
*
* @param	stream - open file context
*			buffer - buffer for the line read include
*			iBufLen - maximum length
*
* @return	number of characters read
*
*********************************************************************/
int getLine(FIL *stream, char *buffer, int iBufLen)
{
    //char bb[2];
    int i = 0;
    uint8_t j = 0;
    //uint8_t GotCR = 0;

    buffer[0] = '\0';

    j = f_getc(stream);

    while(j != (uint8_t)EOF)
    {
        if(j == '\n')
        {
			return i + 1;
        }
        else if(j == '\r')
        {
        	//if(GotCR)
        	//{
    		//	return i + 1;
        	//}
        	//else
        	//{
        		i++;
        	//	GotCR = 1;
        	//}
        }
        else
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
    return i;
}

int getLine2(FIL *stream, char *buffer, int iBufLen)
{
    int i = 0;
    uint8_t j = 0;

    buffer[0] = '\0';

    j = f_getc(stream);

    while(j != (uint8_t)EOF)
    {
        if(j == '\n')
        {
			return i;
        }
        else if(j == '\r')
        {
        	;
        }
        else
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
    return i;
}



