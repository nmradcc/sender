/*******************************************************************************
* @file ShellFile.c
* @brief The file system commands for the shell. The command table is in Shell.c
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
#include "Shell.h"
#include "ff.h"
#include "fattime.h"
#include "BitMask.h"
#include "GetLine.h"
#include <ctype.h>

#include "ansi.h"

//*******************************************************************************
// Global Variables
//*******************************************************************************

//*******************************************************************************
// Definitions
//*******************************************************************************

//*******************************************************************************
// Static Variables
//*******************************************************************************

//*******************************************************************************
// Global Variables
//*******************************************************************************

//extern char szCWD[];

//*******************************************************************************
// Function prototypes
//*******************************************************************************

extern char* strsep(char **stringp, const char *delim);

//*******************************************************************************
// Source
//*******************************************************************************

// file system
CMD_RETURN ShDir(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShType(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShDelete(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShFormat(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShMkdir(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShRmdir(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShChdir(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShCWD(uint8_t bPort, int argc, char *argv[]);

#ifdef REF
typedef struct _DIR_ {
	FATFS*	fs;			/* Pointer to the owner file system object */
	WORD	id;			/* Owner file system mount ID */
	WORD	index;		/* Current read/write index number */
	DWORD	sclust;		/* Table start cluster (0:Static table) */
	DWORD	clust;		/* Current cluster */
	DWORD	sect;		/* Current sector */
	BYTE*	dir;		/* Pointer to the current SFN entry in the win[] */
	BYTE*	fn;			/* Pointer to the SFN (in/out) {file[8],ext[3],status[1]} */
} DIR;

typedef struct _FILINFO_ {
	DWORD	fsize;		/* File size */
	WORD	fdate;		/* Last modified date */
	WORD	ftime;		/* Last modified time */
	BYTE	fattrib;	/* Attribute */
	char	fname[13];	/* Short file name (8.3 format) */
} FILINFO;
#endif

/*********************************************************************
*
* scan_files
*
* @brief Scan the file system looking for files and directories and output them to the proper console (port) 
*
* @param	bPort - port that issued this command
*			path - directory path - absolute or relitive
*			sh - show hidden
*			col - display in color
*
* @return	file result - FR-xx
*
*********************************************************************/
/* Start node to be scanned (***also used as work area***) */
FRESULT scan_files(uint8_t bPort, char* path, _Bool sh, _Bool col)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;
    char szTemp[16];

    ShNL(bPort);
    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK)
    {
        for (;;)
        {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if ((fno.fattrib & AM_SYS) == 0)
            {

                if (sh == 0 && (fno.fattrib & AM_HID) == 0)
                {
					if (fno.fattrib & AM_DIR)
					{
						/* It is a directory */
						if(col)
						{
							TextColor(bPort, FG_Yellow, BG_Black, ATT_Bold);
							ShFieldOut(bPort, fno.fname, 15);
							TextColor(bPort, FG_White, FG_Black, ATT_Normal);
						}
						else
						{
							ShFieldOut(bPort, fno.fname, 15);
						}

						ShFieldOut(bPort, "<dir>", 10);

						fat_to_date(fno.fdate, szTemp);
						ShFieldOut(bPort, szTemp, 12);

						fat_to_time(fno.ftime, szTemp);
						ShFieldOut(bPort, szTemp, 10);

		            	strcpy(szTemp, "RHSVDA");
						for(i = 0; i < 6; i++)
						{
							if((fno.fattrib & bBitMask[i]) == 0)
							{
								szTemp[i] = '-';
							}
						}
						ShFieldOut(bPort, szTemp, 0);
						ShNL(bPort);

						//i = strlen(path);
						//sprintf(&path[i], "/%s", fno.fname);
						//res = scan_files(bPort, path);                    /* Enter the directory */
						////if (res != FR_OK) break;
						//path[i] = 0;
					}
					else
					{
						/* It is a file. */
						if(col)
						{
				           	TextColor(bPort, FG_Cyan, BG_Black, ATT_Bold);
							ShFieldOut(bPort, fno.fname, 15);
					        TextColor(bPort, FG_White, FG_Black, ATT_Normal);
						}
						else
						{
							ShFieldOut(bPort, fno.fname, 15);
						}

						ShFieldNumberOut(bPort, "", fno.fsize, 10);

						fat_to_date(fno.fdate, szTemp);
						ShFieldOut(bPort, szTemp, 12);

						fat_to_time(fno.ftime, szTemp);
						ShFieldOut(bPort, szTemp, 10);

		            	strcpy(szTemp, "RHSVDA");
						for(i = 0; i < 6; i++)
						{
							if((fno.fattrib & bBitMask[i]) == 0)
							{
								szTemp[i] = '-';
							}
						}
						ShFieldOut(bPort, szTemp, 0);
						ShNL(bPort);
				   }
                }
            }
        }
        f_closedir(&dir);
    }

    return res;
}

