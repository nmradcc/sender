/*****************************************************************************
 *
 * File:                 $Workfile: ARGS.CPP $
 * Path:                 $Logfile: /NMRA/SRC/SEND/ARGS.CPP $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 33 $
 * Last Modified on:     $Modtime: 10/03/23 2:30p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 1/08/24 4:23p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	args.cpp	-	This file contains the command line and cfg file argument
 *					processing routines.
 *
 *****************************************************************************/

#include <zlog.h>
#include <dcc.h>
 
#include "args.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

const char		SEPCHARU	  	= '/';					// UNIX character.
const char		SEPCHARM	  	= '\\';			   		// MSDOS character.
const u_short	DEF_LOCO_ADDR	= 3;  					// Default LOCO addr.
const u_short	DEF_FUNC_ADDR	= 3;  					// Default FUNC addr.
const u_short	DEF_ACC_ADDR  	= 1;  					// Default ACC addr.
const u_short	DEF_SIG_ADDR  	= 1;  					// Default SIG addr.
const int		MAX_CFG_LINE  	= 80; 					// Max cfg line.
const u_int		MAX_FILL_MSEC 	= 20000; 				// Max fill time.
const u_int		MIN_TEST_REPS	= 1;					// Min test repeats.
const u_int		MAX_TEST_REPS	= 1000;					// Max test repeats.
const u_int		MIN_MCU_BAUD	= 9600;					// Min MCU baud rate.
const u_int		MAX_MCU_BAUD	= 3000000;				// Max MCU baud rate.

const BYTE		DEF_FUNC_MASK	=	0x1f;				// Function Mask.

const Bits_t	DEF_RUN_MASK	=	~0L;				// Default run mask.

const Bits_t	DEF_CLK_MASK	=	~0L					// Default clock mask.
								  & ~0x00000008 		// No min + 2.
								  & ~0x00000010 		// No min + 1.
								  & ~0x00000100 		// No max - 2.
								  & ~0x00000200;		// No max - 1.


/*
 *	Global Args_obj object for sender args.
 */
Args_obj	Args;


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		Args_obj()						-	 Args_obj constructor.
 *
 *	RETURN VALUE
 *
 *		None.
 *
 *	DESCRIPTION
 *
 *		Args_obj() constructs a Args_obj object.
 */
/*--------------------------------------------------------------------------*/

