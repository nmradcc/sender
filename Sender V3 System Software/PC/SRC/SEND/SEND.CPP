/*****************************************************************************
 *
 * File:                 $Workfile: SEND.CPP $
 * Path:                 $Logfile: /NMRA/SRC/SEND/SEND.CPP $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 39 $
 * Last Modified on:     $Modtime: 8/29/23 6:51p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 9/12/23 8:04p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	send.cpp	-	This file is the main() function for the sender test
 *					program.
 *
 *****************************************************************************/

#include "args.h"							// Includes send.h
#include "send_reg.h"
#include "dec_tst.h"
#include "SENDER_HW_MCU_USB.H"

#include <cksum.h>
#include <zlog.h>

#include <windows.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

#define CMD_MSG		( printf( "Command mode (h for help, q to quit) >>" ) )

const char * const	DOCS_FILE	= "s_user.txt";	// User docs file.
const int			MAXLINE		= 256;			// Maximum input line.

/*
 *	Initial boilerplate questions.
 */
struct Snd_question
{
	const char	*question;
	const char	*log_str;
};

Snd_question	Questions[] =
{
	{	"Enter Manufacturer > ",  		"Manufacturer: %s\n"	},
	{	"Enter Model number > ",  		"Model number: %s\n"	},
	{	"Enter Version number > ",		"Version number: %s\n"	},
    {	"Enter Modified CVs > ",		"Modified CVs: %s\n"	},
    {	"Enter Booster type > ",		"Booster type: %s\n"	},
    {	"Enter Track Direction > ",		"Track Direction: %s\n"	},
};

const u_int	QUESTIONS_SIZE		=	sizeof( Questions ) /
									sizeof( Snd_question );

/*
 *	1 bit clock times used for scope test.
 */
static u_short Scope_time_tbl[] =
{
	100,										// 100 usec. clock period.
	10000,										// 10000 usec. clock period.
	1000,										// 1000 usec. clock period.
	10											// 10 usec clock period.
};

const u_int SCOPE_TIME_TBL_SIZE = sizeof( Scope_time_tbl ) / sizeof( u_short );

/*
 *	key_cmd() return values.
 */
enum Key_val
{
	KEY_NULL		= '\0',						// NULL key command.
	KEY_ESC			= '\033',					// Break test loop.
	KEY_HEADER		= 'h',						// Print header.
	KEY_CLOCK		= 'c',						// Toggle clock.
	KEY_MCLK		= 'C', 						// Toggle many clocks.
	KEY_UNDER		= 'u',						// Clear underflow.
	KEY_ZERO		= '0',						// Send 0 stream.
	KEY_ONE			= '1',						// Send 1 stream.
	KEY_SCOPE_A		= 'a',						// Scope test A.
	KEY_SCOPE_B		= 'b',						// Scope test B.
	KEY_SCOPE_O		= 'o',						// Scope timing test.
	KEY_WARBLE	 	= 'w',						// Send warble packets.
	KEY_STRETCHED	= 'S',						// Send stretched 0s.
	KEY_RESET		= 'r',						// Send resets.
	KEY_HARD		= 'R',						// Send hard resets.
	KEY_IDLE		= 'i',						// Send idles.
	KEY_DCC			= 'd',						// Send Dcc packets.
	KEY_DCC_S		= 'D',						// Send Stretched Dcc packets.
	KEY_SPEED		= 's',					 	// Change speed.
	KEY_E_STOP		= 'e',						// Emergency stop.
	KEY_E_STOP_I	= 'E',						// Emergency stop I.
	KEY_FW			= 'f',						// Toggle forward bit.
	KEY_SELF_TST	= 't',						// Run self tests.
	KEY_DEC_TST		= 'z',						// Perform decoder tests.
    KEY_GEN			= 'g',						// Send generic out read in.
	KEY_KICK		= 'k',						// Kickstart loco for funcs.
	KEY_QUIT		= 'q'						// Quit program.
};

/*
 *	Help message.
 */
const char *Key_help	=
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

static Key_val	get_key_cmd();
static void		parse_key_cmd( Key_val init_key );
static Rslt_t	do_mcu_self_tests( void );
static void		get_log_file( const char *cmd_name );
static void		print_user_docs();
static void		exit_send( int status );
static bool		init_hw_backend();
static void		close_hw_backend();
static bool		hw_is_running();
static Rslt_t	hw_set_clk( u_short clk0t, u_short clk0h, u_short clk1t );
static Rslt_t	hw_start_clk();
static Rslt_t	hw_stop_clk();
static void		hw_clr_under();
static void		hw_set_scope( bool on );
static Rslt_t	hw_send_bytes( u_int count, BYTE val, const char *info );
static Rslt_t	hw_send_stretched_byte(
								u_short clk0t,
								u_short clk0h,
								BYTE val,
								const char *info );
static Rslt_t	hw_send_special( BYTE special_id );
static Rslt_t	hw_send_bits_pkt( Bits &bits, const char *info );

/*
 *	Global copy of Send board registers.
 */
Send_reg 			Dcc_reg;

/*
 *	Decoder test object.
 */
static Dec_tst		Ldec_tst;

static Sender_hw_mcu_usb *Lmcu_hw = NULL;
static Sender_hw_if		 *Lhw_if = NULL;
static bool				  Lhw_running = false;

/*
 *	Scratchpad bits.
 */
static Bits	Dcc_bits( 256 );					// Packet scratchpad.

/*
 *	Possible types of packets to send in the main command
 *	loop.
 */
enum Send_type
{
	SEND_NONE,									// Send nothing.
	SEND_MCLK,									// Send SW clocks.
	SEND_ZERO,									// Send 0s.
	SEND_ONE,									// Send 1s.
	SEND_SCOPE_A,								// Send scope test A.
	SEND_SCOPE_B,								// Send scope test B.
	SEND_SCOPE_O,								// Send scope timing test.
	SEND_WARBLE,								// Send warble packet.
	SEND_STRETCHED,								// Send stretched 0s.
	SEND_RESET,									// Send resets.
	SEND_HARD,									// Send hard resets.
	SEND_IDLE,									// Send idles.
	SEND_DCC,									// Send DCC.
	SEND_DCC_S,									// Send stretched DCC.
	SEND_SELF_TST,								// Send self tests.
	SEND_DEC_TST						   		// Send decoder tests.
};

/*
 *	Possible ramp test states.
 */
