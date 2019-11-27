/*********************************************************************
*
* SOURCE FILENAME:  Service.h
*
* DATE CREATED:     2 May 2019
*
* PROGRAMMER:       Karl Kobel
*
* DESCRIPTION:      <add description>
*
* COPYRIGHT (c)   All Rights Reserved.
*
*********************************************************************/
#ifndef SERVICE_H_
#define SERVICE_H_


/*********************************************************************
*
*                            DEFINITIONS
*
*********************************************************************/

/*********************************************************************
*
*                            FUNCTION PROTOTYPES
*
*********************************************************************/

extern void ServiceMode(void);

extern void ServiceModeWriteCV(unsigned short nCV, unsigned char bValue);

extern void ServiceModeReadCV(word nCV);

#endif
