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
#include "st_vect.h"
#include "self_tst.h"
#include "dec_tst.h"

#include <cksum.h>
#include <zlog.h>

#if SEND_VERSION < 4
  #include <conio.h>
#endif
#include <string.h>
#include <stdlib.h>

#if SEND_VERSION >= 4
extern "C"
{
	#include "Port.h"
	#include <track.h>

	extern int send_main(int argc, char **argv );

	extern void SendZero(void);
	extern void SendOne(void);
	extern void SendScopeA(void);
	extern void SendScopeB(void);
	extern void SendWarble(void);
	extern void SendStretched(void);
	extern void SendReset(void);
	extern void SendHard(void);
	extern void SendIdle(void);

	extern void SetDccTiming(void);

	extern void CabTestInProgress(int status);
	//extern void CabTestComplete(void);

	extern int BuildLocoPacket(unsigned char* pPacket, int nAddress, unsigned short nSpeed, unsigned char fDirection, int SpeedSteps);

};
#endif

static const char sccsid[]      = "@(#) $Workfile: SEND.CPP $$ $Revision: 39 $$";
static const char sccsid_h[]    = SEND_H_DECLARED;

// Func to eliminate bogus sccsid and sccsid_h declared but never used warning.
inline void dummy(const char *, const char *) {}

#define CMD_MSG		( printf( "Command mode (h for help, q to quit) >>" ) )

const char * const	DOCS_FILE	= "s_user.txt";	// User docs file.
#if SEND_VERSION >= 4
const int		MAXLINE			= 80;			// Maximum input line.
#else
const int		MAXLINE			= 256;			// Maximum input line.
#endif

/*
 *	Base I/O port number.
 */
#if SEND_VERSION < 4
u_short			Port_base = PORT_DEF;
#endif

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
#if SEND_VERSION >= 4		// break up so the print buffer can be smaller
const char *Key_help1 = "ESC - Return to command line       h - Print header\n";
const char *Key_help2 = "  c - Send single clock phase      C - Send series of clock phases\n";
const char *Key_help3 = "  u - Clear underflow              0 - Send zeros\n";
const char *Key_help4 = "  1 - Send ones                    a - Send scope A pattern\n";
const char *Key_help5 = "  b - Send scope B pattern         o - Send scope timing packet\n";
const char *Key_help6 = "  w - Send warble packets          S - Send stretched 0 pattern\n";
const char *Key_help7 = "  r - Send DCC reset packets       d - Send DCC packets\n";
const char *Key_help8 = "  D - Send stretched DCC packets   s - Change loco speed, acc. output\n";
const char *Key_help9 = "  e - Set speed to E-STOP          f - Change loco direction, acc. on/off\n";
const char *Key_help10 = "  E - Set speed to E_STOP(I)       t - Run self tests repeatedly\n";
const char *Key_help11 = "  z - Run decoder tests            i - Send DCC idle packets\n";
const char *Key_help12 = "  R - Send hard resets             g - Test generic I/O\n";
const char *Key_help13 = "  z - Run decoder tests            q - Quit program\n";
#else
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
#endif

static Key_val	get_key_cmd();
#if SEND_VERSION >= 4
	static void		parse_key_cmd( Key_val init_key );
#else
	static void		parse_key_cmd( Self_tst &self_tst, Key_val init_key );
#endif
static Rslt_t	do_self_tests( Self_tst	&self_tst );
static void		get_log_file( const char *cmd_name );
static void		print_user_docs();
static void		exit_send( int status );

/*
 *	Global copy of Send board registers.
 */
Send_reg 			Dcc_reg;

/*
 *	Self test object.
 */
static Self_tst		Lself_tst( Dcc_reg, St_vect, St_vect_size );

/*
 *	Decoder test object.
 */
static Dec_tst		Ldec_tst;

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

#if SEND_VERSION >= 4
int
send_main(
	int			argc,						// Count of args.
	char		**argv )					// Command line args.
#else
main(
	int			argc,						// Count of args.
	char		**argv )					// Command line args.
