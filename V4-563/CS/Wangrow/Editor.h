/**********************************************************************
*
* SOURCE FILENAME:	Editor.c
*
* DATE CREATED:		21-Dec-1999
*
* PROGRAMMER:		Karl Kobel
*
* DESCRIPTION:
*
* COPYRIGHT (c) 2000-2002 by K2 Engineering. All Rights Reserved.
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
#include "Cab.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

extern int FormatLen(char* szFormat);

extern void FormatString(char *dest_string, char *src_string, char *format);

extern void EditNumeric(VIRTUAL_CAB* pVirtualCab, int Event);
extern void EditSelection(VIRTUAL_CAB* pVirtualCab, int nEvent);
extern void EditSelection2(VIRTUAL_CAB* pVirtualCab, int nEvent);
extern void EditChoice(VIRTUAL_CAB* pVirtualCab, int nEvent);

extern void EditList(VIRTUAL_CAB* pVirtualCab, int nEvent);


