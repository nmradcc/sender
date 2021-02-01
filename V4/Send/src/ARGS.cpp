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
 * Current version:      $Revision: 22 $
 * Last Modified on:     $Modtime: 10/1/2019 $
 *               by:     $Author: KKobel $
 * Last checkin on:      $Date: 7/03/18 7:28p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	args.cpp	-	This file contains the command line and ini file argument
 *					processing routines.
 *
 *****************************************************************************/

#include <ARGS.h>
#include "pds601.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#if SEND_VERSION == 4
	extern void getcwd(char* pPath, int len);
#endif

static const char sccsid[]      = "@(#) $Workfile: ARGS.CPP $$ $Revision: 21 $$";
static const char sccsid_h[]    = ARGS_H_DECLARED;

// Func to eliminate bogus sccsid and sccsid_h declared but never used warning.
inline void dummy(const char *, const char *) {}

const char		SEPCHARU	  	= '/';					// UNIX character.
const char		SEPCHARM	  	= '\\';			   		// MSDOS character.
const u_short	DEF_LOCO_ADDR	= 3;  					// Default LOCO addr.
const u_short	DEF_FUNC_ADDR	= 3;  					// Default FUNC addr.
const u_short	DEF_ACC_ADDR  	= 1;  					// Default ACC addr.
const u_short	DEF_SIG_ADDR  	= 1;  					// Default SIG addr.
const int		MAX_INI_LINE  	= 80; 					// Max ini line.
const u_int		MAX_FILL_MSEC 	= 10000; 				// Max fill time.
const u_int		MIN_TEST_REPS	= 1;					// Min test repeats.
const u_int		MAX_TEST_REPS	= 1000;					// Max test repeats.

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
	ini_path[0]			= '\0';				// Ini file path name.
	ini_fp				= NULL;				// Ini FILE pointer.
	decoder_address		= 0;				// Decoder address.
	address_set			= false;			// Address was set by user.
	port				= PORT_DEF;			// Base I/O port.
	decoder_type		= DEC_LOCO;	 		// Decoder type.
	crit_flag			= false;			// Protect critical regions.
	fragment_flag  		= false;			// Send all fragment sizes.
	rep_flag			= false;	  		// Repeat decoder tests.
	run_mask			= DEF_RUN_MASK;		// Tests to run.
	clk_mask			= DEF_CLK_MASK;	  	// Clocks to try.
	manual_flag			= false;			// Run tests manually.
	extra_preamble		= 0;				// Extra margin test preambles.
	trig_rev			= false;			// Use E_STOP as trigger command.
	fill_msec			= MSEC_PER_SEC;		// Use 1 second fill time.
	test_repeats		= 2;				// Times to repeat misc. tests.
	print_user			= false;			// Don't print user docs.
	log_pkts			= false;			// Don't send packets to log.
	m_no_abort			= false;			// Stop on error.
	m_late_scope		= false;			// Use normal scope trigger.
	m_ambig_addr_same  	= false;			// Use different address.
    aspect_preset		= 0;				// Sig decoder aspect preset.
    aspect_trigger		= 8;				// Sig decoder aspect trigger.
    m_lamp_rear			= false;			// Use forward lamp.

    // Eliminate bogus sccsid and sccsid_h declared but never used warning.
    dummy( sccsid, sccsid_h );
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
	if ( ini_fp )
	{
		fclose( ini_fp );
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
		"Usage:    %s [-?] [-u] [-m] [-a addr] [-d l|f|a|s] [-n pre] [-N trig]\n",
		cmd_name );
	fprintf( ofp,
		"               [-l] [-p port] [-f] [-x] [-r] [-t mask] [-c mask] [-E pre]\n"
		"               [-T] [-F fill] [-R reps] [-P] [-A] [-s] [-S]\n" );

	if ( ini_path[0] != '\0' )
	{
		fprintf( ofp, "Ini file: <%s>\n", ini_path );
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
		"  -p <port>  PORT       "
		"I/O Port                             <value 0x%04x>\n",
		port );
	fprintf( ofp,
		"  -f         FRAGMENT   "
		"Test all fragments                   <value %s>\n",
		fragment_flag		? "true" : "false" );
	fprintf( ofp,
		"  -x         CRITICAL   "
		"Protect critical regions             <value %s>\n",
		crit_flag		? "true" : "false" );
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
		"  -E <pre>   EXTRA_PRE  "
		"Extra margin test preamble bits      <value %u>\n",
		extra_preamble );
	fprintf( ofp,
		"  -T         TRIG_REV   "
		"Use reverse as trigger command       <value %s>\n",
		trig_rev	 	? "true" : "false" );
	fprintf( ofp,
		"  -F <fill>  FILL_MSEC  "
		"Fill time in milliseconds            <value %u>\n",
		fill_msec );
	fprintf( ofp,
		"  -R <reps>  TEST_REPS  "
		"Non packet acceptance test repeats   <value %u>\n",
		test_repeats );
	fprintf( ofp,
		"  -P         LOG_PKTS   "
		"Send packets to log, not hardware    <value %s>\n",
		log_pkts		? "true" : "false" );
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
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		get_args()		  				-	 Get command line & ini args.
 *
 *	RETURN VALUE
 *
 *		OK		-	Args_obj obtained without error.
 *		FAIL	-	Problem getting args.
 *
 *	DESCRIPTION
 *
 *		get_args() scans for ini file and command line arguments.
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

	if ( iniopen( argv[0], "SEND.INI", "SEND_INI" ) == OK )
	{
		retval	=	iniscan();
		fclose( ini_fp );
		ini_fp	=	NULL;
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

	for ( i = 1; i < argc; i++ )
	{
		if ( strcmp( argv[i], "-?" ) == 0 )			// Print usage message.
		{
			return ( FAIL );
		}
		if ( strcmp( argv[i], "-u" ) == 0 )			// Print user docs.
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
				fprintf( stderr, "-a must have an address argument\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-p" ) == 0 )	// I/O port.
		{
			if ( ++i < argc )
			{
				port			=	(u_short)strtoul( argv[i], NULL, 0 );
			}
			else
			{
				fprintf( stderr, "-p must have an address argument\n" );
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
						"Invalid -d argument <%s>\n", argv[i] );
					return ( FAIL );
				}
			}
			else
			{
				fprintf( stderr, "-d must have a decoder type argument\n" );
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
					fprintf( stderr, "-n %u too large, cannot exceed %u\n",
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
					"-n must have a preset aspect argument\n" );
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
					fprintf( stderr, "-N %u too large, cannot exceed %u\n",
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
					"-n must have a trigger aspect argument\n" );
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
				fprintf( stderr, "-t must have a test mask argument\n" );
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
				fprintf( stderr, "-c must have a clock mask argument\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-x" ) == 0 )	// Protect crit. regions.
		{
			crit_flag	=	crit_flag ? false : true;
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
			}
			else
			{
				fprintf( stderr, "-E must have a preamble number argument\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-T" ) == 0 )	// Trigger cmd is reverse.
		{
			trig_rev	=	trig_rev ? false  : true;
		}
		else if ( strcmp( argv[i], "-F" ) == 0 )	// Fill time in msec.
		{
			if ( ++i < argc )
			{
				t_u_int		=	(u_int)strtoul( argv[i], NULL, 0 );
				if ( t_u_int > MAX_FILL_MSEC )
				{
					fprintf( stderr, "-F %u too big, cannot exceed %u\n",
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
					"-F must have a fill time (msec.) argument\n" );
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
					fprintf( stderr, "-R %u too small, must be at least %u\n",
						t_u_int, MIN_TEST_REPS );
					return ( FAIL );
				}
				if ( t_u_int > MAX_TEST_REPS )
				{
					fprintf( stderr, "-R %u too big, cannot exceed %u\n",
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
					"-R must have a repeat count argument\n" );
				return ( FAIL );
			}
		}
		else if ( strcmp( argv[i], "-P" ) == 0 )	// Log packets.
		{
			log_pkts	=	log_pkts ? false  : true;
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
		else
		{
			fprintf( stderr,
				"Invalid command line argument <%s>\n", argv[1] );
			return ( FAIL );
		}
	}

	return ( OK );
}


