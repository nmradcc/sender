/*****************************************************************************
 *
 * DESCRIPTION:
 *
 *	zlog_test.cpp	-	Unit test program for Zlog class.
 *
 *	The test is self checking.  It directs Zlog output to files, reads the
 *	files back and verifies the contents.  It exits with 0 if every check
 *	passes and 1 otherwise.
 *
 *****************************************************************************/

#include <zlog.h>

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

static const char	*LOG_FILE	= "zlog_test.log";	// Log file for the test.
static const char	*STAT_FILE	= "zlog_test.sum";	// Stat file for the test.

static int			g_checks	= 0;			// Checks run.
static int			g_failures	= 0;			// Checks failed.
static int			g_abort_calls = 0;			// Abort handler call count.
static jmp_buf		g_abort_jmp;				// Escape from errprint abort.

#define CHECK( cond )														\
	do																		\
	{																		\
		++g_checks;															\
		if ( !(cond) )														\
		{																	\
			++g_failures;													\
			fprintf( stdout, "FAIL line %d: %s\n", __LINE__, #cond );		\
		}																	\
	} while ( 0 )

/*
 *	Read an entire file into a string.  Returns an empty string on failure.
 */
static std::string
read_file(
	const char		*fname )
{
	std::string		contents;
	char			buf[256];
	size_t			n;
	FILE			*fp = fopen( fname, "rb" );

	if ( !fp )
	{
		return ( contents );
	}

	while ( (n = fread( buf, 1, sizeof( buf ), fp )) > 0 )
	{
		contents.append( buf, n );
	}

	fclose( fp );

	return ( contents );
}

static bool
contains(
	const std::string	&haystack,
	const char			*needle )
{
	return ( haystack.find( needle ) != std::string::npos );
}

/*
 *	Abort handler that returns control to the test instead of letting
 *	Zlog terminate the process.
 */
static void
test_abort_handler( void )
{
	++g_abort_calls;
	longjmp( g_abort_jmp, 1 );
}

static void
test_cmd_name( void )
{
	Deflog.set_cmd_name( "/bin/test/TESTME" );
	CHECK( std::string( Deflog.get_cmd_name() ) == "testme" );

	Deflog.set_cmd_name( "C:\\dir\\Prog.exe" );
	CHECK( std::string( Deflog.get_cmd_name() ) == "prog" );

	Deflog.set_cmd_name( NULL );
	CHECK( std::string( Deflog.get_cmd_name() ) == "" );

	Deflog.set_cmd_name( "zlog_test" );
	CHECK( std::string( Deflog.get_cmd_name() ) == "zlog_test" );
}

static void
test_open_close( void )
{
	Deflog.set_no_abort_flag( true );

	CHECK( Deflog.open_log( NULL ) == FAIL );
	CHECK( Deflog.open_log( "" ) == FAIL );
	CHECK( Deflog.open_stat( NULL ) == FAIL );
	CHECK( Deflog.open_stat( "" ) == FAIL );
	CHECK( Deflog.open_log( "no_such_dir/zlog_test.log" ) == FAIL );
	CHECK( Deflog.get_fp_log() == NULL );
	CHECK( Deflog.get_fp_stat() == NULL );

	CHECK( Deflog.get_stderr_too() );
	CHECK( Deflog.open_log( LOG_FILE ) == OK );
	CHECK( Deflog.get_fp_log() != NULL );
	CHECK( !Deflog.get_stderr_too() );
	CHECK( Deflog.open_log( LOG_FILE ) == FAIL );		// Already open.

	CHECK( Deflog.open_stat( STAT_FILE ) == OK );
	CHECK( Deflog.get_fp_stat() != NULL );
	CHECK( Deflog.open_stat( STAT_FILE ) == FAIL );		// Already open.

	Deflog.set_no_abort_flag( false );
}

static void
test_statprint( void )
{
	Deflog.statprint( "stat message %d", 42 );

	std::string		log		= read_file( LOG_FILE );
	std::string		stat	= read_file( STAT_FILE );

	CHECK( contains( log, "STATUS" ) );
	CHECK( contains( log, "zlog_test: stat message 42" ) );
	CHECK( contains( stat, "STATUS" ) );
	CHECK( contains( stat, "zlog_test: stat message 42" ) );
}

static void
test_errprint_priorities( void )
{
	static const struct
	{
		Zlog_pri	pri;
		const char	*name;
	} pris[] =
	{
		{ LOG_EMERG,	"EMERG" },
		{ LOG_ALERT,	"ALERT" },
		{ LOG_CRIT,		"CRIT" },
		{ LOG_ERR,		"ERROR" },
		{ LOG_WARNING,	"WARNING" },
		{ LOG_NOTICE,	"NOTICE" },
		{ LOG_INFO,		"INFO" },
		{ LOG_DEBUG,	"DEBUG" }
	};

	Deflog.set_no_abort_flag( true );

	for ( size_t i = 0; i < sizeof( pris ) / sizeof( pris[0] ); i++ )
	{
		char		msg[64];
		std::string	expect;

		sprintf( msg, "priority message %d", (int)i );
		Deflog.errprint( "test_fn", pris[i].pri, "%s", msg );

		expect = std::string( pris[i].name );
		std::string	log = read_file( LOG_FILE );

		CHECK( contains( log, pris[i].name ) );
		CHECK( contains( log, ("zlog_test: test_fn - " + std::string( msg )).c_str() ) );
	}

	Deflog.set_no_abort_flag( false );
}

static void
test_abort_behavior( void )
{
	Deflog.set_abort_handler( test_abort_handler );

	// Priorities less severe than LOG_ERR must not abort.
	g_abort_calls = 0;
	Deflog.errprint( "test_fn", LOG_WARNING, "no abort expected" );
	CHECK( g_abort_calls == 0 );

	// LOG_ERR must invoke the abort handler.
	if ( setjmp( g_abort_jmp ) == 0 )
	{
		Deflog.errprint( "test_fn", LOG_ERR, "abort expected" );
		CHECK( !"errprint returned instead of aborting" );
	}

	CHECK( g_abort_calls == 1 );
	CHECK( contains( read_file( LOG_FILE ), "test_fn - abort expected" ) );

	// The no abort flag must suppress the handler.
	Deflog.set_no_abort_flag( true );
	Deflog.errprint( "test_fn", LOG_EMERG, "suppressed abort" );
	CHECK( g_abort_calls == 1 );
	Deflog.set_no_abort_flag( false );

	Deflog.set_abort_handler( NULL );
}

static void
test_debug_output( void )
{
	Deflog.set_dbg_on( false );
	CHECK( !Deflog.get_dbg_on() );

	Deflog.dbgprint( false, "dbg_marker_false_flag" );
	Deflog.dbgprint( "dbg_marker_off" );
	Deflog.dbgprint( DBG_PKT, "dbg_marker_off_pkt" );

	std::string		log = read_file( LOG_FILE );
	CHECK( !contains( log, "dbg_marker_false_flag" ) );
	CHECK( !contains( log, "dbg_marker_off" ) );
	CHECK( !contains( log, "dbg_marker_off_pkt" ) );

	Deflog.dbgprint( true, "dbg_marker_true_flag %d", 1 );
	Deflog.dbgprint( true, DBG_PKT, "dbg_marker_true_pkt" );

	log = read_file( LOG_FILE );
	CHECK( contains( log, "!> dbg_marker_true_flag 1" ) );
	CHECK( contains( log, "!P dbg_marker_true_pkt" ) );

	Deflog.set_dbg_on( true );
	CHECK( Deflog.get_dbg_on() );
	Deflog.dbgprint( "dbg_marker_on" );
	Deflog.dbgprint( DBG_LOG, "dbg_marker_on_log" );

	log = read_file( LOG_FILE );
	CHECK( contains( log, "!> dbg_marker_on" ) );
	CHECK( contains( log, "!L dbg_marker_on_log" ) );

	// These always print, regardless of the dbg_on state.
	Deflog.set_dbg_on( false );
	Deflog.dbgall( "dbg_marker_all" );
	Deflog.dbgall( DBG_PKT, "dbg_marker_all_pkt" );
	Deflog.dbglog( "dbg_marker_dbglog" );
	Deflog.dbgpkt( "dbg_marker_dbgpkt" );

	log = read_file( LOG_FILE );
	CHECK( contains( log, "!> dbg_marker_all" ) );
	CHECK( contains( log, "!P dbg_marker_all_pkt" ) );
	CHECK( contains( log, "!L dbg_marker_dbglog" ) );
	CHECK( contains( log, "!P dbg_marker_dbgpkt" ) );
}

static void
test_headerless_output( void )
{
	Deflog.to_log( "to_log_marker %d\n", 7 );
	Deflog.to_stat( "to_stat_marker %d\n", 8 );
	Deflog.to_dump( "to_dump_marker %d\n", 9 );		// stderr only.

	std::string		log		= read_file( LOG_FILE );
	std::string		stat	= read_file( STAT_FILE );

	CHECK( contains( log, "to_log_marker 7" ) );
	CHECK( !contains( stat, "to_log_marker" ) );
	CHECK( contains( log, "to_stat_marker 8" ) );
	CHECK( contains( stat, "to_stat_marker 8" ) );
	CHECK( !contains( log, "to_dump_marker" ) );
	CHECK( !contains( stat, "to_dump_marker" ) );
}

static void
test_masks( void )
{
	Deflog.set_log_mask( 0x5 );
	CHECK( Deflog.get_log_mask() == 0x5 );
	Deflog.set_lib_log_mask( 0x3 );
	CHECK( Deflog.get_lib_log_mask() == 0x3 );

	Deflog.set_log_mask( LOG_MASK_INIT );
	Deflog.set_lib_log_mask( LOG_MASK_INIT );
}

static void
test_close( void )
{
	Deflog.close_log();
	CHECK( Deflog.get_fp_log() == NULL );
	CHECK( Deflog.get_stderr_too() );

	// With the log closed, stat only output still reaches the stat file.
	Deflog.set_stderr_too( false );
	Deflog.to_stat( "after_log_close_marker\n" );
	CHECK( contains( read_file( STAT_FILE ), "after_log_close_marker" ) );
	CHECK( !contains( read_file( LOG_FILE ), "after_log_close_marker" ) );

	Deflog.close_stat();
	CHECK( Deflog.get_fp_stat() == NULL );

	Deflog.to_stat( "after_stat_close_marker\n" );
	CHECK( !contains( read_file( STAT_FILE ), "after_stat_close_marker" ) );
	Deflog.set_stderr_too( true );
}

int
main( void )
{
	// Start from clean files because Zlog opens them in append mode.
	remove( LOG_FILE );
	remove( STAT_FILE );

	test_cmd_name();
	test_open_close();
	test_statprint();
	test_errprint_priorities();
	test_abort_behavior();
	test_debug_output();
	test_headerless_output();
	test_masks();
	test_close();

	remove( LOG_FILE );
	remove( STAT_FILE );

	printf( "zlog_test: %d checks, %d failures\n", g_checks, g_failures );

	return ( g_failures == 0 ? 0 : 1 );
}
