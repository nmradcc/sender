/*
 * Uart2i.c
 *
 *  Created on: Jan 30, 2018
 *      Author: Karl Kobel
 */

/**********************************************************************
*
* @file
* @Author Karl Kobel
* @date   4/2/2000
* @brief  Serial Port Driver for USART 2
*
* SOURCE FILENAME:	Uart2.c
*
* DATE CREATED:		2/Apr/2018
*
* PROGRAMMER:
*
* DESCRIPTION:
*
* COPYRIGHT (c) 2000-2018 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/
#include "main.h"
//#include "stm32l4xx_hal.h"
#include "stm32h5xx_ll_usart.h"
#include <string.h>
//#include "GPIO.h"
//#include "Shell.h"
#include "Uart.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define CAB_TX_PIN		GPIO_PIN_5
#define CAB_TX_MODE		GPIO_MODE_AF_PP
#define CAB_TX_PU_PD	GPIO_NOPULL
#define CAB_TX_SPEED	GPIO_SPEED_FREQ_HIGH
#define CAB_TX_AF		GPIO_AF7_USART2
#define CAB_TX_PORT		GPIOD

#define CAB_RX_PIN		GPIO_PIN_6
#define CAB_RX_MODE		GPIO_MODE_AF_PP
#define CAB_RX_PU_PD	GPIO_NOPULL
#define CAB_RX_SPEED	GPIO_SPEED_FREQ_HIGH
#define CAB_RX_AF		GPIO_AF7_USART2
#define CAB_RX_PORT		GPIOD

#define CAB_DIR_PIN		GPIO_PIN_4
//#define CAB_DIR_MODE	GPIO_MODE_AF_PP
#define CAB_DIR_MODE	GPIO_MODE_OUTPUT_PP
#define CAB_DIR_PU_PD	GPIO_NOPULL
#define CAB_DIR_SPEED	GPIO_SPEED_FREQ_HIGH
#define CAB_DIR_AF		0
#define CAB_DIR_PORT	GPIOD

#define MAXTXBUFLEN    	128
#define MAXRXBUFLEN		128

#define OVERFLOW        1

#undef THIS_UART
#define THIS_UART		USART2

#undef THIS_IRQn
#define THIS_IRQn		USART2_IRQn

#undef THIS_CLK_ENABLE
#define THIS_CLK_ENABLE	__USART2_CLK_ENABLE


#define BYTE0(Var)   (*((uint8_t*)&Var + 0))
#define BYTE1(Var)   (*((uint8_t*)&Var + 1))
#define BYTE2(Var)   (*((uint8_t*)&Var + 2))
#define BYTE3(Var)   (*((uint8_t*)&Var + 3))
#define BYTE4(Var)   (*((uint8_t*)&Var + 4))
#define BYTE5(Var)   (*((uint8_t*)&Var + 5))
#define BYTE6(Var)   (*((uint8_t*)&Var + 6))
#define BYTE7(Var)   (*((uint8_t*)&Var + 7))

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

extern uint32_t PCLK1Freq;

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/

static UART_DEF mUartDef;

static uint8_t RxBuf[32];
static uint8_t RxNumberChars;


static uint8_t tx_restart = 0;
static uint16_t tstore_ptr=0;
static uint16_t tload_ptr=0;
static uint16_t rload_ptr=0;
static uint16_t rx_index=0;
static uint8_t tx_buf[MAXTXBUFLEN];
static uint8_t rx_buf[MAXRXBUFLEN];

/**********************************************************************
*
*							CODE
*
**********************************************************************/

