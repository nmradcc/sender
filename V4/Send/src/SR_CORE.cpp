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
 * Current version:      $Revision: 2 $
 * Last Modified on:     $Modtime: 8/27/00 1:07p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 8/31/00 7:57p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	sr_core.cpp	-	Sr_core class methods.
 *
 *****************************************************************************/

#include <stdio.h>
#include <SR_CORE.h>

#if SEND_VERSION >= 4
extern "C"
{
	//extern int get_key_cmd( void );
	extern void putch(char c);
};
#endif

static const char sccsid[]      = "@(#) $Workfile: SR_CORE.CPP $$ $Revision: 2 $$";
static const char sccsid_h[]    = SR_CORE_H_DECLARED;


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
#if SEND_VERSION >= 4
	printf("     D          S  C\n");
	printf("     C S        C  P         O              O              O\n");
	printf("     CSH8D CU OAO IUCB       U              U              U\n");
	printf("     OCO2CDLN BCPCNCPD       TN             TN             TN\n");
	printf("     UOLGCCKD FKELTLUR       PU     B       PU     B       PU     B\n");
	printf("     TPD0QC1E AANRRKES       ULRRMMMC       ULRRMMMC       ULRRMMMC\n");
	printf("  PA DELHH0LR LLLLALNT    0T TL10210D    0H TL10210D    1T TL10210D GEN\n");
	printf("-----------------------------------------------------------------------\n");
#else
	printf(
	"     D          S  C\n"
	"     C S        C  P         O              O              O\n"
	"     CSH8D CU OAO IUCB       U              U              U\n"
	"     OCO2CDLN BCPCNCPD       TN             TN             TN\n"
	"     UOLGCCKD FKELTLUR       PU     B       PU     B       PU     B\n"
	"     TPD0QC1E AANRRKES       ULRRMMMC       ULRRMMMC       ULRRMMMC\n"
	"  PA DELHH0LR LLLLALNT    0T TL10210D    0H TL10210D    1T TL10210D GEN\n"
	"-----------------------------------------------------------------------\n" );
#endif
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
	#if SEND_VERSION >= 4
		putch(' ');
	#else
		putchar(' ');
	#endif
	print_byte( v.pc );
	#if SEND_VERSION >= 4
		putch(' ');
	#else
		putchar(' ');
	#endif

	printf( "%5u ", v.cnt0t );
	print_byte( v.s0t );
	#if SEND_VERSION >= 4
		putch(' ');
	#else
		putchar(' ');
	#endif
	printf( "%5u ", v.cnt0h );
	print_byte( v.s0h );
	#if SEND_VERSION >= 4
		putch(' ');
	#else
		putchar(' ');
	#endif
	printf( "%5u ", v.cnt1t );
	print_byte( v.s1t );
	#if SEND_VERSION >= 4
		putch(' ');
	#else
		putchar(' ');
	#endif
	print_nibble( v.gen );
	#if SEND_VERSION >= 4
		putch('\n');
	#else
		putchar('\n');
	#endif
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

		#if SEND_VERSION >= 4
			if ( mask & skip )
			{
				putch( '?' );
			}
			else
			{
				putch( mask & val ? '1' : '0' );
			}
		#else
			if ( mask & skip )
			{
				putchar( '?' );
			}
			else
			{
				putchar( mask & val ? '1' : '0' );
			}
		#endif

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
		#if SEND_VERSION >= 4
			if ( mask & skip )
			{
				putch( '?' );
			}
			else
			{
				putch( mask & val ? '1' : '0' );
			}
		#else
			if ( mask & skip )
			{
				putchar( '?' );
			}
			else
			{
				putchar( mask & val ? '1' : '0' );
			}
		#endif
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

