/*******************************************************************************
* @file Shell.c
* @brief The main shell module
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
#include <ctype.h>
#include <stdarg.h>

#include "Shell.h"
#include "ShellFile.h"
#include "ShellCS.h"
#include "ShellScript.h"
#include "Track.h"
//#include "Led.h"
#include "Ansi.h"
#include "Variables.h"
//#include "Clock.h"
#include "Settings.h"
#include "ymodem.h"

//#include "usbd_cdc_if.h"
//#include "VCP.h"
//k #include "Uart2.h"
//#include "Uart3.h"
#include "ff.h"
#include "GetLine.h"

#include "cmsis_os.h"

//*******************************************************************************
// Global Variables
//*******************************************************************************

//extern uint8_t DiagPort;
//extern uint8_t DebugPort;
//extern uint8_t CurrentPort;     // for those function which are called from the diagnostic program

extern UART_HandleTypeDef huart3;

//*******************************************************************************
// Definitions
//*******************************************************************************

#define SCRIPT_DEFAULT_EXTENSION	".scp"
#define PACKET_DEFAULT_EXTENSION	".pkt"


#define MAXSHELLBUFLEN		80

#define NUM_HISTORY			10

#define HELP_FIELD_WIDTH	13

//*******************************************************************************
// Static Variables
//*******************************************************************************

static uint8_t shell_index = 0;
static uint8_t shell_buf[MAX_PORTS][MAXSHELLBUFLEN];

static uint8_t HistoryBuf[MAX_PORTS][NUM_HISTORY][MAXSHELLBUFLEN];
static uint8_t HistoryIndex[MAX_PORTS];

//*******************************************************************************
// Global Variables
//*******************************************************************************

char szCWD[20];

int bfShellTick;

unsigned int ShellTemp;

//*******************************************************************************
// Function prototypes
//*******************************************************************************

extern char* strsep(char **stringp, const char *delim);

extern uint8_t VCP_GetRxChar(uint8_t* c);


extern void telnet_putc(char c);
extern void telnet_puts(char *str);
extern void telnet_putbuf(char *buf, int len);
extern void telnet_printf(const char *fmt, ...);
extern int telnet_getc(void);


extern void test_bits(void);

//*******************************************************************************
// Source
//*******************************************************************************

CMD_RETURN ShHelp(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShRem(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShDelay(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShEcho(uint8_t bPort, int argc, char *argv[]);
//CMD_RETURN ShLed(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShVariables(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShClrScreen(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShClrEOL(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShGotoXY(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShCursor(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShTextColor(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShTasks(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShTcp(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShArgs(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShTestBits(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShSend(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShBitTest(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShSendZero(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShSendOne(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShSendScopeA(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShSendScopeB(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShSendWarble(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShSendStretched(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShSendReset(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShSendHard(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShSendIdle(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShSendPacket(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShYmodem(uint8_t bPort, int argc, char *argv[]);


uint8_t ShellColor[] =
{
	FG_White,	// CL_NONE,
	FG_Yellow,	// CL_ANSI,
	FG_Blue, 	// CL_FILE,
	FG_Green,	// CL_SCRIPT,
	FG_Purple,	// CL_CS,
	FG_Cyan,	// CL_SYS,
	FG_Red,		// CL_TEST,
};


int RunScript(uint8_t bPort, char* filename, size_t nargs, char** args);


/*********************************************************************
*
* ShellTable
*
* @brief	Shell command dispatch table 
*
*********************************************************************/
const SHELL_TABLE ShellTable[] =
{
//	Cmd			Binary		Flags							Function
	{"?",		CL_SYS,		SUPPRESS_HELP,					ShHelp,				"command help -a=all -s=script -c=color"},
	{"help",	CL_SYS,		NO_FLAGS,						ShHelp,				"command help -a=all -s=script -c=color"},
    
	{"clrscr",	CL_ANSI,	NO_FLAGS,						ShClrScreen,		"clear the screen"},
	{"clreol",	CL_ANSI,	NO_FLAGS,						ShClrEOL,			"clear from the cursor to the end of the line"},
	{"gotoxy",	CL_ANSI,	NO_FLAGS,						ShGotoXY,			"x, y"},
	{"cursor",	CL_ANSI,	NO_FLAGS,						ShCursor,			"save | restore"},
	{"color",	CL_ANSI,	NO_FLAGS,						ShTextColor,		"[[[fg], bg], att]"},

// scripting
	{"if",		CL_SCRIPT,	SUPPRESS_HELP | SCRIPT_HELP, 	ShIf,				"if condition"},
	{"else",	CL_SCRIPT,	SUPPRESS_HELP | SCRIPT_HELP, 	ShElse,				"not condition"},
	{"endif",	CL_SCRIPT,	SUPPRESS_HELP | SCRIPT_HELP, 	ShEndif,			"end if"},
	{"loop",	CL_SCRIPT,	SUPPRESS_HELP | SCRIPT_HELP, 	ShLoop,				"count"},
	{"endloop",	CL_SCRIPT,	SUPPRESS_HELP | SCRIPT_HELP, 	ShEndLoop,			"end loop"},
	{"break",	CL_SCRIPT,	SUPPRESS_HELP | SCRIPT_HELP, 	ShBreak,			"break out of loop"},
	{"variables",CL_SCRIPT,	NO_FLAGS | SCRIPT_HELP,			ShVariables,		"list all variables"},
	{"pause",	CL_SCRIPT,	SUPPRESS_HELP | SCRIPT_HELP, 	ShDelay,			"ms"},
	{"echo",	CL_SCRIPT,	NO_FLAGS | SCRIPT_HELP,			ShEcho,				"test"},
	{"rem",		CL_SCRIPT,	SUPPRESS_HELP | SCRIPT_HELP,	ShRem,				"ignore line"},
	{"prompt",	CL_SCRIPT,	SUPPRESS_HELP | SCRIPT_HELP,	ShPrompt,			"issue a prompt"},
//	{"led",		CL_SCRIPT,	NO_FLAGS | SCRIPT_HELP,			ShLed,				"Green LED on|off|blink|hex32"},
	{"script",	CL_SCRIPT,	NO_FLAGS | SCRIPT_HELP,			ShScripts,			"<name> pause|resume|kill / List the running scripts"},

// file system
	{"dir",		CL_FILE,	NO_FLAGS,						ShDir,				"list the files on the drive -h=show hidden -c=color"},
	{"type",	CL_FILE,	NO_FLAGS,						ShType,				"display the contents of a file"},
	{"del",		CL_FILE,	NO_FLAGS,						ShDelete,			"delete a file"},
//	{"format",	CL_FILE,	SUPPRESS_HELP, 					ShFormat,			"format the storage"},
	{"md",	    CL_FILE,	NO_FLAGS, 						ShMkdir,			"make directory"},
	{"rd",	    CL_FILE,	SUPPRESS_HELP, 					ShRmdir,			"remove directory"},
	{"cd",	    CL_FILE,	NO_FLAGS, 						ShChdir,			"change directory"},
	{"cwd",	    CL_FILE,	SUPPRESS_HELP, 					ShCWD,				"change working directory"},
	{"atrib",   CL_FILE,	NO_FLAGS, 						ShAtrib,			"Set/Reset attributes +/- R,H,S,A"},
	{"copy",    CL_FILE,	NO_FLAGS,	 					ShCopy,				"copy source destination"},

	{"args",    CL_SYS,		SUPPRESS_HELP, 					ShArgs,				"List arguments"},
	{"tasks",   CL_SYS,		NO_FLAGS, 						ShTasks,			"Task List"},
//	{"tcp",   	CL_SYS,		NO_FLAGS, 						ShTcp,				"TCP/IP Info"},

	// command station
	{"cab",	    CL_CS,		NO_FLAGS,						ShCabStat,			""},
	{"loco",    CL_CS,		NO_FLAGS,						ShLocoStat,			""},
	{"assign",  CL_CS,		SUPPRESS_HELP, 					ShAssign,			""},
	{"status",  CL_CS,		SUPPRESS_HELP, 					ShSystemStatus,		""},
	{"train", 	CL_CS,		NO_FLAGS, 						ShSetLoco,			"<address> [[[[<speed>] <direction 0/1>] <function1>] <function2>]"},
	{"disp",	CL_CS,		NO_FLAGS,						ShCabDisplay,		"<cab> ""Massage"""},
	{"write",	CL_CS,		NO_FLAGS,						ShProgTrackWriteCV,	"CV, Value"},
	{"read",	CL_CS,		NO_FLAGS,						ShProgTrackReadCV,	"CV"},


//	{"test",   	0x00,		NO_FLAGS, 						ShTestBits,			"DCC Bit Test"},
	{"send",   	CL_TEST,	NO_FLAGS, 						ShSend,				"DCC Decoder Tests - type send -? for more info"},
	{"bittest",	CL_TEST,	NO_FLAGS, 						ShBitTest,			"DCC Bit Test"},

	{"sendzero",CL_TEST,	NO_FLAGS, 						ShSendZero,			"DCC Zero Packet(s) [count]"},
	{"sendone",	CL_TEST,	NO_FLAGS, 						ShSendOne,			"DCC one Packet(s) [count]"},
	{"scopea",	CL_TEST,	NO_FLAGS, 						ShSendScopeA,		"DCC Scope A Packet(s) [count]"},
	{"scopeb",	CL_TEST,	NO_FLAGS, 						ShSendScopeB,		"DCC Scope B Packet(s) [count]"},
	{"warble",	CL_TEST,	NO_FLAGS, 						ShSendWarble,		"DCC Warble Packet(s) [count]"},
	{"stretched",CL_TEST,	NO_FLAGS, 						ShSendStretched,	"DCC Stretched Packet(s) [count]"},
	{"reset",	CL_TEST,	NO_FLAGS, 						ShSendReset,		"DCC Reset Packet(s) [count]"},
	{"hard",	CL_TEST,	NO_FLAGS, 						ShSendHard,			"DCC Hard Reset Packet(s) [count]"},
	{"idle",	CL_TEST,	NO_FLAGS, 						ShSendIdle,			"DCC Idle Packet(s) [count]"},
	{"packet",	CL_TEST,	NO_FLAGS, 						ShSendPacket,		"DCC Packet(s) [file &| count]"},

	{"ymodem",	CL_SYS,		NO_FLAGS, 						ShYmodem,			"YModem receive"},
};
#define SHELL_TABLE_COUNT (sizeof(ShellTable) / sizeof(SHELL_TABLE))


