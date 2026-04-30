/*******************************************************************************
* @file Track32.c
* @brief The DCC track output module
*
* @author K. Kobel
* @date 9/15/2019
* @Revision: 24 $
* @Modtime: 10/31/2019
*
* @copyright	(c) 2019  all Rights Reserved.
*******************************************************************************/
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "Track.h"
#include "TrkQueue.h"
#include "Variables.h"
#include "cmsis_os.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define ENABLE_AT_STARTUP

/** 
	@brief Track output pin definitions
 */
#define TRACK_A_PIN			GPIO_PIN_0
#define TRACK_A_MODE_E		GPIO_MODE_AF_PP
#define TRACK_A_MODE_D		GPIO_MODE_OUTPUT_PP
#define TRACK_A_PU_PD		GPIO_NOPULL
#define TRACK_A_SPEED		GPIO_SPEED_FREQ_VERY_HIGH
#define TRACK_A_AF			GPIO_AF1_TIM2
#define TRACK_A_PORT		GPIOA

#define TRACK_ENABLE_PIN	GPIO_PIN_6
#define TRACK_ENABLE_MODE	GPIO_MODE_OUTPUT_PP
#define TRACK_ENABLE_PU_PD	GPIO_NOPULL
#define TRACK_ENABLE_SPEED	GPIO_SPEED_FREQ_VERY_HIGH
#define TRACK_ENABLE_PORT	GPIOB

#define SCOPE_TRIGGER_Pin	GPIO_PIN_7
#define SCOPE_TRIGGER_MODE	GPIO_MODE_OUTPUT_PP
#define SCOPE_TRIGGER_PU_PD	GPIO_NOPULL
#define SCOPE_TRIGGER_SPEED	GPIO_SPEED_FREQ_VERY_HIGH
#define SCOPE_TRIGGER_Port	GPIOE


/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

void TrackIdlePacket(uint16_t no_preambles);
void TrackResetPacket(uint16_t no_preambles);
void TrackOnesPacket(void);
void TrackZerosPacket(void);


/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

PACKET_BITS apIdlePacket[60];
PACKET_BITS apResetPacket[60];
PACKET_BITS apOnesPacket[60];
PACKET_BITS apZerosPacket[60];

PACKET_BITS apPacket[TRACK_QUEUE_DEPTH][TRACK_QUEUE_NO_OF_BITS+1];

static TIM_HandleTypeDef	htim2;

TRACK_IDLE TrackIdleState = TI_NONE;

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/

static uint8_t bTrackState;
static PACKET_BITS* CurrentPacket;
static int CurrentPacketIdx;

static uint8_t TrackRunning = 0;

//static uint32_t ScopeTriggerBitOffset = 12;
//static uint32_t ScopeTriggerBitCount;


/**********************************************************************
*
*							CODE
*
**********************************************************************/


/*********************************************************************
*
* MainTrackConfig
*
* @brief	Track output init 
*
* @param	none
*
* @return	none
*
*********************************************************************/
void MainTrackConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef sConfigOC;
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

	__HAL_RCC_TIM2_CLK_ENABLE();

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = TIMER_PRESCALER;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = ONE_PERIOD;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.RepetitionCounter = 0;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
	{
		Error_Handler();
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
	sClockSourceConfig.ClockPrescaler = TIM_ETRPRESCALER_DIV1;
	sClockSourceConfig.ClockFilter = 0;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
	{
		Error_Handler();
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
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
	if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
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
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim2, &sBreakDeadTimeConfig) != HAL_OK)
	{
		Error_Handler();
	}

	InitTrackQueue();

	TrackIdlePacket(NO_OF_PREAMBLE_BITS);
	TrackResetPacket(NO_OF_PREAMBLE_BITS);
	TrackOnesPacket();
	TrackZerosPacket();

	//#ifdef IDLE_IDLE_PACKETS
	//	pCurrentIdlePacket = apIdlePacket;
	//#endif
	//#ifndef IDLE_IDLE_PACKETS
	//	pCurrentIdlePacket = apOnesPacket;
	//#endif

	CurrentPacket = apIdlePacket;
	CurrentPacketIdx = -1;

	SetTrackIdle(TI_IDLE);

	//CurrentPattern = apIdlePacket+1;

	//__HAL_TIM_SET_AUTORELOAD(&htim2, apIdlePacket[0].period);
	//__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, apIdlePacket[0].pulse);

	//__HAL_RCC_GPIOA_CLK_ENABLE();
	//__HAL_RCC_GPIOB_CLK_ENABLE();
	//__HAL_RCC_GPIOE_CLK_ENABLE();

	/* Tim2 Chan1 GPIO pin configuration  */
	GPIO_InitStruct.Pin       = TRACK_A_PIN;
	GPIO_InitStruct.Mode      = TRACK_A_MODE_D;
	GPIO_InitStruct.Pull      = TRACK_A_PU_PD;
	GPIO_InitStruct.Speed     = TRACK_A_SPEED;
	GPIO_InitStruct.Alternate = TRACK_A_AF;
	HAL_GPIO_Init(TRACK_A_PORT, &GPIO_InitStruct);

	/* Tim2 Enable GPIO pin configuration  */
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
	HAL_NVIC_SetPriority(TIM2_IRQn, 0, 3);
