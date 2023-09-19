/**********************************************************************
*
* SOURCE FILENAME:	port.c
*
* DATE CREATED:		29/Aug/99
*
* PROGRAMMER:
*
* DESCRIPTION:
*
* COPYRIGHT (c) 2019 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/
#include "main.h"
#include "shell.h"
#include <stdarg.h>
//#include "PDS601.h"

// needed
// fputs
// fgets
// puts
// putchar (alias for putch)
// fprintf

/*
The shell has a single function to get characters from how ever many interfaces there are.
It calls the getch (or equlivent) and it returns false if there are no characters,
or true with the character in the buffer using the passed in pointer.
There can also be a isCharAvail (or equlivent) to call in front of getch.

The shell keeps track of which interface the character comes from and will call the appropriate
output method for any shell output.


There are 2 USB interfaces:
  The main USB on the front (which will work after Brian gets the composite USB implemented)
  The USB on the debug interface on the back and is connected to UART3 at 115200 BAUD
  This port is also used for firmware updates.

We will implement STDIN, STDOUT, and STDERR and all of the upper layers for serial streams (fprintf fputs, et all)
There are a few shell commands (like the SEND program and ZModem) that need to know what 'port'
they are supposed to communicate over, and currently this a a bit funky (the shell sets a module variable
to the correct port). If these commands are written to use the STD interface, the shell can just redirect
them to the port in use (the shell command has a wrapper that calls the command).

That can also mean that if a long process is running (like SEND which takes 24 hours for the full test)
a user can connect to a different interface and re-direct the STD interface to it and interact with
the running command.
*/




/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

 /**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

extern UART_HandleTypeDef huart3;
extern HAL_StatusTypeDef HAL_UART_ReceiveReady(UART_HandleTypeDef *huart);

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/

/**********************************************************************
*
*							CODE
*
**********************************************************************/

/**********************************************************************
*
* FUNCTION:		kbhit
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
uint8_t kbhit(void)
{

	if(ShKbHit(PORT1))
	{
		return 1;
	}
	if(ShKbHit(PORT3))
	{
		return HAL_UART_ReceiveReady(&huart3) == HAL_OK;
	}
	if(ShKbHit(PORTT))
	{
		return 1;
	}
	return 0;
}


/**********************************************************************
*
* FUNCTION:		getch
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
uint8_t getch(void)
{
//	uint8_t c;


//	while(1)
//	{
		//c = ShGetChar(PORT1);
		//if(c != 0)
		//{
		//	return c;
		//}
		//c = ShGetChar(PORT3);
		//if(c != 0)
		//{
		//	return c;
		//}
		//c = ShGetChar(PORTT);
		//if(c != 0)
		//{
		//	return c;
		//}
//	}
	return 0;
}


int get_key_cmd( void )
{
	return((int)getch());
}



void putch(char c)
{
	if(c == '\n')
	{
		ShCharOut(ALL_PORTS, '\r');
	}
	ShCharOut(ALL_PORTS, c);
}

int puts(const char* str)
{
	while(*str)
	{
		putch(*str++);
	}
	return 0;
}

char* getstr(char* buf, int n)
{
	uint8_t c;
	int i = 0;

	do
	{
		c = getch();
		if(i < n)
		{
			buf[i] = c;
			putch(c);
			i++;
		}
	} while(c != '\r');
	buf[i] = 0;
	putch(0x0a);
	return buf;
}



int __io_putchar(int ch)
{
	//uint8_t c[1];
	//c[0] = ch & 0x00FF;
	if(ch == '\n')
	{
		ShCharOut(ALL_PORTS, '\r');
	}
	ShCharOut(ALL_PORTS, ch);
	return ch;
}



// ToDo - make these global
#define SCOPE_TRIGGER_Pin	GPIO_PIN_7
#define SCOPE_TRIGGER_Port	GPIOE


void OUT_PC(uint8_t out_pos, uint8_t value)
{
	#define PC_POS_BDRST		0
	#define PC_POS_CPUEN		1
	#define PC_POS_CPUCLKL		2
	#define PC_POS_UNDERCLRL	4
	#define PC_POS_SCOPENL		5
	#define PC_POS_OBFAL		7

	switch(out_pos)
	{
		case PC_POS_BDRST:
			// Reset Board.
			//HAL_GPIO_WritePin(_Port, _Pin, (GPIO_PinState)value);
		break;

		case PC_POS_CPUEN:
			// Enable CPU clock.
			//HAL_GPIO_WritePin(_Port, _Pin, (GPIO_PinState)value);
		break;

		case PC_POS_CPUCLKL:
		{
			// CPU clock NOT.
			//HAL_GPIO_WritePin(_Port, _Pin, (GPIO_PinState)value);
		}
		break;

		case PC_POS_UNDERCLRL:
		{
			// Clear under NOT.
			//HAL_GPIO_WritePin(_Port, _Pin, (GPIO_PinState)value);
		}
		break;

		case PC_POS_SCOPENL:
		{
			// Enable Scope output NOT.
			HAL_GPIO_WritePin(SCOPE_TRIGGER_Port, SCOPE_TRIGGER_Pin, (GPIO_PinState)value);
		}
		break;

		case PC_POS_OBFAL:
		{
			// Output buffer full NOT.
			//HAL_GPIO_WritePin(_Port, _Pin, (GPIO_PinState)value);
		}
		break;
	}
}

/**********************************************************************
*
* FUNCTION:		inportb
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
uint8_t inportb(uint8_t port)
{

	return 0;
}


/**********************************************************************
*
* FUNCTION:		outportb
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
void outportb(uint8_t port, uint8_t value)
{

}


/**********************************************************************
*
* FUNCTION:		enable/disable
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
void enable(void)
{

}

void disable(void)
{

}



int printf(const char *fmt, ... )
{
    va_list arg_prt;
    char responce[120];

	va_start (arg_prt, fmt);
	vsprintf(responce, fmt, arg_prt);
	va_end (arg_prt);

	puts(responce);
	return strlen(responce);
}



int _gettimeofday (struct timeval *__restrict __p,  void *__restrict __tz)
{

	return 0;
}


void GetCTime(char* time_buf)
{
	extern RTC_HandleTypeDef hrtc;
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	if(HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN) == HAL_OK)
	{
		// Www Mmm dd hh:mm:ss yyyy
		// call the date read to unlock the clock
		HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

		sprintf(time_buf, "%d %d %d %2d:%02d:%02d %d", date.WeekDay, date.Month, date.Date, time.Hours, time.Minutes, time.Seconds, date.Year);
	}
}


