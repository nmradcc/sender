/*****************************************************************************
 *
 * File:                 $Workfile: ZL_TST.CPP $
 * Path:                 $Logfile: /NMRA/LIB/ZL_TST.CPP $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 11 $
 * Last Modified on:     $Modtime: 8/29/23 6:06p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 8/29/23 6:16p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	zl_tst.cpp	-	Unit test program for Zlog class.
 *
 *****************************************************************************/

#include <zlog.h>

#include <stdlib.h>

int
main(
	int				argc,
	char			**argv )
{
	int				cnt = 0;

	STATPRINT( "argc %d: NULL cmd_name test message %3d", argc, ++cnt );
	ERRPRINT( "main", LOG_WARNING, "NULL cmd_name test message %3d",
		++cnt );
	Deflog.set_cmd_name( "/bin/test/TESTME" );
	STATPRINT( "Test message %3d", ++cnt );
	ERRPRINT( "main", LOG_WARNING, "Test message %3d", ++cnt );
	Deflog.set_cmd_name( argv[0] );
	STATPRINT( "Test message %3d", ++cnt );
	ERRPRINT( "main", LOG_WARNING, "Test message %3d", ++cnt );
	if ( Deflog.open_log( "zl_tst.log" ) !=  OK )
	{
		ERRPRINT( "main", LOG_WARNING, "Open log FAILED" );
	}
	if ( Deflog.open_stat( "zl_tst.sum" ) !=  OK )
	{
		ERRPRINT( "main", LOG_WARNING, "Open stat FAILED" );
	}

	STATPRINT( "Test message to file %3d", ++cnt );
	ERRPRINT( "main", LOG_WARNING, "Test message to file %3d", ++cnt );
	Deflog.set_stderr_too( true );
	STATPRINT( "Test message to file & stderr %3d", ++cnt );
	ERRPRINT( "main", LOG_WARNING, "Test message to file & stderr %3d",
		++cnt );

	Deflog.set_no_abort_flag( true );
	ERRPRINT( "main", LOG_EMERG,	"Test message %3d", ++cnt );
	ERRPRINT( "main", LOG_ALERT,	"Test message %3d", ++cnt );
	ERRPRINT( "main", LOG_CRIT,		"Test message %3d", ++cnt );
	ERRPRINT( "main", LOG_ERR,		"Test message %3d", ++cnt );
	ERRPRINT( "main", LOG_WARNING,	"Test message %3d", ++cnt );
	ERRPRINT( "main", LOG_NOTICE,	"Test message %3d", ++cnt );
	ERRPRINT( "main", LOG_INFO,		"Test message %3d", ++cnt );
	ERRPRINT( "main", LOG_DEBUG,	"Test message %3d", ++cnt );
	Deflog.set_no_abort_flag( false );

	TO_LOG( "to_log msg %3d\n", ++cnt );
	TO_STAT( "to_stat msg %3d\n", ++cnt );
	TO_DUMP( "to_dump msg %3d\n", ++cnt );

	LOGDUMP( "main", "Test message %3d", ++cnt );
	LOGDUMP( 1, "main", "Test message %3d", ++cnt );

	LOGPRINT( "main", 0x1, "Shouldn't print" );

	Deflog.set_log_mask( 0x1 );
	LOGPRINT( "main", 0x1, "Should print %3d", ++cnt );

	STATPRINT( "Initial debug_on value <%s> , step %3d.",
	        Deflog.get_dbg_on() ? "true" : "false", ++cnt );

	DBGPRINT( false,          "dbgprint() Should not print with DBG_DFLT header, print_msg set to false." );
	DBGPRINT( true,           "dbgprint() Should print with DBG_DFLT header, print_msg set to true, step %3d.", ++cnt );

	DBGPRINT( false, DBG_PKT, "dbgprint() Should not print with DBG_DFLT header, print_msg set to false." );
	DBGPRINT( true,  DBG_PKT, "dbgprint() Should print with DBG_PKT header, print_msg set to true, step %3d.", ++cnt );

	Deflog.set_dbg_on( false );
	DBGPRINT(          "dbgprint() Should not print with DBG_DFLT header, dbg_on not set." );
	DBGPRINT( DBG_PKT, "dbgprint() Should not print with DBG_PKT header, dbg_on not set." );
	Deflog.set_dbg_on( true );
	DBGPRINT(          "dbgprint() Should print with DBG_DFLT header, dbg_on set, step %3d.", ++cnt );
	DBGPRINT( DBG_PKT, "dbgprint() Should print with DBG_PKT header, dbg_on set, step %3d.", ++cnt );

	Deflog.set_dbg_on( false );
	DBGALL(             "dbgall() Should print with DBG_DFLT header regardless of printing state, step %3d.", ++cnt );
	DBGALL(   DBG_PKT,  "dbgall() Should print with DBG_PKT header regardless of printing state, step %3d.", ++cnt );
	DBGLOG(             "dbglog() Should print with DBG_LOG header regardless of printing state, step %3d.", ++cnt );
	DBGPKT(             "dbgpkt() Should print with DBG_PKT header regardless of printing state, step %3d.", ++cnt );
	
	STATPRINT( "torf\( true \) = \"%s\", torf\( false \) = \"%s\", step %3d.",
	           "true", "false", ++cnt );
	
	Deflog.close_log();

	STATPRINT( "After log close %3d", ++cnt );
	TO_STAT( "Should print to stat only file %3d\n", ++cnt );

	Deflog.close_stat();

	ERRPRINT( "main", LOG_ERR, "Abort test message %3d", ++cnt );
	ERRPRINT( "main", "Shouldn't print" );

	exit( 0 );

	return ( 0 );
}