//	HAL_NVIC_EnableIRQ(TIM2_IRQn);
//	__HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);

	#ifdef ENABLE_AT_STARTUP
		EnableTrack();
	#else
		DisableTrack();
	#endif

	//if(HAL_TIMEx_PWMN_Start(&htim2, TIM_CHANNEL_1) != HAL_OK)
	//{
	//	Error_Handler();
	//}

	if(HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
	}
}


/*********************************************************************
*
* TIM2_IRQHandler
*
* @brief	Track output interrupt 
*
* @param	none
*
* @return	none
*
*********************************************************************/
void TIM2_IRQHandler(void)
{

	if(CurrentPacket->scope == 1)
	{
		HAL_GPIO_WritePin(SCOPE_TRIGGER_Port, SCOPE_TRIGGER_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(SCOPE_TRIGGER_Port, SCOPE_TRIGGER_Pin, GPIO_PIN_RESET);
	}

	CurrentPacket++;

	if(CurrentPacket->period == 0)
	{
		// if not a fill packet, pop the old packet off the queue
		ReleasePacket(CurrentPacketIdx);

		// try and get a new packet
		CurrentPacket = GetPacket(&CurrentPacketIdx);
		if(CurrentPacket == NULL)
		{
			switch(GetTrackIdle())
			{
				case TI_NONE:
					// this may stop the output, but it doesn't re-start
//					__HAL_TIM_SET_AUTORELOAD(&htim2, 0);
//					__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);

					HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
					// probably need to clear the timer count
					TrackRunning = 0;
				return;

				case TI_IDLE:
				default:
					CurrentPacket = apIdlePacket;
				break;

				case TI_RESET:
					CurrentPacket = apResetPacket;
				break;

				case TI_ONES:
					CurrentPacket = apOnesPacket;
				break;

				case TI_ZEROS:
					CurrentPacket = apZerosPacket;
				break;
			}
			CurrentPacketIdx = -1;
		}

		__HAL_TIM_SET_AUTORELOAD(&htim2, CurrentPacket->period);
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, CurrentPacket->pulse);
	}
	else
	{
		__HAL_TIM_SET_AUTORELOAD(&htim2, CurrentPacket->period);
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, CurrentPacket->pulse);
	}

	HAL_TIM_IRQHandler(&htim2);
}


/*********************************************************************
*
* TrackIdlePacket
*
* @brief	Pre-build a bit pattern for an idle packet to be ready if 
*			the track generator runs out of packets
*
* @param	number of preambles
*
* @return	none
*
*********************************************************************/
void TrackIdlePacket(uint16_t no_preambles)
{
	int i;
	PACKET_BITS* pPacket = apIdlePacket;

	// ToDo - change this to use the programmed clock registers

	// preamble
	for(i = 0; i < no_preambles; i++)
	{
		pPacket->period = ONE_PERIOD;
		pPacket->pulse = ONE_PULSE;
		pPacket->scope = 0;
		pPacket++;
	}

	// first interbyte
	pPacket->period = ZERO_PERIOD;
	pPacket->pulse = ZERO_PULSE;
	pPacket->scope = 0;
	pPacket++;

	// first byte
	for(i = 0; i < 8; i++)
	{
		pPacket->period = ONE_PERIOD;
		pPacket->pulse = ONE_PULSE;
		pPacket->scope = 0;
		pPacket++;
	}

	// second byte and second interbyte
	for(i = 0; i < 10; i++)
	{
		pPacket->period = ZERO_PERIOD;
		pPacket->pulse = ZERO_PULSE;
		pPacket->scope = 0;
		pPacket++;
	}

	// third byte
	for(i = 0; i < 9; i++)
	{
		pPacket->period = ONE_PERIOD;
		pPacket->pulse = ONE_PULSE;
		pPacket->scope = 0;
		pPacket++;
	}

	// terminator
	pPacket->period = 0;
	pPacket->pulse = 0;
	pPacket->scope = 0;
}


