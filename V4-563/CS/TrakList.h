/**********************************************************************
*
* SOURCE FILENAME:	TrakList.h
*
* DATE CREATED:		7-Jan-2000
*
* PROGRAMMER:			TMP
*
* DESCRIPTION:
*
* COPYRIGHT (c) 2000-2002 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/
#include "loco.h"

extern void InitLocoList(void);

extern Loco* NextLocoMsg(void);

extern void RemoveLocoMsg(Loco* pLoco);

extern void MakeMostRecentLoco(Loco* testLoco);

//extern Loco* NextFunctionMsg1(void);
//extern Loco* NextFunctionMsg2(void);

//extern void MakeMostRecentFunction1(Loco *testLoco);
//extern void MakeMostRecentFunction2(Loco *testLoco);

extern void StopAllLocos(void);

extern Loco *OldestLoco(void);