/*********************************************************************
*
* ShCharOut
*
* @brief	Output a character to the console(s) 
*
* @param	bPort - port or ports to output
*			c - character to output
*
* @return	None
*
*********************************************************************/
void ShCharOut(uint8_t port, char c)
{

    if(port & PORT1)
    {
//		VCP_write ((uint8_t*)&c, 1);
    }

    if(port & PORT3)
    {
    	HAL_UART_Transmit(&huart3, (uint8_t*)&c, 1, 100);
    }

    if(port & PORTT)
    {
//    	telnet_putc(c);
    }
}

/*********************************************************************
*
* ShBuffOut
*
* @brief	Output a buffer to the console(s) 
*
* @param	bPort - port or ports to output
*			s - pointer to buffer to output
*			len - number of characters
*
* @return	None
*
*********************************************************************/
void ShBuffOut(uint8_t port, char* s, int len)
{
//    char buf[80];
//    int i = 0;

//    if(1)	// newline options
//    {
//    	while(*s)
//    	{
//    		if(*s == '\n')
//    		{
//    			buf[i] = '\r';
//    			i++;
//    		}
//    		buf[i] = *s++;
//			i++;
//    	}
//		buf[i] = 0;
//    }

	if(port & PORT1)
    {
//		VCP_write ((uint8_t*)s, len);
    }

    if(port & PORT3)
    {
    	HAL_UART_Transmit(&huart3, (uint8_t*)s, len, 100);
    }

    if(port & PORTT)
    {
//    	telnet_putbuf(s, len);
    }
}

/*********************************************************************
*
* ShStringOut
*
* @brief	Output a null terminated buffer to the console(s) 
*
* @param	bPort - port or ports to output
*			s - pointer to string to output
*
* @return	None
*
*********************************************************************/
void ShStringOut(uint8_t port, char* s)
{

	ShBuffOut(port, s, strlen(s));
}

/*********************************************************************
*
* ShFieldOut
*
* @brief	The main aligned output for shell commands 
*
* @param	bPort - port or ports to output
*			szBuffer - string to output
*			iFieldWidth - pad to this width
*
* @return	None
*
*********************************************************************/
void ShFieldOut(uint8_t port, char* szBuffer, int iFieldWidth)
{
	int iLen;

	iLen = strlen(szBuffer);

	ShBuffOut(port, szBuffer, iLen);

	while(iLen < iFieldWidth)
	{
        ShCharOut(port, ' ');
		iFieldWidth--;
	}
}

/*********************************************************************
*
* ShCharOut
*
* @brief	The main aligned output of numbers for shell commands, with pre-string 
*
* @param	bPort - port or ports to output
*			szBuffer - string to output
*			number - int covertet to a string to output
*			iFieldWidth - pad to this width
*
* @return	None
*
*********************************************************************/
void ShFieldNumberOut(uint8_t bPort, char* szBuffer, int number, int iFieldWidth)
{
	char buf[80];
	char num[10];

	strcpy(buf, szBuffer);
	itoa(number, num, 10);

	if(strlen(buf))
	{
		ShFieldOut(bPort, buf, iFieldWidth-strlen(num));
		ShFieldOut(bPort, num, 0);
	}
	else
	{
		ShFieldOut(bPort, num, iFieldWidth);
	}
}


/*********************************************************************
*
* ShNL
*
* @brief	Output a new line to the console(s) 
*
* @param	bPort - port or ports to output
*
* @return	None
*
*********************************************************************/
void ShNL(uint8_t bPort)
{

    ShCharOut(bPort, '\r');
    ShCharOut(bPort, '\n');
}


/*********************************************************************
*
* shprintf
*
* @brief	Output a 'printf' formatted string to the console(s) 
*
* @param	bPort - port or ports to output
*			fmt - format string (See printf)
*
* @return	None
*
*********************************************************************/
void shprintf(uint8_t port, const char *fmt, ... )
{
    va_list arg_prt;
    char responce[80];

    if(port & PORTT)
    {
//        telnet_printf(fmt, ...);
    }
    else
    {
		va_start (arg_prt, fmt);
		vsprintf(responce, fmt, arg_prt);
		va_end (arg_prt);

		ShBuffOut(port, responce, strlen(responce));
    }
}


/*********************************************************************
*
* ShGetChar
*
* @brief	Get a character from the console(s) 
*
* @param	bPort - port that issued this command
*
* @return	None
* @remarks	If multiple consoles are specified, the first received char is returned
*
*********************************************************************/
char ShGetChar(uint8_t port)
{
	uint8_t c;
	
    if(port & PORT1)
    {
//		if(VCP_GetRxChar(&c))
//		{
//			return c;
//		}
    	return 0;
    }

    if(port & PORT3)
    {
    	if(HAL_UART_Receive(&huart3, &c, 1, 0) == HAL_OK)
    	{
    	   	return c;
    	}
    }


    if(port & PORTT)
    {
//    	c = telnet_getc();
    	{
    		return c;
    	}
    }

	return 0;
}

/*********************************************************************
*
* ShKbHit
*
* @brief	Ceck to see if a key is hit on a concole 
*
* @param	bPort - port that issued this command
*
* @return	0 - no key hit, 1 if hit
*
*********************************************************************/
char ShKbHit(uint8_t port)
{

    if(port & PORT1)
    {
//		if(VCP_KbHit())
		{
			return 0;
		}
    }

    if(port & PORT3)
    {
//    	if(HAL_UART_Receive(&huart3, &c, 1, 0) == HAL_OK)
    	{
    	   	return 0;
    	}
    }

    if(port & PORTT)
    {
//    	c = telnet_kbhit();
    	{
    		return 0;
    	}
    }

	return 0;
}




