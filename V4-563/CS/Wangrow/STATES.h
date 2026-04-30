/*********************************************************************
*
* SOURCE FILENAME:	states.h
*
* DATE CREATED:		21-Dec-1999
*
* PROGRAMMER:		Karl Kobel
*
* DESCRIPTION:  	State processing.
*
* COPYRIGHT (c) 2000-2002 by K2 Engineering.  All Rights Reserved.
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

#ifndef _STATES_H
#define _STATES_H

#include "Main.h"
#include "cab.h"

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/
// STATE MACHINE STATES
enum
{
	STATE_IDLE,
	STATE_SELECT_LOCO,
	STATE_SELECT_ACCESSORIES,
	//STATE_SELECT_MACROS,
	//STATE_SETUP_CONSIST,
	//STATE_ADD_CONSIST,
	//STATE_DELETE_CONSIST,
	//STATE_CLEAR_CONSIST,
	STATE_FUNCTIONS,
	STATE_ESTOP,
	STATE_PROGRAM_ON_MAIN,
	//STATE_ASSIGN_LOCO_CAB,
	//STATE_SET_SYSTEM_CLOCK,
	STATE_PROGRAM_ON_TRACK,
	//STATE_SET_COMMAND_STATION,
	STATE_SET_CAB,
	//STATE_PROGRAM_ACCESSORIES,
	//STATE_PROGRAM_MACROS,
	STATE_PROG_MODE,
	STATE_NO_FUNCTION,
	STATE_SEND,
	STATE_TRACK,
};

typedef struct
{
    int nEvent;
    int nNewState;
    void (*EventFunction)(VIRTUAL_CAB* pVirtualCab, int nEvent);
} EVENT_MAP;


#define NO_MENU_SHOWING			0x00
#define MENU_SHOWING_LINE_1L	0x01
#define MENU_SHOWING_LINE_1R	0x02
#define MENU_SHOWING_LINE_2L	0x04
#define MENU_SHOWING_LINE_2R	0x08
#define MENU_SHOWING_ESTOP		0x10
#define MENU_SHOWING_FUNCTIONS	0x20

//extern int nMenuShowing;

/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

#ifdef NOT_USED
int GetState(VIRTUAL_CAB* pVirtualCab);
int GetLastState(void);
#endif
void InitState(int nState);
void RunState(VIRTUAL_CAB* pVirtualCab, int nEvent);
void ChangeState(VIRTUAL_CAB* pVirtualCab, int nState);
void ReturnState(VIRTUAL_CAB* pVirtualCab);
unsigned char DispatchEvent(VIRTUAL_CAB* pVirtualCab, int nEvent, const EVENT_MAP *pMap);


void StateIdle(VIRTUAL_CAB* pVirtualCab, int nEvent);
void StateSelectLoco(VIRTUAL_CAB* pVirtualCab, int nEvent);
void StateSelectAccessory(VIRTUAL_CAB* pVirtualCab, int nEvent);
//void StateSelectMacro(VIRTUAL_CAB* pVirtualCab, int nEvent);
//void StateSetupConsist(VIRTUAL_CAB* pVirtualCab, int nEvent);
//void StateAddConsist(VIRTUAL_CAB* pVirtualCab, int nEvent);
//void StateDeleteConsist(VIRTUAL_CAB* pVirtualCab, int nEvent);
//void StateClearConsist(VIRTUAL_CAB* pVirtualCab, int nEvent);
void StateFunction(VIRTUAL_CAB* pVirtualCab, int nEvent);
//void StateEStop(VIRTUAL_CAB* pVirtualCab, int nEvent);

void StateProgMode(VIRTUAL_CAB* pVirtualCab, int nEvent);

void StateNoFunction(VIRTUAL_CAB* pVirtualCab, int nEvent);

void StateProgramOnMain(VIRTUAL_CAB* pVirtualCab, int nEvent);
//void StateAssignLocoCab(VIRTUAL_CAB* pVirtualCab, int nEvent);
//void StateSetSystemClock(VIRTUAL_CAB* pVirtualCab, int nEvent);
void StateProgramTrack(VIRTUAL_CAB* pVirtualCab, int nEvent);
//void StateSetCommandStation(VIRTUAL_CAB* pVirtualCab, int nEvent);
//void StateSetCab(VIRTUAL_CAB* pVirtualCab, int nEvent);
void StateProgramAccessories(VIRTUAL_CAB* pVirtualCab, int nEvent);
//void StateProgramMacros(VIRTUAL_CAB* pVirtualCab, int nEvent);

void StateSend(VIRTUAL_CAB* pVirtualCab, int nEvent);
void StateTrack(VIRTUAL_CAB* pVirtualCab, int nEvent);

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

//extern int LastEvent;

#endif