/*********************************************************************
*
* TrackResetPacket
*
* @brief	Pre-build a bit pattern for an Reset packet to be ready if
*			the track generator runs out of packets
*
* @param	number of preambles
*
* @return	none
*
*********************************************************************/
void TrackResetPacket(uint16_t no_preambles)
{
	int i;
	PACKET_BITS* pPacket = apResetPacket;

	// ToDo - change this to use the programmed clock registers

	// preamble
	for(i = 0; i < no_preambles; i++)
	{
		pPacket->period = ONE_PERIOD;
		pPacket->pulse = ONE_PULSE;
		pPacket->scope = 0;
		pPacket++;
	}

	// first interbyte
	pPacket->period = ZERO_PERIOD;
	pPacket->pulse = ZERO_PULSE;
	pPacket->scope = 0;
	pPacket++;

	// first byte
	for(i = 0; i < 8; i++)
	{
		pPacket->period = ZERO_PERIOD;
		pPacket->pulse = ZERO_PULSE;
		pPacket->scope = 0;
		pPacket++;
	}

	// second byte and second interbyte
	for(i = 0; i < 10; i++)
	{
		pPacket->period = ZERO_PERIOD;
		pPacket->pulse = ZERO_PULSE;
		pPacket->scope = 0;
		pPacket++;
	}

	// third byte
	for(i = 0; i < 9; i++)
	{
		pPacket->period = ZERO_PERIOD;
		pPacket->pulse = ZERO_PULSE;
		pPacket->scope = 0;
		pPacket++;
	}

	// terminator
	pPacket->period = 0;
	pPacket->pulse = 0;
	pPacket->scope = 0;
}


/*********************************************************************
*
* TrackOnesPacket
*
* @brief	Pre-build a bit pattern for a packet of ones to be ready if
*			the track generator runs out of packets
*
* @param	none
*
* @return	none
*
*********************************************************************/
void TrackOnesPacket(void)
{
	int i;
	PACKET_BITS* pPacket = apOnesPacket;

	// ones
	for(i = 0; i < 20; i++)
	{
		pPacket->period = ONE_PERIOD;
		pPacket->pulse = ONE_PULSE;
		pPacket->scope = 0;
		pPacket++;
	}

	// terminator
	pPacket->period = 0;
	pPacket->pulse = 0;
	pPacket->scope = 0;
}

/*********************************************************************
*
* TrackZerosPacket
*
* @brief	Pre-build a bit pattern for a packet of zeros to be ready if
*			the track generator runs out of packets
*
* @param	none
*
* @return	none
*
*********************************************************************/
void TrackZerosPacket(void)
{
	int i;
	PACKET_BITS* pPacket = apZerosPacket;

	// ones
	for(i = 0; i < 20; i++)
	{
		pPacket->period = ZERO_PERIOD;
		pPacket->pulse = ZERO_PULSE;
		pPacket->scope = 0;
		pPacket++;
	}

	// terminator
	pPacket->period = 0;
	pPacket->pulse = 0;
	pPacket->scope = 0;
}


/*********************************************************************
*
* CheckTrackRestart
*
* @brief	restart the timer if it is not running
*
* @param	none
*
* @return	none
*
*********************************************************************/
void CheckTrackRestart(void)
{
	// if the track idle is NONE and not running, we need to restart the timer and prime the current packet
	if(GetTrackIdle() == TI_NONE && TrackRunning == 0)
	{
		CurrentPacket = GetPacket(&CurrentPacketIdx);
		// we just loaded a packet, this will not be NULL
		HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
		TrackRunning = 1;
	}
}

