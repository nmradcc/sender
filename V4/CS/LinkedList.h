/**********************************************************************
*
* SOURCE FILENAME:	LnkList.h
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
typedef struct Link
{
	void *m_index;
	int m_key;
	struct Link *m_pnext;
} Link;

typedef struct //implements List
{
	Link *head;		//points to first (or null)
	Link *tail;    //points to last (its next is null) (or null)
} LList;

// the iterators
typedef struct
{
	Link *m_curr;
	LList *m_List;
} ListIterator;

void setup(LList *myList);

void Clear(Link *myLink); 
extern Link *FindByKey(ListIterator* myIter, int iKey);		//returns NULL if not found
extern void BringToTop(LList *myList, Link *it);
void Append(LList *myList, Link *it);
extern void InsertAtTop(LList *myList, Link *it);
void Remove(LList *myList, Link *it);
//the iterator functions
extern Link *First(ListIterator* myIter);
extern Link *Next(ListIterator* myIter);
extern unsigned char isEmpty(LList *myList);

