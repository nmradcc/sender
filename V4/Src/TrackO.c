/**********************************************************************
*
* SOURCE FILENAME:	Track.c
*
* DATE CREATED:		29/Aug/99
*
* PROGRAMMER:
*
* DESCRIPTION:
*
* COPYRIGHT (c) 1999-2017 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "Track.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

//#define IDLE_IDLE_PACKETS
//#define ENABLE_AT_STARTUP

#define __HAL_TIM_SET_REPETITION(__HANDLE__, __REPETITION__) \
  do{                                                    \
    (__HANDLE__)->Instance->RCR = (__REPETITION__);  \
    (__HANDLE__)->Init.RepetitionCounter = (__REPETITION__);    \
  } while(0)


#define TRACK_A_PIN			GPIO_PIN_9
#define TRACK_A_MODE_E		GPIO_MODE_AF_PP
#define TRACK_A_MODE_D		GPIO_MODE_OUTPUT_PP
#define TRACK_A_PU_PD		GPIO_NOPULL
#define TRACK_A_SPEED		GPIO_SPEED_FREQ_VERY_HIGH
#define TRACK_A_AF			GPIO_AF1_TIM1
#define TRACK_A_PORT		GPIOE

#define TRACK_B_PIN			GPIO_PIN_8
#define TRACK_B_MODE_E		GPIO_MODE_AF_PP
#define TRACK_B_MODE_D		GPIO_MODE_OUTPUT_PP
#define TRACK_B_PU_PD		GPIO_NOPULL
#define TRACK_B_SPEED		GPIO_SPEED_FREQ_VERY_HIGH
#define TRACK_B_AF			GPIO_AF1_TIM1
#define TRACK_B_PORT		GPIOE

#define TRACK_ENABLE_PIN	GPIO_PIN_1
#define TRACK_ENABLE_MODE	GPIO_MODE_OUTPUT_PP
#define TRACK_ENABLE_PU_PD	GPIO_NOPULL
#define TRACK_ENABLE_SPEED	GPIO_SPEED_FREQ_VERY_HIGH
#define TRACK_ENABLE_PORT	GPIOB

#define SCOPE_TRIGGER_Pin	GPIO_PIN_7
#define SCOPE_TRIGGER_MODE	GPIO_MODE_OUTPUT_PP
#define SCOPE_TRIGGER_PU_PD	GPIO_NOPULL
#define SCOPE_TRIGGER_SPEED	GPIO_SPEED_FREQ_VERY_HIGH
#define SCOPE_TRIGGER_Port	GPIOE
enum
{
	TRACK_PREAMBLE,
	TRACK_FIRST_INTERBYTE,
	TRACK_INTERBYTE,
	TRACK_DATA,
} MAIN_TRACK_STATES;




#define PACKET_COMPLETE			1
#define PACKET_NOT_COMPLETE		0

#define BUFFER_AVAILABLE		1
#define BUFFER__NOT_AVAILABLE	0


/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

void MarkPacketUnused(PACKET_BITS* p);

void BuildIdlePacket(uint16_t no_preambles);


/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

PACKET_BITS apIdlePacket[6];
PACKET_BITS apPacket1[80];
PACKET_BITS apPacket2[80];

PACKET_BITS* CurrentPacket;
PACKET_BITS* CurrentPattern;

static TIM_HandleTypeDef	htim1;

TRACK_LOCK TrackLock;

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/

uint8_t bTrackState;

static uint32_t ScopeTriggerBitOffset = 20;
static uint32_t ScopeTriggerBitCount;

static uint32_t BufferAvailable;
static uint32_t PacketComplete;

/**********************************************************************
*
*							CODE
*
**********************************************************************/


