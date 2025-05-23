//*******************************************************************************
//
// SOURCE FILENAME: Shell.c
//
// DATE CREATED:    15-Feb-2012
//
// PROGRAMMER:      Karl Kobel
//
// DESCRIPTION:     shell module
//
// COPYRIGHT (c) 1999-2017 by K2 Engineering  All Rights Reserved.
//
//*******************************************************************************
#include "main.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "Shell.h"
#include "ShellFile.h"
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
//#include "fatfs.h"
#include "GetLine.h"

#include "cmsis_os.h"


//#define NULL ((void*)0)

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


#define MAXSHELLBUFLEN		80

#define NUM_HISTORY			10

//*******************************************************************************
// Static Variables
//*******************************************************************************

//static uint8_t nLoopCount;
//static char History[80][NUM_HISTORY];

static uint8_t shell_index = 0;
static uint8_t shell_buf[MAX_PORTS][MAXSHELLBUFLEN];

//static uint32_t HeartbeatSave;
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

//extern int getLine(&fp, szTypeBuf, sizeof(szTypeBuf));



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
//CMD_RETURN ShTime(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShRem(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShDelay(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShEcho(uint8_t bPort, int argc, char *argv[]);
//CMD_RETURN ShLed(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShVariables(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShSave(uint8_t bPort, int argc, char *argv[]);

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

CMD_RETURN ShYmodem(uint8_t bPort, int argc, char *argv[]);


const SHELL_TABLE ShellTable[] =
{
//	Cmd			Binary	Flags							Function
	{"?",		0x00,	SUPPRESS_HELP,					ShHelp,				"command help - help script, help all"},
	{"help",	0x00,	NO_FLAGS,						ShHelp,				"command help - help script, help all"},
//	{"time",	0x00,	NO_FLAGS,						ShTime,				"real and fast time"},
    
	{"clrscr",	0x00,	NO_FLAGS,						ShClrScreen,		"clear the screen"},
	{"clreol",	0x00,	NO_FLAGS,						ShClrEOL,			"clear from the cursor to the end of the line"},
	{"gotoxy",	0x00,	NO_FLAGS,						ShGotoXY,			"x, y"},
	{"cursor",	0x00,	NO_FLAGS,						ShCursor,			"save | restore"},
	{"color",	0x00,	NO_FLAGS,						ShTextColor,		"[[[fg], bg], att]"},

// scripting
//	{"run",		0x00,	SUPPRESS_HELP, 	ShRun},
	{"if",		0x00,	SUPPRESS_HELP | SCRIPT_HELP, 	ShIf,				"if condition"},
	{"else",	0x00,	SUPPRESS_HELP | SCRIPT_HELP, 	ShElse,				"not condition"},
	{"endif",	0x00,	SUPPRESS_HELP | SCRIPT_HELP, 	ShEndif,			"end if"},
	{"loop",	0x00,	SUPPRESS_HELP | SCRIPT_HELP, 	ShLoop,				"count"},
	{"endloop",	0x00,	SUPPRESS_HELP | SCRIPT_HELP, 	ShEndLoop,			"end loop"},
	{"break",	0x00,	SUPPRESS_HELP | SCRIPT_HELP, 	ShBreak,			"break out of loop"},
	{"variables",0x00,	NO_FLAGS | SCRIPT_HELP,			ShVariables,		"list all variables"},
	{"pause",	0x00,	SUPPRESS_HELP | SCRIPT_HELP, 	ShDelay,			"ms"},
	{"echo",	0x00,	NO_FLAGS | SCRIPT_HELP,			ShEcho,				"test"},
	{"rem",		0x00,	SUPPRESS_HELP | SCRIPT_HELP,	ShRem,				"ignore line"},

	{"save",	0x00,	NO_FLAGS, 						ShSave,				""},

// file system
	{"dir",		0x00,	NO_FLAGS,						ShDir,				"list the files on the drive"},
	{"type",	0x00,	NO_FLAGS,						ShType,				"display the contents of a file"},
	{"del",		0x00,	NO_FLAGS,						ShDelete,			"delete a file"},
//	{"format",	0x00,	SUPPRESS_HELP, 					ShFormat,			"format the storage"},
	{"md",	    0x00,	NO_FLAGS, 						ShMkdir,			"make directory"},
	{"rd",	    0x00,	SUPPRESS_HELP, 					ShRmdir,			"remove directory"},
	{"cd",	    0x00,	NO_FLAGS, 						ShChdir,			"change directory"},
	{"cwd",	    0x00,	SUPPRESS_HELP, 					ShCWD,				"change working directory"},
	{"atrib",    0x00,	NO_FLAGS, 						ShAtrib,			"Set/Reset attributes +/- R,H,S,A"},

	{"args",     0x00,	SUPPRESS_HELP, 					ShArgs,				"List arguments"},
	{"tasks",   0x00,	NO_FLAGS, 						ShTasks,			"Task List"},
	{"tcp",   	0x00,	NO_FLAGS, 						ShTcp,				"TCP/IP Info"},

//	{"test",   	0x00,	NO_FLAGS, 						ShTestBits,			"DCC Bit Test"},
	{"send",   	0x00,	NO_FLAGS, 						ShSend,				"DCC Decoder Tests - type send -? for more info"},
	{"bittest",	0x00,	NO_FLAGS, 						ShBitTest,			"DCC Bit Test"},

	{"sendzero",0x00,	NO_FLAGS, 						ShSendZero,			"DCC Zero Packet(s)"},
	{"sendone",	0x00,	NO_FLAGS, 						ShSendOne,			"DCC one Packet(s)"},
	{"scopea",	0x00,	NO_FLAGS, 						ShSendScopeA,		"DCC Scope A Packet(s)"},
	{"scopeb",	0x00,	NO_FLAGS, 						ShSendScopeB,		"DCC Scope B Packet(s)"},
	{"warble",	0x00,	NO_FLAGS, 						ShSendWarble,		"DCC Warble Packet(s)"},
	{"stretched",0x00,	NO_FLAGS, 						ShSendStretched,	"DCC Stretched Packet(s)"},
	{"reset",	0x00,	NO_FLAGS, 						ShSendReset,		"DCC Reset Packet(s)"},
	{"hard",	0x00,	NO_FLAGS, 						ShSendHard,			"DCC Hard Reset Packet(s)"},
	{"idle",	0x00,	NO_FLAGS, 						ShSendIdle,			"DCC Idle Packet(s)"},

	{"ymodem",	0x00,	NO_FLAGS, 						ShYmodem,			"YModem receive"},
};
#define SHELL_TABLE_COUNT (sizeof(ShellTable) / sizeof(SHELL_TABLE))


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