enum Ramp_state
{
	RAMP_INIT,									// Initialize the test.
	RAMP_UP,									// Ramp up.
	RAMP_DOWN_FW,								// Ramp forward down.
	RAMP_DOWN_REV,								// Ramp reverse down.
	RAMP_E_STOP,								// Emergency stop.
	RAMP_END									// End of test.
};

static bool
init_hw_backend( void )
{
	Lmcu_hw = new Sender_hw_mcu_usb( Args.get_mcu_port(), Args.get_mcu_baud() );
	if ( Lmcu_hw == NULL )
	{
		return ( false );
	}

	if ( !Lmcu_hw->init() )
	{
		delete Lmcu_hw;
		Lmcu_hw	= NULL;
		return ( false );
	}

	Lhw_if = Lmcu_hw;
	Lhw_running = false;

	printf( "Using MCU backend on port <%s> at %u baud\n",
		Args.get_mcu_port(), Args.get_mcu_baud() );
	return ( true );
}

static void
close_hw_backend( void )
{
	if ( Lhw_if != NULL )
	{
		Lhw_if->close();
	}

	if ( Lmcu_hw != NULL )
	{
		delete Lmcu_hw;
		Lmcu_hw = NULL;
	}

	Lhw_if = NULL;
	Lhw_running = false;
}

static bool
hw_is_running( void )
{
	if ( Lhw_if == NULL )
	{
		return ( false );
	}

	Sender_hw_status status;
	if ( Lhw_if->get_status( status ) )
	{
		Lhw_running = status.running;
	}

	return ( Lhw_running );
}

static Rslt_t
hw_set_clk(
	u_short		clk0t,
	u_short		clk0h,
	u_short		clk1t )
{
	if ( Lhw_if == NULL )
	{
		return ( FAIL );
	}
	return ( Lhw_if->set_timing( clk0t, clk0h, clk1t ) ? OK : FAIL );
}

static Rslt_t
hw_start_clk( void )
{
	if ( Lhw_if == NULL )
	{
		return ( FAIL );
	}

	if ( Lhw_if->start_clock() )
	{
		Lhw_running = true;
		return ( OK );
	}

	return ( FAIL );
}

static Rslt_t
hw_stop_clk( void )
{
	if ( Lhw_if == NULL )
	{
		return ( FAIL );
	}

	if ( Lhw_if->stop_clock() )
	{
		Lhw_running = false;
		return ( OK );
	}

	return ( FAIL );
}

static void
hw_clr_under( void )
{
	if ( Lhw_if != NULL )
	{
		Lhw_if->clear_underflow();
	}
}

static void
hw_set_scope(
	bool		on )
{
	if ( Lhw_if != NULL )
	{
		Lhw_if->set_scope( on );
	}
}

static Rslt_t
hw_send_bytes(
	u_int		count,
	BYTE		val,
	const char	*info )
{
	if ( Lhw_if == NULL )
	{
		return ( FAIL );
	}
	if ( count > 0xFFFF )
	{
		return ( FAIL );
	}

	return ( Lhw_if->send_bytes( (u_short)count, val, info ) ? OK : FAIL );
}

static Rslt_t
hw_send_stretched_byte(
	u_short		clk0t,
	u_short		clk0h,
	BYTE		val,
	const char	*info )
{
	if ( Lhw_if == NULL )
	{
		return ( FAIL );
	}
	return ( Lhw_if->send_stretched_byte( clk0t, clk0h, val, info ) ? OK : FAIL );
}

static Rslt_t
hw_send_special(
	BYTE		special_id )
{
	if ( Lhw_if == NULL )
	{
		return ( FAIL );
	}

	return ( Lhw_if->send_special( special_id ) ? OK : FAIL );
}

