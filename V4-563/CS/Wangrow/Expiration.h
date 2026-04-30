/**********************************************************************
*
* SOURCE FILENAME:	Expiration.h
*
* DATE CREATED:		11-Apr-2000
*
* PROGRAMMER:
*
* DESCRIPTION:
*
* COPYRIGHT (c) 2000-2002 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define EXPIRATION_05_SECONDS		50
#define EXPIRATION_1_SECONDS		100
#define EXPIRATION_2_SECONDS		200
#define EXPIRATION_5_SECONDS		500

#define EPIRATION_1_MINUTE			(EXPIRATION_1_SECONDS * 60)

 /**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

extern void SetExpiration(VIRTUAL_CAB* pVirtualCab, int nTime);

