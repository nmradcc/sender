/**********************************************************************
*
* SOURCE FILENAME:	Acknowledge.c
*
* DATE CREATED:		4/24/19
*
* PROGRAMMER:
*
* DESCRIPTION:
*
* COPYRIGHT (c) 1999-2019 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/
#include "main.h"
#include "Acknowledge.h"
#include "variables.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define MAX_CURRENT		250.0

#define LOCO_CURRENT	20.0

#define ACK_CURRENT		50.0

#define BASE_DELAY		50		// in 10MS ticks


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

extern ADC_HandleTypeDef hadc1;

ACK_STATUS AckStatus = NO_ACK;

float ProgTrackCurrent = 0.0;

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/

//static ADC_HandleTypeDef AdcHandle;

/* Variable used to get converted value */
static __IO uint16_t uhADCxConvertedValue[2];


static float fBaseAvg;
static float fLevelAvg;

static uint8_t BaseDelay;

static uint8_t bfFirstTime;
static uint8_t bfFirstConversionDone = 0;

/**********************************************************************
*
*							CODE
*
**********************************************************************/

/**********************************************************************
*
* FUNCTION:		InitAcknowledge
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:	Init the ACK pulse detector
*
* RESTRICTIONS:
*
**********************************************************************/
void InitAcknowledge(void)
{

#ifdef IN_MAIN
	ADC_ChannelConfTypeDef sConfig;

	/*##-1- Configure the ADC peripheral #######################################*/
	AdcHandle.Instance = ADC1;

	if (HAL_ADC_DeInit(&AdcHandle) != HAL_OK)
	{
		/* ADC de-initialization Error */
		Error_Handler();
	}

//	AdcHandle.Init.ClockPrescaler        = ADC_CLOCK_ASYNC_DIV1;          /* Asynchronous clock mode, input ADC clock not divided */
	AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;            /* 12-bit resolution for converted data */
	AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;           /* Right-alignment for converted data */
	AdcHandle.Init.ScanConvMode          = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
	AdcHandle.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;           /* EOC flag picked-up to indicate conversion end */
//	AdcHandle.Init.LowPowerAutoWait      = DISABLE;                       /* Auto-delayed conversion feature disabled */
	AdcHandle.Init.ContinuousConvMode    = ENABLE;                        /* Continuous mode disabled to have only 1 conversion at each conversion trig */
	AdcHandle.Init.NbrOfConversion       = 1;                             /* Parameter discarded because sequencer is disabled */
	AdcHandle.Init.DiscontinuousConvMode = DISABLE;                       /* Parameter discarded because sequencer is disabled */
	AdcHandle.Init.NbrOfDiscConversion   = 1;                             /* Parameter discarded because sequencer is disabled */
	AdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;            /* Software start to trig the 1st conversion manually, without external event */
	AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE; /* Parameter discarded because software trigger chosen */
	AdcHandle.Init.DMAContinuousRequests = DISABLE;                       /* DMA one-shot mode selected (not applied to this example) */
//	AdcHandle.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;      /* DR register is overwritten with the last conversion result in case of overrun */
//	AdcHandle.Init.OversamplingMode      = DISABLE;                       /* No oversampling */

	if (HAL_ADC_Init(&AdcHandle) != HAL_OK)
	{
		/* ADC initialization Error */
		Error_Handler();
	}

	/*##-2- Configure ADC regular channel ######################################*/
	sConfig.Channel      = ADC_CHANNEL_0;               /* Sampled channel number */
//	sConfig.Rank         = ADC_REGULAR_RANK_1;          /* Rank of sampled channel number ADCx_CHANNEL */
//	sConfig.SamplingTime = ADC_SAMPLETIME_6CYCLES_5;    /* Sampling time (number of clock cycles unit) */
//	sConfig.SingleDiff   = ADC_SINGLE_ENDED;            /* Single-ended input channel */
//	sConfig.OffsetNumber = ADC_OFFSET_NONE;             /* No offset subtraction */
	sConfig.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */
	if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
	{
	    /* Channel Configuration Error */
	    Error_Handler();
	}

	/* Run the ADC calibration in single-ended mode */
	//if (HAL_ADCEx_Calibration_Start(&AdcHandle, ADC_SINGLE_ENDED) != HAL_OK)
	//{
	//    /* Calibration Error */
	//    Error_Handler();
	//}
#endif

	/*##-3- Start the conversion process #######################################*/
	//if (HAL_ADC_Start_IT(&hadc1) != HAL_OK)
	//{
	//    /* Start Conversation Error */
	//    Error_Handler();
	//}

	bfFirstTime = 1;
	bfFirstConversionDone = 0;

	if(HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&uhADCxConvertedValue, 2) != HAL_OK)
	{
	    /* Start Conversation Error */
	    Error_Handler();
	}
}


