/*********************************************************************
*
* SOURCE FILENAME:  ObjectNames.c
*
* DATE CREATED:     8/Apr/2011
*
* PROGRAMMER:       K Kobel
*
* DESCRIPTION:      Object Names used for Persist and RPC
*
* COPYRIGHT (c) 2017 by K2 Engineering.  All Rights Reserved.
*
*********************************************************************/
#include "Main.h"
//#include "stm32f0xx.h"
//#include "DataQueue.h"
//#include "LiveData.h"
//#include <string.h>
//#include "Settings.h"

/*********************************************************************
*
*                            DEFINITIONS
*
*********************************************************************/

/*********************************************************************
*
*                            STATIC DATA
*
*********************************************************************/

/*********************************************************************
*
*                            FUNCTION PROTOTYPES
*
*********************************************************************/

/*********************************************************************
*
*                            CODE
*
*********************************************************************/

/************************************************************************
* object names
*
*  parameters:
*
*  returns:
*
*  description:
*
*************************************************************************/

// keep these under 12 characters long so they display nice on the console

const char szVersion[] =			"version";
const char szSerialNumber[] =		"serial";
const char szBuildDate[] =			"build";

const char szTime[] =				"time";
const char szDate[] =				"date";
const char szTimeFmt[] =			"timefmt";
const char szDateFmt[] =			"datefmt";

const char szIpAdr[] =				"ipaddress";
const char szIpMsk[] =				"ipmask";
const char szGwAdr[] =				"gwaddress";
const char szDHCP[] =				"dhcp";
const char szName[] = 				"name";

const char szTrack[] =				"track";
const char szTrackIdle[] =			"track-idle";
const char szTrackLock[] =			"track-lock";

const char szPort[] = 				"port";

const char szPath[] = 				"path";

const char szRLed[] = 				"red-led";
const char szYLed[] = 				"yellow-led";
const char szGLed[] = 				"green-led";

const char szInputs[] = 			"inputs";

const char szLoopCount[] = 			"loopcnt";

const char szTheme[] = 				"theme";

const char szTrackCurrent[] =		"track-i";
const char szTrackVoltage[] =		"track-v";