/*********************************************************************
*
* PacketToTrack
*
* @brief	Build a bit pattern for a packet based on the bits in buf,
*			and the bit widths in  clk1t, clk0t, and clk0h
*
* @param	pointer to packet bytes
*			length
*			1 total width
*			0 total width
*			0 first half width
*
* @return	0 = sucess
*
*********************************************************************/
int PacketToTrack(const uint8_t* buf, uint32_t len, uint32_t clk1t, uint32_t clk0t, uint32_t clk0h)
{
	PACKET_BITS* pPacket;
	int PacketIndex;
	int BitCount = 0;
	uint8_t mask;
	uint8_t packet_byte;
	uint32_t Tick1;
	uint32_t Tick0t;
	uint32_t Tick0h;


	pPacket = AcquirePacket(&PacketIndex);
	if(pPacket == NULL)
	{
		osDelay(pdMS_TO_TICKS(1));
		return 1;
	}

	Tick1  = clk1t * TICKS_PER_MICROSECOND;
	Tick0t = clk0t * TICKS_PER_MICROSECOND;
	Tick0h = clk0h * TICKS_PER_MICROSECOND;

	for(int i = 0; i < len; i++)
	{
		mask = 0x80;
		packet_byte = *buf++;
		for(int b = 0; b < 8; b++)
		{
			if(packet_byte & mask)
			{
				// build a one bit
				pPacket->period = Tick1;
				pPacket->pulse = Tick1/2;
			}
			else
			{
				// build a zero bit
				pPacket->period = Tick0t;
				pPacket->pulse = Tick0h;

			}
			pPacket++;
			BitCount++;
			mask >>= 1;

			if(BitCount == 11)
			{
				pPacket->scope = 1;
			}
			else
			{
				pPacket->scope = 0;
			}
		}
	}

	// terminator
	pPacket->period = 0;
	pPacket->pulse = 0;
	pPacket->scope = 0;

	ReadyPacket(PacketIndex);

	CheckTrackRestart();
	return 0;
}



/*********************************************************************
*
* PacketToTrackCS
*
* @brief	Build a bit pattern for a packet based on the bits in buf,
*			and the bit widths in  clk1t, clk0t, and clk0h
*
* @param	pointer to packet bytes
*			length
*			1 total width
*			0 total width
*			0 first half width
*			preamble bits
*
* @return	0 = sucess
*
*********************************************************************/
int PacketToTrackCS(const uint8_t* buf, uint8_t len, uint32_t no_preambles)
{
	PACKET_BITS* pPacket;
	int PacketIndex;
	int BitCount = 0;
	uint8_t mask;
	uint8_t packet_byte;


	pPacket = AcquirePacket(&PacketIndex);
	if(pPacket == NULL)
	{
		return 1;
	}

	for(int i = 0; i < no_preambles; i++)
	{
		pPacket->period = ONE_PERIOD;
		pPacket->pulse = ONE_PULSE;
		pPacket->scope = 0;
		pPacket++;
	}

	// interbyte bit
	pPacket->period = ZERO_PERIOD;
	pPacket->pulse = ZERO_PULSE;
	pPacket->scope = 1;
	pPacket++;

	for(int i = 0; i < len; i++)
	{
		mask = 0x80;
		packet_byte = *buf++;
		for(int b = 0; b < 8; b++)
		{
			if(packet_byte & mask)
			{
				// build a one bit
				pPacket->period = ONE_PERIOD;
				pPacket->pulse = ONE_PULSE;
			}
			else
			{
				// build a zero bit
				pPacket->period = ZERO_PERIOD;
				pPacket->pulse = ZERO_PULSE;
			}
			pPacket->scope = 0;
			pPacket++;
			BitCount++;
			mask >>= 1;
		}

		if(i == len-1)
		{
			// packet end bit
			pPacket->period = ONE_PERIOD;
			pPacket->pulse = ONE_PULSE;
		}
		else
		{
			// interbyte bit
			pPacket->period = ZERO_PERIOD;
			pPacket->pulse = ZERO_PULSE;
		}
		pPacket->scope = 0;
		pPacket++;
	}

	// terminator
	pPacket->period = 0;
	pPacket->pulse = 0;
	pPacket->scope = 0;

	ReadyPacket(PacketIndex);

	// probably don't need to do this, the track idle for the Command Station should be IDLE
	CheckTrackRestart();
	return 0;
}


