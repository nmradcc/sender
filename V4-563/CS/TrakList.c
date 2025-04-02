/**********************************************************************
*
* SOURCE FILENAME:	LinkedList.c
*
* DATE CREATED:		7-Jan-2000
*
* PROGRAMMER:			TMP
*
* DESCRIPTION:  	
*
* COPYRIGHT (c) 2000-2017 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/
//#include "stdafx.h"
//#include <dos.h>
#include <stdio.h>
#include "Main.h"
#include "LinkedList.h"
#include "TrakList.h"
#include "Loco.h"
#include "Accessory.h"

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
Link *GetFree(void);

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

//#pragma section NV_RAM
Link	FreeLinks[MAX_LOCOS];
Link	FreeAccyLinks[MAX_ACCESSORIES];
//#pragma section

LList	FreeLocos;
LList	TrackLocos;
ListIterator	trackSequence;

//LList	LocoFunction1;
//ListIterator	functionSequence1;

//LList	LocoFunction2;
//ListIterator	functionSequence2;

LList	FreeAccys;
LList	TrackAccys;
ListIterator	accySequence;

/**********************************************************************
*
*							STATIC VARIABLES
*
**********************************************************************/

/**********************************************************************
*
*							CODE
*
**********************************************************************/
void InitLocoList(void)
{
	word i;

	setup(&FreeLocos);
	for (i = 0; i < MAX_LOCOS; i++)
	{
		InsertAtTop(&FreeLocos, &FreeLinks[i]);
	}
	setup(&TrackLocos);
	trackSequence.m_List = &TrackLocos; 
	First(&trackSequence);

	//functionSequence1.m_List = &TrackLocos;
	//First(&functionSequence1);
	//functionSequence2.m_List = &TrackLocos;
	//First(&functionSequence2);

	setup(&FreeAccys);
	for (i = 0; i < MAX_ACCESSORIES; i++)
	{
		InsertAtTop(&FreeAccys, &FreeAccyLinks[i]);
	}
	setup(&TrackAccys);
	accySequence.m_List = &TrackAccys; 
	First(&accySequence);
}


/**********************************************************************
*
* FUNCTION:		NextLocoMsg
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:	Called when the Track can take a new msg. (Currently only
*				checks Loco list). Uses Timestamp to time out zero-speed
*				Locos.
*
* RESTRICTIONS:
*
**********************************************************************/
Loco* NextLocoMsg(void)
{
	Link *pnext;
	Loco* pLoco = NULL;

	pnext = Next(&trackSequence);
	if(NULL == pnext)
	{
		//got to end of list last time...
		pnext = First(&trackSequence);
	}

#ifdef MOVED_TO_PACKET_HANDLER
	if(NULL != pnext)
	{
		if(0 == ((Loco*)(pnext->m_index))->Speed)
		{
			if(0 != ((Loco*)(pnext->m_index))->bNumStopPackets)
			{
				--((Loco*)(pnext->m_index))->bNumStopPackets;
//				break;
			}
			else
			{
				Remove((LList *)&trackSequence, pnext);
				return NULL;
//				pnext = Next(&trackSequence);
//				if(NULL == pnext)
//				{	//got to end of list last time...
//					pnext = First(&trackSequence);
//				}
			}
		}
	}
#endif

   if(pnext != NULL && pnext->m_index != 0)
   {
		pLoco = ((Loco*)(pnext->m_index));
   }

	return pLoco;
}


/**********************************************************************
*
* FUNCTION:		MakeMostRecentLoco
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:	Called when a Loco speed or direction changes. The Loco
*				is promoted to most-recent and the message will be sent
*				next to the track.
*
* RESTRICTIONS:
*
**********************************************************************/
void MakeMostRecentLoco(Loco *testLoco)
{
	Link			*pnext;
	ListIterator	testIter;

#ifdef MOVED_TO_LOCO
	if(0 != testLoco->Speed)
	{
		testLoco->bNumStopPackets = 10;
	}
#endif

	//is the loco in the track sequence list?
	testIter.m_List = trackSequence.m_List;
	if(0 == (pnext = FindByKey(&testIter, testLoco->Address)))
	{
		//not found, a new loco...
		pnext = GetFree();
		pnext->m_index = (void*)testLoco;
		pnext->m_key = testLoco->Address; 
		InsertAtTop(trackSequence.m_List, pnext);
	}
	else
	{
		//yes, update to top of list
		BringToTop(trackSequence.m_List, pnext);
	}
	First(&trackSequence);	//reset the track sequence iterator
}