/*---------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *			iniopen()					-	Open the ini file.
 *
 *	RETURN VALUE
 *
 *
 *
 *			OK		-	ini file opened.
 *			FAIL	-	ini file not present.
 *
  *	DESCRIPTION
 *
 *			iniopen() attempts to open the ini file in the usual places.
 *			It returns NULL if no ini file is found.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Args_obj::iniopen(
	const char	*cmdpath,					// Command path and name.
	const char	*inifile,					// ini file name.
	const char	*inienv )					// ini path environment variable.
{
	char		*sptr;						// Temporary char pointer.
	#if SEND_VERSION == 4
		char pathname[80];
		char localPath[40];
		char* plocalPath = localPath;
		char* pPath;
	#else
		char	fdrive[MAXDRIVE];			// Drive name.
		char	fdir[MAXDIR];				// Directory name.
	#endif
	/*
	 *	First, try the local directory for the ini file.
	 */
		if ( (ini_fp = fopen( inifile, "r" )) != NULL  )
		{
//k			getcwd( ini_path, MAXPATH );
			strcat( ini_path, "\\" );
			strcat( ini_path, inifile );
			return ( OK );	   				// Opened the ini file locally.
		}

	/*
	 *	Next, check to see if the ini file path is set in the
	 *	environment variable.
	 */
	#if SEND_VERSION == 4