/*********************************************************************
*
* PacketToTrackBytes
*
* @brief	Build a bit pattern for a packet based on repeating the byte
*			in packet_byte, and the bit widths in  clk1t, clk0t, and clk0h
*
* @param	packet byte
*			count
*			1 total width
*			0 total width
*			0 first half width
*
* @return	0 = success
*
*********************************************************************/
int PacketToTrackBytes(const uint8_t packet_byte, uint8_t count, uint32_t clk1t, uint32_t clk0t, uint32_t clk0h)
{
	uint8_t buf[6];

	if(count < 6)
	{
		memset(buf, packet_byte, count);
		return PacketToTrack(buf, count, clk1t, clk0t, clk0h);
	}

	return 1;
}


/*********************************************************************
*
* PacketToTrackByte
*
* @brief	Build a bit pattern for a packet based on repeating the byte
*			in packet_byte, and the bit widths in  clk1t, clk0t, and clk0h
*
* @param	packet byte
*			count
*			1 total width
*			0 total width
*			0 first half width
*
* @return	0 = success
*
*********************************************************************/
int PacketToTrackByte(const uint8_t packet_byte, uint32_t clk1t, uint32_t clk0t, uint32_t clk0h, uint32_t clk0ts)
{
	PACKET_BITS* pPacket;
	int PacketIndex;
	uint8_t mask;
	uint32_t Tick1;
	uint32_t Tick0t;
	uint32_t Tick0h;
	uint32_t Tick0ts;
	uint32_t FirstZero = 1;

	pPacket = AcquirePacket(&PacketIndex);
	if(pPacket == NULL)
	{
		osDelay(pdMS_TO_TICKS(1));
		return 1;
	}

	Tick1  = clk1t * TICKS_PER_MICROSECOND;
	Tick0t = clk0t * TICKS_PER_MICROSECOND;
	Tick0h = clk0h * TICKS_PER_MICROSECOND;
	Tick0ts = clk0ts * TICKS_PER_MICROSECOND;

	// build a zero byte
	for(int b = 0; b < 8; b++)
	{
		// build a zero bit
		pPacket->period = Tick0t;
		pPacket->pulse = Tick0h;
		pPacket->scope = 0;
		pPacket++;
	}

	// build the preamble
	for(int i = 0; i < 11; i++)
	{
		// build a one bit
		pPacket->period = Tick1;
		pPacket->pulse = Tick1/2;
		pPacket->scope = 0;
		pPacket++;
	}
	// build the last one bit with the scope trigger set
	pPacket->period = Tick1;
	pPacket->pulse = Tick1/2;
	pPacket->scope = 1;
	pPacket++;

	mask = 0x80;
	for(int b = 0; b < 8; b++)
	{
		if(packet_byte & mask)
		{
			// build a one bit
			pPacket->period = Tick1;
			pPacket->pulse = Tick1/2;
		}
		else
		{
			if(FirstZero)
			{
				FirstZero = 0;
				// build a zero bit
				pPacket->period = Tick0ts;
				pPacket->pulse = Tick0h;
			}
			else
			{
				// build a zero bit
				pPacket->period = Tick0t;
				pPacket->pulse = Tick0h;
			}
		}
		pPacket->scope = 0;
		pPacket++;
		mask >>= 1;
	}

	// terminator
	pPacket->period = 0;
	pPacket->pulse = 0;
	pPacket->scope = 0;

	ReadyPacket(PacketIndex);

	CheckTrackRestart();
	return 0;
}



