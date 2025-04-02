/*****************************************************************************
 *
 * File:                 $Workfile: SR_CORE.CPP $
 * Path:                 $Logfile: /NMRA/SRC/SEND/SR_CORE.CPP $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 5 $
 * Last Modified on:     $Modtime: 7/24/19 10:02p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 7/24/19 10:15p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	sr_core.cpp	-	Sr_core class methods.
 *
 *****************************************************************************/

#include "sr_core.h"

#include <stdio.h>

static const char sccsid[]      = "@(#) $Workfile: SR_CORE.CPP $$ $Revision: 5 $$";
static const char sccsid_h[]    = SR_CORE_H_DECLARED;

// Func to eliminate bogus sccsid and sccsid_h declared but never used warning.
inline void dummy(const char *, const char *) {}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		print_header()						  -	 Print register header.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		print_header() prints out the header line showing the positions
 *		of the various Sr_core_vals fields.  The fields line up with the
 *		output produced by the print() method.
 */
/*--------------------------------------------------------------------------*/

void
Sr_core::print_header( void ) const
{

    // Eliminate bogus sccsid and sccsid_h declared but never used warning.
    dummy( sccsid, sccsid_h );
    
	printf(
	"     D          S  C\n"
	"     C S        C  P         O              O              O\n"
	"     CSH8D CU OAO IUCB       U              U              U\n"
	"     OCO2CDLN BCPCNCPD       TN             TN             TN\n"
	"     UOLGCCKD FKELTLUR       PU     B       PU     B       PU     B\n"
	"     TPD0QC1E AANRRKES       ULRRMMMC       ULRRMMMC       ULRRMMMC\n"
	"  PA DELHH0LR LLLLALNT    0T TL10210D    0H TL10210D    1T TL10210D GEN\n"
	"-----------------------------------------------------------------------\n" );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		print()								  -	 Print Sr_core_vals fields.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		print() prints out the various Sr_core_vals fields.  They line up
 *		with the header produced by print_header().
 */
/*--------------------------------------------------------------------------*/

void
Sr_core::print(
	bool		pr_hdr_flag ) const				// If TRUE, print header.
{
	if ( pr_hdr_flag )
	{
		print_header();
	}

	printf( "0x%02x ", v.pa );
	print_byte( v.pb );
	putchar( ' ' );
	print_byte( v.pc );
	putchar( ' ' );

	printf( "%5u ", v.cnt0t );
	print_byte( v.s0t );
	putchar( ' ' );
	printf( "%5u ", v.cnt0h );
	print_byte( v.s0h );
	putchar( ' ' );
	printf( "%5u ", v.cnt1t );
	print_byte( v.s1t );
	putchar( '  ' );
	print_nibble( v.gen );
	putchar( '\n' );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		print_byte()						  -	 Print BYTE as bits.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		print_byte() prints out the 'val' BYTE as 8 '1' or '0' characters.
 *		A '?' will be placed at any position corresponding to a 1
 *		in the optional 'skip' BYTE.
 */
/*--------------------------------------------------------------------------*/

void
Sr_core::print_byte(
	BYTE		val,					// Byte to print.
	BYTE		skip )					// Bits to skip (optional).
{
	BYTE		mask;					// Scan mask.

	for ( mask = 0x80; mask != 0; mask >>=  1 )
	{
		if ( mask & skip )
		{
			putchar( '?' );
		}
		else
		{
			putchar( mask & val ? '1' : '0' );
		}
	}
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		print_nibble()						  -	 Print nibble as bits.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		print_nibble() prints out the lowest nibble of the 'val' BYTE as
  *		4 '1' or '0' characters.
 *		A '?' will be placed at any position corresponding to a 1
 *		in the optional 'skip' BYTE.
 */
/*--------------------------------------------------------------------------*/

void
Sr_core::print_nibble(
	BYTE		val,					// Nibble to print.
	BYTE		skip )					// Bits to skip (optional).
{
	BYTE		mask;					// Scan mask.

	for ( mask = 0x08; mask != 0; mask >>=  1 )
	{
		if ( mask & skip )
		{
			putchar( '?' );
		}
		else
		{
			putchar( mask & val ? '1' : '0' );
		}
	}
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		init()						  		-	 Initialize object.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		init() initializes the object.
 */
/*--------------------------------------------------------------------------*/

void
Sr_core::init( void )
{
	v.pa	=	0;
	v.pb	=	0;
	v.pc	=	0;
	v.s0t	=	0;
	v.s0h	=	0;
	v.s1t	=	0;
	v.cnt0t	=	0;
	v.cnt0h	=	0;
	v.cnt1t	=	0;
	v.gen	=	0;
}


/*****************************************************************************
 * $History: SR_CORE.CPP $
 * 
 * *****************  Version 5  *****************
 * User: Kenw         Date: 7/24/19    Time: 10:15p
 * Updated in $/NMRA/SRC/SEND
 * Removed spurious sccsid warning.
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:57p
 * Updated in $/NMRA/SRC/SEND
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 3:59p
 * Created in $/NMRA/SRC/SEND
 * Methods for 'class Sr_core'.
 *
 *****************************************************************************/

