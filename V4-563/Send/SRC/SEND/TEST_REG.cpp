/*****************************************************************************
 *
 * File:                 $Workfile: TEST_REG.CPP $
 * Path:                 $Logfile: /NMRA/SRC/SEND/TEST_REG.CPP $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 5 $
 * Last Modified on:     $Modtime: 7/24/19 10:09p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 7/24/19 10:15p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	test_reg.cpp -	Test_reg class methods.
 *
 *****************************************************************************/


#include "test_reg.h"

#include <zlog.h>

static const char sccsid[]      = "@(#) $Workfile: TEST_REG.CPP $$ $Revision: 5 $$";
static const char sccsid_h[]    = TEST_REG_H_DECLARED;

// Func to eliminate bogus sccsid and sccsid_h declared but never used warning.
inline void dummy(const char *, const char *) {}

#define CLR_LINE	printf( \
"                                                                            \r" )


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		set_vm()							-	 Set vector using v_index.
 *
 *	RETURN VALUE
 *
 *		OK		-	Successful set.
 *		FAIL	-	v_index out of range.
 *
 *	DESCRIPTION
 *
 *		set_vm() makes the internal vector and 'v_index' the current one by
 *		copying the vector data to the class variables.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Test_reg::set_vm(
	int				v_index )				// Vector table index.
{
	const char		*my_name = "Test_reg::set_vm";

    // Eliminate bogus sccsid and sccsid_h declared but never used warning.
    dummy( sccsid, sccsid_h );

	if ( v_index < 0 || v_index >= vsize )
	{
		ERRPRINT( my_name, LOG_ERR,
			"v_index %d out of range", v_index );
		return ( FAIL );
	}
	else
	{
		set_vm( vects[ v_index ] );
		return ( OK );
	}
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		test()							-	 Core test method.
 *
 *	RETURN VALUE
 *
 *		OK		-	All tested hardware registers match expected values.
 *		FAIL	-	One or more registers do not match.
 *
 *	DESCRIPTION
 *
 *		test() compares each value in 'ireg' against the corresponding
 *		value in 'iv' not masked by an 'im' field.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Test_reg::test(
	const Sr_core		&ireg,				// Register to test against.
	const Sr_core_vals	&iv,				// Test values.
	const Test_reg_mask	&im,				// Test skip masks.
	const char			*fmsg )				// Fail message (optional).
{
	Rslt_t		retval = OK;				// Return value.

	/* Test PA register */
	if ( !im.pa_skip && ( iv.pa != ireg.get_pa() ) )
	{
		if ( retval == OK )		// Clear the line on the first error printed.
		{
			CLR_LINE;
		}

		printf(
			"      ERROR PA    - Hardware     0x%02x != Vector     0x%02x\n",
			ireg.get_pa(), iv.pa );
		retval	=	FAIL;
	}

	/* Test PB register */
	if ( ~im.pb_mask & ( iv.pb ^ ireg.get_pb() ) )
	{
		if ( retval == OK )		// Clear the line on the first error printed.
		{
			CLR_LINE;
		}

		fputs( "      ERROR PB    - Hardware ", stdout );
		print_byte( ireg.get_pb(), im.pb_mask );
		printf( " (0x%02x) != Vector ", ireg.get_pb() );
		print_byte( iv.pb, im.pb_mask );
		printf( " (0x%02x)\n", iv.pb );
        retval	=	FAIL;
	}

	/* Test PC register */
	if ( ~im.pc_mask & ( iv.pc ^ ireg.get_pc() ) )
	{
		if ( retval == OK )		// Clear the line on the first error printed.
		{
			CLR_LINE;
		}

		fputs( "      ERROR PC    - Hardware ", stdout );
		print_byte( ireg.get_pc(), im.pc_mask );
		printf( " (0x%02x) != Vector ", ireg.get_pc() );
		print_byte( iv.pc, im.pc_mask );
		printf( " (0x%02x)\n", iv.pc );
        retval	=	FAIL;
	}

	/* Test S0T register */
	if ( ~im.s0t_mask & ( iv.s0t ^ ireg.get_s0t() ) )
	{
		if ( retval == OK )		// Clear the line on the first error printed.
		{
			CLR_LINE;
		}

		fputs( "      ERROR S0T   - Hardware ", stdout );
		print_byte( ireg.get_s0t(), im.s0t_mask );
		printf( " (0x%02x) != Vector ", ireg.get_s0t() );
		print_byte( iv.s0t, im.s0t_mask );
		printf( " (0x%02x)\n", iv.s0t );
		retval	=	FAIL;
	}

	/* Test S0H register */
	if ( ~im.s0h_mask & ( iv.s0h ^ ireg.get_s0h() ) )
	{
		if ( retval == OK )		// Clear the line on the first error printed.
		{
			CLR_LINE;
		}

		fputs( "      ERROR S0H   - Hardware ", stdout );
		print_byte( ireg.get_s0h(), im.s0h_mask );
		printf( " (0x%02x) != Vector ", ireg.get_s0h() );
		print_byte( iv.s0h, im.s0h_mask );
		printf( " (0x%02x)\n", iv.s0h );
		retval	=	FAIL;
	}

	/* Test S1T register */
	if ( ~im.s1t_mask & ( iv.s1t ^ ireg.get_s1t() ) )
	{
		if ( retval == OK )		// Clear the line on the first error printed.
		{
			CLR_LINE;
		}

		fputs( "      ERROR S1T   - Hardware ", stdout );
		print_byte( ireg.get_s1t(), im.s1t_mask );
		printf( " (0x%02x) != Vector ", ireg.get_s1t() );
		print_byte( iv.s1t, im.s1t_mask );
		printf( " (0x%02x)\n", iv.s1t );
		retval	=	FAIL;
	}

	/* Test CNT0T */
	if ( !im.cnt0t_skip && ( iv.cnt0t != ireg.get_cnt0t() ) )
	{
		if ( retval == OK )		// Clear the line on the first error printed.
		{
			CLR_LINE;
		}

		printf(
			"      ERROR CNT0T - Hardware %8u != Vector %8u\n",
			ireg.get_cnt0t(), iv.cnt0t );
		retval	=	FAIL;
	}

	/* Test CNT0H */
	if ( !im.cnt0h_skip && ( iv.cnt0h != ireg.get_cnt0h() ) )
	{
		if ( retval == OK )		// Clear the line on the first error printed.
		{
			CLR_LINE;
		}

		printf( "      ERROR CNT0H - Hardware %8u != Vector %8u\n",
			ireg.get_cnt0h(), iv.cnt0h );
		retval	=	FAIL;
	}

	/* Test CNT1T */
	if ( !im.cnt1t_skip && ( iv.cnt1t != ireg.get_cnt1t() ) )
	{
		if ( retval == OK )		// Clear the line on the first error printed.
		{
			CLR_LINE;
		}

		printf( "      ERROR CNT1T - Hardware %8u != Vector %8u\n",
			ireg.get_cnt1t(), iv.cnt1t );
		retval	=	FAIL;
	}

	/* Test generic input  register */
	if ( ~im.gen_mask & ( iv.gen ^ ireg.get_gen() ) )
	{
		if ( retval == OK )		// Clear the line on the first error printed.
		{
			CLR_LINE;
		}

		fputs( "      ERROR GEN   - Hardware     ", stdout );
		print_nibble( ireg.get_gen(), im.gen_mask );
		printf( " (0x%02x) != Vector     ", ireg.get_gen() & 0x0f );
		print_nibble( iv.gen, im.gen_mask );
		printf( " (0x%02x)\n", iv.gen & 0x0f );
        retval	=	FAIL;
	}

	if ( fmsg && retval != OK )
	{
		printf( "ERROR <%s>\n", fmsg );
		ireg.print();
	}

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		test()								-	 Vector test method.
 *
 *	RETURN VALUE
 *
 *		OK		-	All tested hardware registers match expected values.
 *		FAIL	-	One or more registers do not match.
 *
 *	DESCRIPTION
 *
 *		test() compares each value in 'ireg' against the corresponding
 *		value in vector at index 'v_index'.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Test_reg::test(
	const Sr_core	&ireg,					// Register to test against.
	int			  	v_index ) const			// Vector table index.
{
	const char		*my_name = "Test_reg::test";

	if ( v_index < 0 || v_index >= vsize )
	{
		ERRPRINT( my_name, LOG_ERR,
			"v_index %d out of range", v_index );
		return ( FAIL );
	}
	else
	{
		return ( test( ireg, vects[ v_index ] ) );
	}
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		set_test()							-	 Set & test method.
 *
 *	RETURN VALUE
 *
 *		OK		-	All tested hardware registers match expected values.
 *		FAIL	-	One or more registers do not match.
 *
 *	DESCRIPTION
 *
 *		set_test() first sets the internal test state to the vector at
 *		index 'v_index' and then compares each value in 'ireg' against
 *		the corresponding value in the vector.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Test_reg::set_test(
	const Sr_core	&ireg,					// Register to test against.
	int				v_index )				// Vector table index.
{
	const char		*my_name = "Test_reg::set_test";
	if ( v_index < 0 || v_index >= vsize )
	{
		ERRPRINT( my_name, LOG_ERR,
			"v_index %d out of range", v_index );
		return ( FAIL );
	}
	else
	{
		return ( set_test( ireg, vects[ v_index ] ) );
	}
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		print()								  -	 Print test fields.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		print() prints out the various Sr_core_vals fields.  They line up
 *		with the header produced by print_header().  Any values masked by
 *		by the corresponding Test_reg_mask field will be replaced by the
 *		'?' character.
 */
/*--------------------------------------------------------------------------*/

void
Test_reg::print(
	bool		pr_hdr_flag ) const				// If TRUE, print header.
{
	if ( pr_hdr_flag )
	{
		printf( "Test vector values\n" );

		print_header();
	}
	if ( m.pa_skip )
	{
		fputs( "0x?? ", stdout );
	}
	else
	{
		printf( "0x%02x ", v.pa );
	}
	print_byte( v.pb, m.pb_mask );
	putchar( ' ' );
	print_byte( v.pc, m.pc_mask );
	putchar( ' ' );

	if ( m.cnt0t_skip )
	{
		fputs( "    ? ", stdout );
	}
	else
	{
		printf( "%5u ", v.cnt0t );
	}
	print_byte( v.s0t, m.s0t_mask );
	putchar( ' ' );
	if ( m.cnt0h_skip )
	{
		fputs( "    ? ", stdout );
	}
	else
	{
		printf( "%5u ", v.cnt0h );
	}
	print_byte( v.s0h, m.s0h_mask );
	putchar( ' ' );
	if ( m.cnt1t_skip )
	{
		fputs( "    ? ", stdout );
	}
	else
	{
		printf( "%5u ", v.cnt1t );
	}
	print_byte( v.s1t, m.s1t_mask );
	putchar( ' ' );
	print_nibble( v.gen, m.gen_mask );
	putchar( '\n' );
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
Test_reg::init( void )
{
	Sr_core::init();

	m.pa_skip		=	false;
	m.pb_mask		=	0;
	m.pc_mask		=	0;
	m.s0t_mask		=	0;
	m.s0h_mask		=	0;
	m.s1t_mask		=	0;
	m.cnt0t_skip	=	false;
	m.cnt0h_skip	=	false;
	m.cnt1t_skip	=	false;
	m.gen_mask		=	0;
};


/*****************************************************************************
 * $History: TEST_REG.CPP $
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
 * User: Kenw         Date: 4/21/96    Time: 4:02p
 * Created in $/NMRA/SRC/SEND
 * Methods for 'class Test_reg'.
 *
 *****************************************************************************/

