/*****************************************************************************
 *
 * File:                 $Workfile: cktest.cpp $
 * Path:                 $Logfile: /NMRA/LIB/cktest.cpp $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 4 $
 * Last Modified on:     $Modtime: 7/24/19 9:25p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 7/24/19 9:26p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	cktest.cpp	-	Unit test program for cksum object.
 *
 *****************************************************************************/

static const char sccsid[]      = "@(#) $Workfile: cktest.cpp $$ $Revision: 4 $$";

// Func to eliminate bogus sccsid declared but never used warning.
inline void dummy(const char *) {}

#include <cksum.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(
	int			argc,					// Count of args.
	char		**argv )				// Command line args.
{
	int				rslt;			 	// Call result.
	unsigned long	nCRC;				// CRC.
	unsigned long	nLength;			// Length.

    // Eliminate bogus sccsid declared but never used warning.
    dummy( sccsid );

	if ( argc != 1 )
	{
		fprintf( stderr, "Test takes no parameters\n" );
		exit( 1 );
	}

	rslt = nCheckSumFile(NULL, argv[0], &nCRC, &nLength );

    if ( rslt == 0 )
    {
    	printf( "nCheckSumFile returned OK - CRC %lu, Length %lu\n  File <%s>\n",
        		nCRC, nLength, argv[0] );
    }
    else
    {
    	printf( "nCheckSumFile Failed - File <%s>\n",
        		argv[0] );
    }

	exit( rslt );
	return rslt;
}


/*****************************************************************************
 * $History: cktest.cpp $
 * 
 * *****************  Version 4  *****************
 * User: Kenw         Date: 7/24/19    Time: 9:26p
 * Updated in $/NMRA/LIB
 * Eliminated sccsid warnings.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 2/01/04    Time: 6:20p
 * Created in $/NMRA/LIB
 * Unit test for CKSUM.CPP
 *
 *****************************************************************************/

