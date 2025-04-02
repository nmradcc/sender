/*
 * Ansi.h
 *
 *  Created on: Sep 27, 2017
 *      Author: Karl Kobel
 */

#ifndef ANSI_H_
#define ANSI_H_

/** @enum FG_COLOR
	@brief List of possible foreground (text) colors
 */

//const uint8_t fg_color[] =
//{
//	30, 31, 32, 33, 34, 35, 36, 37, 39
//};

typedef  enum
{
    FG_Black = 30,
    FG_Red = 31,
    FG_Green = 32,
    FG_Yellow = 33,
    FG_Blue = 34,
    FG_Purple = 35,
    FG_Cyan = 36,
    FG_White = 37,
	FG_Default = 39,
} FG_COLOR;


/** @enum BG_COLOR
	@brief List of possible background colors
 */

//const uint8_t bg_color[] =
//{
//	40, 41, 42, 43, 44, 45, 46, 47, 49
//};

typedef enum
{
    BG_Black = 40,
    BG_Red = 41,
    BG_Green = 42,
    BG_Yellow = 43,
    BG_Blue = 44,
    BG_Purple = 45,
    BG_Cyan = 46,
    BG_White = 47,
	BG_Default = 49,
} BG_COLOR;


/** @enum ATTRIBUTE
	@brief List of possible text attributes
 */
//const uint8_t attribute[] =
//{
//	0, 1, 2, 4, 5, 7, 10, 11, 12, 13, 15, 16, 17, 18, 19, 22, 24, 25, 27
//};

typedef enum
{
    ATT_Normal = 0,
    ATT_Bold = 1,
    ATT_Underline = 4,
    ATT_Blink = 5,
    ATT_Negative = 7,
    ATT_PrimaryFont = 10,
    ATT_FONT1 = 11,
    ATT_FONT2 = 12,
    ATT_FONT3 = 13,
    ATT_FONT4 = 14,
    ATT_FONT5 = 15,
    ATT_FONT6 = 16,
    ATT_FONT7 = 17,
    ATT_FONT8 = 18,
    ATT_FONT9 = 19,
    ATT_NormalColor = 22,
    ATT_UnderlineDouble = 24,
    ATT_BlinkOff = 25,
    ATT_Positive = 27,
} ATTRIBUTE;


extern void ClearScreen(uint8_t bPort);
extern void ClearEOL(uint8_t bPort);
extern void GoToXY(uint8_t bPort, int col, int line);
extern void TextColor(uint8_t bPort, FG_COLOR fg, BG_COLOR bg, ATTRIBUTE att);
extern void ResetColor(uint8_t bPort);

extern void Cursor(uint8_t bPort, uint8_t s_r);

#endif /* ANSI_H_ */
