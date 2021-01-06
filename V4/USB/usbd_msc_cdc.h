/**
  ******************************************************************************
  * @file    usbd_msc_cdc_core.h
  * @author  Brian Barnt
  * @brief   Header file for the usbd_msc_cdc_core.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_MSC_CDC_CORE_H
#define __USB_MSC_CDC_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_msc_cdc
  * @brief This file is the header file for usbd_msc_cdc_core.c
  * @{
  */


/** @defgroup USBD_msc_cdc_Exported_Defines
  * @{
  */
#define MSC_CDC_EPIN_ADDR                 0x81
#define MSC_CDC_EPIN_SIZE                 0x10

#define USB_MSC_CDC_CONFIG_DESC_SIZ       98

#define MSC_INTERFACE_IDX 	0x0			// Index of MSC interface
#define CDC_INTERFACE_IDX 	0x1			// Index of CDC interface

#define USB_FS_MAX_PACKET_SIZE 	0x0040


// endpoints numbers
#define MSC_EP_IDX        	0x01
#define CDC_CMD_EP_IDX    	0x02
#define CDC_EP_IDX        	0x03
#define IN_EP_DIR			0x80			// Adds a direction bit
#define MSC_OUT_EP			MSC_EP_IDX		/* EP1 for BULK OUT */
#define MSC_IN_EP			MSC_EP_IDX | IN_EP_DIR	/* EP1 for BULK IN */
#define CDC_CMD_EP			CDC_CMD_EP_IDX| IN_EP_DIR	/* EP2 for CDC commands */
#define CDC_OUT_EP			CDC_EP_IDX		/* EP3 for data OUT */
#define CDC_IN_EP			CDC_EP_IDX | IN_EP_DIR	/* EP3 for data IN */

/**
  * @}
  */


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */

/**
  * @}
  */



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */

extern USBD_ClassTypeDef  USBD_MSC_CDC_ClassDriver;
/**
  * @}
  */

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USB_MSC_CDC_CORE_H */
/**
  * @}
  */

/**
  * @}
  */

/**************** Portions (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
