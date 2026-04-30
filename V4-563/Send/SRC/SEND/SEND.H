/*****************************************************************************
 *
 * File:                 $Workfile: SEND.H $
 * Path:                 $Logfile: /NMRA/SRC/SEND/SEND.H $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 51 $
 * Last Modified on:     $Modtime: 9/12/23 4:53p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 9/12/23 6:46p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	send.h	-	Public defines for main() Sender test program in send.cpp.
 *
 *****************************************************************************/

#ifndef SEND_H_DECLARED
#define SEND_H_DECLARED	"@(#) $Workfile: SEND.H $$ $Revision: 51 $$"

#include <ztypes.h>

/*
 *	Possible version release levels.
 */
enum Rel_levels
{
	VER_DEB						= 'D',			// Debug only.
	VER_EXP						= 'X', 			// Prerelease experimenatal.
	VER_BETA					= 'B',	 		// Prerelease beta.
	VER_REL						= 'R'	 		// Released.
};

/*
 *	Types of decoders.
 */
enum Dec_types
{
	DEC_LOCO					= 'L',	 		// Locomotive decoder.
	DEC_ACC						= 'A',	 		// Accessory decoder.
    DEC_SIG						= 'S',			// Signal decoder.
    DEC_FUNC					= 'F'			// Function decoder.
};

/*
 *	Version information.
 */
const Rel_levels	Ver_rel		= VER_EXP;		// Release level.
const u_int			Ver_maj		=  5;			// Major version.
const u_int			Ver_min		= 11;			// Minor version.
const u_int			Ver_bld		=  2;			// Build version.

/*
 *	Forward references.
 */
class				Send_reg;					// Defined in send_reg.h

/*
 *	Global copy of Send board registers.
 */
extern Send_reg 	Dcc_reg;

/*
 * Macro to clear the line.
 */
#define CLR_LINE	printf( \
"                                                                            \r" )

#endif /* SEND_H_DECLARED */


