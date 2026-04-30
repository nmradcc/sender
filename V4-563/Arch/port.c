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
//#include "app_threadx.h"
//#include "app_azure_rtos.h"

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

	// doesn't exist yet
	//if(ShKbHit(PORT1))
	//{
	//	return 1;
	//}

	if(__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE) != 0)
	{
		return 1;
	}

	// doesn't exist yet
	//if(ShKbHit(PORTT))
	//{
	//	return 1;
	//}
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
	uint8_t c;


	while(1)
	{
		c = ShGetChar(PORT1);
		if(c != 0)
		{
			return c;
		}
		c = ShGetChar(PORT3);
		if(c != 0)
		{
			return c;
		}
		//c = ShGetChar(PORTT);
		//if(c != 0)
		//{
		//	return c;
		//}
		osDelay(pdMS_TO_TICKS(1));
	}
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