/*********************************************************************
*
* PacketToTrackAmbig1
*
* @brief	Build a bit pattern for a packet based on repeating the byte
*			in packet_byte, and the bit widths in  clk1t, clk0t, and clk0h
*			with 1 ambiguous zero bit
*
* @param	packet byte
*			count
*			1 total width
*			0 total width
*			0 first half width
*			0 total width ambiguous zero bit
*			0 first half width ambiguous zero bit
*
* @return	0 = success
*
*********************************************************************/
int PacketToTrackAmbig1(const uint8_t packet_byte, uint32_t clk1t, uint32_t clk0t1, uint32_t clk0h1, uint32_t clk0t, uint32_t clk0h)
{
	PACKET_BITS* pPacket;
	int PacketIndex;
	uint8_t mask;
	uint32_t Tick1;
	uint32_t Tick0t;
	uint32_t Tick0h;
	uint32_t Tick0t1;
	uint32_t Tick0h1;


	pPacket = AcquirePacket(&PacketIndex);
	if(pPacket == NULL)
	{
		osDelay(pdMS_TO_TICKS(1));
		return 1;
	}

	Tick1  = clk1t * TICKS_PER_MICROSECOND;
	Tick0t = clk0t * TICKS_PER_MICROSECOND;
	Tick0h = clk0h * TICKS_PER_MICROSECOND;
	Tick0t1 = clk0t1 * TICKS_PER_MICROSECOND;
	Tick0h1 = clk0h1 * TICKS_PER_MICROSECOND;

	// set the zero stretch for the first bit
	pPacket->period = Tick0t1;
	pPacket->pulse = Tick0h1;
	pPacket++;

	// do the rest
	mask = 0x02;
	for(int b = 0; b < 7; b++)
	{
		if(packet_byte & mask)
		{
			// build a one bit
			pPacket->period = Tick1;
			pPacket->pulse = Tick1/2;
		}
		else
		{
			// build a zero bit
			pPacket->period = Tick0t;
			pPacket->pulse = Tick0h;
		}
		pPacket++;
		mask <<= 1;
	}

	// terminator
	pPacket->period = 0;
	pPacket->pulse = 0;

	//ScopeTriggerBitCount = ScopeTriggerBitOffset;
	ReadyPacket(PacketIndex);

	CheckTrackRestart();
	return 0;
}


/*********************************************************************
*
* PacketToTrackAmbig2
*
* @brief	Build a bit pattern for a packet based on repeating the byte
*			in packet_byte, and the bit widths in  clk1t, clk0t, and clk0h
*			with 2 ambiguous bits
*
* @param	packet byte
*			count
*			1 total width
*			0 total width
*			0 first half width
*			0 total width first ambiguous zero bit
*			0 first half width first ambiguous zero bit
*			0 total width second ambiguous zero bit
*			0 first half width second ambiguous zero bit
*
* @return	none
*
*********************************************************************/
int PacketToTrackAmbig2(const uint8_t packet_byte, uint32_t clk1t, uint32_t clk0t1, uint32_t clk0h1, uint32_t clk0t2, uint32_t clk0h2, uint32_t clk0t, uint32_t clk0h)
{
	PACKET_BITS* pPacket;
	int PacketIndex;
	uint8_t mask;
	uint32_t Tick1;
	uint32_t Tick0t;
	uint32_t Tick0h;
	uint32_t Tick0t1;
	uint32_t Tick0h1;
	uint32_t Tick0t2;
	uint32_t Tick0h2;

	pPacket = AcquirePacket(&PacketIndex);
	if(pPacket == NULL)
	{
		osDelay(pdMS_TO_TICKS(1));
		return 1;
	}

	Tick1  = clk1t * TICKS_PER_MICROSECOND;
	Tick0t = clk0t * TICKS_PER_MICROSECOND;
	Tick0h = clk0h * TICKS_PER_MICROSECOND;
	Tick0t1 = clk0t1 * TICKS_PER_MICROSECOND;
	Tick0h1 = clk0h1 * TICKS_PER_MICROSECOND;
	Tick0t2 = clk0t2 * TICKS_PER_MICROSECOND;
	Tick0h2 = clk0h2 * TICKS_PER_MICROSECOND;


	// set the zero stretch for the first bit
	pPacket->period = Tick0t1;
	pPacket->pulse = Tick0h1;
	pPacket++;

	// set the zero stretch for the second bit
	pPacket->period = Tick0t2;
	pPacket->pulse = Tick0h2;
	pPacket++;

	// do the rest
	mask = 0x04;
	for(int b = 0; b < 6; b++)
	{
		if(packet_byte & mask)
		{
			// build a one bit
			pPacket->period = Tick1;
			pPacket->pulse = Tick1/2;
		}
		else
		{
			// build a zero bit
			pPacket->period = Tick0t;
			pPacket->pulse = Tick0h;
		}
		pPacket++;
		mask <<= 1;
	}

	// interbyte

	// terminator
	pPacket->period = 0;
	pPacket->pulse = 0;

	//ScopeTriggerBitCount = ScopeTriggerBitOffset;
	ReadyPacket(PacketIndex);

	CheckTrackRestart();
	return 0;
}

