/*
 * VCP.c
 *
 *  Created on: Dec 28, 2017
 *      Author: Karl Kobel
 */

/* Virtual communication port for STM32  USB CDC */


#include "VCP.h"
#include "main.h"
#include "cmsis_os.h"

#define MAXRXBUFLEN		16

static uint16_t rload_ptr=0;
static uint16_t rx_index=0;
static uint8_t rx_buf[MAXRXBUFLEN];

/* Create FIFO*/
FIFO RX_FIFO = {.FIFO_POS=0};


uint8_t VCP_write (uint8_t* message,uint16_t length)
{
	uint8_t result = USBD_OK;

	if ((message == NULL) || (length <= 0))
	{
		return USBD_FAIL;
	}

	do
	{
	   result = CDC_Transmit_FS(message,length);
	}
	while(result == USBD_BUSY);

	return result;
}


uint8_t VCP_read(uint8_t* Buf, uint32_t Len)
{
	uint32_t idx = 0;

	/* Check inputs */
	if ((Buf == NULL))
	{
		return 0;
	}

	while(Len)
	{
		rx_buf[rx_index++] = Buf[idx++]; // store data in buffer
		// check if we are on the buffer wrap
		if( rx_index > MAXRXBUFLEN - 1 )
		{
			rx_index = 0;
		}
		Len--;
	}

    return 1;
}




uint8_t VCP_GetRxChar(uint8_t* c)
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


uint8_t VCP_kbhit(void)
{
	if(rload_ptr == rx_index)
	{
	    return 0;
	}
	return 1;
}