/*********************************************************************
*
* isfloat
*
* @brief	Check a number to see if it is a float 
*
* @param	szNumStr - string
*
* @return	0 = no decimal point in string, 1 if decimal point in string
*
*********************************************************************/
int isfloat(char* szNumStr)
{
	return strchr(szNumStr, '.') != NULL;
}


/*********************************************************************
*
* OpenScript
*
* @brief	Open a script, searching the local directory and all the
* 			'path' directories and without and with the default script
* 			extension appended
*
* @param	fp - pointer to a file object
* 			path - the current path variable
* 			name - the script name
*
* @return	0 = no decimal point in string, 1 if decimal point in string
*
*********************************************************************/
FRESULT OpenScript(FIL* fp, char* path, char* name)
{
	FRESULT ret;

	char szFullName[13];
	char szFullPath[64];
	char pPath[32];
	char* pEnvPath;
	int len;

	strcpy(szFullName, name);

	// try the passed in filename
	ret = f_open(fp, szFullName, FA_READ);
	if(ret == FR_OK)
	{
		return ret;
	}
	else
	{
		// try the default extension
		strcat(szFullName, SCRIPT_DEFAULT_EXTENSION);
		ret = f_open(fp, szFullName, FA_READ);
		if(ret == FR_OK)
		{
			return ret;
		}
		else
		{
			pEnvPath = szPathVar;
			while((len = GetNextPath(pEnvPath, pPath)) != 0)
			{
				// use pPath
				strcpy(szFullPath, pPath);
				if(pPath[strlen(pPath)-1] != '/')
				{
					strcat(szFullPath, "/");
				}
				strcat(szFullPath, name);

				// open file
				ret = f_open(fp, szFullPath, FA_READ);
				if(ret == FR_OK)
				{
					return ret;
				}

				// add the default extension
				strcat(szFullPath, SCRIPT_DEFAULT_EXTENSION);

				// open file
				ret = f_open(fp, szFullPath, FA_READ);
				if(ret == FR_OK)
				{
					return ret;
				}

				pEnvPath += len+1;
			}
		}
	}

	return FR_NO_FILE;
}

/*********************************************************************
*
* ShHelp
*
* @catagory	Shell Command
* @brief	Shell help command 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result (always OK)
*
*********************************************************************/
CMD_RETURN ShHelp(uint8_t bPort, int argc, char *argv[])
{
	int i;
	int cnt = 0;
    FRESULT res;
    DIR dir;
    static FILINFO fno;
    FIL fp;
    char szTypeBuf[80];
    unsigned int bc;
	char pPath[32];
	char* pEnvPath;
	int len;

	if(argc == 2)
	{
		// help for commands
		for(i = 0; i < SHELL_TABLE_COUNT; i++)
		{
			if(strcasecmp(ShellTable[i].szCommand, argv[1]) == 0)
			{
				ShFieldOut(bPort, " - ", 0);
				ShFieldOut(bPort, (char*)ShellTable[i].pszHelp, 0);
				return CMD_OK;
			}
		}

		// help for variables
		i = FindVariable(argv[1]);
		if(i != -1)
		{
			ShFieldOut(bPort, " - ", 0);
			ShFieldOut(bPort, GetVariableHelp(i), 0);
			return CMD_OK;
		}


		// open file
		//res = f_open(&fp, argv[1], FA_READ);
		res = OpenScript(&fp, szPathVar, argv[1]);
		if(res == FR_OK)
		{
			bc = getLine(&fp, szTypeBuf, sizeof(szTypeBuf));
			if(bc != 0)
			{
				if(strncmp(szTypeBuf, "rem", 3) == 0)
				{
					ShFieldOut(bPort, " - ", 0);
					ShFieldOut(bPort, &szTypeBuf[4], 0);
				}
				else
				{
					ShFieldOut(bPort, " - No Help String", 0);
				}
			}
			else
			{
				ShFieldOut(bPort, " - No Help String", 0);
			}
			f_close(&fp);
			return CMD_OK;
		}

		if(strcasecmp("-s", argv[1]) == 0)
		{
			ShNL(bPort);

			ShFieldOut(bPort, "Script Commands:", 0);
			ShNL(bPort);
			for(i = 0; i < SHELL_TABLE_COUNT; i++)
			{
				if((ShellTable[i].bFlags & SCRIPT_HELP) != 0)
				{
					ShFieldOut(bPort, (char*)ShellTable[i].szCommand, HELP_FIELD_WIDTH);
					cnt++;
					if(cnt >= 6)
					{
						ShNL(bPort);
						cnt = 0;
					}
				}
			}
		}
		else if(strcasecmp("-a", argv[1]) == 0)
		{
			ShNL(bPort);

			ShFieldOut(bPort, "All Commands:", 0);
			ShNL(bPort);
			for(i = 0; i < SHELL_TABLE_COUNT; i++)
			{
				ShFieldOut(bPort, (char*)ShellTable[i].szCommand, HELP_FIELD_WIDTH);
				cnt++;
				if(cnt >= 6)
				{
					ShNL(bPort);
					cnt = 0;
				}
			}
		}
		else if(strcasecmp("-c", argv[1]) == 0)
		{
			ShNL(bPort);

			ShFieldOut(bPort, "All Commands:", 0);
			ShNL(bPort);
			for(i = 0; i < SHELL_TABLE_COUNT; i++)
			{
				TextColor(bPort, ShellColor[ShellTable[i].bClass], BG_Black, ATT_Bold);
				ShFieldOut(bPort, (char*)ShellTable[i].szCommand, HELP_FIELD_WIDTH);
				cnt++;
				if(cnt >= 6)
				{
					ShNL(bPort);
					cnt = 0;
				}
			}

			//TextColor(bPort, ShellColor[0], BG_Black, ATT_Bold);
			//ShFieldOut(bPort, "Default ", 0);
			ShNL(bPort);
			TextColor(bPort, ShellColor[1], BG_Black, ATT_Bold);
			ShFieldOut(bPort, "ANSI ", 0);
			TextColor(bPort, ShellColor[2], BG_Black, ATT_Bold);
			ShFieldOut(bPort, "File ", 0);
			TextColor(bPort, ShellColor[3], BG_Black, ATT_Bold);
			ShFieldOut(bPort, "Script ", 0);
			TextColor(bPort, ShellColor[4], BG_Black, ATT_Bold);
			ShFieldOut(bPort, "CS ", 0);
			TextColor(bPort, ShellColor[5], BG_Black, ATT_Bold);
			ShFieldOut(bPort, "Sys ", 0);
			TextColor(bPort, ShellColor[6], BG_Black, ATT_Bold);
			ShFieldOut(bPort, "Test ", 0);
			ShNL(bPort);

			TextColor(bPort, FG_White, BG_Black, ATT_Normal);
		}
		else
		{
			return CMD_NOT_FOUND;
		}
	}
	else
	{
		ShNL(bPort);

		ShFieldOut(bPort, "Commands:", 0);
		ShNL(bPort);
		for(i = 0; i < SHELL_TABLE_COUNT; i++)
		{
			if((ShellTable[i].bFlags & SUPPRESS_HELP) == 0)
			{
				ShFieldOut(bPort, (char*)ShellTable[i].szCommand, HELP_FIELD_WIDTH);
				cnt++;
				if(cnt >= 6)
				{
					ShNL(bPort);
					cnt = 0;
				}
			}
		}

		cnt = 0;
		ShNL(bPort);
		ShNL(bPort);
		ShFieldOut(bPort, "Variables:", 0);
		ShNL(bPort);
		for(i = 0; i < GetNumVariables(); i++)
		{
			ShFieldOut(bPort, GetVariableName(i), HELP_FIELD_WIDTH);
			cnt++;
			if(cnt >= 6)
			{
				ShNL(bPort);
				cnt = 0;
			}
		}


		cnt = 0;
		ShNL(bPort);
		ShNL(bPort);
		ShFieldOut(bPort, "Scripts:", 0);
		ShNL(bPort);


		pEnvPath = szPathVar;
		while((len = GetNextPath(pEnvPath, pPath)) != 0)
		{
			// use pPath
			res = f_opendir(&dir, pPath);                       /* Open the directory */
			if (res == FR_OK)
			{
				for (;;)
				{
					res = f_readdir(&dir, &fno);                   /* Read a directory item */
					if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
					if (!(fno.fattrib & AM_DIR))
					{
						/* It is a file. */
						int len = strlen(fno.fname) - 4;
						strcpy(szTypeBuf, &fno.fname[len]);
						if(strcmp(szTypeBuf, ".SCP") == 0)
						//if(strcmp(fno.fname, "CONFIG.INI") != 0)
						{
							ShFieldOut(bPort, fno.fname, HELP_FIELD_WIDTH);
							cnt++;
							if(cnt >= 6)
							{
								ShNL(bPort);
								cnt = 0;
							}
						}
					}
				}
				f_closedir(&dir);
			}

			pEnvPath += len+1;
		}
	}

	return CMD_OK;
}


