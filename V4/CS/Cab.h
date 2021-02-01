/**********************************************************************
*
* SOURCE FILENAME:	Cab.h
*
* DATE CREATED:		5-Jan-1999
*
* PROGRAMMER:			Karl Kobel
*
* DESCRIPTION:
*
* COPYRIGHT (c) 2000-2002 by K2 Engineering  All Rights Reserved.
*
**********************************************************************
*
* VERSION CONTROL:
*
* $Revision: $
*
* $Log: $
*
*********************************************************************/
#ifndef CAB_H
#define CAB_H

#include "Main.h"
#include "menu.h"
#include "Loco.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

//#define MAX_VIRTUAL_CABS	64
//#define MAX_RACALLS		8

#define	CAB_TYPE_NCE		1
#define	CAB_TYPE_SERIAL		2
#define	CAB_TYPE_XPRESSNET	3
#define	CAB_TYPE_ETHERNET	4


#define CAB_PORT_NONE		0
#define CAB_PORT_LEFT		1
#define CAB_PORT_RIGHT		2

//extern byte NCE_Ports;
//extern byte XpressNet_Ports;


// usage
//
// a CV will hold the configuration for the 2 ports - named left and right
//
// at power up, the two variables NCE_Ports and XpressNet_Ports will get
// set with no bits, one or the other, or both
//
// For XpressNet on the left and NCE/Wangrow on the right
//
// NCE_Ports = CAB_PORT_RIGHT;
// XpressNet_Ports = CAB_PORT_LEFT;
//
// In the cab handler:
//	void InitYourCabCommunication(byte Ports)
//{
//	byte i;
//	UART_DEF uart_def;
//
//	if(Ports == CAB_PORT_RIGHT)
//	{
//		//uart_def.mode = U_IDLE;
//		uart_def.mode = U_CHAR;
//		uart_def.bits = U_DATAWIDTH_8B;
//		uart_def.parity = U_PARITY_NONE;
//		uart_def.stop_bits = U_STOPBITS_2;
//		uart_def.baud = 9600;
//		uart_def.UartRxCallback = RxCabResponse;
//		uart_def.UartTxCallback = TxCabResponse;
//		if(Uart2_Init(&uart_def) != HAL_OK)
//		{
//			_Error_Handler(__FILE__, __LINE__);
//		}
//	}
//
//	if(Ports == CAB_PORT_LEFT)
//	{
//		//uart_def.mode = U_IDLE;
//		uart_def.mode = U_CHAR;
//		uart_def.bits = U_DATAWIDTH_8B;
//		uart_def.parity = U_PARITY_NONE;
//		uart_def.stop_bits = U_STOPBITS_2;
//		uart_def.baud = 9600;
//		uart_def.UartRxCallback = RxCabResponse;
//		uart_def.UartTxCallback = TxCabResponse;
//		if(Uart3_Init(&uart_def) != HAL_OK)
//		{
//			_Error_Handler(__FILE__, __LINE__);
//		}
//	}
//
// Send poll (call)
//	if(Ports == CAB_PORT_RIGHT)
//	{
//		Uart2_SendToken(poll);
//	}
//	if(Ports == CAB_PORT_LEFT)
//	{
//		Uart3_SendToken(poll);
//	}
//
//



/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/
#ifdef MOVED
typedef struct VIRTUAL_CAB_t
{
	// Cab Stuff
	int				Cab;
	byte			CabType;
	byte			CabSubType;
	byte			nState;
	void*			CabHandle;
	unsigned int	nSpeed;
	byte 			bSpeedDisconnect;
	
	// Menu stuff
	int				nEvent;
	MENU*			pMenu;
	int				nMenuIndex;
	int				nCursor;
	BOOL			bFirstTime;
	char			szEditBuffer[6];
	int				nEditVar;
	int				nListCount;
	int				nMenuShowing;
	char			szTitleSave[9];
	
	unsigned int	nLastAccessory;
	unsigned int	nLastMacro;
	
//	unsigned int	nTemp1;
//	unsigned int	nTemp2;
//	unsigned int	nTemp3;
//	unsigned int	nTemp4;
	
	// Loco stuff
	unsigned int	nWhichRecall;
	struct Loco*	pRecall[MAX_RACALLS];
	//unsigned int	nPotSpeed;
	
	int				ExpirationTimer;
	int				nEStopState;
} VIRTUAL_CAB;

extern VIRTUAL_CAB aVirtualCab[MAX_VIRTUAL_CABS];
#endif

extern void HandleCabCommunication(void);

extern VIRTUAL_CAB* WM_RegisterCab(int Cab, int CabType, int CabSubType, void* CabHandle);
extern void WM_UnRegisterCab(VIRTUAL_CAB* pVC);

extern void TaskHandleExpiration(void);
extern void SetExpiration(VIRTUAL_CAB* pVirtualCab, int nTime);

extern void InitExpiration(void);
extern void HandleExpiration(void);

extern void InitVirtualCab(void);

#endif
