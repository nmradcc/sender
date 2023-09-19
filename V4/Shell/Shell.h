/*********************************************************************
*
* SOURCE FILENAME:  Shell.h
*
* DATE CREATED:     Sep 13, 2011
*
* PROGRAMMER:       Karl
*
* DESCRIPTION:      <add description>
*
* COPYRIGHT (c) 2000-2017 by K2 Engineering. All Rights Reserved.
*
*********************************************************************/
#ifndef SHELL_H_
#define SHELL_H_

// <add includes>
#include "main.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>
//#include <stdlib.h>


/*********************************************************************
*
*                            DEFINITIONS
*
*********************************************************************/

// for Shell.h
#define PORT1           0x01
#define PORT3           0x04
#define PORTT           0x80

#define ALL_PORTS   (PORT1 | PORT3 | PORTT)
#define NO_PORTS    0x00

#define PORT_MASK   0xff

#define MAX_PORTS		3

#define NO_FLAGS		0x00
#define VALUE_RETURN	0x01
#define SCRIPT_HELP		0x02
#define SUPPRESS_HELP	0x80

/** @enum CMD_RETURN
	@brief List of possible shell command return codes
 */
typedef enum
{
	CMD_OK,
	CMD_BAD_PARAMS,
	CMD_NOT_FOUND,
	CMD_FAILED,
	CMD_CANCEL_KEY,

	CMD_BAD_NUMBER,
	CMD_READ_ONLY,

	CMD_BAD_PACKET,
	CMD_BAD_BIT_FILE,

//		RUN_NORMAL,
//		RUN_IF,
//		SKIP_IF,
//		SKIP_ELSE,
//	    LOOP,
//		END_LOOP,
//		SKIP_LOOP,
//	    RUN_DELAY,

	CMD_IF_TRUE = 0x400,
	CMD_IF_FALSE = 0x800,
	CMD_ELSE = 0x900,
} CMD_RETURN;

enum
{
	CL_NONE,
	CL_ANSI,
	CL_FILE,
	CL_SCRIPT,
	CL_CS,
	CL_SYS,
	CL_TEST,
} /* SHELL_CLASS */;

/** @struct SHELL_TABLE
	@brief a shell command table entry
 */
typedef struct SHELL_TABLE_t
{
	char szCommand[11];
	unsigned char bClass;
	unsigned char bFlags;
	CMD_RETURN (*Command)(uint8_t bPort, int argc, char *argv[]);
	char* pszHelp;
} SHELL_TABLE;

#define ARG_SIZE	6
#define BUFFER_SIZE 60
#define ARR_SIZE 60

extern int bfShellTick;

//typedef byte* CMD_CONTEXT;

extern const SHELL_TABLE ShellTable[];

extern char* strsep(char **stringp, const char *delim);


/*********************************************************************
*
*                            FUNCTION PROTOTYPES
*
*********************************************************************/

void ShellTask(void *argument);
void ScriptTask(void *argument);

extern CMD_RETURN DoRun(uint8_t bPort, FIL* fp, size_t nargs, char** args);
extern void DoScriptRun(void);
extern void DoShell(void);

extern void ShellInit(void);
extern int ShellMain(uint8_t bPort, char* buuf);

extern void Prompt(uint8_t bPort);

extern int FindCommand(char* szCmdBuffer, size_t* nargs, char** args);
extern int ExecuteCommand(int iCmdIndex, uint8_t bPort, size_t nargs, char** args);

extern void ShCharOut(uint8_t port, char c);
extern void ShStringOut(uint8_t port, char* s);
extern void ShFieldOut(uint8_t bPort, char* szBuffer, int iFieldWidth);
extern void ShBuffOut(uint8_t port, char* s, int len);

extern void ShFieldNumberOut(uint8_t bPort, char* szBuffer, int number, int iFieldWidth);
extern void ShNL(uint8_t bPort);
extern void ShCR(uint8_t bPort);

#ifdef USE_NEW
extern char ShGetChar(uint8_t bPort);
#else
extern char ShGetChar(uint8_t* bPort);
#endif

extern char ShKbHit(uint8_t port);

extern unsigned int GetLoopCount(void);


#endif /* SHELL_H_ */
