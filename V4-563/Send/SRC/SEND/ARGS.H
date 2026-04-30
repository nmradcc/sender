/*****************************************************************************
 *
 * File:                 $Workfile: ARGS.H $
 * Path:                 $Logfile: /NMRA/SRC/SEND/ARGS.H $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 19 $
 * Last Modified on:     $Modtime: 8/05/23 4:49p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 8/05/23 5:50p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	args.h	-	Public defines for command line and ini file
 *				processing routines.
 *
 *****************************************************************************/

#ifndef ARGS_H_DECLARED
#define ARGS_H_DECLARED	"@(#) $Workfile: ARGS.H $$ $Revision: 19 $$"

#include "send.h"

#if SEND_VERSION < 4
 #include <dir.h>
#endif
#include <z_core.h>
#include <stdio.h>

#if SEND_VERSION >= 4
 #define MAXPATH					80
 #define MAXFILE					80
 #define MAXEXT						80

#endif

const int	MAXROOT				= MAXFILE + MAXEXT - 1;	// Max command size.
const u_int	MAX_EXTRA_PREAMBLE	= 8;					// Extra preamble size.

/*
 *	Args_obj object.
 */
class Args_obj : public Z_core
{
  public:
	/* Method section */
	Args_obj( void );

	~Args_obj();

	/* Simple get methods */
	const char	*get_key_help( void ) const
				{ return ( key_help ); }
	const char	*get_cmd_name( void ) const
				{ return ( cmd_name ); }
	const char	*get_ini_path( void ) const
				{ return ( ini_path ); }
	FILE		*get_ini_fp( void ) const
				{ return ( ini_fp ); }
	u_short		get_decoder_address( void ) const
				{ return ( decoder_address ); }
	u_short		get_port( void ) const
				{ return ( port ); }
	Dec_types  	get_decoder_type( void ) const
				{ return ( decoder_type ); }
	bool	  	get_crit_flag( void ) const
				{ return ( crit_flag ); }
	bool	   	get_fragment_flag( void ) const
				{ return ( fragment_flag ); }
	bool	   	get_rep_flag( void ) const
				{ return ( rep_flag ); }
	Bits_t	 	get_run_mask( void ) const
				{ return ( run_mask ); }
	Bits_t		get_clk_mask( void ) const
				{ return ( clk_mask ); }
	bool	   	get_manual_flag( void ) const
				{ return ( manual_flag ); }
	u_int	   	get_extra_preamble( void ) const
				{ return ( extra_preamble ); }
	bool	  	get_trig_rev( void ) const
				{ return ( trig_rev ); }
	bool	  	get_loco_first( void ) const
				{ return ( loco_first ); }
	u_int	   	get_fill_msec( void ) const
				{ return ( fill_msec ); }
	u_int	   	get_test_repeats( void ) const
				{ return ( test_repeats ); }
	bool	  	get_print_user( void ) const
				{ return ( print_user ); }
	bool	  	get_log_pkts( void ) const
				{ return ( log_pkts ); }
	bool	  	get_no_abort( void ) const
				{ return ( m_no_abort ); }
	bool	  	get_late_scope( void ) const
				{ return ( m_late_scope ); }
	bool	  	get_ambig_addr_same( void ) const
				{ return ( m_ambig_addr_same ); }
	BYTE	   	get_aspect_preset( void ) const
				{ return ( aspect_preset ); }
	BYTE	   	get_aspect_trigger( void ) const
				{ return ( aspect_trigger ); }
	bool	  	get_lamp_rear( void ) const
				{ return ( m_lamp_rear ); }
	BYTE	   	get_func_mask( void ) const
				{ return ( func_mask ); }
    BYTE        get_acc_pair( void ) const
                { return ( ( acc_pre + 1 ) / 2 ); }
    BYTE		get_acc_pre( void ) const
                { return ( acc_pre ); }
    BYTE		get_acc_trg( void ) const
                { return ( acc_pre - 1 ); }
    bool	  	get_kick_start( void ) const
				{ return ( m_kick_start ); }


	void		usage( FILE *ofp = stderr );
	Rslt_t		get_args(	int			argc,
							char		**argv );

  protected:
	/* Data section */
	char					cmd_name[MAXROOT];	// Command name.
	char					ini_path[MAXPATH];	// ini file path name.
	FILE					*ini_fp;			// ini FILE pointer.
	u_short	  				decoder_address;   	// Decoder address.
	bool					address_set;		// Address was set by user.
	u_short					port;			 	// Base I/O port.
	Dec_types				decoder_type;	  	// Decoder type.
	bool					crit_flag;		  	// Protect critical regions.
	bool					fragment_flag;	 	// Test all fragments.
	bool					rep_flag;		 	// Repeat decoder tests.
	Bits_t					run_mask;	 		// Tests to run.
	Bits_t					clk_mask;			// Clocks to try.
	bool					manual_flag;		// Run tests manually.
	u_int					extra_preamble;		// Extra margin test preambles.
	bool					trig_rev;			// Use reverse as trigger cmd.
	bool					loco_first;			// Put loco pkt + func pkt.
	u_int					fill_msec;			// Fill time (msec).
	u_int					test_repeats;		// Number of times to do tests.
	bool					print_user;			// If true, print user docs.
	bool					log_pkts;			// If true, send pkts to log.
	bool					m_no_abort;			// If true, no stop on error.
	bool					m_late_scope;		// If true, use late scope.
	bool					m_ambig_addr_same;	// If true, use same address.
    BYTE					aspect_preset;		// Sig decoder aspect preset.
    BYTE					aspect_trigger;		// Sig decoder aspect trigger.
    bool					m_lamp_rear;		// Use rear lamp.
    BYTE					func_mask;			// Function group bit mask.
    BYTE                    acc_pre;			// Acc. out preset (1,3,5,7);
    bool					m_kick_start;		// Briefly start loco.
	const char 				*key_help;			// Keyboard commands help msg.

