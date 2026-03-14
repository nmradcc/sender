/*****************************************************************************
 *
 * File:                 $Workfile: DCC.H $
 * Path:                 $Logfile: /NMRA/INCLUDE/DCC.H $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 16 $
 * Last Modified on:     $Modtime: 4/21/17 7:44p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 9/19/17 12:46p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	dcc.h	-	Fundamental DCC constants.
 *
 *****************************************************************************/

#ifndef DCC_H_DECLARED
#define DCC_H_DECLARED	"@(#) $Workfile: DCC.H $$ $Revision: 16 $$"

#include <ztypes.h>

/*--------------------------------------------------------------------------*/
/*
 * Defines
 */

/*
 *	Decoder time constants from 9.1 in microseconds.
 */
const u_long	USEC_PER_SEC	= 1000000L;			// usecs. per sec.
const u_int		MSEC_PER_SEC	= 1000;				// msecs. per sec.
const u_int		DECODER_0T_MIN	= 180;				// Minimum DCC 0.
const u_int		DECODER_0T_NOM	= 200;				// Nominal DCC 0.
const u_int		DECODER_0T_MAX	= 12000;			// Maximum DCC 0.
const u_int		DECODER_0H_MIN	= 90;				// Minimum DCC 0 high.
const u_int		DECODER_0H_NOM	= 100;				// Nominal DCC 0 high.
const u_int		DECODER_0H_MAX	= 10000;			// Maximum DCC 0 high.
const u_int		DECODER_1T_MIN	= 104;				// Minimum DCC 1.
const u_int		DECODER_1T_NOM	= 116;				// Nominal DCC 1.
const u_int		DECODER_1T_MAX	= 128;				// Maximum DCC 1.
const u_int		DECODER_1H_MIN	= 52;				// Minimum DCC 1 high.
const u_int		DECODER_1H_NOM	= 58;				// Nominal DCC 1 high.
const u_int		DECODER_1H_MAX	= 64;				// Maximum DCC 1 high.
const u_int		DECODER_AVG_MIN	=					// Avg. minimum bit time.
			(DECODER_0T_MIN + DECODER_1T_MIN)/2;
const u_int		DECODER_AVG_NOM	=					// Avg. nominal bit time.
			(DECODER_0T_NOM + DECODER_1T_NOM)/2;
const u_int		DECODER_AVG_MAX	=					// Avg. maximum bit time.
			(DECODER_0T_MAX + DECODER_1T_MAX)/2;
const u_int		CMD_0T_MIN		= 190;				// Cmd min DCC 0.
const u_int		CMD_0T_MAX		= 12000;			// Cmd max DCC 0.
const u_int		CMD_0H_MIN		= 95;				// Cmd min DCC 0 high.
const u_int		CMD_0H_MAX		= 9900;				// Cmd max DCC 0 high.
const u_int		CMD_1T_MIN		= 110;				// Cmd min DCC 1.
const u_int		CMD_1T_MAX		= 122;				// Cmd max DCC 1.
const u_int		CMD_1H_MIN		= 55;				// Cmd min DCC 1 high.
const u_int		CMD_1H_MAX		= 61;				// Cmd max DCC 1 high.
const u_int		END_0T_MIN		= 28;				// After end bit time.
const u_int		END_0H_MIN		= 26;				// After end bit high.
const u_int		AMBIG_0T_MIN  	= 28;				// Ambiguous bit time.
const u_int		AMBIG_0H_MIN	= 8;				// Ambiguous bit high.

/*
 *	Bit pattern metrics from 9.2.
 */
const u_int		BITS_IN_BYTE	= 8;				// Bits in a Byte.
const u_int		MAX_BIT_POS		= BITS_IN_BYTE - 1;	// Max bit position.
const u_int		PRE_BITS		= 12;				// Preamble bit count.
const u_int		THREE_BITS		= 39;				// 3 Byte packet bit count.
const u_int		FOUR_BITS		= 48;				// 4 Byte packet bit count.
const u_int		FIVE_BITS		= 57;				// 5 Byte packet bit count.
const u_int		SIX_BITS		= 66;				// 6 Byte packet bit count.
const u_int		BASE_BITS		= THREE_BITS;		// Baseline bit count.
const u_int		SIG_BITS		= FOUR_BITS;		// Signal packet bit count.
const u_int		RESET_1S		= 12;				// 1s per reset pkt.
const u_int		RESET_0S		= 27;				// 0s per reset pkt.
const u_int		IDLE_1S			= 28;				// 1s per idle pkt.
const u_int		IDLE_0S			= 11;				// 0s per idle pkt.
const u_int		CMD_0S			= 17;				// Approx. 0s per cmd. pkt.
const u_int		CMD_1S			= 22;				// Approx. 1s per cmd. pkt.
const BYTE		CHECK_INIT		= 0x00;				// Initial check byte.
/*
 *	STRETCH_BITS is the number of 0s to add before a preamble such that
 *	the stretched 0 after the preamble will be at the beginning of a BYTE.
 */
const int		STRETCH_BITS	= BITS_IN_BYTE - (PRE_BITS % BITS_IN_BYTE);

/*
 *	Byte and packet rates per second.
 */
const u_int		ZERO_SEC_MIN	= (u_int)  		// 0 bytes per sec @ MIN.
			((USEC_PER_SEC)/((u_long)DECODER_0T_MIN * (u_long)BITS_IN_BYTE));
const u_int		ZERO_SEC_NOM	=	(u_int)		// 0 bytes per sec @ NOM.
			((USEC_PER_SEC)/((u_long)DECODER_0T_NOM * (u_long)BITS_IN_BYTE));
