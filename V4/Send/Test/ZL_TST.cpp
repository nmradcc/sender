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
 * Current version:      $Revision: 3 $
 * Last Modified on:     $Modtime: 8/27/00 12:57p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 8/31/00 7:56p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	zl_tst.cpp	-	Unit test program for Zlog class.
 *
 *****************************************************************************/

static const char sccsid[]      = "@(#) $Workfile: ZL_TST.CPP $$ $Revision: 3 $$";

#include <zlog.h>

#include <stdlib.h>

int
main(
	int				argc,
	char			**argv )
{
	int				cnt = 0;

	STATPRINT( "NULL cmd_name test message %3d", ++cnt );
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