	/* Method section */
	Rslt_t		argscan(	int			argc,
							char		**argv );
	Rslt_t		iniopen(	const char	*cmdpath,
							const char	*inifile,
							const char	*inienv );
	Rslt_t		iniscan(	void );
	Rslt_t		zcmd(		const char	*str );
	char		*get_str(	char		*istr,
							char		*&ostr_ptr );
};

/*
 *	Global Args_obj for sender args.
 */
extern Args_obj		Args;

#endif /* ARGS_H_DECLARED */


/*****************************************************************************
 * $History: ARGS.H $
 * 
 * *****************  Version 19  *****************
 * User: Kenw         Date: 8/05/23    Time: 5:50p
 * Updated in $/NMRA/SRC/SEND
 * Add LOCO+FIRST (-L) command to
 * put loco packet before func packet.
 * 
 * *****************  Version 18  *****************
 * User: Kenw         Date: 6/23/23    Time: 9:24p
 * Updated in $/NMRA/SRC/SEND
 * Added manual keyboard command list to -u usage information.
 * 
 * *****************  Version 17  *****************
 * User: Kenw         Date: 6/19/23    Time: 5:00p
 * Updated in $/NMRA/SRC/SEND
 * Added test the -E extra preamble bits not exceed 
 * MAX_EXTRA_PREAMBLE bits.
 * 
 * *****************  Version 16  *****************
 * User: Kenw         Date: 6/01/23    Time: 2:37p
 * Updated in $/NMRA/SRC/SEND
 * Added "KICKSTART" (-k) boolean switch.
 * 
 * *****************  Version 15  *****************
 * User: Kenw         Date: 3/20/20    Time: 8:55p
 * Updated in $/NMRA/SRC/SEND
 * Intermediate build to 5.10.1.
 * Flipped accessory out preset and trigger outputs..
 * Preset:  1, 3, 5, 7
 * Trigger: 0, 2, 4, 6
 * 
 * *****************  Version 14  *****************
 * User: Kenw         Date: 3/19/20    Time: 7:39p
 * Updated in $/NMRA/SRC/SEND
 * Intermediate build to 5.10.1.
 * "-o ACC_PAIR" added to argument list.
 *
 * *****************  Version 13  *****************
 * User: Kenw         Date: 7/24/19    Time: 7:18p
 * Updated in $/NMRA/SRC/SEND
 * Check in prior to B.5.9.3 release.
 * 
 * *****************  Version 12  *****************
 * User: Kenw         Date: 7/03/18    Time: 7:28p
 * Updated in $/NMRA/SRC/SEND
 * Intermin Check in X.5.8.3
 * 
 * *****************  Version 11  *****************
 * User: Kenw         Date: 4/21/17    Time: 7:06p
 * Updated in $/NMRA/SRC/SEND
 * Interim X.5.7.0 check in.
 * - Added signal decoder (s|S) to the decoder types.
 * - Added PRESET (-n) and TRIGGER (-N) aspect support.
 * 
 * *****************  Version 8  *****************
 * User: Kenw         Date: 12/06/04   Time: 7:15p
 * Updated in $/NMRA/SRC/SEND
 * First set of changes to support Ambiguous bit tests
 * 
 * *****************  Version 7  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:57p
 * Updated in $/NMRA/SRC/SEND
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 6  *****************
 * User: Kenw         Date: 7/11/98    Time: 8:36a
 * Updated in $/NMRA/SRC/SEND
 * Rolling to 2.13.
 * Added tests near maximum packet times.
 * Added ability to move scope trigger before or after trigger packet..
 * 
 * *****************  Version 5  *****************
 * User: Kenw         Date: 6/28/98    Time: 9:37a
 * Updated in $/NMRA/SRC/SEND
 * Changed to abort program at LOG_ERR. Rolling to X.2.12.
 * 
 * *****************  Version 4  *****************
 * User: Kenw         Date: 10/29/97   Time: 4:26p
 * Updated in $/NMRA/SRC/SEND
 * X.2.8 - Added packet log command.
 * 
 * *****************  Version 3  *****************
 * User: Kenw         Date: 10/28/97   Time: 2:38p
 * Updated in $/NMRA/SRC/SEND
 * Added Prior Packet Test & Idle manual command.
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 5/10/96    Time: 9:17a
 * Updated in $/NMRA/SRC/SEND
 * Added support for -E and -T switches and print_user_docs()
 * method.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 3:46p
 * Created in $/NMRA/SRC/SEND
 * Header file for 'class Args_obj' which scans ini files and command line
 * switches.
 *
 *****************************************************************************/

