/**********************************************************************
*
* SOURCE FILENAME:	Text.h
*
* DATE CREATED:		21-Dec-1999
*
* PROGRAMMER:		Karl Kobel
*
* DESCRIPTION:  	
*
* COPYRIGHT (c) 2000-2017 by K2 Engineering. All Rights Reserved.
*
**********************************************************************/

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

enum TextIndex
{
	tNull,
	tSelect_Loco,
	tSelect_Accy,
	tACC_9990,
	tSelect_Macro,
	tNo_Macro,
	tMacro,
	tMacroNumber,
	tAccNumber,
	tAccDirection,
	tConsist_Addr,
	tLeadLoco,
	tLoco_Dir,
	tAdd_Loco,
	tLoco_Dir2,
	tType,
	tCon,
	tConLead,
	tFunction,
	tClock,
	tClockAM,
	tClockPM,
	tSpd_28,
	tSpd_28P,
	tSpd128,
	tSpd128P,
	tSpd_14,
	tSpd_14P,
	tLoc,
	tAnalog,
	tRev,
	tFwd,
	tStop,
	tEStp,
	tL123456,
	t789012,
	tFWD_Default,
	tLOC_Default,
	tPgmTrk,
	tManufacturer,
	tVersion,
	tAddress,
	tStartVoltage,
	tAcceleration,
	tDeceleration,
	tDirection,
	t28SpeedMode,
	tDCMode,
	tAdvancedAck,
	tSpeedTable,
	tConsist,
	tSelectConsist,
	tConsistLeadLoco,
	tConsistDirection,
	tConsistAddLoco,
	tClearConsist,
	tDeleteConsist,
	tConsistDelete,
	//tConsistFcn1,
	//tConsistFcn2,
	tCVAddress,
	tCVValue,
	tOpsPgm,
	tOpsLoco,
	tOpsLocoAddress,
	tOpsChoice,
	tAccelAdj,
	tDecalAdj,
	tSetClock,
	tSetClock12_24,
	tSetClockHours,
	tSetClockMinutes,
	tSetClockRatio,
	tE_Stop,
	tLayoutStopped,
	tFnc7_12,
	tFnc13_18,
	tFnc19_24,
};

extern char *aText[];

