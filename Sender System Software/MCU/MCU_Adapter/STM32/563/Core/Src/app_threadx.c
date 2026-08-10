/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.c
  * @author  MCD Application Team
  * @brief   ThreadX applicative file
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

/* Includes ------------------------------------------------------------------*/
#include "app_threadx.h"
#include "stm32h5xx_nucleo.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sender_app.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define LED_TASK_THREAD_STACK_SIZE         1024U
#define LED_TASK_THREAD_PRIO               20U
#define PKT_MON_THREAD_STACK_SIZE          1024U
#define PKT_MON_THREAD_PRIO                21U


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static TX_THREAD led_task_thread;
static UCHAR *led_task_thread_stack;
static TX_THREAD pkt_mon_thread;
static UCHAR *pkt_mon_thread_stack;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static VOID LedThreadTask(ULONG thread_input);
static VOID PacketMonitorThreadTask(ULONG thread_input);
/* USER CODE END PFP */

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  UINT ret = TX_SUCCESS;
  /* USER CODE BEGIN App_ThreadX_MEM_POOL */
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL *)memory_ptr;

  /* USER CODE END App_ThreadX_MEM_POOL */
  /* USER CODE BEGIN App_ThreadX_Init */
  if (tx_byte_allocate(byte_pool, (VOID **)&led_task_thread_stack, LED_TASK_THREAD_STACK_SIZE,
                       TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  if (tx_thread_create(&led_task_thread,
                       "LED Yellow Blink Task",
                       LedThreadTask,
                       0,
                       led_task_thread_stack,
                       LED_TASK_THREAD_STACK_SIZE,
                       LED_TASK_THREAD_PRIO,
                       LED_TASK_THREAD_PRIO,
                       TX_NO_TIME_SLICE,
                       TX_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }

  if (tx_byte_allocate(byte_pool, (VOID **)&pkt_mon_thread_stack, PKT_MON_THREAD_STACK_SIZE,
                       TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  if (tx_thread_create(&pkt_mon_thread,
                       "Packet Monitor Task",
                       PacketMonitorThreadTask,
                       0,
                       pkt_mon_thread_stack,
                       PKT_MON_THREAD_STACK_SIZE,
                       PKT_MON_THREAD_PRIO,
                       PKT_MON_THREAD_PRIO,
                       TX_NO_TIME_SLICE,
                       TX_AUTO_START) != TX_SUCCESS)
  {
    return TX_THREAD_ERROR;
  }
  /* USER CODE END App_ThreadX_Init */

  return ret;
}

  /**
  * @brief  Function that implements the kernel's initialization.
  * @param  None
  * @retval None
  */
void MX_ThreadX_Init(void)
{
  /* USER CODE BEGIN Before_Kernel_Start */

  /* USER CODE END Before_Kernel_Start */

  tx_kernel_enter();

  /* USER CODE BEGIN Kernel_Start_Error */

  /* USER CODE END Kernel_Start_Error */
}

/* USER CODE BEGIN 1 */
static VOID LedThreadTask(ULONG thread_input)
{
  TX_PARAMETER_NOT_USED(thread_input);

  while (1)
  {
    BSP_LED_Toggle(LED_YELLOW);
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2U);
  }
}

static VOID PacketMonitorThreadTask(ULONG thread_input)
{
  uint32_t last_event_count = 0u;
  uint32_t event_count = 0u;
  uint8_t last_address = 0xFFu;

  TX_PARAMETER_NOT_USED(thread_input);

  while (1)
  {
    sender_app_get_non_idle_packet_event(&event_count, &last_address);
    (void)last_address;
    if (event_count != last_event_count)
    {
      last_event_count = event_count;
      BSP_LED_Toggle(LED_GREEN);
    }

    tx_thread_sleep(2);
  }
}

/* USER CODE END 1 */