void ShStringOut(uint8_t port, char* s)
{

	ShBuffOut(port, s, strlen(s));
}

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


void ShNL(uint8_t bPort)
{

    ShCharOut(bPort, '\r');
    ShCharOut(bPort, '\n');
}


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

#ifdef NOT_YET
char ShKbHit(uint8_t port)
{

    if(port & PORT1)
    {
		if(VCP_KbHit())
		{
			return 1;
		}
    }

    if(port & PORT3)
    {
//    	if(HAL_UART_Receive(&huart3, &c, 1, 0) == HAL_OK)
    	{
    	   	return 1;
    	}
    }

    if(port & PORTT)
    {
//    	c = telnet_kbhit();
    	{
    		return 1;
    	}
    }

	return 0;
}
#endif

int isfloat(char* szNumStr)
{
	return strchr(szNumStr, '.') != NULL;
}


#define HELP_FIELD_WIDTH	13


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
		res = f_open(&fp, argv[1], FA_READ);

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
			}
			f_close(&fp);
			return CMD_OK;
		}

		if(strcasecmp("script", argv[1]) == 0)
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
		else if(strcasecmp("all", argv[1]) == 0)
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

		res = f_opendir(&dir, "/");                       /* Open the directory */
		if (res == FR_OK)
		{
			for (;;)
			{
				res = f_readdir(&dir, &fno);                   /* Read a directory item */
				if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
				if (!(fno.fattrib & AM_DIR))
				{
					/* It is a file. */
					if(strcmp(fno.fname, "CONFIG.INI") != 0)
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
	}

	return CMD_OK;
}


CMD_RETURN ShRem(uint8_t bPort, int argc, char *argv[])
{
	return CMD_OK;
}


CMD_RETURN ShDelay(uint8_t bPort, int argc, char *argv[])
{
	int iDelayCount;

	iDelayCount = atoi(argv[1]);

	while(iDelayCount)
    {
        if(bfShellTick)
	    {
		    bfShellTick = 0;
		    iDelayCount--;
		}
	}
	return CMD_OK;
}


CMD_RETURN ShEcho(uint8_t bPort, int argc, char *argv[])
{

	if(argc >= 2)
	{
		ShFieldOut(bPort, argv[1], 0);
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



CMD_RETURN ShClrScreen(uint8_t bPort, int argc, char *argv[])
{
    ClearScreen(bPort);
    return CMD_OK;
}

CMD_RETURN ShClrEOL(uint8_t bPort, int argc, char *argv[])
{
    ClearEOL(bPort);
    return CMD_OK;
}

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

CMD_RETURN ShCursor(uint8_t bPort, int argc, char *argv[])
{

    if(argc == 2)
    {
		if(strcmp(argv[2], "save") == 0)
		{
			Cursor(bPort, 1);
		}
		else if(strcmp(argv[2], "restore") == 0)
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


CMD_RETURN ShVariables(uint8_t bPort, int argc, char *argv[])
{

	ShowVariables(bPort);
	return CMD_OK;
}




CMD_RETURN ShSave(uint8_t bPort, int argc, char *argv[])
{

//k	SaveLocoState();
//k	SaveMacros();

	SaveSettings();

	return CMD_OK;
}



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

extern uint8_t IP_ADDRESS[4];
extern uint8_t NETMASK_ADDRESS[4];
extern uint8_t GATEWAY_ADDRESS[4];

CMD_RETURN ShTcp(uint8_t bPort, int argc, char *argv[])
{

	ShNL(bPort);

	ShFieldOut(bPort, "IP Address: ", 0);
	ShFieldNumberOut(bPort, "", IP_ADDRESS[0], 0);
	ShFieldNumberOut(bPort, ".", IP_ADDRESS[1], 0);
	ShFieldNumberOut(bPort, ".", IP_ADDRESS[2], 0);
	ShFieldNumberOut(bPort, ".", IP_ADDRESS[3], 0);
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


extern void send_main(int argc, char** argv);

CMD_RETURN ShTestBits(uint8_t bPort, int argc, char *argv[])
{

	ShNL(bPort);

//	test_bits();

	return CMD_OK;
}


CMD_RETURN ShSend(uint8_t bPort, int argc, char *argv[])
{

	ShNL(bPort);

	send_main(argc, argv);

	return CMD_OK;
}



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


CMD_RETURN ShSendZero(uint8_t bPort, int argc, char *argv[])
{

	ShNL(bPort);
	SendZero();

	return CMD_OK;
}

CMD_RETURN ShSendOne(uint8_t bPort, int argc, char *argv[])
{

	ShNL(bPort);
	SendOne();

	return CMD_OK;
}

CMD_RETURN ShSendScopeA(uint8_t bPort, int argc, char *argv[])
{

	ShNL(bPort);
	SendScopeA();

	return CMD_OK;
}

CMD_RETURN ShSendScopeB(uint8_t bPort, int argc, char *argv[])
{

	ShNL(bPort);
	SendScopeB();

	return CMD_OK;
}

CMD_RETURN ShSendWarble(uint8_t bPort, int argc, char *argv[])
{

	ShNL(bPort);
	SendWarble();

	return CMD_OK;
}

CMD_RETURN ShSendStretched(uint8_t bPort, int argc, char *argv[])
{

	ShNL(bPort);
	SendStretched();

	return CMD_OK;
}

CMD_RETURN ShSendReset(uint8_t bPort, int argc, char *argv[])
{

	ShNL(bPort);
	SendReset();

	return CMD_OK;
}

CMD_RETURN ShSendHard(uint8_t bPort, int argc, char *argv[])
{

	ShNL(bPort);
	SendHard();

	return CMD_OK;
}

CMD_RETURN ShSendIdle(uint8_t bPort, int argc, char *argv[])
{

	ShNL(bPort);
	SendIdle();

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

void AddToHistory(uint8_t idx, uint8_t* buf)
{

	strcpy((char*)HistoryBuf[idx][HistoryIndex[idx]], buf);
	HistoryIndex[idx]++;
	if(HistoryIndex[idx] > NUM_HISTORY)
	{
		HistoryIndex[idx] = 0;
	}
}

/************************************************************************
* parse_args
*
*  parameters:	char *buffer
*  				char** args
*  				size_t args_size
*  				size_t *nargs
*
*  returns:
*
*  description:
*
*************************************************************************/
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


int ExecuteCommand(int iCmdIndex, uint8_t bPort, size_t nargs, char** args)
{
	return (*ShellTable[iCmdIndex].Command)(bPort, nargs, args);
}

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

void Prompt(uint8_t bPort)
{
	ShNL(bPort);

//	f_getcwd (szCWD, sizeof(szCWD));
	if(szCWD[0] != '/')
	{
		ShCharOut(bPort, '/');
	}

	ShFieldOut(bPort, szCWD, 0);
	ShFieldOut(bPort, ">", 0);
}

void ShellInit(void)
{
	FRESULT ret;
	static FIL fp;

	strcpy(szCWD, "/");
    // execute a startup script if it exists

//	ret = f_open(&fp, "Startup.scp", FA_READ);
//	if(ret == FR_OK)
//	{
//		ret = DoRun(PORT3, &fp);
////		ShNL(bPort);
////		PrintReturnString(bPort, ret);
//	}

    //ShNL(ALL_PORTS);
    ShNL(PORT3);
	//Prompt(ALL_PORTS);
	Prompt(PORT3);
}


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
			ShFieldOut(bPort, "Command Not Found", 0);
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

	}
}


void ShellMain(uint8_t bPort, char* buf)
{
	int iCmdIndex;
	static char *args[ARG_SIZE];
	static size_t nargs;
	char StrTemp[64];
	int ret;
	static FIL fp;

	iCmdIndex = FindCommand(buf, &nargs, args);
	if(iCmdIndex != -1)
	{
		ret = ExecuteCommand(iCmdIndex, bPort, nargs, args);
		ShNL(bPort);
		PrintReturnString(bPort, ret);
		Prompt(bPort);
	}
	else
	{
		if(IsVariable(args[0]))
		{
			if(args[1][0] == '=')
			{
				ret = SetVariable(args[0], args[2]);
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
			}
			else
			{
				GetVariable(args[0], StrTemp, strlen(StrTemp));
				ShFieldOut(bPort, " = ", 0);
				ShFieldOut(bPort, StrTemp, 0);
				ShNL(bPort);
			}
			Prompt(bPort);
		}
		else
		{
			// find and run a script
			ret = f_open(&fp, args[0], FA_READ);
			if(ret == FR_OK)
			{
				ret = DoRun(bPort, &fp);
				ShNL(bPort);
				PrintReturnString(bPort, ret);
				Prompt(bPort);
			}
			else
			{
				strcat(args[0], SCRIPT_DEFAULT_EXTENSION);
				ret = f_open(&fp, args[0], FA_READ);
				if(ret == FR_OK)
				{
					ret = DoRun(bPort, &fp);
					ShNL(bPort);
					PrintReturnString(bPort, ret);
					Prompt(bPort);
				}
				else
				{
					if(strlen(buf) != 0)
					{
						ShNL(bPort);
						ShFieldOut(bPort, "Not Found", 0);
						Prompt(bPort);
					}
				}
			}
		}
	}

	if(buf[0] == '\0')
	{
		Prompt(bPort);
	}
}


uint8_t PortIndex(uint8_t p)
{
	uint8_t i = 0;
	while((p & 0x01) == 0)
	{
		p >>= 1;
		i++;
	}
	return i;
}

//
//	void DoShell(void)
//
//  DoShell() receives characters on the specified port, and process the line when
//  carriage-return is pressed.  Invalid characters are ignored, and only valid
//  printable ASCII characters are added to the buffer.
//
//
void DoShell(void)
{
	char c;
	uint8_t port;
	uint8_t portidx;

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
			ShellMain(port, (char*)shell_buf[portidx]);
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



void ShellTask(void *argument)
{
	ShellInit();

	while(1)
	{
		DoShell();
		osDelay(10);
	}
}



void ScriptTask(void *argument)
{
//	ScriptInit();

	while(1)
	{
		DoScriptRun();
		osDelay(10);
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