/**********************************************************************
*
* FUNCTION:		MainTrackConfig
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void MainTrackConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef sConfigOC;
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

	__HAL_RCC_TIM1_CLK_ENABLE();

	htim1.Instance = TIM1;
	htim1.Init.Prescaler = TIMER_PRESCALER;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = ONE_PERIOD;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
	{
		Error_Handler();
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
	sClockSourceConfig.ClockPrescaler = TIM_ETRPRESCALER_DIV1;
	sClockSourceConfig.ClockFilter = 0;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
	{
		Error_Handler();
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = ONE_PULSE;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_SET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
	}

	//sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_ENABLE;
	//sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_ENABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_LOW;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
	{
		Error_Handler();
	}

	//__HAL_TIM_MOE_ENABLE(&htim1);


	MarkPacketUnused(apPacket1);
	MarkPacketUnused(apPacket2);

	BuildIdlePacket(NO_OF_PREAMBLE_BITS);
	CurrentPacket = apIdlePacket;
	CurrentPattern = apIdlePacket+1;

	__HAL_TIM_SET_AUTORELOAD(&htim1, apIdlePacket[0].period);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, apIdlePacket[0].pulse);
	__HAL_TIM_SET_REPETITION(&htim1, apIdlePacket[0].count);

	__HAL_RCC_GPIOE_CLK_ENABLE();

	/* Tim1 Chan1 GPIO pin configuration  */
	GPIO_InitStruct.Pin       = TRACK_A_PIN;
	GPIO_InitStruct.Mode      = TRACK_A_MODE_D;
	GPIO_InitStruct.Pull      = TRACK_A_PU_PD;
	GPIO_InitStruct.Speed     = TRACK_A_SPEED;
	GPIO_InitStruct.Alternate = TRACK_A_AF;
	HAL_GPIO_Init(TRACK_A_PORT, &GPIO_InitStruct);

	/* Tim1 Chan1n GPIO pin configuration  */
	GPIO_InitStruct.Pin 	  = TRACK_B_PIN;
	GPIO_InitStruct.Mode      = TRACK_B_MODE_D;
	GPIO_InitStruct.Pull      = TRACK_B_PU_PD;
	GPIO_InitStruct.Speed     = TRACK_B_SPEED;
	GPIO_InitStruct.Alternate = TRACK_B_AF;
	HAL_GPIO_Init(TRACK_B_PORT, &GPIO_InitStruct);

	/* Tim1 Enable GPIO pin configuration  */
	GPIO_InitStruct.Pin 	  = TRACK_ENABLE_PIN;
	GPIO_InitStruct.Mode      = TRACK_ENABLE_MODE;
	GPIO_InitStruct.Pull      = TRACK_ENABLE_PU_PD;
	GPIO_InitStruct.Speed     = TRACK_ENABLE_SPEED;
	HAL_GPIO_Init(TRACK_ENABLE_PORT, &GPIO_InitStruct);

	/*Configure GPIO pin : Scope Trigger */
	GPIO_InitStruct.Pin = SCOPE_TRIGGER_Pin;
	GPIO_InitStruct.Mode = SCOPE_TRIGGER_MODE;
	GPIO_InitStruct.Pull = SCOPE_TRIGGER_PU_PD;
	GPIO_InitStruct.Speed = SCOPE_TRIGGER_SPEED;
	HAL_GPIO_Init(SCOPE_TRIGGER_Port, &GPIO_InitStruct);

	//Enable Timer 1 interrupts
	HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0, 3);
//	HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
//	__HAL_TIM_ENABLE_IT(&htim1, TIM_IT_UPDATE);

	PacketComplete = PACKET_COMPLETE;
	BufferAvailable = BUFFER_AVAILABLE;

	#ifdef ENABLE_AT_STARTUP
		EnableTrack();
	#else
		DisableTrack();
	#endif

	if(HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
	}

	if(HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
	}
}