//*******************************************************************************
//
// FUNCTION     E_AverageFloat
//
// INPUTS:      Old value, new value
//
// OUTPUTS:     New Average
//
// DESCRIPTION: Exponitional average - floating point version
//
//*******************************************************************************
float E_AverageFloat(float OldValue, float NewValue, unsigned char AverageConstant)
{
    float Avg;

    Avg = NewValue * AverageConstant;
    Avg += OldValue * (100.0 - AverageConstant);
    return Avg / 100.0;
}


/**********************************************************************
*
* FUNCTION:		HAL_ADC_ConvCpltCallback
*
* ARGUMENTS:	AdcHandle : AdcHandle handle
*
* RETURNS:
*
* DESCRIPTION:	Conversion complete callback in non blocking mode
*
* RESTRICTIONS:
*
**********************************************************************/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{

	bfFirstConversionDone = 1;

	//if(AdcHandle == &hadc1)
	//{
	//	/* Get the converted value of regular channel */
	//	uhADCxConvertedValue[0] = HAL_ADC_GetValue(AdcHandle);
	//}
}


/**********************************************************************
*
* FUNCTION:		Acknowledge
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:	ACK pulse detector
*
* RESTRICTIONS:
*
**********************************************************************/
void Acknowledge(void)
{
    //int raw;
    //unsigned char Ret = 0;
    float fRaw;
    //float fLevel;

    TrackCurrent = (float)uhADCxConvertedValue[0];
    TrackVoltage = (float)uhADCxConvertedValue[1];

    if(bfFirstConversionDone)
    {
		fRaw = (float)uhADCxConvertedValue[0];

		// convert to real-world current

		fLevelAvg = E_AverageFloat(fLevelAvg, fRaw, 80);

		if(bfFirstTime)
		{
			bfFirstTime = 0;
			fBaseAvg = fLevelAvg;
			BaseDelay = 0;
		}

		ProgTrackCurrent = fLevelAvg;


		if(fLevelAvg > LOCO_CURRENT)
		{
			// indicate loco present
			AckStatus = LOCO_PRESENT;
		}
		else if(fLevelAvg > MAX_CURRENT)
		{
			// indicate over-current
			AckStatus = OVER_CURRENT;
		}
		else if(fLevelAvg > (fBaseAvg + ACK_CURRENT))
		{
			// indicate ACK
			AckStatus = ACK_DETECTED;

			// set a delay before the base average is started again
			BaseDelay = BASE_DELAY;
		}
		else
		{
			--BaseDelay;
			if(BaseDelay == 0)
			{
				BaseDelay = 1;
				fBaseAvg = E_AverageFloat(fBaseAvg, fLevelAvg, 10);
			}
		}
		//return AckStatus;
    }
}

/**********************************************************************
*
* FUNCTION:		ClearAck
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
//void ClearAck(void)
//{
//
//	AckStatus = NO_ACK;
//}


/**********************************************************************
*
* FUNCTION:		IsAck
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
uint8_t IsAck(void)
{

	return AckStatus == ACK_DETECTED;
}


/**********************************************************************
*
* FUNCTION:		GetAck
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
ACK_STATUS GetAck(void)
{

	return AckStatus;
}