Args_obj::Args_obj( void )
{
	cmd_name[0]  		= '\0';				// Command name.
	cfg_path[0]			= '\0';				// cfg file path name.
	cfg_fp				= NULL;				// cfg file pointer.
	decoder_address		= 0;				// Decoder address.
	address_set			= false;			// Address was set by user.
	decoder_type		= DEC_LOCO;	 		// Decoder type.
	fragment_flag  		= false;			// Send all fragment sizes.
	rep_flag			= false;	  		// Repeat decoder tests.
	run_mask			= DEF_RUN_MASK;		// Tests to run.
	clk_mask			= DEF_CLK_MASK;	  	// Clocks to try.
	manual_flag			= false;			// Run tests manually.
	extra_preamble		= 0;				// Extra margin test preambles.
	trig_rev			= false;			// Use E_STOP as trigger command.
	loco_first			= false;;			// Put loco pkt + func pkt.
	fill_msec			= MSEC_PER_SEC;		// Use 1 second fill time.
	test_repeats		= 2;				// Times to repeat misc. tests.
	print_user			= false;			// Don't print user docs.
	log_pkts			= false;			// Don't send packets to log.
	m_mcu_bypass_tst	= false;			// Run MCU self tests.
	m_no_abort			= false;			// Stop on error.
	m_late_scope		= false;			// Use normal scope trigger.
	m_ambig_addr_same  	= false;			// Use different address.
    aspect_preset		= 0;				// Sig decoder aspect preset.
    aspect_trigger		= 8;				// Sig decoder aspect trigger.
    m_lamp_rear			= false;			// Use forward lamp.
    func_mask			= DEF_FUNC_MASK;	// Default to all on.
    acc_pre				= 1;				// Default accessory out preset.
    m_kick_start		= false;			// Kick start motor.
	extra_trg			= 0;				// Extra Ames tst trig packets.
	strcpy( m_mcu_port, "COM3" );			// Default MCU COM port.
	m_mcu_baud			= 115200;			// Default MCU baud.
	key_help			=					// Keyboard commands help msg.
		"ESC - Return to command line       h - Print header\n"
		"  c - Send single clock phase      C - Send series of clock phases\n"
		"  u - Clear underflow              0 - Send zeros\n"
		"  1 - Send ones                    a - Send scope A pattern\n"
		"  b - Send scope B pattern         o - Send scope timing packet\n"
		"  w - Send warble packets          S - Send stretched 0 pattern\n"
		"  r - Send DCC reset packets       d - Send DCC packets\n"
		"  D - Send stretched DCC packets   s - Change loco speed, acc. output\n"
		"  e - Set speed to E-STOP          f - Change loco direction, acc. on/off\n"
		"  E - Set speed to E_STOP(I)       t - Run self tests repeatedly\n"
		"  k - Kickstart loco for funcs     i - Send DCC idle packets\n"
		"  R - Send hard resets             g - Test generic I/O\n"
		"  z - Run decoder tests            q - Quit program\n";
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		~Args_obj()						-	 Args_obj destructor.
 *
 *	RETURN VALUE
 *
 *		None.
 *
 *	DESCRIPTION
 *
 *		~Args_obj() destroys a Args_obj object.
 */
/*--------------------------------------------------------------------------*/

Args_obj::~Args_obj()
{
	if ( cfg_fp )
	{
		fclose( cfg_fp );
	}
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		usage()							-	 Print Usage message.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		Usage prints the usage message to ofp.
 */
/*--------------------------------------------------------------------------*/

void
Args_obj::usage(
	FILE		 *ofp )						// Output FILE stream.
{
	fprintf( ofp,
		"Usage: %8s [-?] [-u] [-m] [-a addr] [-d l|f|a|s] [-n pre] [-N trig]\n"
		"                [-l] [-f] [-r] [-t mask] [-c mask] [-E pre]\n",
		cmd_name );			
    fprintf( ofp,
		"                [-T] [-F fill] [-R reps] [-P] [-A] [-s] [-S] [-g mask]\n"
		"                [-o pair] [-k] [-D] [-e trg] [-J port]\n"
		"                [-Y baud]\n\n" );

	if ( cfg_path[0] != '\0' )
	{
		fprintf( ofp, "Cfg file: <%s>\n\n", cfg_path );
	}
	
	fprintf( ofp,
		"  -?                    Print usage message and exit\n",
		manual_flag		? "true" : "false" );
	fprintf( ofp,
		"  -u                    Print user information to 's_user.txt' and exit\n",
		manual_flag		? "true" : "false" );
	fprintf( ofp,
		"  -m         MANUAL     Start in manual mode                 <value %s>\n",
		manual_flag		? "true" : "false" );
	if ( address_set )
	{
		fprintf( ofp,
		"  -a <addr>  ADDRESS    Decoder address                      <value %u>\n",
		decoder_address );
	}
	else
	{
		fprintf( ofp,
		"  -a <addr>  ADDRESS    Decoder address                      <value ???>\n" );
	}
	fprintf( ofp,
		"  -d l|f|a|s TYPE       "
		"Dec. type(l-LOC,f-FUNC,a-ACC,s-SIG)  <value %c>\n",
		decoder_type );
	fprintf( ofp,
		"  -l         LAMP       "
		"Use rear lamp for function tests     <value %s>\n",
		m_lamp_rear			? "true" : "false" );
	fprintf( ofp,
		"  -n <pre>   PRESET     "
		"Signal decoder preset aspect         <value %u>\n",
		aspect_preset );
	fprintf( ofp,
		"  -N <trig>  TRIGGER    "
		"Signal decoder trigger aspect        <value %u>\n",
		aspect_trigger );
	fprintf( ofp,
		"  -f         FRAGMENT   "
		"Test all fragments                   <value %s>\n",
		fragment_flag		? "true" : "false" );
	fprintf( ofp,
		"  -r         REPEAT     "
		"Repeat decoder tests                 <value %s>\n",
		rep_flag		? "true" : "false" );
	fprintf( ofp,
		"  -t <mask>  TESTS      "
		"Bit mask of tests to run             <value 0x%08lx>\n",
		run_mask );
	fprintf( ofp,
		"  -c <mask>  CLOCKS     "
		"Bit mask of clocks to try            <value 0x%08lx>\n",
		clk_mask );
	fprintf( ofp,
		"  -g <mask>  FUNCS      "
		"Bit mask of active functions         <value 0x%02x>\n",
		func_mask );
	fprintf( ofp,
		"  -E <pre>   EXTRA_PRE  "
		"Extra margin test preamble bits      <value %u>\n",
		extra_preamble );
	fprintf( ofp,
		"  -T         TRIG_REV   "
		"Use loco reverse as trigger packet   <value %s>\n",
		trig_rev	 	? "true" : "false" );
	fprintf( ofp,
		"  -L         LOCO_FIRST "
		"Put loco packet before func packet   <value %s>\n",
		loco_first	 	? "true" : "false" );
	fprintf( ofp,
		"  -F <fill>  FILL_MSEC  "
		"Fill time in milliseconds            <value %u>\n",
		fill_msec );
	fprintf( ofp,
		"  -R <reps>  TEST_REPS  "
		"Non packet acceptance test repeats   <value %u>\n",
		test_repeats );
	fprintf( ofp,
		"  -P         LOG        "
		"Send packet info to log              <value %s>\n",
		log_pkts		? "true" : "false" );
	fprintf( ofp,
		"  -B    MCU_BYPASS_TST  "
		"Skip MCU startup self tests          <value %s>\n",
		m_mcu_bypass_tst	? "true" : "false" );
	fprintf( ofp,
		"  -A         NO_ABORT   "
		"Do not stop program on an error      <value %s>\n",
		m_no_abort		? "true" : "false" );
	fprintf( ofp,
		"  -s         LATE_SCOPE "
		"Put scope trigger after trigger      <value %s>\n",
		m_late_scope   	? "true" : "false" );
	fprintf( ofp,
		"  -S    SAME_AMBIG_ADDR "
		"Use same address for ambig tests     <value %s>\n",
		m_ambig_addr_same	? "true" : "false" );
	fprintf( ofp,
		"  -o        ACC_PAIR    "
		"Accessory output pair (1-4)          <value %u>\n"
        "                        "
        "    output {preset, trigger}         <value {%u, %u}>\n",
		get_acc_pair(), get_acc_pre(), get_acc_trg() );
	fprintf( ofp,
		"  -k        KICK_START  "
		"Kick start motor for function tests  <value %s>\n",
		m_kick_start   	? "true" : "false" );
	fprintf( ofp,
		"  -e <trg>   EXTRA_TRG  "
		"Extra Ames test trigger packets      <value %u>\n",
		extra_trg );
	fprintf( ofp,
		"  -D        DEBUG_ON    "
		"Log debug messages                   <value %s>\n",
		Deflog.get_dbg_on() ? "true" : "false" );
	fprintf( ofp,
		"  -J <port>  MCU_PORT   "
		"MCU serial port name                 <value %s>\n",
		m_mcu_port );
	fprintf( ofp,
		"  -Y <baud>  MCU_BAUD   "
		"MCU serial baud rate                 <value %u>\n",
		m_mcu_baud );
				
	fprintf( ofp, "\nManual keyboard commands >\n\n%s\n", key_help);
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		get_args()		  				-	 Get command line & cfg args.
 *
 *	RETURN VALUE
 *
 *		OK		-	Args_obj obtained without error.
 *		FAIL	-	Problem getting args.
 *
 *	DESCRIPTION
 *
 *		get_args() scans for cfg file and command line arguments.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Args_obj::get_args(
	int			argc,						// Count of arguments.
	char		**argv )					// Argument vector.
{
	Rslt_t		retval;						// Return value.

	if ( zcmd( argv[0] ) != OK )
	{
		return ( FAIL );
	}

	if ( cfgopen( argv[0], "SEND.CFG", "SEND_CFG" ) != OK )
	{
		cfgopen( argv[0], "SEND.INI", "SEND_INI" );
	}

	if ( cfg_fp != NULL )
	{
		retval	=	cfgscan();
		fclose( cfg_fp );
		cfg_fp	=	NULL;
		if ( retval != OK )
		{
			return ( FAIL );
		}
	}

	if ( argscan( argc, argv ) != OK )
	{
		return ( FAIL );
	}

	/*
	 *	Set default address if necessary.
	 */
	if ( !address_set )
	{
		if ( decoder_type == DEC_LOCO )
		{
			decoder_address	=	DEF_LOCO_ADDR;
		}
		else if ( decoder_type == DEC_FUNC )
		{
			decoder_address	=	DEF_FUNC_ADDR;
		}
		else if ( decoder_type == DEC_SIG )
		{
			decoder_address	=	DEF_SIG_ADDR;
		}
		else
		{
			decoder_address	=	DEF_ACC_ADDR;
		}

		address_set	=	true;
	}
	
	return ( OK );
}


/*---------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *			argscan()					-	Scan and process argument vector.
 *
 *	RETURN VALUE
 *
 *			OK		-	Arguments scanned without error.
 *			FAIL	-	Error processing command line arguments.
 *
 *	DESCRIPTION
 *
 *			argscan() scans the argument vector and sets the appropriate
 *			arguments.  It returns FAIL if any arguments are invalid.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Args_obj::argscan(
	int				argc,		  			// Count of arguments.
	char			**argv )	  			// Argument vector.
{
	int				i;			   			// Index variable.
	u_int			t_u_int;				// Temp u_int.
    BYTE			t_byte;					// Temp BYTE.

	for ( i = 1; i < argc; i++ )
	{
		if ( strcmp( argv[i], "-?" ) == 0 )			// Print usage message.
		{
			return ( FAIL );
		}
		else if ( strcmp( argv[i], "-u" ) == 0 )    // Print user docs.
		{
			print_user	=	true;
			return ( FAIL );
		}
		else if ( strcmp( argv[i], "-m" ) == 0 )	// Manual mode.
		{
			manual_flag	=	manual_flag ? false : true;
		}
		else if ( strcmp( argv[i], "-a" ) == 0 )	// Decoder address.
		{
			if ( ++i < argc )
			{
				decoder_address	=	(u_short)strtoul( argv[i], NULL, 0 );
				address_set		=	true;
			}
			else
			{
				fprintf( stderr, "ERROR: -a must have an address argument\n\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-d" ) == 0 )	// Decoder type.
		{
			if ( ++i < argc )
			{
				if (		strcmp( argv[i], "l" ) == 0 )
				{
					decoder_type	=	DEC_LOCO;
				}
				else if (	strcmp( argv[i], "L" ) == 0 )
				{
					decoder_type	=	DEC_LOCO;
				}
				else if (	strcmp( argv[i], "a" ) == 0 )
				{
					decoder_type	=	DEC_ACC;
				}
				else if (	strcmp( argv[i], "A" ) == 0 )
				{
					decoder_type	=	DEC_ACC;
				}
				else if (	strcmp( argv[i], "s" ) == 0 )
				{
					decoder_type	=	DEC_SIG;
				}
				else if (	strcmp( argv[i], "S" ) == 0 )
				{
					decoder_type	=	DEC_SIG;
				}
				else if (	strcmp( argv[i], "f" ) == 0 )
				{
					decoder_type	=	DEC_FUNC;
				}
				else if (	strcmp( argv[i], "F" ) == 0 )
				{
					decoder_type	=	DEC_FUNC;
				}
				else
				{
					fprintf( stderr,
						"ERROR: Invalid -d argument <%s>\n\n", argv[i] );
					return ( FAIL );
				}
			}
			else
			{
				fprintf( stderr, "ERROR: -d must have a decoder type argument\n\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-l" ) == 0 )	// Use rear lamp.
		{
			m_lamp_rear	=	m_lamp_rear ? false : true;
		}
		else if ( strcmp( argv[i], "-n" ) == 0 )	// Preset aspect.
		{
			if ( ++i < argc )
			{
				t_u_int		=	(u_int)strtoul( argv[i], NULL, 0 );
				if ( t_u_int > ASPECT_MAX )
				{
					fprintf( stderr, "ERROR: -n %u too large, cannot exceed %u\n\n",
						t_u_int, ASPECT_MAX );
					return ( FAIL );
				}
				else
				{
					aspect_preset	=	t_u_int;
				}
			}
			else
			{
				fprintf( stderr,
					"ERROR: -n must have a preset aspect argument\n\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-N" ) == 0 )	// Trigger aspect.
		{
			if ( ++i < argc )
			{
				t_u_int		=	(u_int)strtoul( argv[i], NULL, 0 );
				if ( t_u_int > ASPECT_MAX )
				{
					fprintf( stderr, "ERROR: -N %u too large, cannot exceed %u\n\n",
						t_u_int, ASPECT_MAX );
					return ( FAIL );
				}
				else
				{
					aspect_trigger	=	t_u_int;
				}
			}
			else
			{
				fprintf( stderr,
					"ERROR: -N must have a trigger aspect argument\n\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-t" ) == 0 )	// Tests to run mask.
		{
			if ( ++i < argc )
			{
				run_mask	=	strtoul( argv[i], NULL, 0 );
			}
			else
			{
				fprintf( stderr, "ERROR: -t must have a test mask argument\n\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-c" ) == 0 )	// Clocks to try mask.
		{
			if ( ++i < argc )
			{
				clk_mask	=	strtoul( argv[i], NULL, 0 );
			}
			else
			{
				fprintf( stderr, "ERROR: -c must have a clock mask argument\n\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-g" ) == 0 )	// Function mask.
		{
			if ( ++i < argc )
			{
				func_mask	=	(BYTE)(strtoul( argv[i], NULL, 0 )
                               & DEF_FUNC_MASK);
				if ( func_mask == 0 )
				{
					fprintf( stderr,
						"ERROR: -g function mask must not be 0\n\n" );
					return ( FAIL );
				}
			}
			else
			{
				fprintf( stderr, "ERROR: -g must have a function mask argument\n\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-f" ) == 0 )	// Send all fragment sizes.
		{
			fragment_flag	=	fragment_flag ? false : true;
		}
		else if ( strcmp( argv[i], "-r" ) == 0 )	// Repeat tests flag.
		{
			rep_flag	=	rep_flag ? false  : true;
		}
		else if ( strcmp( argv[i], "-E" ) == 0 )	// Extra preamble bits.
		{
			if ( ++i < argc )
			{
				extra_preamble	=	(u_int)strtoul( argv[i], NULL, 0 );
				if ( extra_preamble > MAX_EXTRA_PREAMBLE )
				{
					fprintf( stderr, "ERROR: Extra preamble size of %u must be <= %u\n\n",
							 extra_preamble, MAX_EXTRA_PREAMBLE );
					return ( FAIL );
				}	
			}
			else
			{
				fprintf( stderr, "ERROR: -E must have a preamble number argument\n\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-T" ) == 0 )	// Trigger cmd is reverse.
		{
			trig_rev	=	trig_rev ? false  : true;
		}
		else if ( strcmp( argv[i], "-L" ) == 0 )	// Put loco pkt + func pkt.
		{
			loco_first	=	loco_first ? false  : true;
		}
		else if ( strcmp( argv[i], "-F" ) == 0 )	// Fill time in msec.
		{
			if ( ++i < argc )
			{
				t_u_int		=	(u_int)strtoul( argv[i], NULL, 0 );
				if ( t_u_int > MAX_FILL_MSEC )
				{
					fprintf( stderr, "ERROR: -F %u too big, cannot exceed %u\n\n",
						t_u_int, MAX_FILL_MSEC );
					return ( FAIL );
				}
				else
				{
					fill_msec	=	t_u_int;
				}
			}
			else
			{
				fprintf( stderr,
					"ERROR: -F must have a fill time (msec.) argument\n\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-R" ) == 0 )	// Test repeat count.
		{
			if ( ++i < argc )
			{
				t_u_int		=	(u_int)strtoul( argv[i], NULL, 0 );
				if ( t_u_int < MIN_TEST_REPS )
				{
					fprintf( stderr, "ERROR: -R %u too small, must be at least %u\n\n",
						t_u_int, MIN_TEST_REPS );
					return ( FAIL );
				}
				if ( t_u_int > MAX_TEST_REPS )
				{
					fprintf( stderr, "ERROR: -R %u too big, cannot exceed %u\n\n",
						t_u_int, MAX_TEST_REPS );
					return ( FAIL );
				}
				else
				{
					test_repeats	=	t_u_int;
				}
			}
			else
			{
				fprintf( stderr,
					"ERROR: -R must have a repeat count argument\n\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-P" ) == 0 )	// Log packets.
		{
			log_pkts	=	log_pkts ? false  : true;
		}
		else if ( strcmp( argv[i], "-B" ) == 0 )	// Bypass MCU self tests.
		{
			m_mcu_bypass_tst	= 	m_mcu_bypass_tst ? false  : true;
		}
		else if ( strcmp( argv[i], "-A" ) == 0 )	// No abort flag.
		{
			m_no_abort	=	m_no_abort ? false  : true;
		}
		else if ( strcmp( argv[i], "-s" ) == 0 )	// Late scope flag.
		{
			m_late_scope	=	m_late_scope ? false  : true;
		}
		else if ( strcmp( argv[i], "-S" ) == 0 )	// Same ambig addr flag.
		{
			m_ambig_addr_same	=	m_ambig_addr_same ? false  : true;
		}
		else if ( strcmp( argv[i], "-o" ) == 0 )	// Accessory out pair.
		{
			if ( ++i < argc )
			{
				t_byte	=	(BYTE)strtoul( argv[i], NULL, 0 );
                if ( t_byte < 1 || t_byte > 4 ) {
                    	fprintf( stderr,
                        "ERROR: \"-o %s\" invalid: Accessory output pair value must be between 1 and 4.\n\n",
                        	argv[i] );
                        return ( FAIL );
                }
                acc_pre = ( t_byte * 2 ) - 1;
			}
			else
			{
				fprintf( stderr, "ERROR: -o must have an accessory output pair argument\n\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-k" ) == 0 )	// Kick start motor flag.
		{
			m_kick_start	=	m_kick_start ? false  : true;
		}
		else if ( strcmp( argv[i], "-D" ) == 0 )	// Log dbg msgs flag.
		{
			Deflog.set_dbg_on( !Deflog.get_dbg_on() );
		}
		else if ( strcmp( argv[i], "-e" ) == 0 )	// Extra trigger packets.
		{
			if ( ++i < argc )
			{
				extra_trg	=	(u_int)strtoul( argv[i], NULL, 0 );
				if ( extra_trg > MAX_EXTRA_TRG )
				{
					fprintf( stderr, "ERROR: Extra trigger packet size of %u must be <= %u\n\n",
							 extra_trg, MAX_EXTRA_TRG );
					return ( FAIL );
				}	
			}
			else
			{
				fprintf( stderr, "ERROR: -e must have an extra trigger packets argument\n\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-J" ) == 0 )	// MCU serial port.
		{
			if ( ++i < argc )
			{
				if ( strlen( argv[i] ) >= MAX_MCU_PORT )
				{
					fprintf( stderr,
						"ERROR: -J port argument too long (max %d chars)\n\n",
						MAX_MCU_PORT - 1 );
					return ( FAIL );
				}
				strcpy( m_mcu_port, argv[i] );
			}
			else
			{
				fprintf( stderr, "ERROR: -J must have a serial port argument\n\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-Y" ) == 0 )	// MCU serial baud.
		{
			if ( ++i < argc )
			{
				t_u_int = (u_int)strtoul( argv[i], NULL, 0 );
				if ( t_u_int < MIN_MCU_BAUD || t_u_int > MAX_MCU_BAUD )
				{
					fprintf( stderr,
						"ERROR: -Y %u invalid, must be between %u and %u\n\n",
						t_u_int, MIN_MCU_BAUD, MAX_MCU_BAUD );
					return ( FAIL );
				}
				m_mcu_baud = t_u_int;
			}
			else
			{
				fprintf( stderr, "ERROR: -Y must have a baud rate argument\n\n" );
				return ( FAIL );
			}
		}
		else
		{
			fprintf( stderr,
				"ERROR: Invalid command line argument <%s>\n\n", argv[1] );
			return ( FAIL );
		}
	}

	return ( OK );
}


/*---------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *			cfgopen()					-	Open the config file.
 *
 *	RETURN VALUE
 *
 *
 *
 *			OK		-	config file opened.
 *			FAIL	-	config file not present.
 *
  *	DESCRIPTION
 *
 *			cfgopen() attempts to open the config file in the usual places.
 *			It returns NULL if no config file is found.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Args_obj::cfgopen(
	const char	*cmdpath,					// Command path and name.
	const char	*inifile,					// cfg file name.
	const char	*inienv )					// Cfg path environment variable.
{
	char		*sptr;						// Temporary char pointer.
	char		fdrive[MAXDRIVE];			// Drive name.
	char		fdir[MAXDIR];				// Directory name.
	char		fname[MAXFILE];			// File name.
	char		fext[MAXEXT];				// File extension.

	/*
	 *	First, try the local directory for the cfg file.
	 */
		if ( (cfg_fp = fopen( inifile, "r" )) != NULL  )
		{
			_getcwd( cfg_path, MAXPATH );
			strcat( cfg_path, "\\" );
			strcat( cfg_path, inifile );
			return ( OK );	   				// Opened the cfg file locally.
		}

	/*
	 *	Next, check to see if the cfg file path is set in the
	 *	environment variable.
	 */
	if ( (sptr = getenv( inienv )) != NULL )
	{
		if ( (cfg_fp = fopen( sptr, "r" )) != NULL )
		{
			strcpy( cfg_path, sptr );
			return ( OK );					// Opened the inienv file.
		}
	}

	/*
	 *	Finally, check for the file in the send directory.
	 */
	fdrive[0]	=	'\0';
	fdir[0]		=	'\0';
	fname[0]	=	'\0';
	fext[0]		=	'\0';
	_splitpath( cmdpath, fdrive, fdir, fname, fext );
	sprintf( cfg_path, "%s%s%s", fdrive, fdir, inifile );
	if ( (cfg_fp = fopen( cfg_path, "r" )) != NULL  )
	{
		return ( OK );	   					// Opened the cfg file locally.
	}

	cfg_path[0]	=	'\0';
	return ( FAIL );
}


/*---------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *			cfgscan()					-	Scan and process cfg file.
 *
 *	RETURN VALUE
 *
 *
 *
 *			OK		-	cfg file parsed without error.
 *			FAIL	-	Error processing cfg file.
 *
  *	DESCRIPTION
 *
 *			cfgscan() scans the cfg file and sets the appropriate
 *			arguments.  It returns FAIL if the file is invalid.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Args_obj::cfgscan( void )
{
	char		*itmp;						// Temporary input pointer.
	char		*cmd;						// Command string.
	char		*tptr;						// Temp pointer.
	size_t		line_len;				// Current cfg line length.
	int			targc;						// Argument count.
	char		*targv[3];					// Argument vector.
	char		buf[MAX_CFG_LINE];			// Cfg line buffer.

	while( fgets( buf, MAX_CFG_LINE, cfg_fp ) )
	{
		itmp	=	buf;
		line_len = strlen( buf );

		if ( line_len > 0 && *( tptr = buf + ( line_len - 1 ) ) == '\n' )
		{
			*tptr	=	'\0';
		}

		if ( buf[0] == '\0' )
		{
			continue;
		}

		if ( (itmp = get_str( itmp, cmd )) == NULL )
		{
			fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
			return ( FAIL );
		}

		if (		cmd[0] == '#' ||cmd[0] == ';' )
		{
			continue;						// Skip comment lines.
		}
		else if (	!strcmpi( cmd,	"MANUAL" ) )
		{
			targv[1]	=	"-m";
			targc		=	2;
		}
		else if (	!strcmpi( cmd,	"ADDRESS" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-a";
			targc		=	3;
		}
		else if ( 	!strcmpi( cmd,	"TYPE" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-d";
			targc		=	3;
		}
		else if (	!strcmpi( cmd,	"LAMP" ) )
		{
			targv[1]	=	"-l";
			targc		=	2;
		}
		else if (	!strcmpi( cmd,	"PRESET" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-n";
			targc		=	3;
		}
		else if (	!strcmpi( cmd,	"TRIGGER" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-N";
			targc		=	3;
		}
		else if (	!strcmpi( cmd,	"TESTS" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-t";
			targc		=	3;
		}
		else if (	!strcmpi( cmd,	"CLOCKS" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-c";
			targc		=	3;
		}
		else if (	!strcmpi( cmd,	"FUNCS" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-g";
			targc		=	3;
		}
		else if ( 	!strcmpi( cmd,	"FRAGMENT" ) )
		{
			targv[1]	=	"-f";
			targc		=	2;
		}
		else if ( 	!strcmpi( cmd,	"REPEAT" ) )
		{
			targv[1]	=	"-r";
			targc		=	2;
		}
		else if (	!strcmpi( cmd,	"EXTRA_PRE" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-E";
			targc		=	3;
		}
		else if ( 	!strcmpi( cmd,	"TRIG_REV" ) )
		{
			targv[1]	=	"-T";
			targc		=	2;
		}
		else if ( 	!strcmpi( cmd,	"lOCO_FIRST" ) )
		{
			targv[1]	=	"-L";
			targc		=	2;
		}
		else if (	!strcmpi( cmd,	"FILL_MSEC" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-F";
			targc		=	3;
		}
		else if (	!strcmpi( cmd,	"TEST_REPS" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-R";
			targc		=	3;
		}
		else if ( 	!strcmpi( cmd,	"LOG" ) )
		{
			targv[1]	=	"-P";
			targc		=	2;
		}
		else if ( 	!strcmpi( cmd, 	"MCU_BYPASS_TST" ) )
		{
			targv[1]	= 	"-B";
			targc		= 	2;
		}
		else if ( 	!strcmpi( cmd,	"NO_ABORT" ) )
		{
			targv[1]	=	"-A";
			targc		=	2;
		}
		else if ( 	!strcmpi( cmd,	"LATE_SCOPE" ) )
		{
			targv[1]	=	"-s";
			targc		=	2;
		}
		else if ( 	!strcmpi( cmd,	"SAME_AMBIG_ADDR" ) )
		{
			targv[1]	=	"-S";
			targc		=	2;
		}
		else if (	!strcmpi( cmd,	"ACC_PAIR" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-o";
			targc		=	3;
		}
		else if ( 	!strcmpi( cmd,	"KICK_START" ) )
		{
			targv[1]	=	"-k";
			targc		=	2;
		}
		else if ( 	!strcmpi( cmd,	"DEBUG_ON" ) )
		{
			targv[1]	=	"-D";
			targc		=	2;
		}
		else if (	!strcmpi( cmd,	"EXTRA_TRG" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-e";
			targc		=	3;
		}
		else if (	!strcmpi( cmd,	"MCU_PORT" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-J";
			targc		=	3;
		}
		else if (	!strcmpi( cmd,	"MCU_BAUD" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-Y";
			targc		=	3;
		}
		else
		{
		 	fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
			return ( FAIL );
		}

		if ( argscan( targc, targv ) == FAIL )
		{
			fprintf( stderr, "ERROR parsing cfg file line <%s>\n\n", buf );
			return ( FAIL );
		}
	}

	return ( OK );
}


/*---------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *			zcmd()							-	Find the root command name.
 *
 *	RETURN VALUE
 *
 * 			cmd		-	A pointer to the command root is 'str' contains a
 *						command, or 'cmdin' if no command is found.
 *	DESCRIPTION
 *
 *			zcmd() copies the root portion of the command path into
 *			Cmd_name.  If no command exists, it copies cmdin to Cmd_name.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Args_obj::zcmd(
	const char			*str )				// Ptr to Cmd (usually argv[0]).
{
	register const char	*sptr;				/* Pointer to input 'str' */
	register int		j;					/* Index into 'cmd_name[]' array */

	if (str == NULL || *str == '\0')		/* No command passed */
	{
		fprintf ( stderr, "ERROR zcmd: Invalid command name passed\n\n" );
		return ( FAIL );
	}
	else									/* Command found */
	{
		for (sptr = str; *str != '\0'; str++)
		{
			if (*str == SEPCHARU || *str == SEPCHARM)
			{
				sptr = str + 1;
			}
		}
	}

	for (j=0; *sptr != '\0' && j<(MAXROOT-1) && *sptr != '.'; j++,sptr++)
	{
		cmd_name[j] = tolower(*sptr);
	}
	cmd_name[j] = '\0';

	return ( OK );
}


/*---------------------------------------------------------------------------*/
/*
 *	NAME
 *			get_str()				-	Get a quoted or unquoted string.
 *
 *	RETURN VALUE
 *
 *			ptr				-	Pointer to next input character.
 *			NULL			-	Error parsing string.
 *
 *	DESCRIPTION
 *
 *			get_str() scans istr for a quoted or unquoted string.
 *			If it finds one, it sets ostrptr to point to its beginning,
 *			Replaces the trailing delimiter with '\0',
 *			and returns a pointer to the next character to consume,
 *			On error, get_str() sets ostr_ptr to NULL and returns
 *			NULL.
 */
/*--------------------------------------------------------------------------*/

char *
Args_obj::get_str(
	char	   	*istr,						// Input string.
	char	   	*&ostr_ptr )				// Pointer to output.
{
	char		*itmp;						// Temporary input pointer
	char		*otmp;						// Temporary output pointer

	if ( !istr || !*istr )		 			// Error NULL pointer
	{
		return ( NULL );
	}

	itmp = istr;

	// First skip the white space
	for( ; isspace( *itmp ); itmp++ )
	{
		;
	}

	if ( *itmp == '\0' )					// Error no characters
	{
		return ( NULL );
	}

	// Is it quoted?
	if ( *itmp == '"' )						// String is quoted
	{
		++itmp;
		for ( otmp = itmp; *itmp != '\0'; ++itmp )
		{
			if ( *itmp == '"' )
			{
				break;
			}
		}
		if ( *itmp != '"' )					// Error didn't end with quote
		{
			return ( NULL );
		}
		else
		{
			*itmp	=	'\0';				// Terminate the input string.
			++itmp;							// Skip the ending '"'
		}
	}
	else									// String is not quoted
	{
		for ( otmp = itmp; *itmp != '\0'; ++itmp )
		{
			if ( isspace( *itmp ) )
			{
				*itmp	=	'\0';		 	// Terminate the input string.
				++itmp;					 	// Move to next character.
				break;
			}
		}
	}

	ostr_ptr	=	otmp;
	return ( itmp );
}
