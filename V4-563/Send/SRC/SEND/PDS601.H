/*****************************************************************************
 *
 * File:                 $Workfile: PDS601.H $
 * Path:                 $Logfile: /NMRA/SRC/SEND/PDS601.H $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 5 $
 * Last Modified on:     $Modtime: 5/15/04 8:21a $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 5/15/04 8:32a $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	pds601.h	-	This is the header file containing defines specific
 *					to the JDR Microdevices PDS-601 Breadboard.
 *
 *****************************************************************************/

#ifndef PDS601_H_DECLARED
#define PDS601_H_DECLARED	"@(#) $Workfile: PDS601.H $$ $Revision: 5 $$"

#if SEND_VERSION < 4
 #include <dos.h>
#endif
#include <dcc.h>

#define B_MSK(x)				( (BYTE)(1<<(x)) )
#define CNT_UPPER(x)			( (BYTE)( ((x)>>8) & 0xff) )
#define CNT_LOWER(x)			( (BYTE)((x) & 0xff) )

/*
 *	Base I/O port number.
 */
extern u_short		Port_base;
const u_short	 	PORT_DEF	= 0x340;	 		// Default I/O base port.

/*
 *	Port number defines.
 */
#define GEN			(Port_base)						// Generic inputs.
#define GEN_2		(Port_base + 0x04)				// Generic inputs 2.
#define GEN_OUT		(Port_base + 0x08)				// Generic outputs.
#define PA			(Port_base + 0x10)				// 8255 port A.
#define PB			(Port_base + 0x11)				// 8255 port B.
#define PC			(Port_base + 0x12)				// 8255 port C.
#define P_CNTRL		(Port_base + 0x13)				// 8255 control port.
#define CNT_0T		(Port_base + 0x14)				// 8254 counter 0.
#define CNT_0H		(Port_base + 0x15)				// 8254 counter 1.
#define CNT_1T		(Port_base + 0x16)				// 8254 counter 2.
#define CNT_CNTRL	(Port_base + 0x17)				// 8254 contrl port.

#if SEND_VERSION < 4
 #define OUT_PC( bit_id, val )		outportb(P_CNTRL, ((bit_id)<<1) | ((val) & 0x1))
#endif

/*
 *	Port B input bit positions.
 */
const BYTE	 	PB_POS_UNDER		= 0;			// Underflow.
const BYTE	 	PB_POS_CLK1L		= 1;			// Clk 1 NOT.
const BYTE	 	PB_POS_DCC0			= 2;			// DCC out 0.
const BYTE		PB_POS_DCCQH		= 3;			// DCC data out.
const BYTE		PB_POS_82G0H		= 4;			// 8254 0H gate.
const BYTE		PB_POS_SHOLDL		= 5;			// Shift hold NOT.
const BYTE		PB_POS_SCOPE		= 6;			// Scope output.
const BYTE		PB_POS_DCCOUTD		= 7;			// DCC signal output.

/*
 *	Port B input bit masks.
 */
const BYTE		PB_MSK_UNDER		= B_MSK(0);		// Underflow.
const BYTE		PB_MSK_CLK1L		= B_MSK(1);		// Clk 1 NOT.
const BYTE		PB_MSK_DCC0			= B_MSK(2);		// DCC out 0.
const BYTE		PB_MSK_DCCQH		= B_MSK(3);		// DCC data out.
const BYTE		PB_MSK_82G0H		= B_MSK(4);		// 8254 0H gate.
const BYTE		PB_MSK_SHOLDL		= B_MSK(5);		// Shift hold NOT.
const BYTE		PB_MSK_SCOPE		= B_MSK(6);		// Scope output.
const BYTE		PB_MSK_DCCOUTD		= B_MSK(7);		// DCC signal output.

/*
 *	Port C output bit values.
 */