#endif
{
	const char	*my_name = "main";
	Key_val		init_key = KEY_DEC_TST;		// Initial key value.

    // Eliminate bogus sccsid and sccsid_h declared but never used warning.
    dummy( sccsid, sccsid_h );
    
#if SEND_VERSION >= 4
	#ifdef TRACK_LOCK_NOT_USED
		// ToDo - get the track lock info so it gets put back after Send finishes
		CloseTrack();
		(void)OpenTrack(TR_TESTER, TI_NONE, 0);
	#endif

	CabTestInProgress(1);
	SetTrackIdle(TI_NONE);
#endif

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
		#if SEND_VERSION < 4
			exit_send( 1 );
		#else
			exit_send( 1 );
			return 1;
		#endif
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

	/*
	 *	Set the global I/O port base address.
	 */
#if SEND_VERSION < 4
	/*
	 *	Set the global I/O port base address.
	 */
	Port_base	=	Args.get_port();
#endif

	if ( Args.get_manual_flag() == true )	// Manual mode.
	{
		init_key	=	KEY_NULL;
	}

	/*
	 *	Pass user arguements to decoder test object Ldec_tst.
	 */
#if SEND_VERSION < 4
	Ldec_tst.set_run_mask(     Args.get_run_mask() );
	Ldec_tst.set_clk_mask(     Args.get_clk_mask() );
#endif
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

		if ( Args.get_crit_flag() )
		{
			printf(		"Critical interrupt flag set\n" );
			STATPRINT(	"Critical interrupt flag set"  );
		}

		STATPRINT(	"Beginning self tests" );
		printf(		"Beginning self tests, this will take a few seconds\n" );

		if ( do_self_tests( Lself_tst ) == OK )
		{
			STATPRINT(	"Self tests passed" );
			printf(		"Self tests passed\n" );
			Lself_tst.print_stats();
			Dcc_reg.print();
		}
		else
		{
			printf(	                "%s: Self tests failed\n", my_name );
			ERRPRINT( my_name, LOG_ERR, "Self tests failed" );
			init_key	=	KEY_NULL;			// Don't start tests on fail.
		}
	}
	else
	{
		DBGLOG( "%s: Skipping self tests.", my_name );
	}

	/*
	 *	Set time for PC driven 1 usec delay for later use.
     *  Note: set_clk() is called to set nominal clock values
     *        when set_pc_delay_1usec is called below.
	 */
	if ( Dcc_reg.set_pc_delay_1usec() != OK )
	{
		STATPRINT(	"set_pc_delay_1usec() FAILED" );
		printf(		"set_pc_delay_1usec() FAILED\n" );
		init_key	=	KEY_NULL;			// Don't start tests automatically.
	}

#if SEND_VERSION >= 4
	parse_key_cmd( init_key );
#else
	parse_key_cmd( Lself_tst, init_key );
#endif

	if ( Dcc_reg.get_running() == true )
	{
		Dcc_reg.stop_clk();
	}


#if SEND_VERSION < 4
	Dcc_reg.rst_8255();
	exit_send( 0 );
#endif

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

	if ( Dcc_reg.get_running() == true )
	{
		ERRPRINT( my_name, LOG_WARNING,
			"exit_send() called with clock running." );
		Dcc_reg.stop_clk();
		Dcc_reg.rst_8255();
	}

    printf( "{SEND_END %d}\n", status );
    STATPRINT(	"{SEND_END %d}", status );

	Deflog.close_stat();
	Deflog.close_log();

#if SEND_VERSION < 4
	exit( status );
#else
	CabTestInProgress(0);
	SetTrackIdle(TI_IDLE);
	//exit( status );
	return;
#endif
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
		"Summary of command line and 'SEND.INI' switches:\n\n",
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

#if SEND_VERSION < 4
	rslt = nCheckSumFile(NULL, cmd_name, &nCRC, &nLength );

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
#else
	rslt = nCheckSumFile(NULL, cmd_name, &nCRC, &nLength );

//k	if ( rslt != 0 )
//k    {
//k		ERRPRINT( my_name, LOG_CRIT,
//k			"nCheckSumFile() failed" );
//k		exit_send( 1 );
//k    }

    printf(		"Test software version %c.%u.%u.%u\n",
		Ver_rel, Ver_maj, Ver_min, Ver_bld );
#endif


	#if SEND_VERSION >= 4
		printf("Enter base of log and statistics file name > ");
	#else
		fputs( "Enter base of log and statistics file name > ", stdout );
	#endif

	#if SEND_VERSION >= 4
		if ( !getstr(  buf, MAXLINE ) )
	#else
		if ( !fgets(  buf, MAXLINE, stdin ) )
	#endif
	{
		ERRPRINT( my_name, LOG_CRIT,
			"fgets() failed" );
	#if SEND_VERSION < 4
		exit_send( 1 );
	#else
		exit_send( 1 );
		return;
	#endif
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
		#if SEND_VERSION < 4
			exit_send( 1 );
		#else
			exit_send( 1 );
			return;
		#endif
	}

	strcpy( fname, buf );
	strcat( fname, ".sum" );
	if ( Deflog.open_stat( fname ) != OK )
	{
		ERRPRINT( my_name, LOG_CRIT,
			"Statistics file <%s> could not be opened", buf );
	#if SEND_VERSION < 4
		exit_send( 1 );
	#else
		exit_send( 1 );
		return;
	#endif
	}

	STATPRINT( "BEGINNING decoder test log" );
	STATPRINT(	"Test software version %c.%u.%u.%u",
		Ver_rel, Ver_maj, Ver_min, Ver_bld );
    STATPRINT(	"File <%s>",
    	cmd_name );
    STATPRINT(	"CRC %lu, Length %lu",
    	nCRC, nLength );

#if SEND_VERSION >= 4
	for ( i = 0; i < (int)QUESTIONS_SIZE; i++ )
#else
	for ( i = 0; i < QUESTIONS_SIZE; i++ )
