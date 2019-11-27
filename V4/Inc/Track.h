 /**********************************************************************
*
* SOURCE FILENAME:	Track.h
*
* DATE CREATED:		29/Aug/99
*
* PROGRAMMER:
*
* DESCRIPTION:
*
* COPYRIGHT (c) 1999-2017 by K2 Engineering  All Rights Reserved.
*
**********************************************************************/

#ifndef Track_H
#define Track_H

/**********************************************************************
*
*							DEFINITIONS
*
**********************************************************************/

#define TIMER_PRESCALER			84
#define TICKS_PER_MICROSECOND	2

#define ONE_PERIOD  (116 * TICKS_PER_MICROSECOND)
#define ZERO_PERIOD (200 * TICKS_PER_MICROSECOND)

#define ONE_PULSE   (ONE_PERIOD/2)
#define ZERO_PULSE  (ZERO_PERIOD/2)

#define NO_OF_PREAMBLE_BITS		18

/** @enum PACKET_BITS
	@brief The three registers that define the track output bits
 */
typedef struct packet_t
{
	uint16_t	period;
	uint16_t	pulse;
	uint16_t	count;
} PACKET_BITS;


/** @enum TRACK_RESOURCE
	@brief Which resource is using the track output
 */
typedef enum
{
	TR_NONE,
	TR_TESTER,
	TR_COMMAND_STATION,
} TRACK_RESOURCE;


/** @enum TRACK_IDLE
	@brief What the track output does when it runs out of packets
 */
typedef enum
{
	TI_NONE,
	TI_IDLE,
	TI_RESET,
} TRACK_IDLE;


/** @enum TRACK_LOCK_STATUS
	@brief The current track lock ststus
 */
typedef enum
{
	TL_NONE,
	TL_ASSIGNED,
	TL_LOCKED,
} TRACK_LOCK_STATUS;


/** @struct TRACK_LOCK
	@brief The track lock structure
 */
typedef struct tracklock_t
{
	TRACK_RESOURCE	lock;
	TRACK_IDLE		idle;
	uint16_t		preambles;
} TRACK_LOCK;


/** TRACK_RESOURCE
	@brief Track Lock variable
 */
extern TRACK_LOCK TrackLock;

 /**********************************************************************
*
*							FUNCTION PROTOTYPES
*
**********************************************************************/

extern void MainTrackConfig(void);

extern void EnableTrack(void);
extern void DisableTrack(void);

extern uint8_t GetTrackState(void);

extern int BuildPacket(const uint8_t* buf, uint8_t len, uint16_t clk1t, uint16_t clk0t, uint16_t clk0h);

extern int BuildPacketBits(const PACKET_BITS* packet, uint8_t count);

#endif
