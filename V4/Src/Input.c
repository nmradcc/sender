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

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/

static uint8_t abDebounceValue[8];
unsigned int Inputs;

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
		raw_input = 0;
		raw_input |= HAL_GPIO_ReadPin(IN1_PORT, IN1_PIN);
		raw_input |= HAL_GPIO_ReadPin(IN2_PORT, IN2_PIN) << 1;
		raw_input |= HAL_GPIO_ReadPin(IN3_PORT, IN3_PIN) << 2;
		raw_input |= HAL_GPIO_ReadPin(IN4_PORT, IN4_PIN) << 3;

		Inputs = ~DebounceInputs(raw_input, abDebounceValue, 4);

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

uint32_t GetInputs(void)
{
	return Inputs;
}
