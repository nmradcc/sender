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


#define TIMER_PRESCALER			1
#define TICKS_PER_MICROSECOND	42

#define ONE_PERIOD  (116 * TICKS_PER_MICROSECOND)
#define ZERO_PERIOD (200 * TICKS_PER_MICROSECOND)

#define ONE_PULSE   (ONE_PERIOD/2)
#define ZERO_PULSE  (ZERO_PERIOD/2)

#define NO_OF_PREAMBLE_BITS		18

#define DEFAULT_PREAMBLES		0

/** @enum PACKET_BITS
	@brief The two registers that define the track output bits and scope output position
 */
typedef struct packet_t
{
	uint32_t	period;
	uint32_t	pulse;
	uint32_t	scope;
} PACKET_BITS;

/** @enum TRACK_RESOURCE
	@brief Which resource is using the track output
 */
typedef enum
{
	TR_NONE,
	TR_TESTER,
	TR_SHELL,
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
	TI_ONES,
	TI_ZEROS,
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

extern int BuildPacket(const uint8_t* buf, uint8_t len, uint32_t clk1t, uint32_t clk0t, uint32_t clk0h);

extern int BuildPacketCS(const uint8_t* buf, uint8_t len, uint32_t clk1t, uint32_t clk0t, uint32_t clk0h, uint32_t no_preambles);

extern int BuildPacketBits(const PACKET_BITS* packet, uint8_t count);


TRACK_LOCK_STATUS OpenTrack(TRACK_RESOURCE tr, TRACK_IDLE ti, uint16_t preambles);
void CloseTrack(void);
uint8_t IsTrackOpen(TRACK_RESOURCE tr);
TRACK_RESOURCE GetTrackLock(void);
TRACK_IDLE GetTrackIdle(void);
void SetTrackIdle(TRACK_IDLE ti);

#endif
