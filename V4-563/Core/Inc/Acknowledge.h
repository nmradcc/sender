/*
 * Acknowledge.h
 *
 *  Created on: May 25, 2019
 *      Author: Karl
 */

#ifndef ACKNOWLEDGE_H_
#define ACKNOWLEDGE_H_

typedef enum
{
	NO_ACK,
	LOCO_PRESENT,
	ACK_DETECTED,
	OVER_CURRENT,
} ACK_STATUS;


extern void InitAcknowledge(void);

extern void Acknowledge(void);

//extern void ClearAck(void);
//extern byte IsAck(void);

extern ACK_STATUS GetAck(void);

#endif /* ACKNOWLEDGE_H_ */