/*********************************************************************
*
* ShRem
*
* @catagory	Shell Command
* @brief	Shell command that does nothing
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShRem(uint8_t bPort, int argc, char *argv[])
{
	return CMD_OK;
}




#ifdef MOVE_TO_VARIABLE
/*********************************************************************
*
* ShLed
*
* @catagory	Shell Command
* @brief	Shell command that does nothing
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShLed(uint8_t bPort, int argc, char *argv[])
{
	char buf[10];

	if(argc == 2)
	{
		if(strcasecmp(argv[1], "on") == 0)
		{
			StatusLed(LED_ON);
		}
		else if(strcasecmp(argv[1], "off") == 0)
		{
			StatusLed(LED_OFF);
		}
		else if(strcasecmp(argv[1], "blink") == 0)
		{
			StatusLed(LED_BLINK);
		}
		else
		{
			StatusLed(hexadecimalToDecimal(argv[1]));
		}
	}
	else
	{
		sprintf(buf, "%08x", (int)GetStatusLed());
		ShNL(bPort);
		ShFieldOut(bPort, "Green LED: ", 0);
		ShFieldOut(bPort, buf, 0);
	}

	return CMD_OK;
}
#endif

/*********************************************************************
*
* ShDelay
*
* @catagory	Shell Command
* @brief	Shell delay command 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
* @remark	The script delay (thread aware) is handled in the run script method
*
*********************************************************************/
CMD_RETURN ShDelay(uint8_t bPort, int argc, char *argv[])
{
	int iDelayCount;

	iDelayCount = atoi(argv[1]);

    osDelay(iDelayCount);

//	while(iDelayCount)
//    {
//        if(bfShellTick)
//	    {
//		    bfShellTick = 0;
//		    iDelayCount--;
//		}
//	}
	return CMD_OK;
}


/*********************************************************************
*
* ShEcho
*
* @catagory	Shell Command
* @brief	Shell echo command
* @details
*			the cr argument will add a new line
*			multi-word strings may be surronded with doulbe quotes
* @example
*			echo "Hello World" cr
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShEcho(uint8_t bPort, int argc, char *argv[])
{

	if(argc >= 2)
	{
		if(strcasecmp(argv[1], "cr") == 0)
		{
		    ShNL(bPort);
		}
		else
		{
			ShFieldOut(bPort, argv[1], 0);
		}
	}

	if(argc == 3)
	{
		if(strcasecmp(argv[2], "cr") == 0)
		{
		    ShNL(bPort);
		}
	}
	return CMD_OK;
}



/*********************************************************************
*
* ShClrScreen
*
* @catagory	Shell Command
* @brief	Clear the screen 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShClrScreen(uint8_t bPort, int argc, char *argv[])
{
    ClearScreen(bPort);
    return CMD_OK;
}

/*********************************************************************
*
* ShClrEOL
*
* @catagory	Shell Command
* @brief	Clear to the end of the line 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShClrEOL(uint8_t bPort, int argc, char *argv[])
{
    ClearEOL(bPort);
    return CMD_OK;
}

/*********************************************************************
*
* ShGotoXY
*
* @catagory	Shell Command
* @brief	Set the cursor and col (X) and row (Y)
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShGotoXY(uint8_t bPort, int argc, char *argv[])
{
    int x, y;

    if(argc == 3)
    {
        x = atoi(argv[1]);
        y = atoi(argv[2]);
        GoToXY(bPort, x, y);
        return CMD_OK;
    }
    return CMD_BAD_PARAMS;
}

/*********************************************************************
*
* ShCursor
*
* @catagory	Shell Command
* @brief	Save or restore the cursor
* @details
*			cursor save - save the current cursor position
*			cursor restore - position the cursor bake to the saved position
* @example
*			rem display the time in the uppeer right corner in yellow
*			cursor save
*			gotoxy 60 10
*			color yellow
*			time
*			color
*			cursor restore
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShCursor(uint8_t bPort, int argc, char *argv[])
{

    if(argc == 2)
    {
		if(strcmp(argv[1], "save") == 0)
		{
			Cursor(bPort, 1);
		}
		else if(strcmp(argv[1], "restore") == 0)
		{
			Cursor(bPort, 0);
		}
		else
		{
			return CMD_BAD_PARAMS;
		}
        return CMD_OK;
    }
    return CMD_BAD_PARAMS;
}

/*********************************************************************
*
* strcmpi
*
* @catagory	Shell Command
* @brief	string compare 0 case insensitive 
*
* @param	a - string a
*			b - string b
*
* @return	1 = match
*
*********************************************************************/
int strcmpi(char const *a, char const *b)
{
    for (;; a++, b++)
    {
        int d = tolower(*a) - tolower(*b);
        if (d != 0 || !*a)
            return d;
    }
    return 0;
}

/*********************************************************************
*
* GetColorOffset
*
* @brief	Get color offset based on the color string 
*
* @param	c - color string, -1 if not found
*
* @return	color index
*
*********************************************************************/
int GetColorOffset(char* c)
{
    if(strcmpi(c, "black") == 0)    return 0;
    if(strcmpi(c, "red") == 0)      return 1;
    if(strcmpi(c, "green") == 0)    return 2;
    if(strcmpi(c, "yellow") == 0)   return 3;
    if(strcmpi(c, "blue") == 0)     return 4;
    if(strcmpi(c, "purple") == 0)   return 5;
    if(strcmpi(c, "cyan") == 0)     return 6;
    if(strcmpi(c, "white") == 0)    return 7;
    return -1;
}


/*********************************************************************
*
* GetAttribute
*
* @brief	Get attribute offset based on the attribute string 
*
* @param	c - attribute string, -1 if not found
*
* @return	attribute index
*
*********************************************************************/
int GetAttribute(char* c)
{
    if(strcmpi(c, "normal") == 0)    return 0;
    if(strcmpi(c, "bold") == 0)      return 1;
    if(strcmpi(c, "underline") == 0) return 4;
    if(strcmpi(c, "blink") == 0)     return 5;
    if(strcmpi(c, "negative") == 0)  return 7;
    if(strcmpi(c, "font0") == 0)     return 10;
    if(strcmpi(c, "font1") == 0)     return 11;
    if(strcmpi(c, "font2") == 0)     return 12;
    if(strcmpi(c, "font3") == 0)     return 13;
    if(strcmpi(c, "font4") == 0)     return 14;
    if(strcmpi(c, "font5") == 0)     return 15;
    if(strcmpi(c, "font6") == 0)     return 16;
    if(strcmpi(c, "font7") == 0)     return 17;
    if(strcmpi(c, "font8") == 0)     return 18;
    if(strcmpi(c, "font9") == 0)     return 19;
    if(strcmpi(c, "normcolor") == 0) return 22;
    if(strcmpi(c, "underline2") == 0)return 24;
    if(strcmpi(c, "blinkoff") == 0)  return 25;
    if(strcmpi(c, "positive") == 0)  return 27;
    return -1;
}


