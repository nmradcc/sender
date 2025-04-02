/*******************************************************************************
* @file TrkQueue.h
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
#ifndef _TRK_QUEUE_H
#define _TRK_QUEUE_H

#include "Track.h"

/*********************************************************************
*
*							DEFINITIONS
*
*********************************************************************/

#define TRACK_QUEUE_DEPTH 		4
//#define TRACK_QUEUE_NO_OF_BITS	80

/** @struct PACKET_TYPE
	@brief The queue packet type
 */
typedef enum
{
	PKT_NONE,
	PKT_BUSY,
	PKT_READY,
} PACKET_TYPE;

/*********************************************************************
*
*							FUNCTION PROTOTYPES
*
*********************************************************************/

extern PACKET_BITS* AcquirePacket(int* idx);
extern void ReadyPacket(int idx);
extern PACKET_BITS* GetPacket(int* idx);
extern void ReleasePacket(int idx);

extern int IsPacketAvailable(void);

extern void InitTrackQueue(void);

#endif

