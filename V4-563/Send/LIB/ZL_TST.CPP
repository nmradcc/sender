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

static const char sccsid[]      = "@(#) $Workfile: ZL_TST.CPP $$ $Revision: 11 $$";

// Func to eliminate bogus sccsid and sccsid_h declared but never used warning.
inline void dummy(const char *) {}

#include <zlog.h>
#include <torf.h>

#include <stdlib.h>

int
main(
	int				argc,
	char			**argv )
{
	int				cnt = 0;

    // Eliminate bogus sccsid and sccsid_h declared but never used warning.
    dummy( sccsid );
	
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
	           torf( true ), torf( false ), ++cnt );
	
	Deflog.close_log();

	STATPRINT( "After log close %3d", ++cnt );
	TO_STAT( "Should print to stat only file %3d\n", ++cnt );

	Deflog.close_stat();

	ERRPRINT( "main", LOG_ERR, "Abort test message %3d", ++cnt );
	ERRPRINT( "main", "Shouldn't print" );

	exit( 0 );

	return ( 0 );
}


/*****************************************************************************
 * $History: ZL_TST.CPP $
 * 
 * *****************  Version 11  *****************
 * User: Kenw         Date: 8/29/23    Time: 6:16p
 * Updated in $/NMRA/LIB
 * Added vdbgprint(), dbgprint(), dbgall(), dbglog() and dbgpkt().
 * 
 * *****************  Version 10  *****************
 * User: Kenw         Date: 8/27/23    Time: 6:12p
 * Updated in $/NMRA/LIB
 * Changed dbg_on to always be set to false in the constructor.
 * Added STATPRINT for initial value of dbg_on.
 * 
 * *****************  Version 9  *****************
 * User: Kenw         Date: 8/21/23    Time: 5:08p
 * Updated in $/NMRA/LIB
 * Added test for torf() method declared in TORF.H.
 * 
 * *****************  Version 8  *****************
 * User: Kenw         Date: 8/20/23    Time: 8:52p
 * Updated in $/NMRA/LIB
 * Added dbgall() and DBGALL.
 * 
 * *****************  Version 7  *****************
 * User: Kenw         Date: 8/20/23    Time: 3:10p
 * Updated in $/NMRA/LIB
 * Added dbgprint() and vfdbgprint().
 * 
 * *****************  Version 6  *****************
 * User: Kenw         Date: 8/17/23    Time: 8:12p
 * Updated in $/NMRA/LIB
 * Added dbgprint() method.
 * 
 * *****************  Version 3  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:56p
 * Updated in $/NMRA/LIB
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 6/28/98    Time: 9:35a
 * Updated in $/NMRA/LIB
 * Changed to abort program at LOG_ERROR.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 1:49p
 * Created in $/NMRA/LIB
 * Unit test program for 'class Zlog'.
 *
 *****************************************************************************/