//k		strcpy(localPath, szPathVar);
		// ToDo - use the path variable
		while((pPath = strsep(&plocalPath, ";")) != NULL)
		{
			// prepend the path
			strcpy(pathname, pPath);
			strcat(pathname, "\\");
			strcat(pathname, inifile);
			if ( (ini_fp = fopen( pathname, "r" )) != NULL )
			{
				strcpy( ini_path, sptr );
				return ( OK );					// Opened the inienv file.
			}
		}
	#else
		if ( (sptr = getenv( inienv )) != NULL )
		{
			if ( (ini_fp = fopen( sptr, "r" )) != NULL )
			{
				strcpy( ini_path, sptr );
				return ( OK );					// Opened the inienv file.
			}
		}
	#endif

	/*
	 *	Finally, check for the file in the send directory.
	 */
	#if SEND_VERSION == 4
		#define SEND_PATH "\\SEND\\"
		strcpy(pathname, SEND_PATH);
		strcat(pathname, inifile);
		if ( (ini_fp = fopen( pathname, "r" )) != NULL )
		{
			strcpy( ini_path, sptr );
			return ( OK );					// Opened the inienv file.
		}
	#else
		fdrive[0]	=	'\0';
		fdir[0]		=	'\0';
		fnsplit( cmdpath, fdrive, fdir, NULL, NULL );
		sprintf( ini_path, "%s%s%s", fdrive, fdir, inifile );
		if ( (ini_fp = fopen( ini_path, "r" )) != NULL  )
		{
			return ( OK );	   					// Opened the ini file locally.
		}
	#endif

	ini_path[0]	=	'\0';
	return ( FAIL );
}