/*********************************************************************
*
* @catagory	Shell Command
* ShTextColor
*
* @brief	Set the test color [[[fg] bg] att]
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShTextColor(uint8_t bPort, int argc, char *argv[])
{
    int fg, bg, att;

    if(argc == 1)
    {
        TextColor(bPort, FG_White, FG_Black, ATT_Normal);
        return CMD_OK;
    }
    else if(argc == 2)
    {
        fg = GetColorOffset(argv[1]);
        if(fg != -1)
        {
            TextColor(bPort, (FG_COLOR)FG_Black+fg, (BG_COLOR)FG_Black+fg, ATT_Normal);
            return CMD_OK;
        }
    }
    else if(argc == 3)
    {
        fg = GetColorOffset(argv[1]);
        if(fg != -1)
        {
            bg = GetColorOffset(argv[2]);
            if(bg != -1)
            {
            	TextColor(bPort, (FG_COLOR)FG_Black+fg, (BG_COLOR)BG_Black+bg, ATT_Normal);
            	return CMD_OK;
            }
            else
            {
                att = GetAttribute(argv[2]);
                if(att != -1)
                {
                	TextColor(bPort, (FG_COLOR)FG_Black+fg, (BG_COLOR)BG_Black, (ATTRIBUTE)ATT_Normal+att);
                	return CMD_OK;
                }
            }
        }
    }
    else if(argc == 4)
    {
        fg = GetColorOffset(argv[1]);
        bg = GetColorOffset(argv[2]);
        att = GetAttribute(argv[3]);
        if(fg != -1 && bg != -1 && att != -1)
        {
            TextColor(bPort, (FG_COLOR)FG_Black+fg, (BG_COLOR)BG_Black+bg, (ATTRIBUTE)ATT_Normal+att);
            return CMD_OK;
        }
    }
    return CMD_BAD_PARAMS;
}


/*********************************************************************
*
* ShVariables
*
* @catagory	Shell Command
* @brief	List all of the system variables 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShVariables(uint8_t bPort, int argc, char *argv[])
{

	ShowVariables(bPort);
	return CMD_OK;
}


/*********************************************************************
*
* ShTasks
*
* @brief	List the OS tasks 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShTasks(uint8_t bPort, int argc, char *argv[])
{

	osThreadId_t ThreadIds[20];
	uint32_t Threads;
	uint32_t State;
	char Status[2];


	// print the header
	ShNL(bPort);
	ShFieldOut(bPort, "Name", 16);
	ShFieldOut(bPort, "ST", 3);
	ShFieldOut(bPort, "Pri", 6);
	ShFieldOut(bPort, "Stack", 6);
	ShFieldOut(bPort, "Space", 6);
	ShNL(bPort);

	Threads = osThreadEnumerate (ThreadIds, 20);

	for(int i = 0; i < Threads; i++)
	{
		// print the name
		ShFieldOut(bPort, (char*)osThreadGetName (ThreadIds[i]), 16);


		// print the status
		State = osThreadGetState (ThreadIds[i]);
		Status[1] = 0;
		switch( State )
		{
			case osThreadInactive:
				Status[0] = 'I';
			break;

			case osThreadReady:
				Status[0] = 'X';
			break;

			case osThreadRunning:
				Status[0] = 'R';
			break;

			case osThreadBlocked:
				Status[0] = 'B';
			break;

			case osThreadTerminated:
				Status[0] = 'T';
			break;

			case osThreadError:
				Status[0] = 'E';
			break;

			default:			/* Should not get here, but it is included to prevent static checking errors. */
				Status[0] = 0;
			break;
		}
		ShFieldOut(bPort, Status, 3);

		// print the Priority
		ShFieldNumberOut(bPort, "", osThreadGetPriority (ThreadIds[i]), 6);

		// print the Stack
		ShFieldNumberOut(bPort, "", osThreadGetStackSize (ThreadIds[i]), 6);

		// print the Space
		ShFieldNumberOut(bPort, "", osThreadGetStackSpace (ThreadIds[i]), 6);

		ShNL(bPort);
	}
	return CMD_OK;
}

#ifdef TAKE_OUT
//extern uint8_t IP_ADDRESS[4];
//extern uint8_t NETMASK_ADDRESS[4];
//extern uint8_t GATEWAY_ADDRESS[4];
//extern ip4_addr_t ipaddr;
//extern ip4_addr_t netmask;
//extern ip4_addr_t gw;

static char tempbuf[64];

CMD_RETURN ShTcp(uint8_t bPort, int argc, char *argv[])
{
	uint8_t ip_addr[4];

	ShNL(bPort);

	ip_addr[0] = ipaddr >> 24;
	ip_addr[1] = (ipaddr >> 16) & 0xff;
	ip_addr[2] = (ipaddr >> 8) & 0xff;
	ip_addr[3] = ipaddr & 0xff;
	//sprintf(tempbuf, "%02X.%02X.%02X.%02X", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
	sprintf(tempbuf, "%03d.%03d.%03d.%03d", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);


	ShFieldOut(bPort, "IP Address: ", 0);
	//ShFieldNumberOut(bPort, "", IP_ADDRESS[0], 0);
	//ShFieldNumberOut(bPort, ".", IP_ADDRESS[1], 0);
	//ShFieldNumberOut(bPort, ".", IP_ADDRESS[2], 0);
	//ShFieldNumberOut(bPort, ".", IP_ADDRESS[3], 0);
	ShFieldOut(bPort, tempbuf, 0);
	ShNL(bPort);

	ShFieldOut(bPort, "IP Mask   : ", 0);
	ShFieldNumberOut(bPort, "", NETMASK_ADDRESS[0], 0);
	ShFieldNumberOut(bPort, ".", NETMASK_ADDRESS[1], 0);
	ShFieldNumberOut(bPort, ".", NETMASK_ADDRESS[2], 0);
	ShFieldNumberOut(bPort, ".", NETMASK_ADDRESS[3], 0);
	ShNL(bPort);

	ShFieldOut(bPort, "GW Address: ", 0);
	ShFieldNumberOut(bPort, "", GATEWAY_ADDRESS[0], 0);
	ShFieldNumberOut(bPort, ".", GATEWAY_ADDRESS[1], 0);
	ShFieldNumberOut(bPort, ".", GATEWAY_ADDRESS[2], 0);
	ShFieldNumberOut(bPort, ".", GATEWAY_ADDRESS[3], 0);
	ShNL(bPort);

	return CMD_OK;
}
#endif