/**********************************************************************
*
* FUNCTION:		RemoveLocoMsg
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:	
*
* RESTRICTIONS:
*
**********************************************************************/
void RemoveLocoMsg(Loco* pLoco)
{

	//if(pLoco->bNumStopPackets == 0 && pLoco->bNumFunction1Packets == 0 && pLoco->bNumFunction2Packets == 0)
	if(pLoco->bNumStopPackets == 0 && pLoco->bNumFunctionPackets == 0)
	{
		Remove((LList *)&trackSequence, (Link *)pLoco);
	}
}


/**********************************************************************
*
* FUNCTION:		OldestLoco
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:	Finds least-recently-used loco, removes it from list and
*				returns a pointer to it..
*
* RESTRICTIONS:
*
**********************************************************************/
Loco *OldestLoco(void)
{
	Link			*pnext;
	Link			*pcurr = NULL;
	ListIterator	testIter;
	Loco			*testLoco;

	testIter.m_List = trackSequence.m_List;
	pnext = First(&testIter);
	while(pnext)
	{
		testLoco = (Loco*)pnext->m_index;
		pnext->m_key = testLoco->Address;
		pcurr = pnext;
		pnext = Next(&testIter);
	}
	if(pcurr != NULL)
	{
		pcurr->m_pnext = NULL;	//unlink it
	}
	InsertAtTop(&FreeLocos, pnext);
	return (Loco*)pnext->m_index;
}


/**********************************************************************
*
* FUNCTION:		GetFree
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:	get an unused link from the free list
*
* RESTRICTIONS:
*
**********************************************************************/
Link *GetFree(void)
{
	Link *temp;

	if(isEmpty(&FreeLocos)) return NULL;
	temp = FreeLocos.head;
	Remove(&FreeLocos, FreeLocos.head);
	return temp;
}








//#ifdef NOT_USED
/**********************************************************************
*
* FUNCTION:		MakeMostRecentFunction
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************/
/*
void SetLocoFunctions(unsigned int nLoco, unsigned int nFunctionMap)
{
	ActiveLocos[nLoco].FunctionMap = nFunctionMap;
	MakeMostRecentFunction(&ActiveLocos[nLoco]);
}
*/

//Link			*pnext;
//ListIterator	testIter;

#ifdef MOVED_TO_LOCO
if(0 != testLoco->Speed)
{
	testLoco->bNumStopPackets = 10;
}
#endif

#ifdef NOT_USED
void MakeMostRecentFunction1(Loco *testLoco)
{
	Link			*pnext;
	ListIterator	testIter;

	//testLoco->bNumFunctionPackets = 10;

	//testLoco->Address = locoID;
	//is the loco in the track sequence list?
	testIter.m_List = functionSequence1.m_List;
	if(0 == (pnext = FindByKey(&testIter, testLoco->Address)))
	{
		//not found, a new loco...
		pnext = GetFree();
		pnext->m_index = (void*)testLoco;
		pnext->m_key = testLoco->Address;
		InsertAtTop(functionSequence1.m_List, pnext);
	}
	else
	{
		//yes, update to top of list
		BringToTop(functionSequence1.m_List, pnext);
	}
	First(&functionSequence1);	//reset the track sequence iterator
}

void MakeMostRecentFunction2(Loco *testLoco)
{
	Link			*pnext;
	ListIterator	testIter;

	//testLoco->bNumFunctionPackets = 10;

	//testLoco->Address = locoID;
	//is the loco in the track sequence list?
	testIter.m_List = functionSequence2.m_List;
	if(0 == (pnext = FindByKey(&testIter, testLoco->Address)))
	{
		//not found, a new loco...
		pnext = GetFree();
		pnext->m_index = (void*)testLoco;
		pnext->m_key = testLoco->Address;
		InsertAtTop(functionSequence2.m_List, pnext);
	}
	else
	{
		//yes, update to top of list
		BringToTop(functionSequence2.m_List, pnext);
	}
	First(&functionSequence2);	//reset the track sequence iterator
}
#endif