/*---------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *			iniscan()					-	Scan and process ini file.
 *
 *	RETURN VALUE
 *
 *
 *
 *			OK		-	ini file parsed without error.
 *			FAIL	-	Error processing ini file.
 *
  *	DESCRIPTION
 *
 *			iniscan() scans the ini file and sets the appropriate
 *			arguments.  It returns FAIL if the file is invalid.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Args_obj::iniscan( void )
{
	char		*itmp;						// Temporary input pointer.
	char		*cmd;						// Command string.
	char		*tptr;						// Temp pointer.
	int			targc;						// Argument count.
	char		*targv[3];					// Argument vector.
	char		buf[MAX_INI_LINE];			// Ini line buffer.

	while( fgets( buf, MAX_INI_LINE, ini_fp ) )
	{
		itmp	=	buf;

		if ( *( tptr = buf +  ( strlen( buf ) - 1 ) ) ==  '\n' )
		{
			*tptr	=	'\0';
		}

		if ( itmp[0]  == '\0' )				// Skip blank lines.
		{
			continue;
		}

		if ( (itmp = get_str( itmp, cmd )) == NULL )
		{
			fprintf( stderr, "Error parsing ini file line <%s>\n", buf );
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
				fprintf( stderr, "Error parsing ini file line <%s>\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-a";
			targc		=	3;
		}
		else if (	!strcmpi( cmd,	"PORT" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "Error parsing ini file line <%s>\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-p";
			targc		=	3;
		}
		else if (	!strcmpi( cmd,	"TYPE" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "Error parsing ini file line <%s>\n", buf );
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
				fprintf( stderr, "Error parsing ini file line <%s>\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-n";
			targc		=	3;
		}
		else if (	!strcmpi( cmd,	"TRIGGER" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "Error parsing ini file line <%s>\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-N";
			targc		=	3;
		}
		else if (	!strcmpi( cmd,	"TESTS" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "Error parsing ini file line <%s>\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-t";
			targc		=	3;
		}
		else if (	!strcmpi( cmd,	"CLOCKS" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "Error parsing ini file line <%s>\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-c";
			targc		=	3;
		}
		else if (  	!strcmpi( cmd,	"CRITICAL" ) )
		{
			targv[1]	=	"-x";
			targc		=	2;
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
				fprintf( stderr, "Error parsing ini file line <%s>\n", buf );
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
		else if (	!strcmpi( cmd,	"FILL_MSEC" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "Error parsing ini file line <%s>\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-F";
			targc		=	3;
		}
		else if (	!strcmpi( cmd,	"TEST_REPS" ) )
		{
			if ( (itmp = get_str( itmp, targv[2] )) == NULL )
			{
				fprintf( stderr, "Error parsing ini file line <%s>\n", buf );
				return ( FAIL );
			}

			targv[1]	=	"-R";
			targc		=	3;
		}
		else if ( 	!strcmpi( cmd,	"LOG_PKTS" ) )
		{
			targv[1]	=	"-P";
			targc		=	2;
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
		else
		{
		 	fprintf( stderr, "Error parsing ini file line <%s>\n", buf );
			return ( FAIL );
		}

		if ( argscan( targc, targv ) == FAIL )
		{
			fprintf( stderr, "Error parsing ini file line <%s>\n", buf );
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
		fprintf ( stderr, "zcmd: Invalid command name passed\n" );
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


/*****************************************************************************
 * $History: ARGS.CPP $
 * 
 * *****************  Version 22  *****************
 * User: KKObel       Date: 10/1/2019
 * Updated in $/NMRA/SRC/SEND
 * Port to V4
 *
 * *****************  Version 21  *****************
 * User: Kenw         Date: 7/03/18    Time: 7:28p
 * Updated in $/NMRA/SRC/SEND
 * Intermin Check in X.5.8.3
 * 
 * *****************  Version 20  *****************
 * User: Kenw         Date: 4/21/17    Time: 7:06p
 * Updated in $/NMRA/SRC/SEND
 * Interim X.5.7.0 check in.
 * - Added signal decoder (s|S) to the decoder types.
 * - Added PRESET (-n) and TRIGGER (-N) aspect support.
 * 
 * *****************  Version 19  *****************
 * User: Kenw         Date: 9/15/16    Time: 1:58p
 * Updated in $/NMRA/SRC/SEND
 * Changed default accessory address to 1.
 * 
 * *****************  Version 17  *****************
 * User: Kenw         Date: 9/17/14    Time: 4:43p
 * Updated in $/NMRA/SRC/SEND
 * Added function decoder support for manual mode.
 * Modified decoder quiestions.
 * Print <SEND_END n> when program exits.
 * 
 * *****************  Version 15  *****************
 * User: Kenw         Date: 11/09/13   Time: 7:41p
 * Updated in $/NMRA/SRC/SEND
 * Added Function tests.
 * Added test summary to screen after each test phase.
 * 
 * *****************  Version 14  *****************
 * User: Kenw         Date: 12/21/04   Time: 12:11p
 * Updated in $/NMRA/SRC/SEND
 * Updating to Build 5.2 X.
 * 
 * *****************  Version 13  *****************
 * User: Kenw         Date: 12/06/04   Time: 7:15p
 * Updated in $/NMRA/SRC/SEND
 * First set of changes to support Ambiguous bit tests
 * 
 * *****************  Version 12  *****************
 * User: Kenw         Date: 5/20/04    Time: 6:24p
 * Updated in $/NMRA/SRC/SEND
 * Changed "FLAKEY" .ini keyword to "FRAGMENT"
 * 
 * *****************  Version 11  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:57p
 * Updated in $/NMRA/SRC/SEND
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 10  *****************
 * User: Kenw         Date: 8/10/98    Time: 7:06p
 * Updated in $/NMRA/SRC/SEND
 * Moving to B.2.14
 * 
 * *****************  Version 9  *****************
 * User: Kenw         Date: 7/11/98    Time: 8:36a
 * Updated in $/NMRA/SRC/SEND
 * Rolling to 2.13.
 * Added tests near maximum packet times.
 * Added ability to move scope trigger before or after trigger packet..
 * 
 * *****************  Version 8  *****************
 * User: Kenw         Date: 6/28/98    Time: 9:37a
 * Updated in $/NMRA/SRC/SEND
 * Changed to abort program at LOG_ERR. Rolling to X.2.12.
 * 
 * *****************  Version 7  *****************
 * User: Kenw         Date: 10/30/97   Time: 5:52p
 * Updated in $/NMRA/SRC/SEND
 * X.2.9 - Changed default test repeats to 4.
 * 
 * *****************  Version 6  *****************
 * User: Kenw         Date: 10/29/97   Time: 4:26p
 * Updated in $/NMRA/SRC/SEND
 * X.2.8 - Added packet log command.
 * 
 * *****************  Version 5  *****************
 * User: Kenw         Date: 10/28/97   Time: 2:38p
 * Updated in $/NMRA/SRC/SEND
 * Added Prior Packet Test & Idle manual command.
 * 
 * *****************  Version 4  *****************
 * User: Kenw         Date: 10/15/96   Time: 4:32p
 * Updated in $/NMRA/SRC/SEND
 * Added zero as last bit of packet fragment test.
 * Turned this test off by default
 * 
 * *****************  Version 3  *****************
 * User: Kenw         Date: 5/12/96    Time: 1:06p
 * Updated in $/NMRA/SRC/SEND
 * Shortened some comments to fit on a single line.
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 5/10/96    Time: 9:16a
 * Updated in $/NMRA/SRC/SEND
 * Added -T and -E switches and print_user_docs() to print user
 * summary.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 3:56p
 * Created in $/NMRA/SRC/SEND
 * Methods for 'class Args_obj' which scans ini files and command line
 * switches.
 *
 *****************************************************************************/