/*********************************************************************
*
* ShDir
* @catagory	Shell Command
*
* @brief	FileSystem Directory shell command 
*
* @param	bPort - port that issued this command
*			argc - argument country
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShDir(uint8_t bPort, int argc, char *argv[])
{
    char buff[256];
	_Bool ShowHidden = 0;
	_Bool color = 0;
	int i;
	int argidx = 1;

	// dir -c -h <dir>
	// dir <dir> -c -h

	for(i = 1; i < argc; i++)
	{
		if(stricmp(argv[i], "-h") == 0)
		{
			ShowHidden = 1;
			if(argidx == i)
			{
				argidx++;
			}
			argc--;
		}
		if(stricmp(argv[i], "-c") == 0)
		{
			color = 1;
			if(argidx == i)
			{
				argidx++;
			}
			argc--;
		}
	}

    if(argc == 1)
    {
		f_getcwd(buff, sizeof(buff));
	}
    else if(argc == 2)
    {
		if(argv[argidx][0] != '/' && argv[argidx][0] != '\\')
		{
			f_getcwd(buff, sizeof(buff));
			strcat(buff, argv[argidx]);
		}
		else
		{
			strcpy(buff, argv[argidx]);
		}
    }
	else
	{
		return CMD_BAD_PARAMS;
	}
	
	
	if(color)
	{
		TextColor(bPort, FG_White, FG_Black, ATT_Bold);
	}

	ShNL(bPort);
	ShFieldOut(bPort, "Directory of ", 0);
	ShFieldOut(bPort, buff, 0);

    ShNL(bPort);
    ShFieldOut(bPort, "Name", 15);
    ShFieldOut(bPort, "Size", 10);
    ShFieldOut(bPort, "Modified Date - Time", 22);
    ShFieldOut(bPort, "Attributes", 0);
	if(color)
	{
		TextColor(bPort, FG_White, FG_Black, ATT_Normal);
	}

	(void)scan_files(bPort, buff, ShowHidden, color);

	return CMD_OK;
}


/*********************************************************************
*
* ShCWD
* @catagory	Shell Command
*
* @brief	FileSystem Current Working Directory shell command 
*
* @param	bPort - port that issued this command
*			argc - argument country
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
* @note		This command currently does nothing
*
*********************************************************************/
CMD_RETURN ShCWD(uint8_t bPort, int argc, char *argv[])
{
    #define BUFFLEN 256
//    char buffer[BUFFLEN];
//    unsigned char ret;

    //uprintf(bPort, "\r\n\r\n");
    ShNL(bPort);

    ShFieldOut(bPort, ">", 0);

    //szCWD

    // get the current directory and print the path
//    ret = f_getcwd(buffer, BUFFLEN);
//    if (!ret)
//    {
//        //uprintf(bPort, "dir %s\r\n", buffer);
//        ShFieldOut(bPort, buffer, 0);
//        //uprintf(bPort, "\n\r\n\r");
//        ShNL(bPort);
//    }

	return CMD_OK;
}


/*********************************************************************
*
* ShType
* @catagory	Shell Command
*
* @brief	Output the text of a file shell command 
*
* @param	bPort - port that issued this command
*			argc - argument country
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShType(uint8_t bPort, int argc, char *argv[])
{
    FRESULT ret;
    char szTypeBuf[80];
    unsigned int bc;
    FIL fp;

    ShNL(bPort);



    if(argc == 2)
    {
        f_getcwd (szTypeBuf, sizeof(szTypeBuf));
    	if(szTypeBuf[strlen(szTypeBuf)-1] != '\\')
    	{
        	strcat(szTypeBuf, "\\");
    	}
    	strcat(szTypeBuf, argv[1]);

    	// open file
        ret = f_open(&fp, szTypeBuf, FA_READ);

    	if(ret == FR_OK)
    	{
    		while(1)
    		{
    			f_read(&fp, szTypeBuf, sizeof(szTypeBuf), &bc);
    			if(bc > 80)
    			{
    				bc = 80;
    			}
    			if(bc)
    			{
    				for(int i = 0; i < bc; i++)
    				{
    					ShCharOut(bPort, szTypeBuf[i]);
    				}
    			}
    			else
    			{
    				break;
    			}
    		}
            f_close(&fp);
    	}
    	else
    	{
    		ShFieldOut(bPort, "File not found.\r\n", 0);
    		return CMD_NOT_FOUND;
    	}
    }
    else
    {
    	return 	CMD_BAD_PARAMS;
    }

	return CMD_OK;
}


/*********************************************************************
*
* ShDelete
* @catagory	Shell Command
*
* @brief	FileSystem Delete File shell command 
*
* @param	bPort - port that issued this command
*			argc - argument country
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShDelete(uint8_t bPort, int argc, char *argv[])
{
	FILINFO fnfo;
	
	if(argc == 2)
	{
		if(f_stat(argv[1], &fnfo) == FR_OK)
		{
			if ((fnfo.fattrib & AM_DIR) == 0)
			{
				// make sure the argument is a file & not a directory
				if(f_unlink (argv[1]) == FR_OK)
				{
					return CMD_OK;
				}
				else
				{
					return CMD_FAILED;
				}
			}
		}
	}

	return CMD_OK;
}

/*********************************************************************
*
* ShMkdir
* @catagory	Shell Command
*
* @brief	FileSystem Make Directory shell command 
*
* @param	bPort - port that issued this command
*			argc - argument country
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShMkdir(uint8_t bPort, int argc, char *argv[])
{

	if(argc == 2)
	{
		if(f_mkdir (argv[1]) == 0)
		{
			return CMD_OK;
		}
		else
		{
			return CMD_FAILED;
		}
	}
	return CMD_BAD_PARAMS;
}


/*********************************************************************
*
* ShRmdir
* @catagory	Shell Command
*
* @brief	FileSystem Remove Directory shell command 
*
* @param	bPort - port that issued this command
*			argc - argument country
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShRmdir(uint8_t bPort, int argc, char *argv[])
{
	FILINFO fnfo;
	
	if(argc == 2)
	{
		if(f_stat(argv[1], &fnfo) == FR_OK)
		{
			if (fnfo.fattrib & AM_DIR)
			{
				// make sure the argument is a directory & not a file
				if(f_unlink (argv[1]) == FR_OK)
				{
					return CMD_OK;
				}
				else
				{
					return CMD_FAILED;
				}
			}
		}
	}
	return CMD_BAD_PARAMS;
}

/*********************************************************************
*
* ShChdir
* @catagory	Shell Command
*
* @brief	FileSystem Change Directory shell command 
*
* @param	bPort - port that issued this command
*			argc - argument country
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShChdir(uint8_t bPort, int argc, char *argv[])
{
	if(argc == 2)
	{
		if(f_chdir(argv[1]) == 0)
		{
			//strcpy(szCWD, argv[1]);
			return CMD_OK;
		}
		else
		{
			return CMD_FAILED;
		}
	}
	return CMD_BAD_PARAMS;
}


/*********************************************************************
*
* GetAtribMask
* @catagory	Shell Command
*
* @brief	Convert the attribute character (byte) into the FF bit mask
*
* @param	Attribute byte
*
* @return	File System bit mask
*
*********************************************************************/
uint8_t GetAtribMask(char l)
{

	l = toupper(l);
	switch(l)
	{
		case 'R':
			return AM_RDO;
		break;

		case 'H':
			return AM_HID;
		break;

		case 'S':
			return AM_SYS;
		break;

		case 'A':
			return AM_ARC;
		break;

		default:
			return 0;
		break;
	}
}