/*********************************************************************
*
* ShArgs
*
* @catagory	Shell Command
* @brief	A Shell command that just echos the argument list
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShArgs(uint8_t bPort, int argc, char *argv[])
{

    ShNL(bPort);

	ShFieldNumberOut(bPort, "Command Arguments: ", argc, 0);

	for(int i = 0; i < argc; i++)
	{
	    ShNL(bPort);
	    ShFieldOut(bPort, argv[i], 0);
	}
	return CMD_OK;
}


#ifdef TAKE_OUT
/*********************************************************************
*
* ShTestBits
*
* @brief	test command 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShTestBits(uint8_t bPort, int argc, char *argv[])
{

	ShNL(bPort);

//	test_bits();

	return CMD_OK;
}
#endif

extern void send_main(int argc, char** argv);

/*********************************************************************
*
* @catagory	Shell Command
* ShSend
*
* @brief	Execute the decoder test main function 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShSend(uint8_t bPort, int argc, char *argv[])
{

	ShNL(bPort);

	send_main(argc, argv);

	return CMD_OK;
}



/*********************************************************************
*
* @catagory	Shell Command
* ShYmodem
*
* @brief	Transfer (Receive only) a file using YModem 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShYmodem(uint8_t bPort, int argc, char *argv[])
{
	uint8_t buffer[1024];


    ShNL(bPort);
    ShFieldOut(bPort, "YModem Receive - start sending", 0);
    ShNL(bPort);

    Ymodem_SetPort(bPort);
	if(Ymodem_Receive(buffer) != 0)
	//if(ymodem_receive(buffer, 128) != 0)

	{
		return CMD_FAILED;
	}
	return CMD_OK;
}


extern void bit_test_main(void);

/*********************************************************************
*
* @catagory	Shell Command
* ShBitTest
*
* @brief	Send test command 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShBitTest(uint8_t bPort, int argc, char *argv[])
{


	bit_test_main();

	// ToDo - return something meaningful
	return CMD_OK;
}




extern void SendZero(void);
extern void SendOne(void);
extern void SendScopeA(void);
extern void SendScopeB(void);
extern void SendWarble(void);
extern void SendStretched(void);
extern void SendReset(void);
extern void SendHard(void);
extern void SendIdle(void);

extern void SetDccTiming(void);

/*********************************************************************
*
* ShSendZero
*
* @catagory	Shell Command
* @brief	Send a zero packet 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShSendZero(uint8_t bPort, int argc, char *argv[])
{
	int i;
	int iCount;

	SetDccTiming();

	ShNL(bPort);

	if(argc == 2)
    {
		iCount = atoi(argv[1]);
		for(i = 0; i < iCount; i++)
		{
			SendZero();
		}
    }
	else
	{
		SendZero();
	}

	return CMD_OK;
}

/*********************************************************************
*
* ShSendOne
*
* @catagory	Shell Command
* @brief	Send a one packet 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShSendOne(uint8_t bPort, int argc, char *argv[])
{
	int i;
	int iCount;

	SetDccTiming();

	ShNL(bPort);
	if(argc == 2)
    {
		iCount = atoi(argv[1]);
		for(i = 0; i < iCount; i++)
		{
			SendOne();
		}
    }
	else
	{
		SendOne();
	}

	return CMD_OK;
}

/*********************************************************************
*
* ShSendScopeA
*
* @catagory	Shell Command
* @brief	Send a scope A packet 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShSendScopeA(uint8_t bPort, int argc, char *argv[])
{
	int i;
	int iCount;

	SetDccTiming();


	ShNL(bPort);
	if(argc == 2)
    {
		iCount = atoi(argv[1]);
		for(i = 0; i < iCount; i++)
		{
			SendScopeA();
		}
    }
	else
	{
		SendScopeA();
	}

	return CMD_OK;
}

/*********************************************************************
*
* @catagory	Shell Command
* ShSendScopeB
*
* @brief	Send a scope B packet 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShSendScopeB(uint8_t bPort, int argc, char *argv[])
{
	int i;
	int iCount;

	SetDccTiming();

	ShNL(bPort);
	if(argc == 2)
    {
		iCount = atoi(argv[1]);
		for(i = 0; i < iCount; i++)
		{
			SendScopeB();
		}
    }
	else
	{
		SendScopeB();
	}

	return CMD_OK;
}

/*********************************************************************
*
* @catagory	Shell Command
* ShSendWarble
*
* @brief	Send a warble packet 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShSendWarble(uint8_t bPort, int argc, char *argv[])
{
	int i;
	int iCount;

	SetDccTiming();

	ShNL(bPort);
	if(argc == 2)
    {
		iCount = atoi(argv[1]);
		for(i = 0; i < iCount; i++)
		{
			SendWarble();
		}
    }
	else
	{
		SendWarble();
	}

	return CMD_OK;
}

/*********************************************************************
*
* @catagory	Shell Command
* ShSendStretched
*
* @brief	Send a stretched zero packet 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShSendStretched(uint8_t bPort, int argc, char *argv[])
{
	int i;
	int iCount;

	SetDccTiming();

	ShNL(bPort);
	if(argc == 2)
    {
		iCount = atoi(argv[1]);
		for(i = 0; i < iCount; i++)
		{
			SendStretched();
		}
    }
	else
	{
		SendStretched();
	}

	return CMD_OK;
}

/*********************************************************************
*
* @catagory	Shell Command
* ShSendReset
*
* @brief	Send a reset packet 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShSendReset(uint8_t bPort, int argc, char *argv[])
{
	int i;
	int iCount;

	SetDccTiming();

	ShNL(bPort);
	if(argc == 2)
    {
		iCount = atoi(argv[1]);
		for(i = 0; i < iCount; i++)
		{
			SendReset();
		}
    }
	else
	{
		SendReset();
	}

	return CMD_OK;
}

/*********************************************************************
*
* @catagory	Shell Command
* ShSendHard
*
* @brief	Send a hard reset packet 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShSendHard(uint8_t bPort, int argc, char *argv[])
{
	int i;
	int iCount;

	SetDccTiming();

	ShNL(bPort);
	if(argc == 2)
    {
		iCount = atoi(argv[1]);
		for(i = 0; i < iCount; i++)
		{
			SendHard();
		}
    }
	else
	{
		SendHard();
	}

	return CMD_OK;
}

/*********************************************************************
*
* @catagory	Shell Command
* ShSendIdle
*
* @brief	Send a idle packet (sends 4 right now)
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShSendIdle(uint8_t bPort, int argc, char *argv[])
{
	int i;
	int iCount;

	SetDccTiming();

	ShNL(bPort);
	if(argc == 2)
    {
		iCount = atoi(argv[1]);
		for(i = 0; i < iCount; i++)
		{
			SendIdle();
		}
    }
	else
	{
		SendIdle();
	}

	return CMD_OK;
}



#ifdef NOW_A_VARIABLE
CMD_RETURN ShTrack(uint8_t bPort, int argc, char *argv[])
{


	if(argc == 2)
	{
		if(strcmpi(argv[1], "on") == 0)
		{
			EnableTrack();
		}
		else if(strcmpi(argv[1], "off") == 0)
		{
			DisableTrack();
		}
	}
	//else
	//{
	//	return CMD_BAD_PARAMS;
	//}

	ShNL(bPort);
	if(GetTrackState())
	{
		ShFieldOut(bPort, "Track is on", 0);
	}
	else
	{
		ShFieldOut(bPort, "Track is off", 0);
	}
	return CMD_OK;
}
#endif


//***********************************************************************************************************************
//***********************************************************************************************************************

/*********************************************************************
*
* AddToHistory
*
* @brief	Shell command line history save
*
* @param	idx - index to which console
*			buf - command line to save
*
* @return	None
*
*********************************************************************/
void AddToHistory(uint8_t idx, uint8_t* buf)
{

	strcpy((char*)HistoryBuf[idx][HistoryIndex[idx]], (char*)buf);
	HistoryIndex[idx]++;
	if(HistoryIndex[idx] > NUM_HISTORY)
	{
		HistoryIndex[idx] = 0;
	}
}

/*********************************************************************
*
* parse_args
*
* @brief	parse the command line arguments into argv and argc
*
* @param	char *buffer
*  			char** args
*  			size_t args_size
*  			size_t *nargs
*
* @return	None
*
*********************************************************************/
void parse_args(char *buffer, char** args, size_t args_size, size_t *nargs)
{
    #define args_size 40
    char *buf_args[args_size];
    char **cp;
    char *wbuf;
    size_t i, j;
    wbuf = buffer;
    buf_args[0] = buffer;
    args[0] = buffer;

    for(cp = buf_args; (*cp = strsep(&wbuf, " \r\n\t\"")) != NULL ;)
    {
        if ((*cp != 0) && (++cp >= &buf_args[args_size]))
        {
            break;
        }
    }

    for (j = i = 0; buf_args[i] != NULL; i++)
    {
        if(strlen(buf_args[i])>0)
        {
            args[j++]=buf_args[i];
        }
    }

    *nargs = j;
    args[j] = NULL;
}