/**********************************************************************
*
* FUNCTION:		TIM1_BRK_UP_TRG_COM_IRQHandler
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void TIM1_UP_TIM10_IRQHandler(void)
{


	__HAL_TIM_SET_AUTORELOAD(&htim1, CurrentPattern->period);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, CurrentPattern->pulse);
	__HAL_TIM_SET_REPETITION(&htim1, CurrentPattern->count);


	ScopeTriggerBitCount--;
	if(ScopeTriggerBitCount == 0)
	{
		HAL_GPIO_WritePin(SCOPE_TRIGGER_Port, SCOPE_TRIGGER_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(SCOPE_TRIGGER_Port, SCOPE_TRIGGER_Pin, GPIO_PIN_RESET);
	}


	CurrentPattern++;

	if(CurrentPattern->period == 0)
	{
		if(CurrentPacket != apIdlePacket)
		{
			MarkPacketUnused(CurrentPacket);
		}

		if(apPacket1[0].period != 0)
		{
			CurrentPacket = apPacket1;
			CurrentPattern = apPacket1;
			ScopeTriggerBitCount = ScopeTriggerBitOffset;
		}
		else if(apPacket2[0].period != 0)
		{
			CurrentPacket = apPacket2;
			CurrentPattern = apPacket2;
			ScopeTriggerBitCount = ScopeTriggerBitOffset;
		}
		else
		{
			#ifdef IDLE_IDLE_PACKETS
				CurrentPacket = apIdlePacket;
				CurrentPattern = apIdlePacket;
				ScopeTriggerBitCount = ScopeTriggerBitOffset;
			#else
				DisableTrack();
			#endif

			PacketComplete = PACKET_COMPLETE;
		}
	}

    HAL_TIM_IRQHandler(&htim1);

}




/**********************************************************************
*
* FUNCTION:		MarkPacketUnused
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void MarkPacketUnused(PACKET_BITS* p)
{
	p->count = 0;
	p->period = 0;
	p->pulse = 0;
}




PACKET_BITS* BuildPreamble(PACKET_BITS* p, uint16_t cnt, uint16_t clk1t)
{

	for(int i = 0; i < cnt; i++)
	{
		p->count = 0;
		p->period = clk1t;
		p->pulse = clk1t/2;
		p++;
	}
	return p;
}

/**********************************************************************
*
* FUNCTION:		BuileIdlePacket
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void BuildIdlePacket(uint16_t no_preambles)
{

	// ToDo - change this to use the programmed clock registers
	// preamble
	apIdlePacket[0].count = no_preambles-1;
	apIdlePacket[0].period = ONE_PERIOD;
	apIdlePacket[0].pulse = ONE_PULSE;
	//pBuildPacket = BuildPreambel(pBuildPacket, no_preambles, ONE_PERIOD);

	// first interbyte
	apIdlePacket[1].count = 0;
	apIdlePacket[1].period = ZERO_PERIOD;
	apIdlePacket[1].pulse = ZERO_PULSE;

	// first byte
	apIdlePacket[2].count = 7;
	apIdlePacket[2].period = ONE_PERIOD;
	apIdlePacket[2].pulse = ONE_PULSE;

	// second byte and second interbyte
	apIdlePacket[3].count = 9;
	apIdlePacket[3].period = ZERO_PERIOD;
	apIdlePacket[3].pulse = ZERO_PULSE;

	// third byte
	apIdlePacket[4].count = 8;
	apIdlePacket[4].period = ONE_PERIOD;
	apIdlePacket[4].pulse = ONE_PULSE;

	// terminator
	apIdlePacket[5].count = 0;
	apIdlePacket[5].period = 0;
	apIdlePacket[5].pulse = 0;
}


/**********************************************************************
*
* FUNCTION:		BuildPacket
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void BuildPacket(const uint8_t* buf, uint8_t len, uint16_t clk1t, uint16_t clk0t, uint16_t clk0h)
{
	PACKET_BITS* pBuildPacket;
	PACKET_BITS* pPacket;
	uint8_t mask;
	uint8_t packet_byte;


	if(apPacket1[0].period == 0)
	{
		pBuildPacket = apPacket1;
	}
	else if(apPacket2[0].period == 0)
	{
		pBuildPacket = apPacket2;
	}
	else
	{
		return;		// return an error?
	}

	clk1t *= TICKS_PER_MICROSECOND;
	clk0t *= TICKS_PER_MICROSECOND;
	clk0h *= TICKS_PER_MICROSECOND;

	pPacket = pBuildPacket;

	// preamble
	pBuildPacket = BuildPreamble(pBuildPacket, 18, clk1t);

	// first interbyte
	pBuildPacket->count = 0;
	pBuildPacket->period = clk0t;
	pBuildPacket->pulse = clk0h;
	pBuildPacket++;

	for(int i = 0; i < len; i++)
	{
		mask = 0x01;
		packet_byte = *buf++;
		for(int b = 0; b < 8; b++)
		{
			if(packet_byte & mask)
			{
				// build a one bit
				pBuildPacket->count = 0;
				pBuildPacket->period = clk1t;
				pBuildPacket->pulse = clk1t/2;
			}
			else
			{
				// build a zero bit
				pBuildPacket->count = 0;
				pBuildPacket->period = clk0t;
				pBuildPacket->pulse = clk0h;
			}
			pBuildPacket++;
			mask <<= 1;
		}

		// interbyte
		pBuildPacket->count = 0;
		pBuildPacket->period = clk0t;
		pBuildPacket->pulse = clk0h;
		pBuildPacket++;
	}

	// terminator
	pBuildPacket->count = 0;
	pBuildPacket->period = 0;
	pBuildPacket->pulse = 0;

	if(PacketComplete == PACKET_COMPLETE)
	{
		__HAL_TIM_SET_AUTORELOAD(&htim1, pPacket->period);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pPacket->pulse);
		__HAL_TIM_SET_REPETITION(&htim1, pPacket->count);

		ScopeTriggerBitCount = ScopeTriggerBitOffset;
		CurrentPacket = apIdlePacket;
		CurrentPattern = apIdlePacket;

		PacketComplete = PACKET_NOT_COMPLETE;

		EnableTrack();
	}
}


/**********************************************************************
*
* FUNCTION:		BuildPacketBytes
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void BuildPacketBytes(const uint8_t packet_byte, uint8_t count, uint16_t clk1t, uint16_t clk0t, uint16_t clk0h)
{
	PACKET_BITS* pBuildPacket;
	PACKET_BITS* pPacket;
	uint8_t mask;


	if(apPacket1[0].period == 0)
	{
		pBuildPacket = apPacket1;
	}
	else if(apPacket2[0].period == 0)
	{
		pBuildPacket = apPacket2;
	}
	else
	{
		return;		// return an error?
	}

	clk1t *= TICKS_PER_MICROSECOND;
	clk0t *= TICKS_PER_MICROSECOND;
	clk0h *= TICKS_PER_MICROSECOND;

	pPacket = pBuildPacket;

	for(int i = 0; i < count; i++)
	{
		mask = 0x01;
		for(int b = 0; b < 8; b++)
		{
			if(packet_byte & mask)
			{
				// build a one bit
				pBuildPacket->count = 0;
				pBuildPacket->period = clk1t;
				pBuildPacket->pulse = clk1t/2;
			}
			else
			{
				// build a zero bit
				pBuildPacket->count = 0;
				pBuildPacket->period = clk0t;
				pBuildPacket->pulse = clk0h;
			}
			pBuildPacket++;
			mask <<= 1;
		}

		// interbyte
		pBuildPacket->count = 0;
		pBuildPacket->period = clk0t;
		pBuildPacket->pulse = clk0h;
		pBuildPacket++;
	}

	// terminator
	pBuildPacket->count = 0;
	pBuildPacket->period = 0;
	pBuildPacket->pulse = 0;

	//__HAL_TIM_SET_AUTORELOAD(&htim1, CurrentPattern->period);
	//__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, CurrentPattern->pulse);
	//__HAL_TIM_SET_REPETITION(&htim1, CurrentPattern->count);
	//ScopeTriggerBitCount = ScopeTriggerBitOffset;
	//PacketComplete = PACKET_NOT_COMPLETE;
	//EnableTrack();

	if(PacketComplete == PACKET_COMPLETE)
	{
		__HAL_TIM_SET_AUTORELOAD(&htim1, pPacket->period);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pPacket->pulse);
		__HAL_TIM_SET_REPETITION(&htim1, pPacket->count);

		ScopeTriggerBitCount = ScopeTriggerBitOffset;
		CurrentPacket = apIdlePacket;
		CurrentPattern = apIdlePacket;

		PacketComplete = PACKET_NOT_COMPLETE;

		EnableTrack();
	}
}


/**********************************************************************
*
* FUNCTION:		BuildPacketAmbig1
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void BuildPacketAmbig1(const uint8_t packet_byte, uint16_t clk1t, uint16_t clk0t1, uint16_t clk0h1, uint16_t clk0t, uint16_t clk0h)
{
	PACKET_BITS* pBuildPacket;
	PACKET_BITS* pPacket;
	uint8_t mask;


	if(apPacket1[0].period == 0)
	{
		pBuildPacket = apPacket1;
	}
	else if(apPacket2[0].period == 0)
	{
		pBuildPacket = apPacket2;
	}
	else
	{
		return;		// return an error?
	}

	clk1t *= TICKS_PER_MICROSECOND;
	clk0t1 *= TICKS_PER_MICROSECOND;
	clk0h1 *= TICKS_PER_MICROSECOND;
	clk0t *= TICKS_PER_MICROSECOND;
	clk0h *= TICKS_PER_MICROSECOND;

	pPacket = pBuildPacket;

	// set the zero stretch for the first bit
	pBuildPacket->count = 0;
	pBuildPacket->period = clk0t1;
	pBuildPacket->pulse = clk0h1;
	pBuildPacket++;

	// do the rest
	mask = 0x02;
	for(int b = 0; b < 7; b++)
	{
		if(packet_byte & mask)
		{
			// build a one bit
			pBuildPacket->count = 0;
			pBuildPacket->period = clk1t;
			pBuildPacket->pulse = clk1t/2;
		}
		else
		{
			// build a zero bit
			pBuildPacket->count = 0;
			pBuildPacket->period = clk0t;
			pBuildPacket->pulse = clk0h;
		}
		pBuildPacket++;
		mask <<= 1;
	}

	// interbyte
	pBuildPacket->count = 0;
	pBuildPacket->period = clk0t1;
	pBuildPacket->pulse = clk0h1;
	pBuildPacket++;


	// terminator
	pBuildPacket->count = 0;
	pBuildPacket->period = 0;
	pBuildPacket->pulse = 0;

	//__HAL_TIM_SET_AUTORELOAD(&htim1, CurrentPattern->period);
	//__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, CurrentPattern->pulse);
	//__HAL_TIM_SET_REPETITION(&htim1, CurrentPattern->count);
	//ScopeTriggerBitCount = ScopeTriggerBitOffset;
	//PacketComplete = PACKET_NOT_COMPLETE;
	//EnableTrack();

	if(PacketComplete == PACKET_COMPLETE)
	{
		__HAL_TIM_SET_AUTORELOAD(&htim1, pPacket->period);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pPacket->pulse);
		__HAL_TIM_SET_REPETITION(&htim1, pPacket->count);

		ScopeTriggerBitCount = ScopeTriggerBitOffset;
		CurrentPacket = apIdlePacket;
		CurrentPattern = apIdlePacket;

		PacketComplete = PACKET_NOT_COMPLETE;

		EnableTrack();
	}
}


/**********************************************************************
*
* FUNCTION:		BuildPacketAmbig2
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void BuildPacketAmbig2(const uint8_t packet_byte, uint16_t clk1t, uint16_t clk0t1, uint16_t clk0h1, uint16_t clk0t2, uint16_t clk0h2, uint16_t clk0t, uint16_t clk0h)
{
	PACKET_BITS* pBuildPacket;
	PACKET_BITS* pPacket;
	uint8_t mask;


	if(apPacket1[0].period == 0)
	{
		pBuildPacket = apPacket1;
	}
	else if(apPacket2[0].period == 0)
	{
		pBuildPacket = apPacket2;
	}
	else
	{
		return;		// return an error?
	}

	clk1t *= TICKS_PER_MICROSECOND;
	clk0t1 *= TICKS_PER_MICROSECOND;
	clk0h1 *= TICKS_PER_MICROSECOND;
	clk0t2 *= TICKS_PER_MICROSECOND;
	clk0h2 *= TICKS_PER_MICROSECOND;
	clk0t *= TICKS_PER_MICROSECOND;
	clk0h *= TICKS_PER_MICROSECOND;

	pPacket = pBuildPacket;

	// set the zero stretch for the first bit
	pBuildPacket->count = 0;
	pBuildPacket->period = clk0t1;
	pBuildPacket->pulse = clk0h1;
	pBuildPacket++;

	// set the zero stretch for the second bit
	pBuildPacket->count = 0;
	pBuildPacket->period = clk0t2;
	pBuildPacket->pulse = clk0h2;
	pBuildPacket++;

	// do the rest
	mask = 0x04;
	for(int b = 0; b < 6; b++)
	{
		if(packet_byte & mask)
		{
			// build a one bit
			pBuildPacket->count = 0;
			pBuildPacket->period = clk1t;
			pBuildPacket->pulse = clk1t/2;
		}
		else
		{
			// build a zero bit
			pBuildPacket->count = 0;
			pBuildPacket->period = clk0t;
			pBuildPacket->pulse = clk0h;
		}
		pBuildPacket++;
		mask <<= 1;
	}

	// interbyte
	pBuildPacket->count = 0;
	pBuildPacket->period = clk0t1;
	pBuildPacket->pulse = clk0h1;
	pBuildPacket++;


	// terminator
	pBuildPacket->count = 0;
	pBuildPacket->period = 0;
	pBuildPacket->pulse = 0;

	//__HAL_TIM_SET_AUTORELOAD(&htim1, CurrentPattern->period);
	//__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, CurrentPattern->pulse);
	//__HAL_TIM_SET_REPETITION(&htim1, CurrentPattern->count);
	//ScopeTriggerBitCount = ScopeTriggerBitOffset;
	//PacketComplete = PACKET_NOT_COMPLETE;
	//EnableTrack();

	if(PacketComplete == PACKET_COMPLETE)
	{
		__HAL_TIM_SET_AUTORELOAD(&htim1, pPacket->period);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pPacket->pulse);
		__HAL_TIM_SET_REPETITION(&htim1, pPacket->count);

		ScopeTriggerBitCount = ScopeTriggerBitOffset;
		CurrentPacket = apIdlePacket;
		CurrentPattern = apIdlePacket;

		PacketComplete = PACKET_NOT_COMPLETE;

		EnableTrack();
	}
}

/**********************************************************************
*
* FUNCTION:		BuildPacketBytes
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void BuildPacketBits(const PACKET_BITS* packet, uint8_t count)
{
	PACKET_BITS* pBuildPacket;
	PACKET_BITS* pPacket;
	//uint8_t mask;


	if(apPacket1[0].period == 0)
	{
		pBuildPacket = apPacket1;
	}
	else if(apPacket2[0].period == 0)
	{
		pBuildPacket = apPacket2;
	}
	else
	{
		return;		// return an error?
	}

	pPacket = pBuildPacket;

	memcpy((char*)pBuildPacket, (char*)packet, sizeof(PACKET_BITS));
	
	if(PacketComplete == PACKET_COMPLETE)
	{
		__HAL_TIM_SET_AUTORELOAD(&htim1, pPacket->period);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pPacket->pulse);
		__HAL_TIM_SET_REPETITION(&htim1, pPacket->count);

		ScopeTriggerBitCount = ScopeTriggerBitOffset;
		CurrentPacket = apIdlePacket;
		CurrentPattern = apIdlePacket;

		PacketComplete = PACKET_NOT_COMPLETE;

		EnableTrack();
	}
}


/**********************************************************************
*
* FUNCTION:		EnableTrackA / DisableTrackA
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void EnableTrack(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	HAL_GPIO_WritePin(TRACK_ENABLE_PORT, TRACK_ENABLE_PIN, GPIO_PIN_SET);


	/* Tim1 Chan1 GPIO pin configuration  */
	GPIO_InitStruct.Pin       = TRACK_A_PIN;
	GPIO_InitStruct.Mode      = TRACK_A_MODE_E;
	GPIO_InitStruct.Pull      = TRACK_A_PU_PD;
	GPIO_InitStruct.Speed     = TRACK_A_SPEED;
	GPIO_InitStruct.Alternate = TRACK_A_AF;
	HAL_GPIO_Init(TRACK_A_PORT, &GPIO_InitStruct);

	/* Tim1 Chan1n GPIO pin configuration  */
	GPIO_InitStruct.Pin 	  = TRACK_B_PIN;
	GPIO_InitStruct.Mode      = TRACK_B_MODE_E;
	GPIO_InitStruct.Pull      = TRACK_B_PU_PD;
	GPIO_InitStruct.Speed     = TRACK_B_SPEED;
	GPIO_InitStruct.Alternate = TRACK_B_AF;
	HAL_GPIO_Init(TRACK_B_PORT, &GPIO_InitStruct);

	HAL_TIM_Base_Start(&htim1);

	//if(HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1) != HAL_OK)
	//{
	//    Error_Handler();
	//}

	//if(HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK)
	//{
	//    Error_Handler();
	//}

	HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
	__HAL_TIM_ENABLE_IT(&htim1, TIM_IT_UPDATE);

	bTrackState = 1;
}

