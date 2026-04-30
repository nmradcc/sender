//*******************************************************************************
//
// SOURCE FILENAME: file.c
//
// DATE CREATED:    14-Aug-2019
//
// PROGRAMMER:      Karl Kobel
//
// DESCRIPTION:     File system compatibility layer
//					Use <stdio.h>
//
// COPYRIGHT (c) 1999-2019 by K2 Engineering  All Rights Reserved.
//
//*******************************************************************************
#include "main.h"
#include "fatfs.h"
//#include <stdio.h>	- ToDo - including this cause conflicts
#include <stdarg.h>

//#include "shell.h"
extern int vsprintf(char *str, const char *format, va_list arg);
extern void ShBuffOut(uint8_t port, char* s, int len);
#define PORT3           0x04

//*******************************************************************************
// Global Variables
//*******************************************************************************

//*******************************************************************************
// Definitions
//*******************************************************************************

#define FILE	FIL

#define	SEEK_SET	0
#define	SEEK_CUR	1
#define	SEEK_END	2

//*******************************************************************************
// Static Variables
//*******************************************************************************

FIL File1;
FIL File2;
FIL File3;
FIL File4;

#define NUM_FILE_POINTERS	4
//FIL* FilArray[NUM_FILE_POINTERS] = {NULL, NULL, NULL, NULL};

typedef struct
{
	FIL* File;
	int Inuse;
} FILES;

FILES Files[NUM_FILE_POINTERS] =
{
	{&File1, 0},
	{&File2, 0},
	{&File3, 0},
	{&File4, 0}
};

//*******************************************************************************
// Global Variables
//*******************************************************************************

//*******************************************************************************
// Function prototypes
//*******************************************************************************

//*******************************************************************************
// Source
//*******************************************************************************