/*********************************************************************
*
* ExecuteCommand
*
* @brief	run the shell command and return the result
*
* @param	iCmdIndex - which command in the table
*  			bPort - port that issued this command
*  			nargs - number of arguments
*  			args - array of argument values
*
* @return	None
*
*********************************************************************/
int ExecuteCommand(int iCmdIndex, uint8_t bPort, size_t nargs, char** args)
{
	return (*ShellTable[iCmdIndex].Command)(bPort, nargs, args);
}

/*********************************************************************
*
* FindCommand
*
* @brief	Fined the command in the table
*
* @param	szCmdBuffer - the entered command line
*  			nargs - number of arguments
*  			args - array of argument values
*
* @return	None
*
*********************************************************************/
int FindCommand(char* szCmdBuffer, size_t* nargs, char** args)
{
	int i;

    parse_args(szCmdBuffer, args, ARR_SIZE, nargs);

    if(nargs != 0)
    {
    	for(i = 0; i < SHELL_TABLE_COUNT; i++)
    	{
    		if(strcasecmp(ShellTable[i].szCommand, args[0]) == 0)
    		{
    			return i;
    		}
    	}
    }
    return -1;
}

/*********************************************************************
*
* Prompt
*
* @brief	display the current working directory and the prompt character '>'
*
* @param	bPort - port that issued this command
*
* @return	None
*
*********************************************************************/
void Prompt(uint8_t bPort)
{
	ShNL(bPort);

	f_getcwd (szCWD, sizeof(szCWD));
	if(szCWD[0] != '/')
	{
		ShCharOut(bPort, '/');
	}

	ShFieldOut(bPort, szCWD, 0);
	ShFieldOut(bPort, ">", 0);
}

/*********************************************************************
*
* ShellInit
*
* @brief	Shell init
*
* @param	None
*
* @return	None
*
*********************************************************************/
void ShellInit(void)
{
//	FRESULT ret;
//	static FIL fp;

	//strcpy(szCWD, "/");
    // execute a startup script if it exists

//	ret = f_open(&fp, "Startup.scp", FA_READ);
//	if(ret == FR_OK)
//	{
//		ret = DoRun(PORT3, &fp);
////		ShNL(bPort);
////		PrintReturnString(bPort, ret);
//	}

	static char *args[ARG_SIZE];
	static size_t nargs;

//    parse_args("Startup.scp", args, ARR_SIZE, &nargs);

//	RunScript(PORT3, args[0], nargs, args);

	// ToDo - fix this
    //ShNL(ALL_PORTS);
    ShNL(PORT3);
	//Prompt(ALL_PORTS);
	Prompt(PORT3);
}


/*********************************************************************
*
* PrintReturnString
*
* @brief	print the string related to the return code
*
* @param	bPort - port that issued this command
*  			ret_value - return code
*
* @return	None
*
*********************************************************************/
void PrintReturnString(uint8_t bPort, int ret_value)
{

	switch(ret_value)
	{
		case CMD_BAD_PARAMS:
			ShNL(bPort);
			ShFieldOut(bPort, "Bad Parameters", 0);
			ShNL(bPort);
		break;

		case CMD_NOT_FOUND:
			ShNL(bPort);
			ShFieldOut(bPort, "Command/File Not Found", 0);
			ShNL(bPort);
		break;

		case CMD_FAILED:
			ShNL(bPort);
			ShFieldOut(bPort, "Command Failed", 0);
			ShNL(bPort);
		break;

		case CMD_CANCEL_KEY:
			ShNL(bPort);
			ShFieldOut(bPort, "Command Canceled", 0);
			ShNL(bPort);
		break;

		case CMD_BAD_NUMBER:
			ShNL(bPort);
			ShFieldOut(bPort, "Bad Number", 0);
			ShNL(bPort);
		break;

		case CMD_READ_ONLY:
			ShNL(bPort);
			ShFieldOut(bPort, "Read Only", 0);
			ShNL(bPort);
		break;

		case CMD_BAD_PACKET:
			ShNL(bPort);
			ShFieldOut(bPort, "Bad Packet", 0);
			ShNL(bPort);
		break;

		case CMD_BAD_BIT_FILE:
			ShNL(bPort);
			ShFieldOut(bPort, "Bad Bit File", 0);
			ShNL(bPort);
		break;
	}
}


/*********************************************************************
*
* RunScript
*
* @brief	find and run a script
*
* @param	bPort - port that issued this command
*  			filename - base filename of the script
*  			nargs - number of arguments
*  			args - array of argument values
*
* @return	1 = run the script
*
*********************************************************************/
int RunScript(uint8_t bPort, char* filename, size_t nargs, char** args)
{
	//char pathname[80];
	//char localPath[40];
	//char* plocalPath = localPath;
	//char* pPath;
	//uint8_t run = 0;
	FRESULT ret;
	static FIL fp;

	ret = OpenScript(&fp, szPathVar, filename);

	if(ret == FR_OK)
	{
		ret = DoRun(bPort, &fp, nargs, args);	// this will close the file
		ShNL(bPort);
		PrintReturnString(bPort, ret);
		return 1;
	}
	else
	{
		return 0;
	}
}



/*********************************************************************
*
* ShellMain
*
* @brief	Shell Main
*
* @param	bPort - port that issued this command
*  			buf - entered command line
*
* @return	None
*
*********************************************************************/
int ShellMain(uint8_t bPort, char* buf)
{
	int iCmdIndex;
	static char *args[ARG_SIZE];
	static size_t nargs;
	char StrTemp[64];
	int ret;

	iCmdIndex = FindCommand(buf, &nargs, args);
	if(iCmdIndex != -1)
	{
		ret = ExecuteCommand(iCmdIndex, bPort, nargs, args);
	}
	else if(IsVariable(args[0]))
	{
		if(args[1][0] == '=')
		{
			if(nargs == 4)
			{
				// check for a math operation
				switch(args[2][0])
				{
					case '+':
						ret = MathVariable(args[0], args[3], MV_ADD);
					break;

					case '-':
						ret = MathVariable(args[0], args[3], MV_SUBTRACT);
					break;

					case '*':
						ret = MathVariable(args[0], args[3], MV_MULTIPLY);
					break;

					case '/':
						ret = MathVariable(args[0], args[3], MV_DIVIDE);
					break;
				}
			}
			else if(nargs == 3)
			{
				// set the variable
				ret = SetVariable(args[0], args[2]);
#ifdef NOT_NEEDED
				if(ret != CMD_OK)
				{
					// print out the error
					switch(ret)
					{
						case CMD_BAD_NUMBER:
							ShNL(bPort);
							ShFieldOut(bPort, "Bad Number", 0);
							ShNL(bPort);
						break;

						case CMD_READ_ONLY:
							ShNL(bPort);
							ShFieldOut(bPort, "Variable Read Only", 0);
							ShNL(bPort);
						break;
					}
				}
#endif
			}
		}
		else
		{
			if(nargs == 3)
			{
				// check for a math operation
				switch(args[1][0])
				{
					case '+':
						ret = MathVariable(args[0], args[1], MV_ADD);
					break;

					case '-':
						ret = MathVariable(args[0], args[1], MV_SUBTRACT);
					break;

					case '*':
						ret = MathVariable(args[0], args[1], MV_MULTIPLY);
					break;

					case '/':
						ret = MathVariable(args[0], args[1], MV_DIVIDE);
					break;
				}
			}
			else if(nargs == 2)
			{
				// set the variable
				ret = SetVariable(args[0], args[1]);
#ifdef NOT_NEEDED
				if(ret != CMD_OK)
				{
					// print out the error
					switch(ret)
					{
						case CMD_BAD_NUMBER:
							ShNL(bPort);
							ShFieldOut(bPort, "Bad Number", 0);
							ShNL(bPort);
						break;

						case CMD_READ_ONLY:
							ShNL(bPort);
							ShFieldOut(bPort, "Variable Read Only", 0);
							ShNL(bPort);
						break;
					}
				}
#endif
			}
			else
			{
				// get the variable
				GetVariable(args[0], StrTemp, strlen(StrTemp));
				//ShFieldOut(bPort, " = ", 0);
				ShFieldOut(bPort, " ", 0);
				ShFieldOut(bPort, StrTemp, 0);
				//ShNL(bPort);
				ret = CMD_OK;
			}
		}
	}
	else if(RunScript(bPort, args[0], nargs, args) == 0)
	{
		if(strlen(buf) != 0)
		{
			ShNL(bPort);
			ShFieldOut(bPort, "Not Found", 0);
		}
		ret = CMD_OK;
	}
	return ret;
}


