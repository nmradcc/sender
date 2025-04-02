/*******************************************************************************
* @file Input.c
* @brief The input module
*
* @author K. Kobel
* @date 11/29/2020
* @Revision: 1 $
*
* @copyright	(c) 2020  all Rights Reserved.
*******************************************************************************/
#include "main.h"
#include "BitMask.h"
#include "cmsis_os.h"
//#include "Acknowledge.h"
//#include "app_threadx.h"
//#include "app_azure_rtos.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define DEBOUNCE_NONE   0x01
#define DEBOUNCE_20MS   0x03
#define DEBOUNCE_30MS   0x07
#define DEBOUNCE_40MS   0x0f
#define DEBOUNCE_50MS   0x1f
#define DEBOUNCE_60MS   0x3f
#define DEBOUNCE_70MS   0x7f
#define DEBOUNCE_80MS   0xff

uint8_t abDebounce[] =
{
	0x0f,
	0x0f,
	0x0f,
	0x0f,
	0x0f,
	0x0f,
	0x0f,
	0x0f,
};

/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

static unsigned int DebounceInputs(uint32_t Raw, unsigned char const abDebounceValue[], int num_inputs);

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

//TX_THREAD input_thread;

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/

static uint8_t abDebounceValue[8];
unsigned int Inputs;

//__ALIGN_BEGIN static UCHAR  input_byte_pool_buffer[INPUT_APP_MEM_POOL_SIZE] __ALIGN_END;
//static TX_BYTE_POOL input_app_byte_pool;

/**********************************************************************
*
*							CODE
*
**********************************************************************/

//*******************************************************************************
//
// DebounceInputs
//
//  Inputs are debounced by shifting a byte depending on the value of the input
//
//  If the input is high, the debounce register is shifted left and a high bit is
//  added to the LSB. If the input continues to be high, the debounce register
//  will fill in the bits and match the debounce value for that input.
//  The coorisponding bit in the input status is set high.
//
//  If the input is low, the debounce register is shifted right (zeros will fill in).
//  If the input continues to be low, the debounce register will eventually equal zero.
//  The coorisponding bit in the input status is set low.
//
//*******************************************************************************
static unsigned int DebounceInputs(uint32_t Raw, unsigned char const abDebounceValue[], int num_inputs)
{
    int i;
    uint32_t  xDebounced = 0;
    unsigned char bDebounceValue = 0x01;

    for(i = 0; i < num_inputs; i++)
    {
        abDebounce[i] <<= 1;

        if(Raw & wBitMask[i])
        {
            abDebounce[i] |= 1;
        }

        if((abDebounce[i] & bDebounceValue) == bDebounceValue)
        {
            xDebounced |= wBitMask[i];
        }
        else if((abDebounce[i] & bDebounceValue) == 0)
        {
            xDebounced &= ~wBitMask[i];
        }
    }
    return xDebounced;
}


/*********************************************************************
*
* InputTask
*
* @brief	Input task
*
* @param	none
*
* @return	none
*
*********************************************************************/
void InputTask(void* argument)
{
	unsigned int raw_input;

	for(int i = 0; i > 8; i++)
	{
		abDebounceValue[i] = 0;
	}

	while(1)
	{
//k		Acknowledge();
		raw_input = 0;
		raw_input |= HAL_GPIO_ReadPin(IN0_GPIO_Port, IN0_Pin);
		raw_input |= HAL_GPIO_ReadPin(IN1_GPIO_Port, IN1_Pin) << 1;
		raw_input |= HAL_GPIO_ReadPin(IN2_GPIO_Port, IN2_Pin) << 2;
		raw_input |= HAL_GPIO_ReadPin(IN3_GPIO_Port, IN3_Pin) << 3;
		if(HAL_GPIO_ReadPin(USR_BTN_GPIO_Port, USR_BTN_Pin) == GPIO_PIN_RESET)
		{
			raw_input |= 0x10;
		}

		raw_input |= 0x20;
//k		if(GetAck() == ACK_DETECTED)
//k		{
//k			raw_input &= ~0x20;
//k		}

		Inputs = (~DebounceInputs(raw_input, abDebounceValue, 6)) & 0x3f;

		osDelay(pdMS_TO_TICKS(10));
	}
}


/*********************************************************************
*
* GetInputx
*
* @brief	Inputs
*
* @param	none
*
* @return	input state
*
*********************************************************************/
uint32_t GetInput1(void)
{
	return (Inputs & 0x01) != 0;
}

uint32_t GetInput2(void)
{
	return (Inputs & 0x02) != 0;
}

uint32_t GetInput3(void)
{
	return (Inputs & 0x04) != 0;
}

uint32_t GetInput4(void)
{
	return (Inputs & 0x08) != 0;
}

uint32_t GetInput5(void)
{
	return (Inputs & 0x10) != 0;
}

uint32_t GetInput6(void)
{
	return (Inputs & 0x20) != 0;
}

uint32_t GetInputs(void)
{
	return Inputs;
}