/*********************************************************************
*
* PacketToTrackBits
*
* @brief	Build a bit pattern for a packet based on repeating the byte
*			Move the preloaded bit pattern (typically from a file)
*			to an empty packet buffer.
*
* @param	pointer to packet bytes
*			count
*
* @return	0 = success
*
*********************************************************************/
int PacketToTrackBitArray(const PACKET_BITS* packet, uint8_t count)
{
	PACKET_BITS* pPacket;
	int PacketIndex;
	int BitCount = 0;


	pPacket = AcquirePacket(&PacketIndex);
	if(pPacket == NULL)
	{
		osDelay(pdMS_TO_TICKS(1));
		return 1;
	}

	while(packet->period != 0)
	{
		pPacket->period = packet->period;
		pPacket->pulse = packet->pulse;
		pPacket++;
		packet++;

		BitCount++;
		if(BitCount == 11)
		{
			pPacket->scope = 1;
		}
		else
		{
			pPacket->scope = 0;
		}
	}
	// terminator
	// terminator
	pPacket->period = 0;
	pPacket->pulse = 0;
	pPacket->scope = 0;
	
	//ScopeTriggerBitCount = ScopeTriggerBitOffset;
	ReadyPacket(PacketIndex);

	CheckTrackRestart();
	return 0;
}


/*********************************************************************
*
* EnableTrack
*
* @brief	Turn the track output on
*
* @param	none
*
* @return	none
*
*********************************************************************/
void EnableTrack(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	HAL_GPIO_WritePin(TRACK_ENABLE_PORT, TRACK_ENABLE_PIN, GPIO_PIN_SET);


	/* Tim2 Chan1 GPIO pin configuration  */
	GPIO_InitStruct.Pin       = TRACK_A_PIN;
	GPIO_InitStruct.Mode      = TRACK_A_MODE_E;
	GPIO_InitStruct.Pull      = TRACK_A_PU_PD;
	GPIO_InitStruct.Speed     = TRACK_A_SPEED;
	GPIO_InitStruct.Alternate = TRACK_A_AF;
	HAL_GPIO_Init(TRACK_A_PORT, &GPIO_InitStruct);

	HAL_TIM_Base_Start(&htim2);

	//if(HAL_TIMEx_PWMN_Start(&htim2, TIM_CHANNEL_1) != HAL_OK)
	//{
	//    Error_Handler();
	//}

	//if(HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1) != HAL_OK)
	//{
	//    Error_Handler();
	//}

	HAL_NVIC_EnableIRQ(TIM2_IRQn);
	__HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);

	bTrackState = 1;
}

/*********************************************************************
*
* EnableTrack
*
* @brief	Turn the track output off
*
* @param	none
*
* @return	none
*
*********************************************************************/
void DisableTrack(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	// stop the timer
	__HAL_TIM_DISABLE_IT(&htim2, TIM_IT_UPDATE);

//#define TIM_CCx_ENABLE                   0x00000001U                            /*!< Input or output channel is enabled */
//#define TIM_CCx_DISABLE                  0x00000000U                            /*!< Input or output channel is disabled */
//#define TIM_CCxN_ENABLE                  0x00000004U                            /*!< Complementary output channel is enabled */
//#define TIM_CCxN_DISABLE                 0x00000000U                            /*!< Complementary output channel is enabled */

	//__HAL_TIM_DISABLE(&htim2);
	//__HAL_TIM_MOE_DISABLE(&htim2);
	//__HAL_TIM_MOE_DISABLE_UNCONDITIONALLY(&htim2);


	HAL_GPIO_WritePin(TRACK_ENABLE_PORT, TRACK_ENABLE_PIN, GPIO_PIN_RESET);

	/* Tim2 Chan1 GPIO pin configuration  */
	GPIO_InitStruct.Pin       = TRACK_A_PIN;
	GPIO_InitStruct.Mode      = TRACK_A_MODE_D;
	GPIO_InitStruct.Pull      = TRACK_A_PU_PD;
	GPIO_InitStruct.Speed     = TRACK_A_SPEED;
	GPIO_InitStruct.Alternate = TRACK_A_AF;
	HAL_GPIO_Init(TRACK_A_PORT, &GPIO_InitStruct);

	HAL_NVIC_DisableIRQ(TIM2_IRQn);

	bTrackState = 0;
}