/*********************************************************************
*
* PortIndex
*
* @brief	parse the command line arguments into argv and argc
*
* @param	p -Port mask
*
* @return	port index
*
*********************************************************************/
uint8_t PortIndex(uint8_t p)
{
	uint8_t i = 0;
	while((p & 0x01) == 0)
	{
		p >>= 1;
		i++;
	}
	if(i > MAX_PORTS)
	{
		i = 0;
	}
	return i;
}

/*********************************************************************
*
* DoShell
*
* @brief	Main shell handling
*
* @details	receives characters on the specified port, and process the line when
*			carriage-return is pressed.  Invalid characters are ignored, and only valid
*			printable ASCII characters are added to the buffer. The backspace character
*			will remove the previous character from the buffer
*
* @param	char *buffer
*  			char** args
*  			size_t args_size
*  			size_t *nargs
*
* @return	None
*
*********************************************************************/
void DoShell(void)
{
	char c;
	uint8_t port;
	uint8_t portidx;
	int ret;

	// get the next character to process
	port = PORT1;
	c = ShGetChar(PORT1);
	if(c == 0)
	{
		port = PORT3;
		c = ShGetChar(PORT3);
		if(c == 0)
		{
//			port = PORTT;
//			c = ShGetChar(PORTT);
		}
	}

	if(c)
	{
		portidx = PortIndex(port);
		if(c == '\r')
		{
			ret = ShellMain(port, (char*)shell_buf[portidx]);
			Prompt(port);
			//ShNL(port);
			PrintReturnString(port, ret);
			shell_index = 0;
			shell_buf[portidx][0] = 0;

			AddToHistory(portidx, shell_buf[portidx]);
		}
		//
		// Backspace feature added 20-Jan-18 by BJB
		//
		else if (c == 0x7F || c == 8)        // Rubout (DEL) or Backspace
		{
			if (shell_index > 0)
			{
				ShCharOut(port,8);
				ShCharOut(port,' ');
				ShCharOut(port,8);
				shell_buf[portidx][--shell_index] = 0;
			}
		}
		else if(c == '\n')
		{
			//ShCharOut(port, c);
			// ignore
		}
		else if (c >= 32 && c <= 126)
		{
			ShCharOut(port, c);
			shell_buf[portidx][shell_index++] = c;
			shell_buf[portidx][shell_index] = 0;
		}
		else
		{
			// ignore invalid characters
		}
	}
}


/*********************************************************************
*
* GetInput
*
* @brief	Gather console input
*
* @param	buf - a buffer to receive the characters
*
* @return	
*
*********************************************************************/
int GetInput(char* buf)
{
	char c;
	uint8_t port;

	while(1)
	{
		// get the next character to process
		port = PORT1;
		c = ShGetChar(PORT1);
		if(c == 0)
		{
			port = PORT3;
			c = ShGetChar(PORT3);
			if(c == 0)
			{
//				port = PORTT;
//				c = ShGetChar(PORTT);
			}
		}

		if(c)
		{
			if(c == '\r')
			{
				return port;
			}
			//
			// Backspace feature added 20-Jan-18 by BJB
			//
			else if (c == 0x7F || c == 8)        // Rubout (DEL) or Backspace
			{
				if (shell_index > 0)
				{
					ShCharOut(port,8);
					ShCharOut(port,' ');
					ShCharOut(port,8);
					buf[--shell_index] = 0;
				}
			}
			else if(c == '\n')
			{
				//ShCharOut(port, c);
				// ignore
			}
			else if (c >= 32 && c <= 126)
			{
				ShCharOut(port, c);
				buf[shell_index++] = c;
				buf[shell_index] = 0;
			}
			else
			{
				// ignore invalid characters
			}
		}
	}
}


/*********************************************************************
*
* ShellTask
*
* @brief	Task for the shell
*
* @param	argument (unused)
*
* @return	None
*
*********************************************************************/
void ShellTask(void *argument)
{
	ShellInit();

	while(1)
	{
		DoShell();
		osDelay(10);
	}
}



/*********************************************************************
*
* ScriptTask
*
* @brief	a seperate task for shell scripts
*
* @param	argument (unused)
*
* @return	None
*
*********************************************************************/
/**********************************************************************
*
* FUNCTION:
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
void ScriptTask(void *argument)
{
//	ScriptInit();

	while(1)
	{
		DoScriptRun();
		osDelay(1);
	}
}









#ifdef NOT_USED

/*********************************************************************
*
* FUNCTION:		com_RecordScript
*
* ARGUMENTS:	com
*
* RETURNS:		none
*
* DESCRIPTION:
*
*********************************************************************/
int com_RecordScript(struct command *com)
{
    FILEHANDLE fh;
    char key;
	char acLineBuf[100];
	int iLineIndex;

	if(com->nargs == 1)
	{
        fh = fopen(com->args[0], "w+b");  /* open file */

        ShellPutString(com->stdout, "Recording> ");

		iLineIndex = 0;
		while(iLineIndex != -1)
		{
			key = get_kbd_char();
			if(key == 0x1b)
			{
				iLineIndex = -1;
	            ShellPutString(com->stdout, "\r\n");
			}
			else if(key == 0x08)
			{
				if(iLineIndex)
				{
					ShellPutString(com->stdout, "\033[1D");
					ShellPutString(com->stdout, "\033[s");
					ShellPutChar(com->stdout, ' ');
					ShellPutString(com->stdout, "\033[u");
					iLineIndex--;
				}
			}
			else if(key == 0x0d)
			{
				acLineBuf[iLineIndex++] = '\r';
				acLineBuf[iLineIndex++] = '\n';
		        sfs_fwrite(acLineBuf, iLineIndex, 1, fh);
				iLineIndex = 0;
	            ShellPutString(com->stdout, "\r\nRecording> ");
			}
			else if(key >= ' ')
			{
				acLineBuf[iLineIndex++] = key;
	            ShellPutChar(com->stdout, key);
			}
		}
        sfs_fclose(fh); /* close file */
	}
	return ESUCCESS;
}



//**************************************
//
// Name: ^NEW^ -- wildcard string compar
//     e (globbing)
// Description:matches a string against
//     a wildcard string such as "*.*" or "bl?h
//     .*" etc. This is good for file globbing
//     or to match hostmasks.
// By: Jack Handy
//
// Returns:1 on match, 0 on no match.
//
//This code is copyrighted and has// limited warranties.Please see http://
//     www.Planet-Source-Code.com/vb/scripts/Sh
//     owCode.asp?txtCodeId=1680&lngWId=3//for details.//**************************************
//

int wildcmp(const char *wild, const char *string)
{
	const char *cp = NULL, *mp = NULL;


	while ((*string) && (*wild != '*'))
	{
		if ((*wild != *string) && (*wild != '?'))
		{
			return 0;
		}
		wild++;
		string++;
	}

	while (*string)
	{
		if (*wild == '*')
		{
			if (!*++wild)
			{
				return 1;
			}
			mp = wild;
			cp = string+1;
		}
		else if ((*wild == *string) || (*wild == '?'))
		{
			wild++;
			string++;
		}
		else
		{
			wild = mp;
			string = cp++;
		}
	}

	while (*wild == '*')
	{
		wild++;
	}
	return !*wild;
}
#endif