static Rslt_t
hw_send_bits_pkt(
	Bits		&bits,
	const char	*info )
{
	BYTE			pbyte;
	std::vector<BYTE>	buf;
	if ( Lhw_if == NULL )
	{
		return ( FAIL );
	}

	bits.rst_out();
	while ( bits.get_byte( pbyte ) == OK )
	{
		buf.push_back( pbyte );
	}

	if ( buf.empty() )
	{
		return ( FAIL );
	}

	return ( Lhw_if->send_packet( &buf[0], buf.size(), info ) ? OK : FAIL );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		main()							-	 main function.
 *
 *	RETURN VALUE
 *
 *		0		-	Successful test run.
 *		<> 0	-	Problem while running test program.
 *
 *	DESCRIPTION
 *
 *		main() is called first and runs the sender test program.
 *		It first runs the hardware self test and then enter the
 *		keyboard command loop.
 */
/*--------------------------------------------------------------------------*/

int
main(
	int			argc,						// Count of args.
	char		**argv )					// Command line args.
{
	const char	*my_name = "main";
	Key_val		init_key = KEY_DEC_TST;		// Initial key value.

	if ( Args.get_args( argc, argv ) != OK )
	{
		if ( Args.get_print_user() )
		{
			print_user_docs();
		}
		else
		{
			Args.usage();
		}
		exit_send( 1 );
	}
	
	/*
	 *	Set Bits class static variables.
	 */
	Bits::set_static_variables( Args.get_trig_rev(), Args.get_loco_first(),
    							Args.get_lamp_rear(), Args.get_func_mask() );
	
	/*
	 *	Don't abort on errors if NO_ABORT flag set.
	 */
	Deflog.set_no_abort_flag( Args.get_no_abort() );

	/*
	 *	Tell Dcc_reg whether to log packets or send them to
	 *	hardware.
	 */
	Dcc_reg.set_log_pkts( Args.get_log_pkts() );

	if ( !init_hw_backend() )
	{
		fprintf( stderr,
			"ERROR: MCU backend init failed on <%s> at %u baud\n",
			Args.get_mcu_port(), Args.get_mcu_baud() );
		exit_send( 1 );
	}

	Dcc_reg.set_hw_if( Lhw_if );

	if ( Args.get_manual_flag() == true )	// Manual mode.
	{
		init_key	=	KEY_NULL;
	}

	/*
	 *	Pass user arguements to decoder test object Ldec_tst.
	 */
	Ldec_tst.set_run_mask(     Args.get_run_mask() );
	Ldec_tst.set_clk_mask(     Args.get_clk_mask() );
	Ldec_tst.set_trig_rev(     Args.get_trig_rev() );
	Ldec_tst.set_fill_msec(    Args.get_fill_msec() );
	Ldec_tst.set_decoder_type( Args.get_decoder_type() );

	get_log_file( argv[0] );

	if ( !Dcc_reg.get_log_pkts() )
	{
		STATPRINT(	"Starting decoder tests, address %u, type %c",
			Args.get_decoder_address(), Args.get_decoder_type() );
		printf(		"Starting decoder tests, address %u, type %c\n",
			Args.get_decoder_address(), Args.get_decoder_type() );

		STATPRINT( "Beginning MCU selftests" );
		printf(    "Beginning MCU selftests\n" );

		if ( do_mcu_self_tests() == OK )
		{
			STATPRINT( "MCU selftests passed" );
			printf(    "MCU selftests passed\n" );
		}
		else
		{
			printf(	                "%s: MCU selftests failed\n", my_name );
			ERRPRINT( my_name, LOG_ERR, "MCU selftests failed" );
			init_key	=	KEY_NULL;			// Don't start tests on fail.
		}
	}
	else
	{
		DBGLOG( "%s: Skipping self tests.", my_name );
	}

	parse_key_cmd( init_key );

	if ( hw_is_running() == true )
	{
		hw_stop_clk();
	}

	Dcc_reg.set_hw_if( NULL );
	close_hw_backend();

	exit_send( 0 );
	return ( 0 );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		exit_send()							-    Print exit message and exit.
 *
 *	RETURN VALUE
 *
 *		int status				 			-	exit() status.
 *
 *	DESCRIPTION
 *
 *		exit_send() prints <PROGRAM END n> to the console and the logs
 *		and then calls exit() with status.
 */
/*--------------------------------------------------------------------------*/

static void
exit_send(
	int			status )					// exit() status.
{
	const char		*my_name = "exit_send";

	if ( hw_is_running() == true )
	{
		ERRPRINT( my_name, LOG_WARNING,
			"exit_send() called with clock running." );
		hw_stop_clk();
	}

    printf( "{SEND_END %d}\n", status );
    STATPRINT(	"{SEND_END %d}", status );

	Deflog.close_stat();
	Deflog.close_log();
	close_hw_backend();

	exit( status );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		print_user_docs()		  				-	 Print user docs.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		print_user_docs() prints the user documentation summary to
 *		DOCS_FILE.
 */
/*--------------------------------------------------------------------------*/

static void
print_user_docs( void )
{
	FILE		*fp;						// FILE pointer.

	if ( (fp = fopen( DOCS_FILE, "w" )) == NULL )
	{
		fprintf( stderr,
			"User documentation file <%s> could not be opened", DOCS_FILE );
		return;
	}

	printf( "Printing user documentation summary to <%s>\n", DOCS_FILE );

	/*
	 *	Write user documentation summary to DOCS_FILE
	 */
	fprintf( fp,
		"User documenation summary for test software version %c.%u.%u.%u\n\n",
		Ver_rel, Ver_maj, Ver_min, Ver_bld );
	fputs(
		"Summary of command line and 'SEND.CFG' switches:\n\n",
		fp );
	Args.usage( fp );

	fputs(
		"\nSummary of decoder tests and clocks:\n\n",
		fp );
	Ldec_tst.print_user_docs( fp );

	fclose( fp );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		get_log_file()		  				-	 Get log file from user.
 *
 *	RETURN VALUE
 *
 *		char *cmdname			 			-	Name of command.
 *
 *	DESCRIPTION
 *
 *		get_log_file() asks the user for a logging file and data about
 *		the type of test.
 */
/*--------------------------------------------------------------------------*/

static void
get_log_file(
	const char	*cmd_name )					// Name of .exe file.
{
	const char	*my_name = "get_log_file";
	int			i;							// Index variable.
	static char	buf[ MAXLINE ];				// Base name buffer.
	static char	fname[  MAXLINE + 8 ];		// File name.
    u_long		nCRC;						// Sender file CRC.
    u_long		nLength;					// Sender file length.
    int			rslt;						// Result of checksum method.
    FILE		*fp;						// FILE for command line switches.
	char			full_path[ MAX_PATH ];		// Full path to executable.
	const char	*checksum_path;				// Path to use for checksum.
	DWORD			path_len;

	path_len = GetModuleFileNameA( NULL, full_path, MAX_PATH );
	if ( path_len == 0 || path_len >= MAX_PATH )
	{
		checksum_path = cmd_name;
	}
	else
	{
		checksum_path = full_path;
	}

	rslt = nCheckSumFile( NULL, checksum_path, &nCRC, &nLength );

    if ( rslt != 0 )
    {
		ERRPRINT( my_name, LOG_CRIT,
			"nCheckSumFile() failed" );
		exit_send( 1 );
    }

	printf(		"Test software version %c.%u.%u.%u\n",
		Ver_rel, Ver_maj, Ver_min, Ver_bld );
    printf(		"File <%s>\n",
    	cmd_name );
    printf(		"CRC %lu, Length %lu\n",
    	nCRC, nLength );
	fputs( "Enter base of log and statistics file name > ", stdout );

	if ( !fgets(  buf, MAXLINE, stdin ) )
	{
		ERRPRINT( my_name, LOG_CRIT,
			"fgets() failed" );
		exit_send( 1 );
	}
	else
	{
		buf[ strlen( buf ) - 1 ] = '\0';
	}

	strcpy( fname, buf );
	strcat( fname, ".log" );
	if ( Deflog.open_log( fname ) != OK )
	{
		ERRPRINT( my_name, LOG_CRIT,
			"Log file <%s> could not be opened", buf );
		exit_send( 1 );
	}

	strcpy( fname, buf );
	strcat( fname, ".sum" );
	if ( Deflog.open_stat( fname ) != OK )
	{
		ERRPRINT( my_name, LOG_CRIT,
			"Statistics file <%s> could not be opened", buf );
		exit_send( 1 );
	}

	STATPRINT( "BEGINNING decoder test log" );
	STATPRINT(	"Test software version %c.%u.%u.%u",
		Ver_rel, Ver_maj, Ver_min, Ver_bld );
    STATPRINT(	"File <%s>",
    	cmd_name );
    STATPRINT(	"CRC %lu, Length %lu",
    	nCRC, nLength );

	for ( i = 0; i < QUESTIONS_SIZE; i++ )
	{
		fputs( Questions[i].question, stdout );

		if ( !fgets( buf, MAXLINE, stdin ) )
		{
			ERRPRINT( my_name, LOG_CRIT,
				"fgets() failed" );
			exit_send( 1 );
		}
		else
		{
			buf[ strlen( buf ) - 1 ] = '\0';
		}

		TO_STAT( Questions[i].log_str, buf );
	}

	puts( "Enter comments.  Begin line with '.' to end input" );
    puts(    "--------------------------------" );
	TO_STAT( "--------------------------------\n" );
	while ( true )
	{
		if ( !fgets( buf, MAXLINE, stdin ) )
		{
			if ( feof( stdin ) )
			{
				break;
			}
			ERRPRINT( my_name, LOG_CRIT,
				"fgets() failed" );
			exit_send( 1 );
		}

		if ( !strcmp( buf, ".\n" ) )
		{
			break;
		}

		TO_STAT( "%s", buf );
	}
    puts(    "--------------------------------" );
	TO_STAT( "--------------------------------\n\n" );

	// Log summary of command line switches and .CFG file
	if ( (fp = Deflog.get_fp_log()) != NULL )
	{
    	fputs(
			"Summary of command line and 'SEND.CFG' switches:\n\n",
			fp );
		Args.usage( fp );
        fputc( '\n', fp );
		fflush( fp );
	}

	if ( (fp = Deflog.get_fp_stat()) != NULL )
	{
    	fputs(
			"Summary of command line and 'SEND.CFG' switches:\n\n",
			fp );
		Args.usage( fp );
        fputc( '\n', fp );
		fflush( fp );
	}

	if ( Deflog.get_stderr_too() == true )
	{
    	fp = stderr;
    	fputs(
			"Summary of command line and 'SEND.CFG' switches:\n\n",
			fp );
		Args.usage( fp );
        fputc( '\n', fp );
	}
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		do_mcu_self_tests()					-	 Run MCU self tests.
 *
 *	RETURN VALUE
 *
 *		OK		-	All tests passed.
 *		FAIL	-	One or more tests failed.
 *
 *	DESCRIPTION
 *
 *		do_mcu_self_tests() exercises the MCU sender hardware via the
 *		Sender_hw_if interface: timing, clock start/stop, packet send,
 *		status and stats queries.
 */
/*--------------------------------------------------------------------------*/

/*
 *	SELF_CHECK logs PASS/FAIL for each individual test to the stat log
 *	and prints to stdout.  Failures are additionally logged at LOG_ERR.
 */
#define SELF_CHECK( name_, expr_ ) \
	do { \
		bool _ok = (expr_); \
		printf( "  %-40s %s\n", (name_), _ok ? "PASS" : "FAIL" ); \
		STATPRINT( "  %-40s %s", (name_), _ok ? "PASS" : "FAIL" ); \
		if ( !_ok ) \
		{ \
			ERRPRINT( my_name, LOG_ERR, "FAILED: %s", (name_) ); \
			++failures; \
		} \
	} while ( 0 )

static Rslt_t
do_mcu_self_tests( void )
{
	const char	*my_name = "do_mcu_self_tests";
	int			 failures = 0;

	STATPRINT( "MCU self test" );
	STATPRINT( "----------------------------------------" );
	printf( "MCU self test\n" );
	printf( "----------------------------------------\n" );

	/* 1. RESET_STATS */
	SELF_CHECK( "1. RESET_STATS", Lhw_if->reset_stats() );

	/* 2. SET_TIMING */
	SELF_CHECK( "2. SET_TIMING (200/100/116 us)",
		Lhw_if->set_timing( 200, 100, 116 ) );

	/* 3. START_CLK */
	{
		bool started = Lhw_if->start_clock();
		SELF_CHECK( "3. START_CLK", started );
		if ( started )
		{
			Lhw_running = true;
		}
	}

	/* 4. SEND_BYTES (4 x 0xFF idle bytes) */
	SELF_CHECK( "4. SEND_BYTES (4 x 0xFF)",
		Lhw_if->send_bytes( 4, 0xFF, "selftest" ) );

	/* 5. GET_STATUS */
	{
		Sender_hw_status st;
		bool ok = Lhw_if->get_status( st );
		SELF_CHECK( "5. GET_STATUS succeeds", ok );
		if ( ok )
		{
			SELF_CHECK( "5a. running == true",    st.running );
			SELF_CHECK( "5b. underflow == false", !st.underflow );
		}
	}

	/* 5c. GET_STATS after GET_STATUS (SEND_BYTES phase) */
	{
		Sender_hw_stats stats;
		bool ok = Lhw_if->get_stats( stats );
		SELF_CHECK( "5c. GET_STATS succeeds", ok );
		if ( ok )
		{
			SELF_CHECK( "5d. bytes_sent == 4",   stats.bytes_sent   == 4 );
			SELF_CHECK( "5e. packets_sent == 1", stats.packets_sent == 1 );
			SELF_CHECK( "5f. underruns == 0",    stats.underruns    == 0 );
		}
	}

	/* 6. RESET_STATS again */
	SELF_CHECK( "6. RESET_STATS (before SEND_PACKET)", Lhw_if->reset_stats() );

	/* 7. SEND_PACKET (DCC idle FF 00 FF) */
	{
		static const uint8_t idle_pkt[] = { 0xFF, 0x00, 0xFF };
		SELF_CHECK( "7. SEND_PACKET (DCC idle FF 00 FF)",
			Lhw_if->send_packet( idle_pkt, sizeof( idle_pkt ), "selftest" ) );
	}

	/* 8. GET_STATS */
	{
		Sender_hw_stats stats;
		bool ok = Lhw_if->get_stats( stats );
		SELF_CHECK( "8. GET_STATS succeeds", ok );
		if ( ok )
		{
			SELF_CHECK( "8a. bytes_sent == 3",    stats.bytes_sent   == 3 );
			SELF_CHECK( "8b. packets_sent == 1",  stats.packets_sent == 1 );
			SELF_CHECK( "8c. underruns == 0",     stats.underruns    == 0 );
		}
	}

	/* 9. STOP_CLK */
	{
		bool stopped = Lhw_if->stop_clock();
		SELF_CHECK( "9. STOP_CLK", stopped );
		if ( stopped )
		{
			Lhw_running = false;
		}
	}

	/* 10. RESET_STATS */
	SELF_CHECK( "10. RESET_STATS", Lhw_if->reset_stats() );

#undef SELF_CHECK

	STATPRINT( "----------------------------------------" );
	printf( "----------------------------------------\n" );

	if ( failures == 0 )
	{
		STATPRINT( "MCU selftests: ALL TESTS PASSED" );
		printf(    "MCU selftests: ALL TESTS PASSED\n" );
		return ( OK );
	}

	ERRPRINT( my_name, LOG_ERR,
		"MCU selftests FAILED: %d failure(s)", failures );
	STATPRINT( "MCU selftests: %d TEST(S) FAILED", failures );
	printf(    "MCU selftests: %d TEST(S) FAILED\n", failures );
	return ( FAIL );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		get_key_cmd()						-	 return keyboard command.
 *
 *	RETURN VALUE
 *
 *		key		-	Keyboard character typed.
 *
 *	DESCRIPTION
 *
 *		get_key_cmd() waits for the user to type a key and then returns
 *		the key the user typed.
 */
/*--------------------------------------------------------------------------*/

static Key_val
get_key_cmd( void )
{
	return( (Key_val)getch() );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		parse_key_cmd()						-	 keyboard parse loop.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		parse_key_cmd() is the keyboard command loop.  It loops getting
 *		keyboard commands, parses each command, and takes the appropriate
 *		action.  It return on error or when the user issues the 'KEY_QUIT'
 *		command.  This function must not block once it is sending DCC packets
 *		so it calls kbhit() to make sure a keyboard command is available
 *		prior to getting it via get_key_cmd().
 */
/*--------------------------------------------------------------------------*/

static void
parse_key_cmd(
Key_val			init_key )					// Initial key value.
{
	const char	*my_name = "parse_key_cmd";
	int			i;							// Index value.
	Key_val		key_val;					// Present key value.
	Send_type	rep_type;					// Type of packet to repeat.
	bool		c_flag;						// Continue if true.
	bool		fw;							// Go forward if true.
	int			speed;						// Speed.
	bool		ramp;						// Ramp up if true.
	Rslt_t		ret_decoder;		  		// decoder_test() return value.
	Rslt_t		tst_rslt;					// Decoder test result.
	int			sp_max;						// Maximum speed or output id.
	BYTE		pbyte;						// Present byte to send.
	u_int		o_time_index;				// Scope timing test index.

	rep_type	=	SEND_NONE;

    if (      Args.get_decoder_type() == DEC_LOCO )
    {
    	sp_max = SP_28_MAX;
    }
	else if ( Args.get_decoder_type() == DEC_FUNC )
    {
    	sp_max = 0;	// sp_max not used for function decoders.
    }
	else if ( Args.get_decoder_type() == DEC_SIG )
    {
    	sp_max = ASPECT_MAX;
    }
    else // Accessory decoder.
    {
    	sp_max = ACC_MAX;
    }

	/*	Print the CMD_MSG to the screen if we're not going
	 *	straight into the decoder tests.
	 */
	if ( init_key == KEY_NULL )
	{
		CLR_LINE;
		CMD_MSG;
	}

	/*
	 *	Keyboard command loop.  Do 'init_key' once if set then
	 *	Wait for the user to type a key.
	 */
	while (		( (key_val = init_key)		!=	KEY_NULL )
		||		( (key_val = get_key_cmd())	!=	KEY_QUIT ) )
	{
		init_key	=	KEY_NULL;

		CLR_LINE;

		/*
		 *	Reset clocks to nominal if we're  leaving the scope timing test.
		 */
		if (	rep_type ==  SEND_SCOPE_O
			&&	(	key_val != KEY_SCOPE_O
				 &&	key_val != KEY_HEADER ) )
		{
			hw_set_clk(	DECODER_0T_NOM,
						DECODER_0H_NOM,
						DECODER_1T_NOM );
		}

		switch ( key_val )					// Parse the key command.
		{
		case KEY_ESC:						// Stop anything in progress.
			rep_type	=	SEND_NONE;
			if ( hw_is_running() == true )
			{
				hw_stop_clk();
			}
			printf( "Returning to command line\n" );
			break;

		case KEY_HEADER:					// Print the help message header.
			printf( "%s", Args.get_key_help() );
			Dcc_reg.up_print( true );  		// Print out hardware state.
			if ( hw_is_running() == true )
			{
				hw_clr_under();
			}
			break;

		/*
		 *	The following commands are for low level board debugging
		 *	and use the software generated clock.
		 */
		case KEY_CLOCK:						// Change static clock phase.
			rep_type	=	SEND_NONE;
			if ( hw_is_running() == true )
			{
				hw_stop_clk();
			}
			printf( "Single SW clock toggle is not supported in MCU mode\n" );
			Dcc_reg.up_print();
			break;

		case KEY_MCLK:						// Send a series of static clocks.
			rep_type	=	SEND_MCLK;
			if ( hw_is_running() == true )
			{
				hw_stop_clk();
			}
			printf( "SW clock burst is not supported in MCU mode\n" );
			Dcc_reg.up_print();
			break;

		case KEY_UNDER:						// Clear the underflow flag.
			rep_type	=	SEND_NONE;
			if ( hw_is_running() == true )
			{
				hw_stop_clk();
			}
			hw_clr_under();
			printf( "Underflow reset\n" );
			Dcc_reg.up_print();
			break;

		case KEY_GEN	:					// Toggle GEN_2, read inputs.
			printf( "Generic port test is not supported in MCU mode\n" );
			if ( hw_is_running() == true )
			{
				hw_clr_under();
			}
			break;

		/*
		 *	The following commands are for board and fixture debugging
		 *	and use the hardware generated clock.
		 */
		case KEY_ZERO:						// Send zero bytes.
			rep_type	=	SEND_ZERO;
			if ( hw_is_running() != true )
			{
				hw_start_clk();
			}
			printf( "Sending Zeros\n" );
			hw_clr_under();
			break;

		case KEY_ONE:						// Send one bytes.
			rep_type	=	SEND_ONE;
			if ( hw_is_running() != true )
			{
				hw_start_clk();
			}
			printf( "Sending Ones\n" );
			hw_clr_under();
			break;

		case KEY_SCOPE_A:					// Send the scope A test.
			rep_type	=	SEND_SCOPE_A;
			if ( hw_is_running() != true )
			{
				hw_start_clk();
			}
			printf( "Sending scope test A (0 then 1)\n" );
			hw_clr_under();
			break;

		case KEY_SCOPE_B:					// Send the scope B test.
			rep_type	=	SEND_SCOPE_B;
			if ( hw_is_running() != true )
			{
				hw_start_clk();
			}
			printf( "Sending scope test B (1 then 0)\n" );
			hw_clr_under();
			break;

		case KEY_SCOPE_O:					// Send the scope timing test.
			if ( rep_type != SEND_SCOPE_O )
			{
				rep_type		=	SEND_SCOPE_O;
				o_time_index	=	0;
			}
			else
			{
				if ( ++o_time_index == SCOPE_TIME_TBL_SIZE )
				{
					o_time_index	=	0;
				}
			}

			if ( hw_is_running() != true )
			{
				hw_start_clk();
			}
			printf( "Sending scope timing test with period %5d\n",
					Scope_time_tbl[ o_time_index ] );
			hw_set_clk(	Scope_time_tbl[ o_time_index ],
						Scope_time_tbl[ o_time_index ] / 2,
						DECODER_1T_NOM );
			hw_clr_under();
			break;

		case KEY_WARBLE:				  	// Send warble packets.
			rep_type	=	SEND_WARBLE;
			if ( hw_is_running() != true )
			{
				hw_start_clk();
			}
			printf( "Sending warble packets\n" );
			hw_clr_under();
			break;

		case KEY_STRETCHED:					// Send the stretched 0 test.
			rep_type	=	SEND_STRETCHED;
			if ( hw_is_running() != true )
			{
				hw_start_clk();
			}
			printf( "Sending stretched 0s (0xff then 0x02)\n" );
			hw_clr_under();
			break;

		/*
		 *	The following commands are used to manually test the decoder.
		 */
		case KEY_RESET:						// Send reset packets.
			rep_type	=	SEND_RESET;
			if ( hw_is_running() != true )
			{
				hw_start_clk();
			}
			printf( "Sending Reset packets\n" );
			hw_clr_under();
			break;

		case KEY_HARD:						// Send hard reset packets.
			rep_type	=	SEND_HARD;
			if ( hw_is_running() != true )
			{
				hw_start_clk();
			}
			printf( "Sending HARD Reset packets\n" );
			hw_clr_under();
			break;

		case KEY_IDLE:						// Send idle packets.
			rep_type	=	SEND_IDLE;
			if ( hw_is_running() != true )
			{
				hw_start_clk();
			}
			printf( "Sending Idle packets\n" );
			hw_clr_under();
			break;

		/*
		 *	Send DCC packets.  The direction and speed
		 *	are changed by the set of commands that follow this one.
		 */
		case KEY_DCC:
		case KEY_DCC_S:
			rep_type	=	(key_val == KEY_DCC) ? SEND_DCC : SEND_DCC_S;
			if ( hw_is_running() != true )
			{
				hw_start_clk();
			}
			fw 			=	true;
			ramp		=	true;
			if ( Args.get_decoder_type() == DEC_LOCO )
			{
				speed	  =	0;
				printf( "Sending%s Loco packets, GEN  ",
                		(key_val == KEY_DCC) ? "" : " Stretched" );
			}
			else if ( Args.get_decoder_type() == DEC_FUNC )
			{
				printf( "Sending%s %s Function packets, GEN  ",
               			(key_val == KEY_DCC) ? "" : " Stretched",
                        Args.get_lamp_rear() ? "Reverse" : "Forward");
			}
			else if ( Args.get_decoder_type() == DEC_SIG )
			{
				speed	  =	0;
				printf( "Sending%s Signal packets, GEN  ",
                		(key_val == KEY_DCC) ? "" : " Stretched" );
			}
			else // Accessory decoder.
			{
				speed	  =	0;
				printf( "Sending%s Accessory packets, GEN  ",
               			(key_val == KEY_DCC) ? "" : " Stretched" );
			}
			Dcc_reg.gen_print();
			putchar( '\n' );
			hw_clr_under();
			break;

		case KEY_SPEED:						// Change loco speed, acc. output.
			if ( rep_type == SEND_DCC || rep_type == SEND_DCC_S )
			{
            	if ( Args.get_decoder_type() == DEC_FUNC )
                {
					printf(
						"Speed does not work with function decoders\n" );
                }
                else
                {
					if ( ramp )
					{
						if ( ++speed > sp_max )
						{
							speed	-=	2;
							ramp	=	!( ramp );
						}
					}
					else
					{
						if ( --speed < SP_MIN )
						{
							speed	+=	2;
							ramp	=	!( ramp );
						}
					}
                    if (      Args.get_decoder_type() == DEC_LOCO )
                    {
                    	printf( "Setting loco speed to %d, GEN ", speed );
                    }
                    else if ( Args.get_decoder_type() == DEC_FUNC )
                    {
                    	printf( "ERROR: speed unused for function decoder " );
                    }
                    else if ( Args.get_decoder_type() == DEC_SIG )
                    {
                    	printf( "Setting signal aspect to %d, GEN ", speed );
                    }
                    else // Accessory decoder.
                    {
                    	printf( "Setting accessory output to %d, GEN ", speed );
                    }
					Dcc_reg.gen_print();
					putchar( '\n' );
                }
			}
			else
			{
				printf(
					"Press 'd' or 'D' key first to use this command\n" );
			}
			hw_clr_under();
			break;

		case KEY_E_STOP:					// Send DCC emergency stop.
			if ( rep_type == SEND_DCC || rep_type == SEND_DCC_S )
			{
				if ( Args.get_decoder_type() == DEC_LOCO )
				{
					speed	=	SP_E_STOP;
					printf( "Setting Emergency Stop, GEN " );
					Dcc_reg.gen_print();
					putchar( '\n' );
				}
				else
				{
					printf(
						"Emergency stop only works with loco decoders\n" );
				}
			}
			else
			{
				printf(
					"Press 'd' key first to use this command\n" );
			}
			hw_clr_under();
			break;

		case KEY_E_STOP_I:					// Send DCC emergency stop I.
			if ( rep_type == SEND_DCC || rep_type == SEND_DCC_S )
			{
				if ( Args.get_decoder_type() == DEC_LOCO )
				{
					speed	=	SP_E_STOP_I;
					printf( "Setting Emergency Stop I, GEN " );
					Dcc_reg.gen_print();
					putchar( '\n' );
				}
				else
				{
					printf(
						"Emergency stop I only works with loco decoders\n" );
				}
			}
			else
			{
				printf(
					"Press 'd' key first to use this command\n" );
			}
			hw_clr_under();
			break;

		case KEY_FW:						// Change DCC packet direction.
			if ( rep_type == SEND_DCC || rep_type == SEND_DCC_S )
			{
            	fw			=	!( fw );
				if (      Args.get_decoder_type() == DEC_LOCO )
				{
					printf( "Setting direction <%s>, GEN ",
						fw ?  "FORWARD" : "BACK" );
					Dcc_reg.gen_print();
					putchar( '\n' );
				}
				else if ( Args.get_decoder_type() == DEC_FUNC )
				{
					printf( "Setting function output <%s>, GEN ",
						fw ?  "ON" : "OFF" );
					Dcc_reg.gen_print();
					putchar( '\n' );
				}
            	else if ( Args.get_decoder_type() == DEC_SIG )
                {
					printf(
						"Direction does not work with signal decoders\n" );
                }
				else // Accessory decoder.
				{
					printf( "Setting accessory output <%s>, GEN ",
						fw ?  "ON" : "OFF" );
					Dcc_reg.gen_print();
					putchar( '\n' );
				}
			}
			else
			{
				printf(
					"Press 'd' key first to use this command\n" );
			}
			hw_clr_under();
			break;

		/*
		 *	The following commands, KEY_SELF_TST & KEY_DEC_TST, run
		 *	for a significant period of time and print messages to
		 *	the screen.
		 */
		case KEY_SELF_TST:				// Run repeated self tests.
			rep_type	=	SEND_SELF_TST;
			if ( hw_is_running() == true )
			{
				hw_stop_clk();
			}
			printf(
				"Starting MCU selftest cycles, type '<ESC>' to stop tests\n" );
			break;

		case KEY_DEC_TST:				// Run one or more decoder test cycles.
			rep_type	=	SEND_DEC_TST;
			if ( hw_is_running() != true )
			{
				hw_start_clk();
			}
			printf(
				"Starting Decoder tests, type '<ESC> q' to stop tests\n" );
			hw_clr_under();
			break;

		case KEY_KICK:					// Run kickstart followed by idles.
			if ( Args.get_decoder_type() == DEC_FUNC )
            {
				printf( "Starting loco kickstart to enable functions\n" );
				
				rep_type	=	SEND_IDLE;
				if ( hw_is_running() != true )
				{
					hw_start_clk();
				}
				Dcc_reg.kick_start( Dcc_bits, Args );
				hw_clr_under();
            }
			else
			{
				printf( "Kickstart only works with function decoders, TYPE = f\n" ); 
			}
			/*
			rep_type	=	SEND_IDLE;
			if ( Dcc_reg.get_running() != true )
			{
				Dcc_reg.start_clk();
			}
			printf(
				"Starting Decoder tests, type '<ESC> q' to stop tests\n" );
			Dcc_reg.clr_under();
			*/
			break;

		default:
			rep_type	=	SEND_NONE;
			if ( hw_is_running() == true )
			{
				hw_stop_clk();
			}
			printf( "Unknown key <%c>\n", (char)key_val );
			break;
		}

		/*
		 *	Print the CMD_MSG prompt to the screen for those commands
		 *	that don't print anything by themselves as they repeat.
		 */
		if (	rep_type != SEND_DEC_TST
			&&	rep_type != SEND_SELF_TST )
		{
			CMD_MSG;
		}

		/*
		 *	Loop sending packets if the user requested it.
		 *	This section loops sending the selected packet
		 *	repeatedly until the user strikes another key or
		 *	an error occurs.
		 */
		if ( ( rep_type != SEND_NONE ) && ( rep_type != SEND_SCOPE_O ) )
		{
			do
			{
				ret_decoder	=	OK;

				switch( rep_type )
				{
				case SEND_NONE:				// Error, shouldn't get this.
					printf(
						"Unexpected SEND_NONE in repeat loop\n" );
					ERRPRINT( my_name, LOG_CRIT,
						"Unexpected SEND_NONE in repeat loop\n" );
					exit_send( 1 );
				case SEND_MCLK:				// Send a string of CPU clocks.
					printf( "SW clock burst is not supported in MCU mode\n" );
					c_flag	=	false;
					break;

				case SEND_ZERO:				// Repeat zero bytes.
					hw_set_scope( true );
					hw_send_bytes( 2, 0x00, "SEND_ZERO." );
					break;

				case SEND_ONE:				// Repeat one bytes.
					hw_set_scope( true );
					hw_send_bytes( 2, 0xff, "SEND_ONE." );
					break;

				case SEND_SCOPE_A:			// Repeat scope A (0-1) test.
					Dcc_bits.clr_in();
					Dcc_bits.put_byte( 0x00 ).put_byte( 0xff ).done();
					hw_set_scope( true );
					hw_send_bits_pkt( Dcc_bits, "SEND_SCOPE_A." );
					break;

				case SEND_SCOPE_B:			// Repeat scope B (1-0) test.
					Dcc_bits.clr_in();
					Dcc_bits.put_byte( 0xff ).put_byte( 0x00 ).done();
				 	hw_set_scope( true );
					hw_send_bits_pkt( Dcc_bits, "SEND_SCOPE_B." );
					break;

				case SEND_WARBLE:	 		// Repeat the warble packet.
					hw_set_scope( true );
					hw_send_special( SHP_SPECIAL_WARBLE );
					break;

				case SEND_STRETCHED:   		// Repeat stretched 0 test.
					Dcc_bits.clr_in();
					Dcc_bits.put_byte( 0xff ).put_byte( 0x02 ).done();
					Dcc_bits.rst_out();
					Dcc_bits.get_byte( pbyte );
					hw_set_scope( true );
					hw_send_bytes( 1, pbyte, "SEND_STRETCHED preamble." );
					Dcc_bits.get_byte( pbyte );
					hw_send_stretched_byte(	DECODER_0T_NOM * 4,
												DECODER_0H_NOM * 2,
												pbyte,
												"SEND_STRETCHED 0 bit." );
					break;

				case SEND_RESET:			// Repeat the reset packet.
					hw_send_bytes( 1, 0x00, "SEND_RESET." );
					hw_set_scope( true );
					hw_send_special( SHP_SPECIAL_RESET );
					for ( i = 0; i < 15; i++ )	// Send 15 normal idles.
					{
						hw_send_special( SHP_SPECIAL_IDLE );
					}
					break;

				case SEND_HARD:			// Repeat the hard reset packet.
					hw_send_bytes( 1, 0x00, "SEND_HARD." );
					hw_set_scope( true );
					hw_send_special( SHP_SPECIAL_HARD_RESET );
					for ( i = 0; i < 15; i++ )	// Send 15 normal idles.
					{
						hw_send_special( SHP_SPECIAL_IDLE );
					}
					break;

				case SEND_IDLE:				// Repeat the idle packet.
                	Dcc_bits.clr_in();
                    Dcc_bits.put_idle_pkt( 4 );
                    Dcc_bits.put_1s( 1 ).done();

					hw_send_bytes( 1, 0x00, "SEND_IDLE." );
					hw_set_scope( true );
					hw_send_bits_pkt( Dcc_bits, "SEND_IDLE." );
					break;

				case SEND_DCC:				// Repeat the DCC packet.
					Dcc_bits.clr_in();

                    if ( Args.get_decoder_type() == DEC_LOCO )
                    {
                    	Dcc_bits.put_cmd_pkt_28( 	Args.get_decoder_address(),
                    							 	fw, speed );
                    }
                    else if ( Args.get_decoder_type() == DEC_FUNC )
                    {
						// See NMRA\put_........_pktx Info.txt
                   	    Dcc_bits.put_func_grp_pktx( 	Args.get_decoder_address(),
                                        GRP_1, fw ? Args.get_func_mask() : 0 );
 						Dcc_bits.put_1s( 1 ).put_byte( 0x00 );	// Align next packet on Byte boundary.
                   }
                    else if ( Args.get_decoder_type() == DEC_SIG )
                    {
                    	Dcc_bits.put_sig_pkt( 	   	Args.get_decoder_address(),
                        						   	speed );
                    }
                    else // Accessory decoder.
                    {
                    	Dcc_bits.put_acc_pkt(		Args.get_decoder_address(),
                        					  		fw, speed );
                    }

                    Dcc_bits.put_idle_pkt( 1 );
					Dcc_bits.put_1s( 1 ).done();

					hw_send_bytes( 1, 0x00, "SEND_DCC Send 0." );
					hw_set_scope( true );
					hw_send_bits_pkt( Dcc_bits, "SEND_DCC Send pkt." );
					break;

				case SEND_DCC_S:   			// Repeat stretched 0 test.
					Dcc_bits.clr_in();

                    // Align first 0 on bit boundary.
					Dcc_bits.put_0s( STRETCH_BITS );

					if ( Args.get_decoder_type() == DEC_LOCO )
					{
						Dcc_bits.put_cmd_pkt_28(   	Args.get_decoder_address(),
													fw, speed );
					}
                    else if ( Args.get_decoder_type() == DEC_FUNC )
                    {
                    	// Set the direction based on the lamp rear value.
                    	Dcc_bits.put_cmd_pkt_28( 	Args.get_decoder_address(),
                        							!Args.get_lamp_rear(), 0 );
                    	Dcc_bits.put_idle_pkt( 1 );
                        Dcc_bits.put_func_grp_pktx(	Args.get_decoder_address(),
                        	GRP_1, fw ? Args.get_func_mask() : 0);
                    }
                    else if ( Args.get_decoder_type() == DEC_SIG )
                    {
                    	Dcc_bits.put_sig_pkt( 	   	Args.get_decoder_address(),
                        						   	speed );
                    }
					else // Accessory decoder.
					{
						Dcc_bits.put_acc_pkt(		Args.get_decoder_address(),
													fw, speed );
					}

					Dcc_bits.put_idle_pkt( 1 );
					Dcc_bits.put_1s( 1 ).done();
        			/*
        			 * Send BYTES immediately before single stretched 0.
					 */
					Dcc_bits.rst_out();
					Dcc_bits.get_byte( pbyte );
					hw_send_bytes( 1, pbyte, "SEND_DCC_S preamble 1." );
					Dcc_bits.get_byte( pbyte );
					hw_send_bytes( 1, pbyte, "SEND_DCC_S preamble 2." );
                    /*
                     * Send single stretched 0 just after preamble.
                     */
					hw_set_scope( true );
					Dcc_bits.get_byte( pbyte );
					hw_send_stretched_byte( DECODER_0T_MAX,
												DECODER_0T_MAX-DECODER_0H_MAX,
												pbyte,
												"SEND_DCC_S stretched 0." );
					/*
					 * Send remaining part of packet.
                     */
					while ( Dcc_bits.get_byte( pbyte ) == OK )
					{
						hw_send_bytes( 1, pbyte, "SEND_DCC_S end." );
					}
					break;

				case SEND_SELF_TST:		  	// Run MCU selftests.
								do_mcu_self_tests();
									break;

				case SEND_DEC_TST:			// Run decoder tests.
					tst_rslt	=	OK;
					Dcc_reg.rst_stats();
					ret_decoder	=	Ldec_tst.decoder_test( tst_rslt );
					STATPRINT( "Packets sent %lu, Bytes sent %lu",
						Dcc_reg.get_p_cnt(), Dcc_reg.get_b_cnt() );

					if ( ret_decoder == FAIL )
					{
						if ( tst_rslt == OK )
						{
							STATPRINT(
								"Tests INTERRUPTED, All tests passed" );
							printf(
								"Tests INTERRUPTED, All tests passed\n" );
						}
						else
						{
							STATPRINT(
								"Tests INTERRUPTED, Some tests failed" );
							printf(
								"Tests INTERRUPTED, Some tests failed\n" );
						}
						c_flag		=	false;

						/*
						 * Stop the clock and reset it to nominal values
						 * if the decoder tests are interrupted.
						 */
						if ( hw_is_running() == true )
						{
							hw_stop_clk();
						}
						hw_set_clk(	DECODER_0T_NOM,
									DECODER_0H_NOM,
									DECODER_1T_NOM );
					}
					else
					{
						if ( tst_rslt == OK )
						{
							STATPRINT(
								"Tests COMPLETED, All tests passed" );
							printf(
								"Tests COMPLETED, All tests passed\n" );
						}
						else
						{
							STATPRINT(
								"Tests COMPLETED, Some tests failed" );
							printf(
								"Tests COMPLETED, Some tests failed\n" );
						}
						if ( Args.get_rep_flag() == true )
						{
							c_flag	=	true;
						}
						else
						{
							return;
						}
					}
					break;

				default:
					printf(
						"Unexpected rep_type %d\n", rep_type );
					ERRPRINT( my_name, LOG_CRIT,
						"Unexpected rep_type %d", rep_type );
					exit_send( 1 );
				}

				/*
				 *	Check for new commands or errors for all repeat commands
				 *	except for the decoder test which handles this by itself.
				 */
				if ( rep_type != SEND_DEC_TST )
				{
					if ( Dcc_reg.get_obj_errs() || kbhit())
					{
						c_flag	=	false;
					}
					else
					{
						c_flag	=	true;
					}
				}
			} while ( c_flag );

			CLR_LINE;
			if ( rep_type == SEND_SELF_TST )
			{
				// test summary already printed inside do_mcu_self_tests()
			}
			else
			{
				Dcc_reg.print_stats();
				Dcc_reg.rst_stats();
			}
			Dcc_reg.clear_warn();

			/*
			 *	Print the CMD_MSG to the screen after the decoder test since
			 *	we're about to block waiting for a key press.
			 */
			if ( rep_type == SEND_DEC_TST )
			{
				rep_type = SEND_NONE;		// Don't restart tests.
				CMD_MSG;
			}
		}
	}

    if ( key_val == KEY_QUIT )				// Print '\n' if program ending.
    {
    	putchar( '\n' );
    }

	return;
}