/*********************************************************************
*
* GetTrackState
*
* @brief	Get the track state (on or off)
*
* @param	none
*
* @return	Track state
*
*********************************************************************/
uint8_t GetTrackState(void)
{
	return bTrackState;
}


/*********************************************************************
*
* IsPacketBufferAvailable
*
* @brief	Non-zero if buffer is available, 0 otherwize
*
* @param	none
*
* @return	1 = available
*
*********************************************************************/
//uint32_t IsPacketBufferAvailable(void)
//{
	// ToDo - guard this against the interrupt
//	if(apPacket1[0].period == 0)
//	{
//		return 1;
//	}
//	else if(apPacket2[0].period == 0)
//	{
//		return 1;
//	}
//	else
//	{
//		return 0;
//	}

//	return BufferAvailable;

//}


/*********************************************************************
*
* IsPacketComplete
*
* @brief	Non-zero if packet(s) are done sending (track off, idle, or reset)
*
* @param	none
*
* @return	1 = complete
*
*********************************************************************/
//uint32_t IsPacketComplete(void)
//{
//	return PacketComplete;
//}

#ifdef TRACK_LOCK_NOT_USED
/*********************************************************************
*
* OpenTrack
*
* @brief	Non-zero if packet(s) are done sending (track off, idle, or reset)
*
* @param	tr - which procees is requesting the track, one of TRACK_RESOURCE 
*			ti - what should the track generator do when it runs out of packets,
*				 one of TRACK_IDLE 
*			preambles -  number of preambles, 0 = used default
*
* @return	TRACK_LOCK_STATUS
*
*********************************************************************/
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


/*********************************************************************
*
* CloseTrack
*
* @brief	Close the track (make it available to another process)
*
* @param	none
*
* @return	none
*
*********************************************************************/
void CloseTrack(void)
{
	// ToDo - make this threadsafe

	TrackLock.lock = TR_NONE;
	TrackLock.idle = TI_NONE;
	TrackLock.preambles = 0;
}


/*********************************************************************
*
* IsTrackOpen
*
* @brief	Check to see if the track resource is open
*
* @param	Track resource
*
* @return	1 = open
*
*********************************************************************/
uint8_t IsTrackOpen(TRACK_RESOURCE tr)
{
	// ToDo - make this threadsafe

	return TrackLock.lock == tr;
}

/*********************************************************************
*
* GetTrackLock
*
* @brief	Return the track resource ID
*
* @param	Track Resource ID
*
* @return	Track Resource ID
*
*********************************************************************/
TRACK_RESOURCE GetTrackLock(void)
{
	// ToDo - make this threadsafe

	return TrackLock.lock;
}

/*********************************************************************
*
* GetTrackIdle
*
* @brief	Return the track resource ID
*
* @param	Track Resource ID
*
* @return	Track Resource ID
*
*********************************************************************/
TRACK_IDLE GetTrackIdle(void)
{
	// ToDo - make this threadsafe

	return TrackLock.idle;
}


/*********************************************************************
*
* GetTrackPresmbles
*
* @brief	Return the track preambles
*
* @param	Track Resource ID
*
* @return	Track Resource ID
*
*********************************************************************/
uint16_t GetTrackPresmbles(void)
{
	// ToDo - make this threadsafe

	return TrackLock.preambles;
}



/*********************************************************************
*
* SetTrackIdle
*
* @brief	Set the track idle
*
* @param	Track Resource ID
*
* @return	Track Resource ID
*
*********************************************************************/
void SetTrackIdle(TRACK_IDLE ti)
{
	// ToDo - make this threadsafe

	if(TrackLock.lock == TR_SHELL)
	{
		// only allow this while the track is open in the shell
		TrackLock.idle = ti;
	}
}
#else
TRACK_IDLE GetTrackIdle(void)
{

	return TrackIdleState;
}

void SetTrackIdle(TRACK_IDLE ti)
{

	TrackIdleState = ti;
}

#endif

