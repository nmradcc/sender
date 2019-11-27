#include "main.h"
#include "cmsis_os.h"

#include "lwip.h"
#include "lwip/init.h"
#include "lwip/netif.h"

#include "LED.h"

struct netif gnetif;

static int LedBlink = 0;

void StatusLed(int s)
{

	if(s == LED_BLINK)
	{
		LedBlink = 1;
	}
	else if(s == LED_ON)
	{
		LedBlink = 0;
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
	}
	else
	{
		LedBlink = 0;
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
	}
}


void LedTask(void* argument)
{

	while(1)
	{
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
		if(LedBlink)
		{
			HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
		}
		osDelay(pdMS_TO_TICKS(500));
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
		if(LedBlink)
		{
			HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
		}
		osDelay(pdMS_TO_TICKS(500));


		if(netif_is_link_up(&gnetif))
		{
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		}
		else
		{
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		}
	}
}
