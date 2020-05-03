#include "main.h"
#include "cmsis_os.h"

#include "lwip.h"
#include "lwip/init.h"
#include "lwip/netif.h"

#include "LED.h"

#define LED_ON	GPIO_PIN_SET
#define LED_OFF GPIO_PIN_RESET


struct netif gnetif;

static uint32_t LedMask = 1;

static uint32_t RedPattern;
static uint32_t GreenPattern;


/*********************************************************************
*
* HeartbeatLed
*
* @brief	Run the heartbeat (red) LED
*
* @param	uint32_t nPattern
*
* @return	None
*
*********************************************************************/
void HeartbeatLed(uint32_t nPattern)
{
	RedPattern = nPattern;
	if(RedPattern == 0)
	{
		HAL_GPIO_WritePin(RED_LED_PORT, RED_LED, LED_OFF);
	}
}

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
void StatusLed(uint32_t nPattern)
{
	GreenPattern = nPattern;
	if(GreenPattern == 0)
	{
		HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED, LED_OFF);
	}
}


/*********************************************************************
*
* GetStatusLed
*
* @brief	Get the status (green) LED
*
* @param	None
*
* @return	uint32_t nPattern
*
*********************************************************************/
uint32_t GetStatusLed(void)
{
	return GreenPattern;
}


void DoRedLed(void)
{

	if(RedPattern)
	{
		if((RedPattern & LedMask) != 0)
		{
			HAL_GPIO_WritePin(RED_LED_PORT, RED_LED, LED_ON);
		}
		else
		{
			HAL_GPIO_WritePin(RED_LED_PORT, RED_LED, LED_OFF);
		}
	}
}

void DoGreenLed(void)
{

	if(GreenPattern)
	{
		if((GreenPattern & LedMask) != 0)
		{
			HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED, LED_ON);
		}
		else
		{
			HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED, LED_OFF);
		}
	}
}

void LedTask(void* argument)
{

	while(1)
	{
		DoRedLed();
		DoGreenLed();

		LedMask <<= 1;
		if(LedMask == 0)
		{
			LedMask = 1;
		}

		osDelay(pdMS_TO_TICKS(100));


		if(netif_is_link_up(&gnetif))
		{
			HAL_GPIO_WritePin(BLUE_LED_PORT, BLUE_LED, LED_ON);
		}
		else
		{
			HAL_GPIO_WritePin(BLUE_LED_PORT, BLUE_LED, LED_OFF);
		}
	}
}