/*********************************************************************
*
* ShAtrib
* @catagory	Shell Command
*
* @brief	FileSystem Attribute set/clear shell command 
*
* @param	bPort - port that issued this command
*			argc - argument country
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShAtrib(uint8_t bPort, int argc, char *argv[])
{
	int c = 2;
	uint8_t att = 0;
	uint8_t mask = 0;
	char a;
    char szTemp[16];
	FILINFO fnfo;

	if(argc == 2)
	{
		if(f_stat(argv[1], &fnfo) == FR_OK)
		{
        	strcpy(szTemp, "RHSVDA");
			for(int i = 0; i < 6; i++)
			{
				if((fnfo.fattrib & bBitMask[i]) == 0)
				{
					szTemp[i] = '-';
				}
			}
			ShCharOut(bPort, ' ');
			ShFieldOut(bPort, szTemp, 0);
			ShNL(bPort);
		}
	}
	else
	{
		while(c < argc)
		{
			a = argv[c][1];
			if(argv[c][0] == '+')
			{
				att |= GetAtribMask(a);
				mask |= GetAtribMask(a);
			}
			else if(argv[c][0] == '-')
			{
				att &= ~GetAtribMask(a);	// seems redundant
				mask |= GetAtribMask(a);
			}
			// else - ignore this unknown attribute
			c++;
		}

		if(f_chmod(argv[1], att, mask) == FR_OK)
		{
			return CMD_OK;
		}

		return CMD_FAILED;
	}
}


/*********************************************************************
*
* ShCopy
* @catagory	Shell Command
*
* @brief	FileSystem Copy shell command 
*
* @param	bPort - port that issued this command
*			argc - argument country
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShCopy(uint8_t bPort, int argc, char *argv[])
{
    FIL srcfp;
    FIL destfp;
    int c;
    unsigned int byts;
    int ret;


    if(argc == 3)
    {
		ret = f_open(&srcfp, argv[1], FA_READ);
		if(ret != FR_OK)
		{
			return CMD_NOT_FOUND;
		}

		ret = f_open(&destfp, argv[2], FA_READ);
		if(ret == FR_OK)
		{
	        f_close(&destfp);
	        ShNL(bPort);
			ShFieldOut(bPort, "File already exists", 0);
			return CMD_BAD_PARAMS;
		}

		ret = f_open(&destfp, argv[2], FA_WRITE | FA_CREATE_NEW);
		if(ret != FR_OK)
		{
			return CMD_BAD_PARAMS;
		}

		while(1)
		{
			ret = f_read(&srcfp, &c, 1, &byts);
			if(byts == 0)
			{
				break;
			}
			ret = f_write(&destfp, &c, 1, &byts);
		}

        f_close(&srcfp);
        f_close(&destfp);
		return CMD_OK;
    }
    else
    {
		return CMD_BAD_PARAMS;
    }
}



