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
 * Current version:      $Revision: 20 $
 * Last Modified on:     $Modtime: 10/03/23 2:08p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 1/08/24 4:23p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	args.h	-	Public defines for command line and cfg file
 *				processing routines.
 *
 *****************************************************************************/

#ifndef ARGS_H_DECLARED
#define ARGS_H_DECLARED	"@(#) $Workfile: ARGS.H $$ $Revision: 20 $$"

#include "send.h"

#if defined(_MSC_VER)
#include <direct.h>
#include <stdlib.h>
#ifndef MAXPATH
#define MAXPATH _MAX_PATH
#endif
#ifndef MAXDRIVE
#define MAXDRIVE _MAX_DRIVE
#endif
#ifndef MAXDIR
#define MAXDIR _MAX_DIR
#endif
#ifndef MAXFILE
#define MAXFILE _MAX_FNAME
#endif
#ifndef MAXEXT
#define MAXEXT _MAX_EXT
#endif
#else
#include <dir.h>
#endif
#include <z_core.h>
#include <stdio.h>

const int	MAXROOT				= MAXFILE + MAXEXT - 1;	// Max command size.
const int	MAX_MCU_PORT		= 64;					// Max MCU port name size.
const u_int	MAX_EXTRA_PREAMBLE	=  8;					// Max extra preamble bits.
const u_int MAX_EXTRA_TRG       = 50;                   // Max extra trig pkts.

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
	const char	*get_cfg_path( void ) const
				{ return ( cfg_path ); }
	FILE		*get_cfg_fp( void ) const
				{ return ( cfg_fp ); }
	u_short		get_decoder_address( void ) const
				{ return ( decoder_address ); }
	Dec_types  	get_decoder_type( void ) const
				{ return ( decoder_type ); }
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
	bool	  		get_mcu_bypass_tst( void ) const
				{ return ( m_mcu_bypass_tst ); }
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
	u_int	   	get_extra_trg( void ) const
				{ return ( extra_trg ); }
	const char	*get_mcu_port( void ) const
				{ return ( m_mcu_port ); }
	u_int		get_mcu_baud( void ) const
				{ return ( m_mcu_baud ); }


	void		usage( FILE *ofp = stderr );
	Rslt_t		get_args(	int			argc,
							char		**argv );

  protected:
	/* Data section */
	char					cmd_name[MAXROOT];	// Command name.
	char					cfg_path[MAXPATH];	// cfg file path name.
	FILE					*cfg_fp;			// cfg file pointer.
	u_short	  				decoder_address;   	// Decoder address.
	bool					address_set;		// Address was set by user.
	Dec_types				decoder_type;	  	// Decoder type.
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
	bool				m_mcu_bypass_tst;	// If true, skip MCU self tests.
	bool					m_no_abort;			// If true, no stop on error.
	bool					m_late_scope;		// If true, use late scope.
	bool					m_ambig_addr_same;	// If true, use same address.
    BYTE					aspect_preset;		// Sig decoder aspect preset.
    BYTE					aspect_trigger;		// Sig decoder aspect trigger.
    bool					m_lamp_rear;		// Use rear lamp.
    BYTE					func_mask;			// Function group bit mask.
    BYTE                    acc_pre;			// Acc. out preset (1,3,5,7);
    bool					m_kick_start;		// Briefly start loco.
	u_int					extra_trg;		    // Extra Ames tst trig packets.	
	char					m_mcu_port[MAX_MCU_PORT]; // MCU COM port name.
	u_int					m_mcu_baud;		// MCU serial baud rate.
	const char 				*key_help;			// Keyboard commands help msg.

	/* Method section */
	Rslt_t		argscan(	int			argc,
							char		**argv );
	Rslt_t		cfgopen(	const char	*cmdpath,
							const char	*inifile,
							const char	*inienv );
	Rslt_t		cfgscan(	void );
	Rslt_t		zcmd(		const char	*str );
	char		*get_str(	char		*istr,
							char		*&ostr_ptr );
};

/*
 *	Global Args_obj for sender args.
 */
extern Args_obj		Args;

#endif /* ARGS_H_DECLARED */