/**********************************************************************
*
* FUNCTION:		GetFilPointer / FreeFilePointer
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
FIL* GetFilPointer()
{
	for(int i = 0; i < NUM_FILE_POINTERS; i++)
	{
		if(Files[i].Inuse == 0)
		{
			Files[i].Inuse = 1;
			return Files[i].File;
		}
	}
	return NULL;
}

void FreeFilePointer(FIL* fp)
{
	for(int i = 0; i < NUM_FILE_POINTERS; i++)
	{
		if(Files[i].File == fp)
		{
			Files[i].Inuse = 0;
			return;
		}
	}
}


/**********************************************************************
*
* FUNCTION:		fopen
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
FILE* fopen(const char* filename, const char* mode)
{
	uint8_t m;
	FIL *fp;

	//fp = (FIL*)malloc(sizeof(FIL));
	//fp = (FIL*)pvPortMalloc(sizeof(FIL));
	fp = GetFilPointer();

	if(fp)
	{
		// r – Opens a file in read mode and sets pointer to the first character in the file. It returns null if file does not exist.
		// w – Opens a file in write mode. It returns null if file could not be opened. If file exists, data are overwritten.
		// a – Opens a file in append mode.  It returns null if file couldn’t be opened.
		// r+ – Opens a file for read and write mode and sets pointer to the first character in the file.
		// w+ – opens a file for read and write mode and sets pointer to the first character in the file.
		// a+ – Opens a file for read and write mode and sets pointer to the first character in the file. But, it can’t modify existing contents.
		// no need for the binary mode

		switch(mode[0])
		{
			default:
			case 'r':
				m = FA_READ;
				if(mode[1] == '+')
				{
					m |= FA_WRITE | FA_CREATE_NEW;
				}
			break;

			case 'w':
				m = FA_WRITE;
				if(mode[1] == '+')
				{
					m |= FA_READ | FA_CREATE_NEW;
				}
			break;

			case 'a':
				m = FA_OPEN_APPEND;
				if(mode[1] == '+')
				{
					m |= FA_CREATE_NEW;
				}
			break;

		}

		if(f_open(fp, filename, m) == FR_OK)
		{
			return fp;
		}
	}
	//vPortFree(fp);
	FreeFilePointer(fp);
	return NULL;
}


/**********************************************************************
*
* FUNCTION:		fclose
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
void fclose(FILE* fp)
{
	
	if(fp)
	{
		f_close(fp);

		if(fp)
		{
			//free(fp);
			//vPortFree(fp);
			FreeFilePointer(fp);
		}
	}
}


/**********************************************************************
*
* FUNCTION:		fread
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
size_t fread(void *ptr, size_t size_of_elements, size_t number_of_elements, FILE* fp)
{
	unsigned int br;
	if(f_read (fp, ptr, size_of_elements * number_of_elements, &br) == FR_OK)
	{
		return br;
	}
	return 0;
}
              

/**********************************************************************
*
* FUNCTION:		fwrite
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
size_t fwrite(const void *ptr, size_t size_of_elements, size_t number_of_elements, FILE* fp)
{
	unsigned int bw;
	
	if(f_write (fp, ptr, size_of_elements * number_of_elements, &bw) == FR_OK)
	{
		return bw;
	}
	return 0;
}



/**********************************************************************
*
* FUNCTION:		fseek
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
int	fseek (FILE *fp, long ofs, int pos)
{
	long fofs;

	switch(pos)
	{
		case SEEK_SET:
			fofs = f_size(fp) + ofs;
		break;

		case SEEK_CUR:
			fofs = f_tell(fp) + ofs;
		break;

		case SEEK_END:
			fofs = ofs;
		break;
	}


	return f_lseek (fp, fofs);
}


/**********************************************************************
*
* FUNCTION:		remove
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
int	remove(const char* path)
{
	return f_unlink(path);
}


/**********************************************************************
*
* FUNCTION:		rename
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
int	rename(const char* path_old, const char* path_new)
{
	return f_rename (path_old, path_new);
}



/**********************************************************************
*
* FUNCTION:		fflush
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
//int fflush(FILE* fp)
//{
//	return f_sync(fp);
//}


/**********************************************************************
*
* FUNCTION:		fprintf
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
#ifdef DUP
int	fprintf (FILE* fp, const char *fmt, ...)
{
	//return f_printf (fp, str, ...);

	va_list arg_prt;
	char responce[80];

	va_start (arg_prt, fmt);
	vsprintf(responce, fmt, arg_prt);
	va_end (arg_prt);

	//ShBuffOut(PORT3, responce, strlen(responce));
	return f_puts (responce, fp);

	//return 0;
}
#endif

/**********************************************************************
*
* FUNCTION:		fscanf
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
//int	fscanf (FILE *__restrict, const char *__restrict, ...)
//               _ATTRIBUTE ((__format__ (__scanf__, 2, 3)));


/**********************************************************************
*
* FUNCTION:		fgetc
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
//int fgetc (FILE *fp)
//{
//	return 0;
//}


/**********************************************************************
*
* FUNCTION:		fgets
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
char* fgets (char* buf, int len, FILE * fp)
{

	return f_gets(buf, len, fp);
}


/**********************************************************************
*
* FUNCTION:		fputc
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
int	fputc (int c, FILE* fp)
{
	return f_putc(c, fp);
}


/**********************************************************************
*
* FUNCTION:		fputs
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
int	fputs (const char *str, FILE *fp)
{
	return f_puts (str, fp);
}



/**********************************************************************
*
* FUNCTION:		ftell
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
long ftell(FILE *fp)
{
	return f_tell(fp);
}


/**********************************************************************
*
* FUNCTION:		rewind
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
void rewind(FILE *fp)
{
	f_rewind(fp);
}


/**********************************************************************
*
* FUNCTION:		feof
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
int feof(FILE *fp)
{
	return f_eof(fp);
}


/**********************************************************************
*
* FUNCTION:		ferror
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
int	ferror(FILE *fp)
{
	return f_error(fp);
}


/**********************************************************************
*
* FUNCTION:		getcwd
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
void getcwd(char* pPath, int len)
{
	f_getcwd (pPath, len);
}



//FRESULT f_truncate (FIL* fp);										/* Truncate the file */
//int	fgetpos (FILE *, _fpos_t *);
//int	fsetpos (FILE *, const fpos_t *);