#endif
	{
		#if SEND_VERSION >= 4
			printf(Questions[i].question);
		#else
			fputs( Questions[i].question, stdout );
		#endif

		#if SEND_VERSION >= 4
			if ( !getstr(  buf, MAXLINE ) )
		#else
			if ( !fgets(  buf, MAXLINE, stdin ) )
		#endif
		{
			ERRPRINT( my_name, LOG_CRIT,
				"fgets() failed" );
			#if SEND_VERSION < 4
				exit_send( 1 );
			#else
				exit_send( 1 );
				return;
			#endif
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
		#if SEND_VERSION >= 4
			if ( !getstr(  buf, MAXLINE ) )
		#else
			if ( !fgets(  buf, MAXLINE, stdin ) )
		#endif
		{
			ERRPRINT( my_name, LOG_CRIT,
				"fgets() failed" );
			#if SEND_VERSION < 4
				exit_send( 1 );
			#else
				exit_send( 1 );
				return;
			#endif
		}

		if ( !strcmp( buf, ".\n" ) ||  !strcmp( buf, ".\r" ) )
		{
			break;
		}

		TO_STAT( "%s", buf );
	}
    puts(    "--------------------------------" );
	TO_STAT( "--------------------------------\n\n" );

	// Log summary of command line switches and .INI file
	if ( (fp = Deflog.get_fp_log()) != NULL )
	{
    	fputs(
    		"Summary of command line and 'SEND.INI' switches:\n\n",
			fp );
		Args.usage( fp );
        fputc( '\n', fp );
		fflush( fp );
	}

	if ( (fp = Deflog.get_fp_stat()) != NULL )
	{
    	fputs(
    		"Summary of command line and 'SEND.INI' switches:\n\n",
			fp );
		Args.usage( fp );
        fputc( '\n', fp );
		fflush( fp );
	}

	if ( Deflog.get_stderr_too() == true )
	{
    	fp = stderr;
    	fputs(
    		"Summary of command line and 'SEND.INI' switches:\n\n",
			fp );
		Args.usage( fp );
        fputc( '\n', fp );
	}
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		do_self_tests()						-	 Run multiple self tests.
 *
 *	RETURN VALUE
 *
 *		OK		-	All tests passed.
 *		FAIL	-	Error in test.
 *
 *	DESCRIPTION
 *
 *		do_self_tests() runs the board self tests.
 */
/*--------------------------------------------------------------------------*/

static Rslt_t
do_self_tests(
	Self_tst	&self_tst )					// Self test object.
{
	const char	*my_name = "do_self_tests";
	bool		tstderr_too;				// Present stderr_too flag.
	Rslt_t		retval = OK;				// Return value.

	tstderr_too	=	Deflog.get_stderr_too();
	Deflog.set_stderr_too( true );

	return ( retval );



	if ( self_tst.test_send() != OK )
	{
		ERRPRINT( my_name, LOG_ERR, "Self test FAILED" );
		retval	=	FAIL;
	}

	Deflog.set_stderr_too( tstderr_too );

	CLR_LINE;
	ERRPRINT( my_name, LOG_INFO,
		"%10ld Cycles passed, %10ld Cycles failed",
		self_tst.get_tst_pass(), self_tst.get_tst_fail() );
	printf(
		"%10ld Cycles passed, %10ld Cycles failed\r",
		self_tst.get_tst_pass(), self_tst.get_tst_fail() );

	Dcc_reg.clear_warn();

	return ( retval );
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
#if SEND_VERSION >= 4
static void
	parse_key_cmd(
	Key_val			init_key )					// Initial key value.
{
#else
static void
	parse_key_cmd(
	Self_tst		&self_tst,					// Self test object.
	Key_val			init_key )					// Initial key value.
{
#endif
	const char	*my_name = "parse_key_cmd";
	int			i;							// Index value.
	Key_val		key_val;					// Present key value.
#if SEND_VERSION < 4
	bool		clk_state;					// Present SW clock value.
#endif
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
#if SEND_VERSION < 4
    static BYTE	byGen = 0x15;				// Generic output BYTE.
#endif

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
			Dcc_reg.set_clk(	DECODER_0T_NOM,
								DECODER_0H_NOM,
								DECODER_1T_NOM );
		}

		switch ( key_val )					// Parse the key command.
		{
		case KEY_ESC:						// Stop anything in progress.
			rep_type	=	SEND_NONE;
			if ( Dcc_reg.get_running() == true )
			{
				Dcc_reg.stop_clk();
			}
			printf( "Returning to command line\n" );
			break;

		case KEY_HEADER:					// Print the help message header.
			#if SEND_VERSION >= 4
				printf( "%s", Key_help1 );
				printf( "%s", Key_help2 );
				printf( "%s", Key_help3 );
				printf( "%s", Key_help4 );
				printf( "%s", Key_help5 );
				printf( "%s", Key_help6 );
				printf( "%s", Key_help7 );
				printf( "%s", Key_help8 );
				printf( "%s", Key_help9 );
				printf( "%s", Key_help10 );
				printf( "%s", Key_help11 );
				printf( "%s", Key_help12 );
				printf( "%s", Key_help13 );
			#else
				//printf( "%s", Key_help );
				printf( "%s", Args.get_key_help() );
			#endif

			Dcc_reg.up_print( true );  		// Print out hardware state.
			if ( Dcc_reg.get_running() == true )
			{
				Dcc_reg.clr_under();
			}
			break;

		/*
		 *	The following commands are for low level board debugging
		 *	and use the software generated clock.
		 */
		case KEY_CLOCK:						// Change static clock phase.
			#if SEND_VERSION < 4
				rep_type	=	SEND_NONE;
				if ( Dcc_reg.get_running() == true )
				{
					Dcc_reg.stop_clk();
				}
				clk_state	=	inportb( PC ) & PC_MSK_CPUCLKL ? true : false;
				clk_state	=	!( clk_state );
				OUT_PC( PC_POS_CPUCLKL, clk_state );
				printf( "Toggled SW clock to %d\n", clk_state );
				Dcc_reg.up_print();
			#endif
			break;

		case KEY_MCLK:						// Send a series of static clocks.
			rep_type	=	SEND_MCLK;
			if ( Dcc_reg.get_running() == true )
			{
				Dcc_reg.stop_clk();
			}
			OUT_PC( PC_POS_CPUCLKL, 0 );
			printf( "Sending SW clocks\n" );
			Dcc_reg.up_print();
			break;

		case KEY_UNDER:						// Clear the underflow flag.
			rep_type	=	SEND_NONE;
			if ( Dcc_reg.get_running() == true )
			{
				Dcc_reg.stop_clk();
			}

		    if ( !Args.get_log_pkts() )
			{
				OUT_PC( PC_POS_UNDERCLRL, 1 );
				OUT_PC( PC_POS_UNDERCLRL, 0 );
				OUT_PC( PC_POS_UNDERCLRL, 1 );
			}
			else
			{
				Dcc_reg.clr_under();
			}
			printf( "Underflow reset\n" );
			Dcc_reg.up_print();
			break;

		case KEY_GEN	:					// Toggle GEN_2, read inputs.
			#if SEND_VERSION < 4
        		outportb( GEN_OUT, byGen );
        		printf( "Gen Out 0x%02x - Gen 1 0x%02x, Gen 2 0x%02x\n",
        				byGen, inportb( GEN ), inportb( GEN_2 ) );
            byGen	=	(~byGen) &  GEN_OUT_MASK;
			if ( Dcc_reg.get_running() == true )
			{
				Dcc_reg.clr_under();
			}
			#endif
			break;

		/*
		 *	The following commands are for board and fixture debugging
		 *	and use the hardware generated clock.
		 */
		case KEY_ZERO:						// Send zero bytes.
			rep_type	=	SEND_ZERO;
			if ( Dcc_reg.get_running() != true )
			{
				Dcc_reg.start_clk();
			}
			printf( "Sending Zeros\n" );
			Dcc_reg.clr_under();
			break;

		case KEY_ONE:						// Send one bytes.
			rep_type	=	SEND_ONE;
			if ( Dcc_reg.get_running() != true )
			{
				Dcc_reg.start_clk();
			}
			printf( "Sending Ones\n" );
			Dcc_reg.clr_under();
			break;

		case KEY_SCOPE_A:					// Send the scope A test.
			rep_type	=	SEND_SCOPE_A;
			if ( Dcc_reg.get_running() != true )
			{
				Dcc_reg.start_clk();
			}
			printf( "Sending scope test A (0 then 1)\n" );
			Dcc_reg.clr_under();
			break;

		case KEY_SCOPE_B:					// Send the scope B test.
			rep_type	=	SEND_SCOPE_B;
			if ( Dcc_reg.get_running() != true )
			{
				Dcc_reg.start_clk();
			}
			printf( "Sending scope test B (1 then 0)\n" );
			Dcc_reg.clr_under();
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

			if ( Dcc_reg.get_running() != true )
			{
				Dcc_reg.start_clk();
			}
			printf( "Sending scope timing test with period %5d\n",
					Scope_time_tbl[ o_time_index ] );
			Dcc_reg.set_clk(	Scope_time_tbl[ o_time_index ],
								Scope_time_tbl[ o_time_index ] / 2,
								DECODER_1T_NOM );
			Dcc_reg.clr_under();
			break;

		case KEY_WARBLE:				  	// Send warble packets.
			rep_type	=	SEND_WARBLE;
			if ( Dcc_reg.get_running() != true )
			{
				Dcc_reg.start_clk();
			}
			printf( "Sending warble packets\n" );
			Dcc_reg.clr_under();
			break;

		case KEY_STRETCHED:					// Send the stretched 0 test.
			rep_type	=	SEND_STRETCHED;
			if ( Dcc_reg.get_running() != true )
			{
				Dcc_reg.start_clk();
			}
			printf( "Sending stretched 0s (0xff then 0x02)\n" );
			Dcc_reg.clr_under();
			break;

		/*
		 *	The following commands are used to manually test the decoder.
		 */
		case KEY_RESET:						// Send reset packets.
			rep_type	=	SEND_RESET;
			if ( Dcc_reg.get_running() != true )
			{
				Dcc_reg.start_clk();
			}
			printf( "Sending Reset packets\n" );
			Dcc_reg.clr_under();
			break;

		case KEY_HARD:						// Send hard reset packets.
			rep_type	=	SEND_HARD;
			if ( Dcc_reg.get_running() != true )
			{
				Dcc_reg.start_clk();
			}
			printf( "Sending HARD Reset packets\n" );
			Dcc_reg.clr_under();
			break;

		case KEY_IDLE:						// Send idle packets.
			rep_type	=	SEND_IDLE;
			if ( Dcc_reg.get_running() != true )
			{
				Dcc_reg.start_clk();
			}
			printf( "Sending Idle packets\n" );
			Dcc_reg.clr_under();
			break;

		/*
		 *	Send DCC packets.  The direction and speed
		 *	are changed by the set of commands that follow this one.
		 */
		case KEY_DCC:
		case KEY_DCC_S:
			rep_type	=	(key_val == KEY_DCC) ? SEND_DCC : SEND_DCC_S;
			if ( Dcc_reg.get_running() != true )
			{
				Dcc_reg.start_clk();
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
			Dcc_reg.clr_under();
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
//k					Dcc_reg.gen_print();
					putchar( '\n' );
                }
			}
			else
			{
				printf(
					"Press 'd' or 'D' key first to use this command\n" );
			}
			Dcc_reg.clr_under();
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
			Dcc_reg.clr_under();
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
			Dcc_reg.clr_under();
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
//k					Dcc_reg.gen_print();
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
//k					Dcc_reg.gen_print();
					putchar( '\n' );
				}
			}
			else
			{
				printf(
					"Press 'd' key first to use this command\n" );
			}
			Dcc_reg.clr_under();
			break;

		/*
		 *	The following commands, KEY_SELF_TST & KEY_DEC_TST, run
		 *	for a significant period of time and print messages to
		 *	the screen.
		 */
		case KEY_SELF_TST:				// Run repeated self tests.
			rep_type	=	SEND_SELF_TST;
			if ( Dcc_reg.get_running() == true )
			{
				Dcc_reg.stop_clk();
			}
//k			self_tst.rst_stats();
			printf(
				"Starting self test cycles, type '<ESC>' to stop tests\n" );
			break;

		case KEY_DEC_TST:				// Run one or more decoder test cycles.
			rep_type	=	SEND_DEC_TST;
			if ( Dcc_reg.get_running() != true )
			{
				Dcc_reg.start_clk();
			}
			printf(
				"Starting Decoder tests, type '<ESC> q' to stop tests\n" );
			Dcc_reg.clr_under();
			break;

		case KEY_KICK:					// Run kickstart followed by idles.
			if ( Args.get_decoder_type() == DEC_FUNC )
            {
				printf( "Starting loco kickstart to enable functions\n" );
				
				rep_type	=	SEND_IDLE;
				if ( Dcc_reg.get_running() != true )
				{
					Dcc_reg.start_clk();
				}
				Dcc_reg.kick_start( Dcc_bits, Args );
				Dcc_reg.clr_under();
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
			if ( Dcc_reg.get_running() == true )
			{
				Dcc_reg.stop_clk();
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
					#if SEND_VERSION < 4
						for ( i = 10000;  i > 0; i-- )
						{
							OUT_PC( PC_POS_CPUCLKL, 1 );
							OUT_PC( PC_POS_CPUCLKL, 0 );
						}
					#endif
					break;

				case SEND_ZERO:				// Repeat zero bytes.
					Dcc_reg.set_scope( true );
					Dcc_reg.send_bytes( 2, 0x00, "SEND_ZERO." );
					break;

				case SEND_ONE:				// Repeat one bytes.
					Dcc_reg.set_scope( true );
					Dcc_reg.send_bytes( 2, 0xff, "SEND_ONE." );
					break;

				case SEND_SCOPE_A:			// Repeat scope A (0-1) test.
					Dcc_bits.clr_in();
					Dcc_bits.put_byte( 0x00 ).put_byte( 0xff ).done();
					Dcc_reg.set_scope( true );
					Dcc_reg.send_pkt( Dcc_bits, "SEND_SCOPE_A." );
					break;

				case SEND_SCOPE_B:			// Repeat scope B (1-0) test.
					Dcc_bits.clr_in();
					Dcc_bits.put_byte( 0xff ).put_byte( 0x00 ).done();
 					Dcc_reg.set_scope( true );
					Dcc_reg.send_pkt( Dcc_bits, "SEND_SCOPE_B." );
					break;

				case SEND_WARBLE:	 		// Repeat the warble packet.
					Dcc_reg.set_scope( true );
					Dcc_reg.send_bytes( 3072, 0xff, "SEND_WARBLE 0xff." );
					Dcc_reg.send_bytes( 1024, 0x00, "SEND_WARBLE 0x00." );
					break;

				case SEND_STRETCHED:   		// Repeat stretched 0 test.
					Dcc_bits.clr_in();
					Dcc_bits.put_byte( 0xff ).put_byte( 0x02 ).done();
					Dcc_bits.rst_out();
					Dcc_bits.get_byte( pbyte );
					Dcc_reg.set_scope( true );
					Dcc_reg.send_bytes( 1, pbyte, "SEND_STRETCHED preamble." );
					Dcc_bits.get_byte( pbyte );
					Dcc_reg.send_stretched_byte(	DECODER_0T_NOM * 4,
													DECODER_0H_NOM * 2,
													pbyte,
													"SEND_STRETCHED 0 bit." );
					break;

				case SEND_RESET:			// Repeat the reset packet.
					Dcc_reg.send_bytes( 1, 0x00, "SEND_RESET." );
					Dcc_reg.set_scope( true );
					Dcc_reg.send_rst();
					for ( i = 0; i < 15; i++ )	// Send 15 normal idles.
					{
						Dcc_reg.send_idle();
					}
					break;

				case SEND_HARD:			// Repeat the hard reset packet.
					Dcc_reg.send_bytes( 1, 0x00, "SEND_HARD." );
					Dcc_reg.set_scope( true );
					Dcc_reg.send_hard_rst();	// Send the hard reset.
					for ( i = 0; i < 15; i++ )	// Send 15 normal idles.
					{
						Dcc_reg.send_idle();
					}
					break;

				case SEND_IDLE:				// Repeat the idle packet.
                	Dcc_bits.clr_in();
                    Dcc_bits.put_idle_pkt( 4 );
                    Dcc_bits.put_1s( 1 ).done();

					Dcc_reg.send_bytes( 1, 0x00, "SEND_IDLE." );
					Dcc_reg.set_scope( true );
					Dcc_reg.send_pkt( Dcc_bits, "SEND_IDLE." );
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

					Dcc_reg.send_bytes( 1, 0x00, "SEND_DCC Send 0." );
					Dcc_reg.set_scope( true );
					Dcc_reg.send_pkt( Dcc_bits, "SEND_DCC Send pkt." );
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
					Dcc_reg.send_bytes( 1, pbyte, "SEND_DCC_S preamble 1." );
					Dcc_bits.get_byte( pbyte );
					Dcc_reg.send_bytes( 1, pbyte, "SEND_DCC_S preamble 2." );
                    /*
                     * Send single stretched 0 just after preamble.
                     */
					Dcc_reg.set_scope( true );
					Dcc_bits.get_byte( pbyte );
					Dcc_reg.send_stretched_byte( DECODER_0T_MAX,
                                                 DECODER_0T_MAX-DECODER_0H_MAX,
                                                 pbyte,
                                                 "SEND_DCC_S stretched 0." );
					/*
					 * Send remaining part of packet.
                     */
					while ( Dcc_bits.get_byte( pbyte ) == OK )
					{
						Dcc_reg.send_bytes( 1, pbyte, "SEND_DCC_S end." );
					}
					break;

				case SEND_SELF_TST:		  	// Run board self tests.
//k					do_self_tests( self_tst );
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
						if ( Dcc_reg.get_running() == true )
						{
							Dcc_reg.stop_clk();
						}
						Dcc_reg.set_clk(	DECODER_0T_NOM,
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
							#if SEND_VERSION >= 4
								exit_send(0);
							#endif
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
//k				self_tst.print_stats();
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

#if SEND_VERSION >= 4		//Add accessor methods

void SetDccTiming(void)
{
	//Dcc_reg.set_clk( DECODER_0T_NOM, DECODER_0H_NOM, DECODER_1T_NOM );
	Dcc_reg.init_8254();
}

void SendZero(void)
{
	Dcc_reg.set_scope( true );
	Dcc_reg.send_bytes( 2, 0x00, "SEND_ZERO." );
}


void SendOne(void)
{
	Dcc_reg.set_scope( true );
	Dcc_reg.send_bytes( 2, 0xff, "SEND_ONE." );
}

void SendScopeA(void)
{

	if ( Dcc_reg.get_running() != true )
	{
		Dcc_reg.start_clk();
	}
	//printf( "Sending scope test A (0 then 1)\n" );
	//OUT_PC( PC_POS_UNDERCLRL, 0 );
	Dcc_bits.clr_in();
	Dcc_bits.put_byte( 0x00 ).put_byte( 0xff ).done();
	Dcc_reg.set_scope( true );
	Dcc_reg.send_pkt( Dcc_bits, "SEND_SCOPE_A." );
}

void SendScopeB(void)
{
	if ( Dcc_reg.get_running() != true )
	{
		Dcc_reg.start_clk();
	}
	//printf( "Sending scope test B (1 then 0)\n" );
	//OUT_PC( PC_POS_UNDERCLRL, 0 );
	Dcc_bits.clr_in();
	Dcc_bits.put_byte( 0xff ).put_byte( 0x00 ).done();
	Dcc_reg.set_scope( true );
	Dcc_reg.send_pkt( Dcc_bits, "SEND_SCOPE_B." );
}

void SendWarble(void)
{
	Dcc_reg.set_scope( true );
	Dcc_reg.send_bytes( 3072, 0xff, "SEND_WARBLE 0xff." );
	Dcc_reg.send_bytes( 1024, 0x00, "SEND_WARBLE 0x00." );
}

void SendStretched(void)
{
	BYTE		pbyte;						// Present byte to send.

	Dcc_bits.clr_in();
	Dcc_bits.put_byte( 0xff ).put_byte( 0x02 ).done();
	Dcc_bits.rst_out();
	Dcc_bits.get_byte( pbyte );
	Dcc_reg.set_scope( true );
	Dcc_reg.send_bytes( 1, pbyte, "SEND_STRETCHED preamble." );
	Dcc_bits.get_byte( pbyte );
	Dcc_reg.send_stretched_byte(	DECODER_0T_NOM * 4,	DECODER_0H_NOM * 2,	pbyte, "SEND_STRETCHED 0 bit." );
}

void SendReset(void)
{
	Dcc_reg.send_bytes( 1, 0x00, "SEND_RESET." );
	Dcc_reg.set_scope( true );
	Dcc_reg.send_rst();
	//for (int i = 0; i < 15; i++ )	// Send 15 normal idles.
	//{
	//  Dcc_reg.send_idle();
	//}
}

void SendHard(void)
{
	Dcc_reg.send_bytes( 1, 0x00, "SEND_HARD." );
	Dcc_reg.set_scope( true );
	Dcc_reg.send_hard_rst();	// Send the hard reset.
	//for (int i = 0; i < 15; i++ )	// Send 15 normal idles.
	//{
	//	Dcc_reg.send_idle();
	//}
}

void SendIdle(void)
{
	//Dcc_bits.clr_in();
	//Dcc_bits.put_idle_pkt( 4 );
	//Dcc_bits.put_1s( 1 ).done();

	//Dcc_reg.send_bytes( 1, 0x00, "SEND_IDLE." );
	//Dcc_reg.set_scope( true );
	//Dcc_reg.send_pkt( Dcc_bits, "SEND_IDLE." );

	Dcc_reg.send_bytes( 1, 0x00, "SEND_IDLE." );
	Dcc_reg.set_scope( true );
	Dcc_reg.send_idle();

}
#endif

/*****************************************************************************
 * $History: SEND.CPP $
 * 
 * *****************  Version 39  *****************
 * User: Kenw         Date: 9/12/23    Time: 8:04p
 * Updated in $/NMRA/SRC/SEND
 * Changed TO_LOG() to DBGLOG() in main().
 * 
 * *****************  Version 38  *****************
 * User: Kenw         Date: 8/21/23    Time: 8:36p
 * Updated in $/NMRA/SRC/SEND
 * Removed duplicate Ldec_tst.set_trig_rev() call.
 * 
 * *****************  Version 37  *****************
 * User: Kenw         Date: 8/14/23    Time: 6:50p
 * Updated in $/NMRA/SRC/SEND
 * Added setting for Dec_tst::decoder_type and
 * Dec_tst::trig_rev.
 * 
 * *****************  Version 36  *****************
 * User: Kenw         Date: 8/06/23    Time: 6:32p
 * Updated in $/NMRA/SRC/SEND
 * Replaced put_func_grp_pkt() with
 * put_func_grp_pktx() in stretched DCC
 * for function packets.
 * 
 * *****************  Version 35  *****************
 * User: Kenw         Date: 8/06/23    Time: 4:47p
 * Updated in $/NMRA/SRC/SEND
 * Added  updated put_func_grp_pktx() method to  manual 'd' command
 * for the dunction type.
 * 
 * *****************  Version 34  *****************
 * User: Kenw         Date: 7/22/23    Time: 9:25p
 * Updated in $/NMRA/SRC/SEND
 * Changed initial manual 'd' command loco speed to 0.
 * 
 * *****************  Version 33  *****************
 * User: Kenw         Date: 7/04/23    Time: 8:30p
 * Updated in $/NMRA/SRC/SEND
 * Changed parse_key_cmd() SEND_DCC repeat command
 * to use 
 * Bits::put_func_grp_pktx() rather than
 * Bits::put_func_grp_pkt()
 * Skipped preceding put_cmd_pkt_28() if put_func_grp_pktx()
 * will repeat put_cmd_pkt_28().
 * 
 * *****************  Version 32  *****************
 * User: Kenw         Date: 6/23/23    Time: 9:25p
 * Updated in $/NMRA/SRC/SEND
 * Added manual keyboard command list to -u usage information.
 * 
 * *****************  Version 31  *****************
 * User: Kenw         Date: 6/22/23    Time: 9:08p
 * Updated in $/NMRA/SRC/SEND
 * Changed "OUT_PC( PC_POS_UNDERCLRL, 0 )" to
 * "Dcc_reg.clr_under()" when logging output.
 * 
 * *****************  Version 29  *****************
 * User: Kenw         Date: 5/29/23    Time: 6:19p
 * Updated in $/NMRA/SRC/SEND
 * Added note above pc_delay() call in main() ti indicate
 * it calls setclock() iwhch sets t nominal clock values.
 * 
 * *****************  Version 28  *****************
 * User: Kenw         Date: 9/19/20    Time: 4:22p
 * Updated in $/NMRA/SRC/SEND
 * Rolling to release B.5.10.2
 * 
 * *****************  Version 27  *****************
 * User: Kenw         Date: 9/19/20    Time: 3:42p
 * Updated in $/NMRA/SRC/SEND
 * Rolled to X.5.10.2
 * 
 * *****************  Version 26  *****************
 * User: Kenw         Date: 3/26/20    Time: 9:20p
 * Updated in $/NMRA/SRC/SEND
 * Intermediate build to 5.10.1
 * Changed <SEND...> to {SEND...}.
 * 
 * *****************  Version 25  *****************
 * User: Kenw         Date: 3/20/20    Time: 9:43p
 * Updated in $/NMRA/SRC/SEND
 * Intermediate build to 5.10.1
 * Cleaned up 1 message string.
 * 
 * *****************  Version 24  *****************
 * User: Kenw         Date: 7/24/19    Time: 7:18p
 * Updated in $/NMRA/SRC/SEND
 * Check in prior to B.5.9.3 release.
 * 
 * *****************  Version 23  *****************
 * User: Kenw         Date: 12/05/18   Time: 4:55p
 * Updated in $/NMRA/SRC/SEND
 * Added 15 Idle packets after Reset packet in manual mode.
 * 
 * *****************  Version 22  *****************
 * User: Kenw         Date: 7/03/18    Time: 7:28p
 * Updated in $/NMRA/SRC/SEND
 * Intermin Check in X.5.8.3
 * 
 * *****************  Version 21  *****************
 * User: Kenw         Date: 4/17/17    Time: 8:39p
 * Updated in $/NMRA/SRC/SEND
 * Added Booster Type to initial questions.
 * 
 * *****************  Version 19  *****************
 * User: Kenw         Date: 9/17/14    Time: 4:43p
 * Updated in $/NMRA/SRC/SEND
 * Added function decoder support for manual mode.
 * Modified decoder quiestions.
 * Print <SEND_END n> when program exits.
 * 
 * *****************  Version 17  *****************
 * User: Kenw         Date: 5/16/14    Time: 9:37p
 * Updated in $/NMRA/SRC/SEND
 * Added Ver-bld number to Ver_maj & Ver_min
 * 
 * *****************  Version 16  *****************
 * User: Kenw         Date: 3/30/14    Time: 3:18p
 * Updated in $/NMRA/SRC/SEND
 * Added printf statements indicating test state for interrupted test
 * runs.
 * 
 * *****************  Version 15  *****************
 * User: Kenw         Date: 6/06/04    Time: 2:26p
 * Updated in $/NMRA/SRC/SEND
 * Extended -E command to add preambles to Ramp, Address, and bits
 * test.  Changed manual -i and -d command to send 4 packet sequences
 * to help debug LV102
 * 
 * *****************  Version 14  *****************
 * User: Kenw         Date: 2/01/04    Time: 6:28p
 * Updated in $/NMRA/SRC/SEND
 * Updated to B4.3.
 * Includes support for checksum calculation and logging
 * of command line switches.
 * 
 * *****************  Version 13  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:57p
 * Updated in $/NMRA/SRC/SEND
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 11  *****************
 * User: Kenw         Date: 1/09/99    Time: 1:51p
 * Updated in $/NMRA/SRC/SEND
 * Updated for B.2.15
 * Changed to print out current hardware state when header is printed.
 * 
 * *****************  Version 10  *****************
 * User: Kenw         Date: 6/28/98    Time: 9:38a
 * Updated in $/NMRA/SRC/SEND
 * Changed to abort program at LOG_ERR. Rolling to X.2.12.
 * 
 * *****************  Version 9  *****************
 * User: Kenw         Date: 5/30/98    Time: 9:11p
 * Updated in $/NMRA/SRC/SEND
 * Added support for manual hard reset 'R' command.
 * Rolled to X.2.11
 * 
 * *****************  Version 8  *****************
 * User: Kenw         Date: 10/29/97   Time: 4:26p
 * Updated in $/NMRA/SRC/SEND
 * X.2.8 - Added packet log command.
 * 
 * *****************  Version 7  *****************
 * User: Kenw         Date: 10/28/97   Time: 2:38p
 * Updated in $/NMRA/SRC/SEND
 * Added Prior Packet Test & Idle manual command.
 * 
 * *****************  Version 6  *****************
 * User: Kenw         Date: 7/26/96    Time: 9:04a
 * Updated in $/NMRA/SRC/SEND
 * Changes for version 2.2
 * Added support for 'o' oscilloscope setup  command.
 * Renamed 1/2 fast and 1/2 slow to command station max/min
 * 
 * *****************  Version 5  *****************
 * User: Kenw         Date: 5/12/96    Time: 1:17p
 * Updated in $/NMRA/SRC/SEND
 * Added some comments and shortened some others to fit on a single line.
 * 
 * *****************  Version 4  *****************
 * User: Kenw         Date: 5/10/96    Time: 9:34a
 * Updated in $/NMRA/SRC/SEND
 * Added support for print_user_docs().
 * Cleaned up manual tests based on review.
 * Cleaned up formatting.
 * 
 * *****************  Version 3  *****************
 * User: Kenw         Date: 4/30/96    Time: 10:05p
 * Updated in $/NMRA/SRC/SEND
 * Put back SEND_TEST state and command I accidentally erased.
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 4/30/96    Time: 8:29p
 * Updated in $/NMRA/SRC/SEND
 * Added changes to support single stretched single 0 test.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 3:58p
 * Created in $/NMRA/SRC/SEND
 * File containing source for main() and other primary functions for
 * decoder
 * test program.
 *
 *****************************************************************************/