void DisableTrack(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	HAL_GPIO_WritePin(TRACK_ENABLE_PORT, TRACK_ENABLE_PIN, GPIO_PIN_RESET);

	/* Tim1 Chan1 GPIO pin configuration  */
	GPIO_InitStruct.Pin       = TRACK_A_PIN;
	GPIO_InitStruct.Mode      = TRACK_A_MODE_D;
	GPIO_InitStruct.Pull      = TRACK_A_PU_PD;
	GPIO_InitStruct.Speed     = TRACK_A_SPEED;
	GPIO_InitStruct.Alternate = TRACK_A_AF;
	HAL_GPIO_Init(TRACK_A_PORT, &GPIO_InitStruct);

	/* Tim1 Chan1n GPIO pin configuration  */
	GPIO_InitStruct.Pin 	  = TRACK_B_PIN;
	GPIO_InitStruct.Mode      = TRACK_B_MODE_D;
	GPIO_InitStruct.Pull      = TRACK_B_PU_PD;
	GPIO_InitStruct.Speed     = TRACK_B_SPEED;
	GPIO_InitStruct.Alternate = TRACK_B_AF;
	HAL_GPIO_Init(TRACK_B_PORT, &GPIO_InitStruct);

	// stop the timer
	__HAL_TIM_DISABLE_IT(&htim1, TIM_IT_UPDATE);

//k	__HAL_TIM_SET_AUTORELOAD(&htim1, ONE_PERIOD);
//k	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ONE_PERIOD);

	//HAL_TIM_Base_Stop(&htim1);

	HAL_NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn);

	//if(HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1) != HAL_OK)
	//{
	//    Error_Handler();
	//}

	//if(HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1) != HAL_OK)
	//{
	//    Error_Handler();
	//}

	bTrackState = 0;
}


