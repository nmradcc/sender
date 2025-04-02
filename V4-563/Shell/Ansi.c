/*******************************************************************************
* @file ANSI.c
* @brief Implementation of ANSI console commands for color and cursor positioning
*
* @author K. Kobel
* @date 9/15/2019
* @Revision: 24 $
* @Modtime: 10/31/2019
*
* @copyright	(c) 2019  all Rights Reserved.
*******************************************************************************/
#include "main.h"
//#include <stdlib.h>
//#include <stdio.h>
//#include <stdarg.h>
//#include <string.h>
#include "Shell.h"
//#include "ff.h"
#include "Ansi.h"

//*******************************************************************************
// Definitions
//*******************************************************************************


//#define CLRSCR          "\033[0J"	// from cursor down
#define M_CLRSCR          "\033[2J"	// entire screen
#define M_CLREOL          "\033[K"
#define M_GOTOXY          "\033[%d;%dH"
#define M_RESET_COLOR     "\033[0m"

//#define TEXT_COLOR      "\033[44m\033[37m\033[1m"
//#define SELECT_COLOR    "\033[47m\033[37m\033[1m"
//#define STATUS_COLOR    "\033[0m\033[47m\033[30m"

//#define BOLD            "\033[1m"
//#define UNBOLD          RESET_COLOR


#define CSI		"\033["

//*******************************************************************************
// GLOBAL VARIABLES
//*******************************************************************************

//*******************************************************************************
// STATIC VARIABLES
//*******************************************************************************

static char buf[64];

//*******************************************************************************
// FUNCTION PROTOTYPES
//*******************************************************************************

//*******************************************************************************
// CODE
//*******************************************************************************

const uint8_t fg_color[] =
{
	30, 31, 32, 33, 34, 35, 36, 37, 39
};


const uint8_t bg_color[] =
{
	40, 41, 42, 43, 44, 45, 46, 47, 49
};


const uint8_t attribute[] =
{
	0, 1, 2, 4, 5, 7, 10, 11, 12, 13, 15, 16, 17, 18, 19, 22, 24, 25, 27
};

/*********************************************************************
*
* ClearScreen
*
* @brief	Output the ANSI clear screen escape sequence 
*
* @param	bPort - port that issued this command
*
* @return	None
*
*********************************************************************/
void ClearScreen(uint8_t bPort)
{
    ShStringOut(bPort, M_CLRSCR);
}


/*********************************************************************
*
* ClearEOL
*
* @brief	Output the ANSI clear to eol of line escape sequence
*
* @param	bPort - port that issued this command
*
* @return	None
*
*********************************************************************/
void ClearEOL(uint8_t bPort)
{
    ShStringOut(bPort, M_CLREOL);
}



/*********************************************************************
*
* GoToXY
*
* @brief	Output the ANSI cursor escape sequence
*
* @param	bPort - port that issued this command
*			col - the X position relative to the upper left corner
*			line - the Y position
*
* @return	None
*
*********************************************************************/
void GoToXY(uint8_t bPort, int col, int line)
{
    //char buf[32];

    sprintf(buf, M_GOTOXY, line + 1, col + 1);
    ShStringOut(bPort, buf);
}


/*********************************************************************
*
* TextColor
*
* @brief	Set the color of the forground, background, and the attribute of the text
*
* @param	bPort - port that issued this command
*			fg - text color (see FG_COLOR)
*			bg - background color (see BG_COLOR)
*			att - text attribute (see ATTRIBUTE)
*
* @return	None
*
*********************************************************************/
void TextColor(uint8_t bPort, FG_COLOR fg, BG_COLOR bg, ATTRIBUTE att)
{

    snprintf(buf, sizeof(buf), CSI "%d;%d;%dm", att, bg, fg);
    ShStringOut(bPort, buf);
}

void ResetColor(uint8_t bPort)
{

    ShStringOut(bPort, M_RESET_COLOR);
}


/*********************************************************************
*
* Cursor
*
* @brief	Saves or restores the cursor 
*
* @param	bPort - port that issued this command
*			s_r - save = non-zero, restore = zero
*
* @return	None
*
*********************************************************************/
void Cursor(uint8_t bPort, uint8_t s_r)
{

	if(s_r)
	{
		sprintf(buf, CSI "s");
	}
	else
	{
		sprintf(buf, CSI "u");
	}
		
    ShStringOut(bPort, buf);
}