#ifdef NOT_USED
/**********************************************************************
*
* FUNCTION:		NextFunctionMsg
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:	Called when the Track can take a new msg. (Currently only
*				checks Loco list). Uses Timestamp to time out zero-speed
*				Locos.
*
* RESTRICTIONS:
*
**********************************************************************/
Loco* NextFunctionMsg1(void)
{
	//Link *pnext;
	Loco* pLoco = NULL;

	//pnext = Next(&functionSequence1);
	//if(NULL == pnext)
	//{
	//	//got to end of list last time...
	//	pnext = First(&functionSequence1);
	//}

   //if(pnext != NULL && pnext->m_index != 0)
   //{
	//	pLoco = ((Loco*)(pnext->m_index));
   //}

   return pLoco;


//	Link *pnext;
//	Loco* pLoco = NULL;

//	pnext = Next(&functionSequence1);
//	if(NULL == pnext)
//	{	//got to end of list last time...
//		pnext = First(&functionSequence1);
//	}

//	if(NULL != pnext)
//	{
//		if(0 != ((Loco*)(pnext->m_index))->bNumFunction1Packets)
//		{
//			--((Loco*)(pnext->m_index))->bNumFunction1Packets;
//		}
//		else
//		{
//			Remove((LList *)&functionSequence1, pnext);
//			return NULL;
////			pnext = Next(&functionSequence1);
////			if(NULL == pnext)
////			{	//got to end of list last time...
////				pnext = First(&functionSequence1);
////			}
//		}
//	}

//   if(pnext != NULL && pnext->m_index != 0)
//   {
//		pLoco = ((Loco*)(pnext->m_index));
//   }
//
//	return pLoco;
}


Loco* NextFunctionMsg2(void)
{
	//Link *pnext;
	Loco* pLoco = NULL;

	//pnext = Next(&functionSequence2);
	//if(NULL == pnext)
	//{
	//	//got to end of list last time...
	//	pnext = First(&functionSequence2);
	//}

   //if(pnext != NULL && pnext->m_index != 0)
   //{
//		pLoco = ((Loco*)(pnext->m_index));
   //}

   return pLoco;

//	Link *pnext;
//	Loco* pLoco = NULL;
//
//	pnext = Next(&functionSequence2);
//	if(NULL == pnext)
//	{	//got to end of list last time...
//		pnext = First(&functionSequence2);
//	}
//
//	if(NULL != pnext)
//	{
//		if(0 != ((Loco*)(pnext->m_index))->bNumFunction2Packets)
//		{
//			--((Loco*)(pnext->m_index))->bNumFunction2Packets;
//		}
//		else
//		{
//			Remove((LList *)&functionSequence2, pnext);
//         return NULL;
////			pnext = Next(&functionSequence2);
////			if(NULL == pnext)
////			{	//got to end of list last time...
////				pnext = First(&functionSequence2);
////			}
//		}
//	}
//
//   if(pnext != NULL && pnext->m_index != 0)
//   {
//		pLoco = ((Loco*)(pnext->m_index));
//   }
//
//	return pLoco;
}
#endif

#ifdef NOT_USED
/**********************************************************************
*
* FUNCTION:		StopAllLocos
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:	Called at start-up. All speeds are set to zero
*					Without changing order.
*
* RESTRICTIONS:
*
**********************************************************************/
void StopAllLocos(void)
{
	Link			*pnext;
	ListIterator	testIter;
	Loco			*testLoco;

	testIter.m_List = trackSequence.m_List;
	pnext = First(&testIter);
	while(pnext)
	{
		testLoco = (Loco*)pnext->m_index;
		pnext->m_key = testLoco->Address;
		testLoco->Speed = 0;	//can't use SetSpeed() 'cos it would re-order list
		pnext = Next(&testIter);
	}
	First(&trackSequence);	//reset the track sequence iterator
}
#endif

