/*
 * Uart.h
 *
 *  Created on: Jan 23, 2018
 *      Author: Karl Kobel
 */

#ifndef UART_H_
#define UART_H_

#include "stm32h5xx_ll_usart.h"


typedef enum
{
//	U_MATCH,	// setup a callback to fire when a received character matches
	U_IDLE,		// setup a callback to fire after line idle
	U_CHAR,		// setup a callback for each received character
	//U_MATCH_9B,	// setup a callback to fire when a received character matches - 9 bit mode receive
	U_IDLE_9B,	// setup a callback to fire after N time of line idle - 9 bit mode receive
	//U_CHAR_9B,	// setup a callback for each received character - 9 bit mode receive
} U_MODE;


#define U_DATAWIDTH_7B      LL_USART_DATAWIDTH_7B
#define U_DATAWIDTH_8B      LL_USART_DATAWIDTH_8B
#define U_DATAWIDTH_9B		LL_USART_DATAWIDTH_9B


#define U_PARITY_NONE       LL_USART_PARITY_NONE
#define U_PARITY_EVEN       LL_USART_PARITY_EVEN
#define U_PARITY_ODD        LL_USART_PARITY_ODD


#define U_STOPBITS_0_5		LL_USART_STOPBITS_0_5
#define U_STOPBITS_1        LL_USART_STOPBITS_1
#define U_STOPBITS_1_5      LL_USART_STOPBITS_1_5
#define U_STOPBITS_2        LL_USART_STOPBITS_2


typedef struct
{
	U_MODE		mode;			// U_MATCH / U_IDLE / U_CHAR
	char		match;			// the match character for U_MATCH
	uint32_t	bits;			// number of bits. Ex 8 or 9
	uint32_t	parity;			// parity emun from hal_uart.h
	uint32_t	stop_bits;		// number of stopbits emun from hal_uart.h
	uint32_t	baud;			// the real BAUD value, ie: 19200
	void		(*UartRxCallback)(uint8_t* buf, uint8_t* len);	// called when the receive is complete (mode dependant)
	void		(*UartTxCallback)(void);				// called when the transmit is complete
} UART_DEF;


// Example Usage
//
// 	UART_DEF uart_def;
//
// uart_def.mode = U_CHAR;
// uart_def.bits = U_DATAWIDTH_8B;
// uart_def.parity = U_PARITY_NONE;
// uart_def.stop_bits = U_STOPBITS_2;
// uart_def.baud = 9600;
// uart_def.UartRxCallback = ReceivedChar;
// uart_def.UartTxCallback = TransmitComplete;
// if(Uart2_Init(&uart_def) != HAL_OK)
// {
//   _Error_Handler(__FILE__, __LINE__);
// }
//
// The Rx callback will get called for each character
// The Tx callback will get called when all of the characters have been transmitted
//
// NOTE: both callbacks are called in an interrupt domain.
// Don't block within the callback
//
// Special note for U_IDLE_9B mode
// This behaves like U_IDLE mode, but the character receiver returns two bytes.
// The 8 bit character will be in the first (odd) location of the character array,
// and the 9th bit will be in the second location (either a 0 or a 1).
// The number of characters returned does not include the extra byte for the 9th bit (a true count of the characters)
// Cast the uint8_t array into a uint16_t array so the indexing works correctly
//


#endif /* UART_H_ */
