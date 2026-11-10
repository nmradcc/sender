/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_hal.h"

#include "stm32h5xx_nucleo.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void MX_USART3_UART_Init(void);
void MX_USB_PCD_Init(void);

/* USER CODE BEGIN EFP */
uint32_t freq_meas_hz(uint32_t gate_ms);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define IN2_Pin GPIO_PIN_6
#define IN2_GPIO_Port GPIOE
#define STLK_MCO_Pin GPIO_PIN_0
#define STLK_MCO_GPIO_Port GPIOH
#define SCOPE_Pin GPIO_PIN_7
#define SCOPE_GPIO_Port GPIOE
#define AUXOUT_Pin GPIO_PIN_8
#define AUXOUT_GPIO_Port GPIOE
#define IN0_Pin GPIO_PIN_10
#define IN0_GPIO_Port GPIOE
#define IO1_Pin GPIO_PIN_11
#define IO1_GPIO_Port GPIOE
#define IN1_Pin GPIO_PIN_12
#define IN1_GPIO_Port GPIOE
#define IO2_Pin GPIO_PIN_13
#define IO2_GPIO_Port GPIOE
#define IN3_Pin GPIO_PIN_15
#define IN3_GPIO_Port GPIOE
#define IO3_Pin GPIO_PIN_12
#define IO3_GPIO_Port GPIOB
#define REF4MHZ_Pin GPIO_PIN_13
#define REF4MHZ_GPIO_Port GPIOD
#define IO4_Pin GPIO_PIN_14
#define IO4_GPIO_Port GPIOD
#define BR_ENABLE_Pin GPIO_PIN_6
#define BR_ENABLE_GPIO_Port GPIOG
#define OUT_DE_Pin GPIO_PIN_15
#define OUT_DE_GPIO_Port GPIOG
#define DCC_IN_Pin GPIO_PIN_3
#define DCC_IN_GPIO_Port GPIOB
#define FUNC_IN_Pin GPIO_PIN_5
#define FUNC_IN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
