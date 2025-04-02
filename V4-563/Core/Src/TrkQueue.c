/*******************************************************************************
* @file TrkQueue.c
* @brief Packet queue for the DCC track generator
*
* @author K. Kobel
* @date 5/2/2020
* @Revision: 1 $
* @Modtime: 10/31/2019
*
* @description  	Track Queue Management
* 					Due to the high speed requirement (DCC bit interrupt), this is a zero copy queue.
* 					 - AcquirePacket queues an indicator and returns the packet pointer and the index (for ReadyPacket)
* 					   The packet is not active for the GetPacket method until ReadyPacket and may be constructed.
* 					 - ReadyPacket makes the packet available to GetPacket.
* 					 - GetPacket returns the packet pointer and the index (for ReleasePacket)
* 					 - ReleasePacket dequeues the packet
*
* @copyright	(c) 2020  all Rights Reserved.
*******************************************************************************/
#include "Main.h"
#include "TrkQueue.h"

/*********************************************************************
*
*							DEFINITIONS
*
*********************************************************************/

PACKET_TYPE TrackQueue[TRACK_QUEUE_DEPTH];	// Queue
int track_queue_in;  						// queue input index
int track_queue_out; 						// queue output index

extern PACKET_BITS apPacket[TRACK_QUEUE_DEPTH][TRACK_QUEUE_NO_OF_BITS+1];

/*********************************************************************
*
*							FUNCTION PROTOTYPES
*
*********************************************************************/


/*********************************************************************
*
*							CODE
*
*********************************************************************/


/*********************************************************************
*
* AcquirePacket
*
* @brief	Get a new packet and the index (for ReadyPacket)
*
* @param	idx - point to the index variable
*
* @return	NULL if the queue is full
*  			Packet pointer and index if successful
*
*********************************************************************/
PACKET_BITS* AcquirePacket(int* idx)
{
	PACKET_BITS* pkt;

	if((track_queue_in == track_queue_out) && (TrackQueue[track_queue_in] != PKT_NONE))
	{
		/* queue is full */
		*idx = -1;
		return NULL;
	}
	else
	{
		/* queue is not full */
		TrackQueue[track_queue_in] = PKT_BUSY;
		*idx = track_queue_in;
		pkt = apPacket[track_queue_in];

		++track_queue_in;
		if(track_queue_in == TRACK_QUEUE_DEPTH)
		{
			track_queue_in = 0;
		}
		return pkt;
	}
}


/*********************************************************************
*
* ReadyPacket
*
* @brief	Mark a built packet for use
*
* @param	idx - index from AcquirePacket
*
* @return	none
*
*********************************************************************/
void ReadyPacket(int idx)
{

	if(idx != -1)
	{
		TrackQueue[idx] = PKT_READY;
	}
}


/*********************************************************************
*
* GetPacket
*
* @brief	Get a 'ready' packet off the queue
*
* @param	idx - point to the index variable
*
* @return	NULL if the queue is full
*  			Packet pointer and index if successful
*
*********************************************************************/
PACKET_BITS* GetPacket(int* idx)
{

	if((track_queue_in == track_queue_out) && (TrackQueue[track_queue_out] == PKT_NONE))
	{
		/* queue is empty */
		*idx = -1;
		return NULL;
	}
	else
	{
		/* queue is not empty */
		*idx = track_queue_out;
		return apPacket[track_queue_out];
	}
}


/*********************************************************************
*
* ReleasePacket
*
* @brief	mark a paket available for AcquirePacket
*
* @param	idx - index
*
* @return	none
*
*********************************************************************/
void ReleasePacket(int idx)
{

	if(idx != -1)
	{
		TrackQueue[idx] = PKT_NONE;

		++track_queue_out;
		if(track_queue_out == TRACK_QUEUE_DEPTH)
		{
			track_queue_out = 0;
		}
	}
}


/*********************************************************************
*
* IsPacketAvailable
*
* @brief	return a true if at least one packet is available
*
* @param	none
*
* @return	return a true if at least one packet is available
*
*********************************************************************/
int IsPacketAvailable(void)
{
	int index;

	for(index = 0; index < TRACK_QUEUE_DEPTH; ++index)
	{
		if(TrackQueue[index] == PKT_NONE)
		{
			return 1;
		}
	}
	return 0;
}


/*********************************************************************
*
* InitTrackQueue
*
* @brief	Setup the track queue
*
* @param	none
*
* @return	none
*
*********************************************************************/
void InitTrackQueue(void)
{
	int index;


	track_queue_in = 0;
	track_queue_out = 0;

	for(index = 0; index < TRACK_QUEUE_DEPTH; ++index)
	{
		TrackQueue[index] = PKT_NONE;
	}
}