/**********************************************************************
*
* FUNCTION:		Uart2_Init
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:	Initializes communication peripheral and buffers
*
* RESTRICTIONS:
*
**********************************************************************/
HAL_StatusTypeDef Uart2_Init(UART_DEF* uart_def)
{
	GPIO_InitTypeDef GPIO_InitStruct;


	// save the uart definition in a local store
	memcpy(&mUartDef, uart_def, sizeof(UART_DEF));


	THIS_CLK_ENABLE();

	LL_USART_Disable(THIS_UART);

    /* TX/RX direction */
    LL_USART_SetTransferDirection(THIS_UART, LL_USART_DIRECTION_TX_RX);

    /* 8 data bit, 1 start bit, 1 stop bit, no parity */
    LL_USART_ConfigCharacter(THIS_UART, uart_def->bits, uart_def->parity, uart_def->stop_bits);

    /* No Hardware Flow control */
    LL_USART_SetHWFlowCtrl(THIS_UART, LL_USART_HWCONTROL_NONE);

    /* Oversampling by 16 */
    LL_USART_SetOverSampling(THIS_UART, LL_USART_OVERSAMPLING_16);

    /* Set Baudrate to 115200 using APB frequency set to 100000000/APB_Div Hz */
    /* Frequency available for USART peripheral can also be calculated through LL RCC macro */
    /* Ex :
        Periphclk = LL_RCC_GetUSARTClockFreq(Instance); or
        LL_RCC_GetUARTClockFreq(Instance); depending on USART/UART instance

        In this example, Peripheral Clock is expected to be equal to
        100000000/APB_Div Hz => equal to SystemCoreClock/APB_Div
    */

    LL_USART_SetBaudRate(THIS_UART, PCLK1Freq, LL_USART_PRESCALER_DIV1, LL_USART_OVERSAMPLING_16, uart_def->baud);

//k	LL_USART_SetDEDeassertionTime(THIS_UART, 0);
//k	LL_USART_SetDEAssertionTime(THIS_UART, 0);
//k	LL_USART_SetDESignalPolarity(THIS_UART, LL_USART_DE_POLARITY_HIGH);
//k	LL_USART_EnableDEMode(THIS_UART);


	//LL_USART_DisableIT_ERROR(THIS_UART);
	//LL_USART_DisableIT_PE(THIS_UART);
	//LL_USART_DisableIT_RTO(THIS_UART);
	//LL_USART_DisableIT_EOB(THIS_UART);
	//LL_USART_DisableIT_CTS(THIS_UART);
	//LL_USART_DisableIT_WKUP(THIS_UART);
	//LL_USART_DisableIT_CM(THIS_UART);


//	if(uart_def->mode == U_MATCH)
//	{
//		LL_USART_ConfigNodeAddress(THIS_UART, LL_USART_ADDRESS_DETECT_7B, uart_def->match);
//		LL_USART_EnableIT_CM(THIS_UART);
//	}
	if(uart_def->mode == U_IDLE || uart_def->mode == U_IDLE_9B)
	{
		LL_USART_EnableIT_IDLE(THIS_UART);
	}

	LL_USART_EnableIT_RXNE(THIS_UART);

    LL_USART_Enable(THIS_UART);

    tx_restart = 1;

	GPIO_InitStruct.Pin = CAB_TX_PIN;
	GPIO_InitStruct.Mode = CAB_TX_MODE;
	GPIO_InitStruct.Pull = CAB_TX_PU_PD;
	GPIO_InitStruct.Speed = CAB_TX_SPEED;
	GPIO_InitStruct.Alternate = CAB_TX_AF;
	HAL_GPIO_Init(CAB_TX_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CAB_RX_PIN;
	GPIO_InitStruct.Mode = CAB_RX_MODE;
	GPIO_InitStruct.Pull = CAB_RX_PU_PD;
	GPIO_InitStruct.Speed = CAB_RX_SPEED;
	GPIO_InitStruct.Alternate = CAB_RX_AF;
	HAL_GPIO_Init(CAB_RX_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CAB_DIR_PIN;
	GPIO_InitStruct.Mode = CAB_DIR_MODE;
	GPIO_InitStruct.Pull = CAB_DIR_PU_PD;
	GPIO_InitStruct.Speed = CAB_DIR_SPEED;
	GPIO_InitStruct.Alternate = CAB_DIR_AF;
	HAL_GPIO_Init(CAB_DIR_PORT, &GPIO_InitStruct);

	/* Peripheral interrupt init*/
	HAL_NVIC_SetPriority(THIS_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(THIS_IRQn);

	return HAL_OK;
}




/**********************************************************************
*
* FUNCTION:		GetRxChar
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************///
static uint8_t GetRxChar(uint8_t* c)
{
	if(rload_ptr == rx_index)
	{
	    return 0;
	}

	*c = rx_buf[rload_ptr++];
	if(rload_ptr > MAXRXBUFLEN - 1)
	{
	  rload_ptr = 0;
	}
	return 1;
}


/**********************************************************************
*
* FUNCTION:		USART2_IRQHandler
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************///
void USART2_IRQHandler(void)
{
	uint8_t c;
	uint16_t w;

	if(LL_USART_IsActiveFlag_RXNE(THIS_UART) && LL_USART_IsEnabledIT_RXNE(THIS_UART))
	{
		if(mUartDef.mode == U_IDLE_9B)
		{
			w = LL_USART_ReceiveData9(THIS_UART);

			if(mUartDef.mode == U_CHAR)
			{
				rx_buf[0] = BYTE0(w);
		        rx_buf[1] = BYTE1(w);
				RxNumberChars = 2;
				if(mUartDef.UartRxCallback != NULL)
				{
					(*mUartDef.UartRxCallback)(RxBuf, &RxNumberChars);
				}
			}
			else
			{
				rx_buf[rx_index++] = BYTE0(w);
		        if( rx_index > MAXRXBUFLEN - 1 )
		        {
		            rx_index = 0;
		        }
		        rx_buf[rx_index++] = BYTE1(w);
		        if( rx_index > MAXRXBUFLEN - 1 )
		        {
		            rx_index = 0;
		        }
		        RxNumberChars++;
			}
		}
		else
		{
			c = LL_USART_ReceiveData8(THIS_UART);

			if(mUartDef.mode == U_CHAR)
			{
				RxBuf[0] = c;
				RxNumberChars = 1;
				if(mUartDef.UartRxCallback != NULL)
				{
					(*mUartDef.UartRxCallback)(RxBuf, &RxNumberChars);
				}
			}
			else
			{
				rx_buf[rx_index++] = c; // store data in buffer
				// check if we are on the buffer wrap
				if( rx_index > MAXRXBUFLEN - 1 )
				{
					rx_index = 0;
				}
				RxNumberChars++;
			}
		}
	}

//	if(LL_USART_IsActiveFlag_CM(THIS_UART) && LL_USART_IsEnabledIT_CM(THIS_UART))
//	{
//		LL_USART_ClearFlag_CM(THIS_UART);
//
//		if(mUartDef.mode == U_MATCH)
//		{
//			RxNumberChars = 0;
//			while(GetRxChar(&c))
//			{
//				RxBuf[RxNumberChars++] = c;
//			}
//			if(mUartDef.UartRxCallback != NULL)
//			{
//				(*mUartDef.UartRxCallback)(RxBuf, &RxNumberChars);
//			}
//		}
//	}

	if(LL_USART_IsActiveFlag_IDLE(THIS_UART) && LL_USART_IsEnabledIT_IDLE(THIS_UART))
	{
		LL_USART_ClearFlag_IDLE(THIS_UART);
		if(mUartDef.mode == U_IDLE || mUartDef.mode == U_IDLE_9B)
		{
			RxNumberChars = 0;
			while(GetRxChar(&c))
			{
				RxBuf[RxNumberChars++] = c;
			}
			if(mUartDef.UartRxCallback != NULL)
			{
				(*mUartDef.UartRxCallback)(RxBuf, &RxNumberChars);
			}
		}
	}

	if(LL_USART_IsActiveFlag_TXE(THIS_UART) && LL_USART_IsEnabledIT_TXE(THIS_UART))
	{

	    if( tstore_ptr != tload_ptr ){    // check if we have data in the buffer
	        c = tx_buf[tload_ptr++];
	        if( tload_ptr > MAXTXBUFLEN - 1 ){
	            tload_ptr = 0;
	        }
			LL_USART_TransmitData8(THIS_UART, c);
	        tx_restart = 0;
	    }
	    else
	    {
	        // reset pointers
			LL_USART_DisableIT_TXE(THIS_UART);
	        tx_restart = 1;

			if(mUartDef.UartTxCallback != NULL)
			{
				(*mUartDef.UartTxCallback)();
			}

			LL_USART_ClearFlag_TC(THIS_UART);
			LL_USART_EnableIT_TC(THIS_UART);
	    }
	}


	if(LL_USART_IsActiveFlag_TC(THIS_UART))
	{
		HAL_GPIO_WritePin(CAB_DIR_PORT, CAB_DIR_PIN, GPIO_PIN_RESET);
		LL_USART_DisableIT_TC(THIS_UART);

//		LL_USART_DisableIT_TXE(THIS_UART);
//        tx_restart = 1;
	}

	LL_USART_ClearFlag_PE(THIS_UART);
	LL_USART_ClearFlag_FE(THIS_UART);
	LL_USART_ClearFlag_NE(THIS_UART);
	LL_USART_ClearFlag_ORE(THIS_UART);
	LL_USART_ClearFlag_IDLE(THIS_UART);
	LL_USART_ClearFlag_TC(THIS_UART);
//	LL_USART_ClearFlag_CM(THIS_UART);
}


/*********************************************************************
*
* FUNCTION:		Uart_SendChar
*
* ARGUMENTS:	the character to send
*
* RETURNS:		none
*
* DESCRIPTION:	Put a character in the send FIFO and make the transmit
* 				interrupt is on
*
*********************************************************************/
static void Uart2_SendChar(const char c)
{
	int j=0;

	HAL_GPIO_WritePin(CAB_DIR_PORT, CAB_DIR_PIN, GPIO_PIN_SET);

	// check if we on the buffer wrap
    while((tstore_ptr+1) == tload_ptr)
    {
        j++;// allow bytes to get clear
    }
    while(((tstore_ptr+1) == MAXTXBUFLEN) && (tload_ptr==0))
    {
        j++;// allow bytes to get clear
    }
    if( tstore_ptr < tload_ptr ){
        if( tload_ptr - tstore_ptr > OVERFLOW ){
            tx_buf[tstore_ptr++] = c;
            if( tstore_ptr > MAXTXBUFLEN - 1 ){
                tstore_ptr = 0;
            }
        }
    }else{
        if( tstore_ptr > tload_ptr ){
            if( (( tload_ptr - tstore_ptr ) + MAXTXBUFLEN ) > OVERFLOW ){
                tx_buf[tstore_ptr++] = c;
                if( tstore_ptr > MAXTXBUFLEN - 1 ){
                    tstore_ptr = 0;
                }
            }
        }else{
            tx_buf[tstore_ptr++] = c;
            if( tstore_ptr > MAXTXBUFLEN - 1 ){
                tstore_ptr = 0;
            }
        }
    }

    // Start the interrupt
    if( tx_restart )
    {
		LL_USART_EnableIT_TXE(THIS_UART);
//		LL_USART_EnableIT_TC(THIS_UART);
    }
    return;
}


/*********************************************************************
*
* FUNCTION:		SendMessage
*
* ARGUMENTS:	pointer to the data buffer, number of bytes
*
* RETURNS:		none
*
* DESCRIPTION:	Sends a message by looping through the buffer for the
* 				number of bytes using the Uart_SendChar function
*
*********************************************************************/
void Uart2_SendPacket(uint8_t *data, uint8_t size)
{
    int i = 0;

    // sanity check
    if(size > MAXTXBUFLEN)
    {
        return;
    }

    // Loop through the buffer
    for( i = 0; i < size; i++ )
    {
        Uart2_SendChar(data[i]);
    }
}



/**********************************************************************
*
* FUNCTION:		Uart2_SendToken
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:		Sends data via communication peripheral
*
* RESTRICTIONS:		This does not check for transmitter not busy
*
**********************************************************************///
void Uart2_SendToken(uint8_t token)
{

	HAL_GPIO_WritePin(CAB_DIR_PORT, CAB_DIR_PIN, GPIO_PIN_SET);
	if(mUartDef.bits == U_DATAWIDTH_9B)
	{
		LL_USART_TransmitData9(THIS_UART, token | 0x100);
	}
	else
	{
		LL_USART_TransmitData8(THIS_UART, token);
	}

    if( tx_restart )
    {
		LL_USART_EnableIT_TXE(THIS_UART);
//		LL_USART_EnableIT_TC(THIS_UART);
    }
}

