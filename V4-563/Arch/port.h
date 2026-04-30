 /**********************************************************************
*
* SOURCE FILENAME:	Port.h
*
* DATE CREATED:		29/Jul/19
*
* PROGRAMMER:
*
* DESCRIPTION:
*
* COPYRIGHT (c) 1999-2019 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/
#ifndef Port_H
#define Port_H

#include "main.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define MAXFILE		8
#define MAXEXT		3

#define MAXPATH		24



#define BYTE	uint8_t


 /**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

extern uint8_t kbhit(void);

extern uint8_t getch(void);
//extern int gets(char* buf, int n);
char* getstr(char* buf, int n);


extern uint8_t inportb(uint8_t port);

extern void outportb(uint8_t port, uint8_t value);


//extern int get_key_cmd( void );

//extern void putch(char c);
//extern int puts(const char* str);
//extern int __io_putchar(int ch);

extern void OUT_PC(uint8_t out_pos, uint8_t value);

//extern int printf(const char *fmt, ... );
//extern int _gettimeofday (struct timeval *__restrict __p,  void *__restrict __tz);


extern void enable(void);

extern void disable(void);



#endif

