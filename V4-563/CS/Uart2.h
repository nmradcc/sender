/*
 * Uart2.h
 *
 *  Created on: Dec 12, 2017
 *      Author: Karl Kobel
 */

#ifndef UART2_H_
#define UART2_H_

#include "Uart.h"

// See setup discription in uart.h
extern HAL_StatusTypeDef Uart2_Init(UART_DEF* uart_def);

// call to send the polling / call byte character
// The 9th bit will be set if 9 bit mode is configures
extern void Uart2_SendToken(uint8_t t);

// call after Uart2_SendToken() to be sure the transmitter is ready
extern uint8_t IsTxBusy(void);

// send buffered data
extern void Uart2_SendPacket(uint8_t *data, uint8_t size);


#endif /* UART2_H_ */
