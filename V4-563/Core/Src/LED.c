#include "main.h"
#include "cmsis_os.h"
#include "LED.h"

#define LED_PIN_ON	GPIO_PIN_SET
#define LED_PIN_OFF GPIO_PIN_RESET


extern struct netif gnetif;

static uint32_t LedMask = 1;

uint32_t RedPattern;
uint32_t GreenPattern;
uint32_t YellowPattern;

/*********************************************************************
*
* StatusLed
*
* @brief	Run the status (green) LED
*
* @param	uint32_t nPattern
*
* @return	None
*
*********************************************************************/
void RedLed(uint32_t nPattern)
{

	RedPattern = nPattern;
	if(RedPattern == 0)
	{
		HAL_GPIO_WritePin(RED_LED_PORT, RED_LED, LED_PIN_OFF);
	}
}


/*********************************************************************
*
* YellowLed
*
* @brief	Run the Yellow LED
*
* @param	uint32_t nPattern
*
* @return	None
*
*********************************************************************/
void YellowLed(uint32_t nPattern)
{

	YellowPattern = nPattern;
	if(YellowPattern == 0)
	{
		HAL_GPIO_WritePin(YELLOW_LED_PORT, YELLOW_LED, LED_PIN_OFF);
	}
}


/*********************************************************************
*
* GreenLed
*
* @brief	Run the Green LED
*
* @param	uint32_t nPattern
*
* @return	None
*
*********************************************************************/
void GreenLed(uint32_t nPattern)
{

	GreenPattern = nPattern;
	if(GreenPattern == 0)
	{
		HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED, LED_PIN_OFF);
	}
}


/*********************************************************************
*
* GetLed
*
* @brief	Get the red LED
*
* @param	None
*
* @return	uint32_t nPattern
*
*********************************************************************/
uint32_t GetRedLed(void)
{
	return RedPattern;
}

uint32_t GetYellowLed(void)
{
	return RedPattern;
}

uint32_t GetGreenLed(void)
{
	return GreenPattern;
}


void DoYellowLed(void)
{

	if(YellowPattern)
	{
		if((YellowPattern & LedMask) != 0)
		{
			HAL_GPIO_WritePin(YELLOW_LED_PORT, YELLOW_LED, LED_PIN_ON);
		}
		else
		{
			HAL_GPIO_WritePin(YELLOW_LED_PORT, YELLOW_LED, LED_PIN_OFF);
		}
	}
}

void DoRedLed(void)
{

	if(RedPattern)
	{
		if((RedPattern & LedMask) != 0)
		{
			HAL_GPIO_WritePin(RED_LED_PORT, RED_LED, LED_PIN_ON);
		}
		else
		{
			HAL_GPIO_WritePin(RED_LED_PORT, RED_LED, LED_PIN_OFF);
		}
	}
}

void DoGreenLed(void)
{

	if(GreenPattern)
	{
		if((GreenPattern & LedMask) != 0)
		{
			HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED, LED_PIN_ON);
		}
		else
		{
			HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED, LED_PIN_OFF);
		}
	}
}

void LedTask(void *argument)
{

	while(1)
	{
		DoRedLed();
		DoGreenLed();
		DoYellowLed();

		LedMask <<= 1;
		if(LedMask == 0)
		{
			LedMask = 1;
		}

		osDelay(pdMS_TO_TICKS(100));
	}
}

