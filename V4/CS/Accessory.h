/*********************************************************************!

  @file
  @subpage Accessory Packet Queue
 
  @brief Accessory Queue functions to create, find, get and set queue entries
 
 These functions create new accessory decoder addresses in the
 the accessory queue.  It can be searched with the find() function,
 and the status can be read via get() or writen via set().  New
 entries are created with the new() fuction.

 
*		Accessary.h
*
* @date 8/19/98
*
* PROGRAMMER:			Karl Kobel
*
* COPYRIGHT (c) 1999-2002 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define MAX_ACCESSORIES	16		///< Accessory queue size
//#define MAX_ACCESSORIES	10

/// Accessory structure holds the queue information for accessory decoders
typedef struct 
{
	unsigned int	Address;		///< the address of the Accessory
	unsigned char	State;			///< the state of the Accessory
	unsigned int	TimeStamp;		///< an indication of how old this Accessory is
} ACCESSORY;


//extern static ACCESSORY aAccessory[MAX_ACCESSORIES];

/**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

/*! @brief	NewAccessory() creates an entry in the queue
	
	NewAccessory() creates an entry in the queue for an accessory output 
	point.  If there is no available entry in the queue, the function returns
	ACC_EMPTY, otherwise it returns the position in the queue.
 
	@param	[in] nAddress - DCC address of the accessory point
 
	@return	unsigned int - queue position or ACC_EMPTY
 
 */

unsigned int NewAccessory(unsigned int nAddress);

/*! @brief	FindAccessory() finds an entry in the queue
	
	FindAccessory() searches the queue for an accessory output
	point.  If there is no available entry in the queue, the function returns
	ACC_EMPTY, otherwise it returns the position in the queue.
 
	@param	[in] nAddress - DCC address of the accessory point
 
	@return	unsigned int - queue position or ACC_EMPTY
 
 */
unsigned int FindAccessory(unsigned int nAddress);

/*! @brief	GetAccessoryState() gets the status of an entry in the queue
	
	GetAccessory() retrieves the status an entry in the queue
	based on it's position in the queue.  The function returns
	ACC_EMPTY, otherwise it returns the position in the queue.
 
	@param	[in] nAccessory - index of the queue entry
 
	@return	byte - queue entry State
 
 */
unsigned char GetAccessoryState(unsigned int nAccessory);

/*! @brief	SetAccessoryState() sets the statsus an entry in the queue
	
	SetAccessoryState() sets the state an entry in the queue for
	an accessory output point.  It then generates an DCC Accessory
	Packet and adds (5) copies of the packet to the /u Programming /u
	Track Queue.
 
	@param	[in] nAccessory - index of the queue entry
	@param	[in] nState - State of the entry
 
	@return	(none)
 
 */
void SetAccessoryState(unsigned int nAccessory, unsigned char nState);

/*! @brief	GetAccessoryAddress() returns the address an entry in the queue
	
	GetAccessoryAddress() retrieves the DCC Address of an entry in 
	the queue.
 
	@param	[in] nAccessory - index of the queue entry
 
	@return	unsigned int nAddress - address of the accessory point
 
 */
unsigned int GetAccessoryAddress(unsigned int nAccessory);