const u_int		ZERO_SEC_MAX	=	(u_int)		// 0 bytes per sec @ MAX.
			((USEC_PER_SEC)/((u_long)DECODER_0T_MAX * (u_long)BITS_IN_BYTE));
const u_int		ONE_SEC_MIN		=	(u_int)		// 1 bytes per sec @ MIN.
			((USEC_PER_SEC)/((u_long)DECODER_1T_MIN * (u_long)BITS_IN_BYTE));
const u_int		ONE_SEC_NOM		=	(u_int)		// 1 bytes per sec @ NOM.
			((USEC_PER_SEC)/((u_long)DECODER_1T_NOM * (u_long)BITS_IN_BYTE));
const u_int		ONE_SEC_MAX		=	(u_int)		// 1 bytes per sec @ MAX.
			((USEC_PER_SEC)/((u_long)DECODER_1T_MAX * (u_long)BITS_IN_BYTE));
const u_int		AVG_SEC_MIN		=	(u_int)		// Avg bytes per sec @ MIN.
			((USEC_PER_SEC)/((u_long)DECODER_AVG_MIN * (u_long)BITS_IN_BYTE));
const u_int		AVG_SEC_NOM		=	(u_int)		// Avg bytes per sec @ NOM.
			((USEC_PER_SEC)/((u_long)DECODER_AVG_NOM * (u_long)BITS_IN_BYTE));
const u_int		AVG_SEC_MAX		=	(u_int)		// Avg bytes per sec @ MAX.
			((USEC_PER_SEC)/((u_long)DECODER_AVG_MAX * (u_long)BITS_IN_BYTE));
const u_int		BASE_SEC_MIN	=	(u_int)		// Avg pkts per sec @ MIN.
			((USEC_PER_SEC)/((u_long)DECODER_AVG_MIN * (u_long)BASE_BITS));
const u_int		BASE_SEC_NOM	=	(u_int)		// Avg pkts per sec @ NOM.
			((USEC_PER_SEC)/((u_long)DECODER_AVG_NOM * (u_long)BASE_BITS));
const u_int		BASE_SEC_MAX	=	(u_int)		// Avg pkts per sec @ MAX.
			((USEC_PER_SEC)/((u_long)DECODER_AVG_MAX * (u_long)BASE_BITS));
const u_int		RESET_SEC_MIN	=	(u_int)		// Reset pkts per sec @ MIN.
			((USEC_PER_SEC)/
			((u_long)DECODER_0T_MIN*(u_long)RESET_0S
			+ (u_long)DECODER_1T_MIN*(u_long)RESET_1S));
const u_int		RESET_SEC_NOM	=	(u_int)		// Reset pkts per sec @ NOM.
			((USEC_PER_SEC)/
			((u_long)DECODER_0T_NOM*(u_long)RESET_0S
			+ (u_long)DECODER_1T_NOM*(u_long)RESET_1S));
const u_int		RESET_SEC_MAX	=	(u_int)		// Reset pkts per sec @ MAX.
			((USEC_PER_SEC)/
			((u_long)DECODER_0T_MAX*(u_long)RESET_0S
			+ (u_long)DECODER_1T_MAX*(u_long)RESET_1S));
const u_int		IDLE_SEC_MIN	=	(u_int)		// Idle pkts per sec @ MIN.
			((USEC_PER_SEC)/
			((u_long)DECODER_0T_MIN*(u_long)IDLE_0S
			+ (u_long)DECODER_1T_MIN*(u_long)IDLE_1S));
const u_int		IDLE_SEC_NOM	=	(u_int)		// Idle pkts per sec @ NOM.
			((USEC_PER_SEC)/
			((u_long)DECODER_0T_NOM*(u_long)IDLE_0S
			+ (u_long)DECODER_1T_NOM*(u_long)IDLE_1S));
const u_int		IDLE_SEC_MAX	=	(u_int)		// Idle pkts per sec @ MAX.
			((USEC_PER_SEC)/
			((u_long)DECODER_0T_MAX*(u_long)IDLE_0S
			+ (u_long)DECODER_1T_MAX*(u_long)IDLE_1S));

/*
 *	Speed related values.
 */
const int		SP_STOP_I		= -1;				// stop ignore dir.
const int		SP_E_STOP		= -2;				// Emerg stop.
const int		SP_E_STOP_I		= -3;				// Emerg stop ignore dir.
const int		SP_MIN			= 0;				// Min speed (stop).
const int		SP_14_MAX		= 14;				// Max 14 step speed.
const int		SP_28_MAX		= 28;				// Max 14 step speed.
const int		ACC_MAX			= 7;				// Max accessory id.

/*
 *	Address related values.
 */
const u_short	LOC_ADDR_MIN	= 1;				// Min loco address.
const u_short	LOC_ADDR_MAX	= 127;				// Max loco address.
const u_short	LOC_ADDR_BROAD	= 0;				// Loco broadcast addr.
const u_short	ACC_ADDR_MIN	= 1;				// Min accessory address.
const u_short	ACC_ADDR_MAX	= 510;				// Max accessory address.
const u_short	ACC_ADDR_BROAD	= 511;				// Acc. broadcast addr.
const u_short	SIG_ADDR_MIN	= 1;				// Min signal address.
const u_short	SIG_ADDR_MAX	= 2043;				// Max signal address.
const u_short	SIG_ADDR_BROAD	= 2044;				// Signal broadcast addr.
const BYTE		ASPECT_MAX		= 0x1f;				// Max signal aspect.

/*
 *	Invalid bit time value.
 */
const u_int		T_INV			= 0;				// Invalid time value.

#endif /* DCC_H_DECLARED */