const BYTE		PC_POS_BDRST		= 0; 			// Reset Board.
const BYTE		PC_POS_CPUEN		= 1;			// Enable CPU clock.
const BYTE		PC_POS_CPUCLKL		= 2;			// CPU clock NOT.
const BYTE		PC_POS_INTRA		= 3;			// Interrupt active.
const BYTE		PC_POS_UNDERCLRL	= 4;			// Clear under NOT.
const BYTE		PC_POS_SCOPENL		= 5;			// Enable Scope output NOT.
const BYTE		PC_POS_ACKAL		= 6;			// Data Ack. (Input)
const BYTE		PC_POS_OBFAL		= 7;			// Output buffer full NOT.

/*
 *	Port C output bit masks.
 */
const BYTE	  	PC_MSK_BDRST		= B_MSK(0);		// Reset Board.
const BYTE	  	PC_MSK_CPUEN		= B_MSK(1);		// Enable CPU clock.
const BYTE	  	PC_MSK_CPUCLKL		= B_MSK(2);		// CPU clock NOT.
const BYTE	  	PC_MSK_INTRA		= B_MSK(3);		// Interrupt active.
const BYTE	  	PC_MSK_UNDERCLRL	= B_MSK(4);		// Clear under NOT.
const BYTE	  	PC_MSK_SCOPENL		= B_MSK(5);		// Enable Scope output NOT.
const BYTE	  	PC_MSK_ACKAL		= B_MSK(6);		// Data Ack. (Input)
const BYTE	  	PC_MSK_OBFAL		= B_MSK(7);		// Output buffer full NOT.

/*
 *	8255 Control port values.
 */
const BYTE		P_CNTRL_RST			= 0x9b;			// Power up (all inputs).
const BYTE		P_CNTRL_NORM		= 0xa2;			// Normal mode.
													// A		-	Mode 1 out
													// B		-	Mode 0 in
													// C upper	-	Out
													// C lower	-	Out

/*
 *	8254 Control port values.
 */
const BYTE	   	CNTRL_0T_NORM		= 0x36;			// 0T normal (mode 3).
const BYTE	   	CNTRL_0H_RST		= 0x70;			// 0H reset (mode 0).
const BYTE	   	CNTRL_0H_NORM		= 0x7a;			// 0H normal (mode 5).
const BYTE	   	CNTRL_1T_NORM		= 0xb6;			// 1T normal (mode 3).
const BYTE		CNTRL_0T_STATUS		= 0xe2;			// 0T status readback.

/*
 *	8254 counter values.
 */
const u_short  	CNT_0T_INIT	  = DECODER_0T_NOM;		// 0T initial count.
const u_short  	CNT_0H_INIT	  = DECODER_0H_NOM - 2;	// 0H initial count.
const u_short  	CNT_1T_INIT	  = DECODER_1T_NOM;		// 1T initial count.

/*
 *	8254 Status inputs bit mask values.
 */
const BYTE      CNT_STATUS_OUTPUT	= B_MSK(7);		// Cnt output bit state.

/*
 *	Generic output mask.
 */
const BYTE		GEN_OUT_MASK		= 0x3f;			// 0 out bits 6 & 7.
const BYTE	  	GEN_OUT_SHIFT 		= B_MSK(7);		// Shift out bit.

#endif /* PDS601_H_DCLARED */


/*****************************************************************************
 * $History: PDS601.H $
 * 
 * *****************  Version 5  *****************
 * User: Kenw         Date: 5/15/04    Time: 8:32a
 * Updated in $/NMRA/SRC/SEND
 * Added 1 interior feedback bit test
 * 
 * *****************  Version 3  *****************
 * User: Kenw         Date: 5/10/96    Time: 9:18a
 * Updated in $/NMRA/SRC/SEND
 * Cleaned up formatting of columns.
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 4/30/96    Time: 8:26p
 * Updated in $/NMRA/SRC/SEND
 * Added () to fully delimit macros.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 3:47p
 * Created in $/NMRA/SRC/SEND
 * Header file which defines hardware specific constants for the sender
 * test
 * board.  This is based on the PDS601 prototype board.
 *
 *****************************************************************************/