/*****************************************************************************
 * $History: SEND.H $
 * 
 * *****************  Version 51  *****************
 * User: Kenw         Date: 9/12/23    Time: 6:46p
 * Updated in $/NMRA/SRC/SEND
 * Rolled release version fromm X.5.11.1 to X.5.11.2,
 * 
 * *****************  Version 50  *****************
 * User: Kenw         Date: 6/22/23    Time: 7:27p
 * Updated in $/NMRA/SRC/SEND
 * Added send_reg::kick_start() public method.
 * Verified is worked with a manual 'k' command.
 * 
 * *****************  Version 49  *****************
 * User: Kenw         Date: 9/19/20    Time: 4:22p
 * Updated in $/NMRA/SRC/SEND
 * Rolling to release B.5.10.2
 * 
 * *****************  Version 48  *****************
 * User: Kenw         Date: 9/19/20    Time: 3:42p
 * Updated in $/NMRA/SRC/SEND
 * Rolled to X.5.10.2
 * 
 * *****************  Version 47  *****************
 * User: Kenw         Date: 3/26/20    Time: 8:55p
 * Updated in $/NMRA/SRC/SEND
 * Intermediate build to 5.10.1
 * Checking in as X.5.10.1.
 * 
 * *****************  Version 45  *****************
 * User: Kenw         Date: 7/26/19    Time: 3:29p
 * Updated in $/NMRA/SRC/SEND
 * Moved from X.5.9.3 to B.5.9.3
 * 
 * *****************  Version 44  *****************
 * User: Kenw         Date: 7/24/19    Time: 7:18p
 * Updated in $/NMRA/SRC/SEND
 * Check in prior to B.5.9.3 release.
 * 
 * *****************  Version 43  *****************
 * User: Kenw         Date: 12/05/18   Time: 4:56p
 * Updated in $/NMRA/SRC/SEND
 * Changed build number to X.5.9.1.
 * 
 * *****************  Version 42  *****************
 * User: Kenw         Date: 7/03/18    Time: 7:28p
 * Updated in $/NMRA/SRC/SEND
 * Intermin Check in X.5.8.3
 * 
 * *****************  Version 41  *****************
 * User: Kenw         Date: 4/17/17    Time: 8:41p
 * Updated in $/NMRA/SRC/SEND
 * Added Ver_bld item.
 * 
 * *****************  Version 39  *****************
 * User: Kenw         Date: 9/17/14    Time: 4:44p
 * Updated in $/NMRA/SRC/SEND
 * Added function decoder support for manual mode.
 * Modified decoder quiestions.
 * Print <SEND_END n> when program exits.
 * 
 * *****************  Version 37  *****************
 * User: Kenw         Date: 5/20/14    Time: 7:13p
 * Updated in $/NMRA/SRC/SEND
 * Updated to B.5.5.1
 * 
 * *****************  Version 36  *****************
 * User: Kenw         Date: 5/02/14    Time: 4:05p
 * Updated in $/NMRA/SRC/SEND
 * Rolled to X.5.5 after fix to SEND_REG.CPP
 * send_2_ambig_bits() method.
 * 
 * *****************  Version 35  *****************
 * User: Kenw         Date: 11/09/13   Time: 7:50p
 * Updated in $/NMRA/SRC/SEND
 * Added Function tests.
 * Added test summary to screen after each test phase.
 * 
 * *****************  Version 34  *****************
 * User: Kenw         Date: 9/15/05    Time: 7:13p
 * Updated in $/NMRA/SRC/SEND
 * 
 * *****************  Version 33  *****************
 * User: Kenw         Date: 12/15/04   Time: 3:51p
 * Updated in $/NMRA/SRC/SEND
 * Updated to roll to Version 5.2 X.
 * Added back 11800 single stretch times
 * 
 * *****************  Version 32  *****************
 * User: Kenw         Date: 12/15/04   Time: 12:30p
 * Updated in $/NMRA/SRC/SEND
 * Updated for Version 5.1 Experimental
 * 
 * *****************  Version 31  *****************
 * User: Kenw         Date: 11/01/04   Time: 7:15p
 * Updated in $/NMRA/SRC/SEND
 * Added very long (13000 usec) stretched 0 test.
 * 
 * *****************  Version 30  *****************
 * User: Kenw         Date: 7/20/04    Time: 2:27p
 * Updated in $/NMRA/SRC/SEND
 * Updated for X.4.8.  Added extra preamble support to
 * Feedback 1 and Feedback 2 tests.
 * 
 * *****************  Version 29  *****************
 * User: Kenw         Date: 7/01/04    Time: 7:31p
 * Updated in $/NMRA/SRC/SEND
 * Updated to include testing feedback bits in all error Byte bits
 * 
 * *****************  Version 28  *****************
 * User: Kenw         Date: 6/06/04    Time: 2:26p
 * Updated in $/NMRA/SRC/SEND
 * Extended -E command to add preambles to Ramp, Address, and bits
 * test.  Changed manual -i and -d command to send 4 packet sequences
 * to help debug LV102
 * 
 * *****************  Version 27  *****************
 * User: Kenw         Date: 6/04/04    Time: 4:39p
 * Updated in $/NMRA/SRC/SEND
 * Added Feedback 1 Test
 * 
 * *****************  Version 26  *****************
 * User: Kenw         Date: 5/15/04    Time: 8:32a
 * Updated in $/NMRA/SRC/SEND
 * Added 1 interior feedback bit test
 * 
 * *****************  Version 25  *****************
 * User: Kenw         Date: 2/01/04    Time: 6:28p
 * Updated in $/NMRA/SRC/SEND
 * Updated to B4.3.
 * Includes support for checksum calculation and logging
 * of command line switches.
 * 
 * *****************  Version 24  *****************
 * User: Kenw         Date: 1/11/04    Time: 12:30p
 * Updated in $/NMRA/SRC/SEND
 * Updated for Version 4.1 Beta.
 * Adds feedback test.
 * 
 * *****************  Version 23  *****************
 * User: Kenw         Date: 9/01/00    Time: 6:33p
 * Updated in $/NMRA/SRC/SEND
 * Increased tests for 6 BYTE packets to max of 21
 * 
 * *****************  Version 22  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:57p
 * Updated in $/NMRA/SRC/SEND
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 19  *****************
 * User: Kenw         Date: 1/09/99    Time: 1:51p
 * Updated in $/NMRA/SRC/SEND
 * Updated for B.2.15
 * Changed to print out current hardware state when header is printed.
 * 
 * *****************  Version 18  *****************
 * User: Kenw         Date: 8/10/98    Time: 7:06p
 * Updated in $/NMRA/SRC/SEND
 * Moving to B.2.14
 * 
 * *****************  Version 17  *****************
 * User: Kenw         Date: 8/08/98    Time: 7:40p
 * Updated in $/NMRA/SRC/SEND
 * Updated to X.2.14
 * 
 * *****************  Version 16  *****************
 * User: Kenw         Date: 7/11/98    Time: 8:36a
 * Updated in $/NMRA/SRC/SEND
 * Rolling to 2.13.
 * Added tests near maximum packet times.
 * Added ability to move scope trigger before or after trigger packet..
 * 
 * *****************  Version 15  *****************
 * User: Kenw         Date: 6/28/98    Time: 9:38a
 * Updated in $/NMRA/SRC/SEND
 * Changed to abort program at LOG_ERR. Rolling to X.2.12.
 * 
 * *****************  Version 14  *****************
 * User: Kenw         Date: 5/30/98    Time: 9:11p
 * Updated in $/NMRA/SRC/SEND
 * Added support for manual hard reset 'R' command.
 * Rolled to X.2.11
 * 
 * *****************  Version 13  *****************
 * User: Kenw         Date: 11/01/97   Time: 8:24p
 * Updated in $/NMRA/SRC/SEND
 * X.2.10 - Cleared accessory decoder init packet in dcc_bits3.
 * 
 * *****************  Version 12  *****************
 * User: Kenw         Date: 10/30/97   Time: 5:53p
 * Updated in $/NMRA/SRC/SEND
 * X.2.9 - Format clean ups only.
 * 
 * *****************  Version 11  *****************
 * User: Kenw         Date: 10/29/97   Time: 4:26p
 * Updated in $/NMRA/SRC/SEND
 * X.2.8 - Added packet log command.
 * 
 * *****************  Version 10  *****************
 * User: Kenw         Date: 10/28/97   Time: 2:38p
 * Updated in $/NMRA/SRC/SEND
 * Added Prior Packet Test & Idle manual command.
 * 
 * *****************  Version 9  *****************
 * User: Kenw         Date: 1/21/97    Time: 7:41p
 * Updated in $/NMRA/SRC/SEND
 * Increased Bits buffer size to 256.  Added error message if Bits objects
 * have
 * errors.
 * 
 * *****************  Version 8  *****************
 * User: Kenw         Date: 10/15/96   Time: 4:32p
 * Updated in $/NMRA/SRC/SEND
 * Added zero as last bit of packet fragment test.
 * Turned this test off by default
 * 
 * *****************  Version 7  *****************
 * User: Kenw         Date: 9/25/96    Time: 7:54p
 * Updated in $/NMRA/SRC/SEND
 * Added nested packet test
 * 
 * *****************  Version 6  *****************
 * User: Kenw         Date: 7/26/96    Time: 5:46p
 * Updated in $/NMRA/SRC/SEND
 * Rolled to revision 2.3
 * Added support for 1 pulse duty cycle test.
 * Added Reset test to Address and Bad bit tests.
 * Did some text cleanup.
 * 
 * *****************  Version 5  *****************
 * User: Kenw         Date: 7/26/96    Time: 9:04a
 * Updated in $/NMRA/SRC/SEND
 * Changes for version 2.2
 * Added support for 'o' oscilloscope setup  command.
 * Renamed 1/2 fast and 1/2 slow to command station max/min
 * 
 * *****************  Version 4  *****************
 * User: Kenw         Date: 5/12/96    Time: 1:15p
 * Updated in $/NMRA/SRC/SEND
 * Rolled to revision 2.1.
 * 
 * *****************  Version 3  *****************
 * User: Kenw         Date: 5/10/96    Time: 9:32a
 * Updated in $/NMRA/SRC/SEND
 * Rolled to revision 2.0.
 * Cleaned up formatting.
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 4/30/96    Time: 8:29p
 * Updated in $/NMRA/SRC/SEND
 * Rolled to version X 1.9
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 3:49p
 * Created in $/NMRA/SRC/SEND
 * Header file associated with SEND.CPP which contains main() for the
 * decoder
 * test program.
 *
 *****************************************************************************/