/**********************************************************************
*
* FUNCTION:		GetTrackState
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
uint8_t GetTrackState(void)
{
	return bTrackState;
}


/**********************************************************************
*
* FUNCTION:		IsPacketBufferAvailable
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
uint32_t IsPacketBufferAvailable(void)
{
	// ToDo - guard this against the interrupt
	if(apPacket1[0].period == 0)
	{
		return 1;
	}
	else if(apPacket2[0].period == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}

	return BufferAvailable;

}


/**********************************************************************
*
* FUNCTION:		IsPacketComplete
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
uint32_t IsPacketComplete(void)
{
	return PacketComplete;
}


/**********************************************************************
*
* FUNCTION:		OpenTrack
*
* ARGUMENTS:	tr - which procees is requesting the track, one of TRACK_RESOURCE 
*				ti - what should the track generator do when it runs out of packets,
*					 one of TRACK_IDLE 
*				preambles -  number of preambles, 0 = used default
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
TRACK_LOCK_STATUS OpenTrack(TRACK_RESOURCE tr, TRACK_IDLE ti, uint16_t preambles)
{
	// ToDo - make this threadsafe
	
	if(TrackLock.lock == tr)
	{
		// track resource assigned
		return TL_ASSIGNED;
	}
	else if(TrackLock.lock == TR_NONE)
	{
		// assign track resource
		TrackLock.lock = tr;
		TrackLock.idle = ti;
		TrackLock.preambles = preambles;
		return TL_ASSIGNED;
	}
	else
	{
		// track resource locked
		return TL_LOCKED;
	}
}


/**********************************************************************
*
* FUNCTION:		CloseTrack
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
void CloseTrack(void)
{
	// ToDo - make this threadsafe

	TrackLock.lock = TR_NONE;
	TrackLock.idle = TI_NONE;
	TrackLock.preambles = 0;
}


/**********************************************************************
*
* FUNCTION:		IsTrackOpen
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
uint8_t IsTrackOpen(TRACK_RESOURCE tr)
{
	// ToDo - make this threadsafe

	return TrackLock.lock == tr;
}
