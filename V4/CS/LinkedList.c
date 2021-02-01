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
* COPYRIGHT (c) 2000-2002 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/
//#include <dos.h>
//#include <stdafx.h>
#include <stdio.h>
#include "Main.h"
#include "LinkedList.h"
#include "Loco.h"

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

/**********************************************************************
*
*							GLOBAL VARIABLES
*
**********************************************************************/

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
  	int GetKey(Link *myLink) { return myLink->m_key; }
	Link *isInList(LList *myList, Link *it);
	unsigned char isFull(LList *myList);
  	Link * GetNext(Link *myLink) { return myLink->m_pnext; }
  	void SetNext(Link *myLink, Link *pNext) { myLink->m_pnext = pNext; }


void Clear(Link *myLink)
{
	myLink->m_index = NULL;
	myLink->m_key = 0;
	myLink->m_pnext = NULL;
}


void setup(LList *myList)
{
   //build all the elements and link them...
   //head = curr = First, Tail = last
	myList->head = NULL;		//points to first (or null)
	myList->tail = NULL;    //points to last (its next is null) (or null)
}

// the insert method.
void InsertAtTop(LList *myList, Link *it)
{
	Link *temp;

	if(NULL == myList->tail) myList->tail = it; //the first
	temp = myList->head;
	myList->head = it;
	SetNext(it, temp);
}


// The append method
void Append(LList *myList, Link *it)
{
	SetNext(myList->tail, it);			//point last tail to new
	myList->tail = it;					//make new the tail
}




//remove a link 
void Remove(LList *myList, Link *it)
{
	Link *temp;

	if(myList->head == it)
	{
		myList->head = GetNext(it);
	}
	else
	{
		if ((temp = isInList(myList, it)) == NULL) 
		{
			return;
		}
		else 
		{
			SetNext(temp, GetNext(it));	//link round removed
		}
	}
	//InsertAtTop(&FreeLocos, it);		//put away in free list
}


//removed a link and put it at the top of the list
void BringToTop(LList *myList, Link *it)
{
	Link *temp;

	if(myList->head != it)	//NOT already at top
	{
		if ((temp = isInList(myList, it)) == NULL) return;
		SetNext(temp, GetNext(it)); //link round removed
		temp = myList->head;
		myList->head = it;
		SetNext(it, temp);
	}
}


//isEmpty()
unsigned char isEmpty(LList *myList)
{
   return (myList->head == NULL);
}


//isFull()
//BOOL isFull(LList *myList)
//{
//   return (myList->tail == myList->head);
//}


//returns the link that points to the 'it' (will fail if 'it' is first)
Link *isInList(LList *myList, Link *it)
{
	Link *temp;

	if(isEmpty(myList)) return NULL;
	temp = myList->head;
	while(it != GetNext(temp))
	{
		if(GetNext(temp) == NULL)
		{
   			return NULL;
		}
   		temp = GetNext(temp);
	}
	return temp;
}


//Iterator functions...
Link *First(ListIterator* myIter)
{
	myIter->m_curr = myIter->m_List->head;
   return myIter->m_curr;
}


Link *Next(ListIterator* myIter)			//returns NULL if no more in list
{
	if(myIter->m_curr == myIter->m_List->tail)	//already done last one
   	return NULL;
	myIter->m_curr = GetNext(myIter->m_curr);
	return myIter->m_curr;
}


Link *FindByKey(ListIterator* myIter, int iKey)		//returns NULL if not found
{
	First(myIter);
	if(myIter->m_List->head == NULL)	//empty list
	{
		return NULL;
	}
	while(iKey != GetKey(myIter->m_curr))
	{
   		if(NULL == Next(myIter))
		{
      		return NULL;
		}
	}
	return myIter->m_curr;
}


