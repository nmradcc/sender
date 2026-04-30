/*****************************************************************************
 *
 * File:                 $Workfile: SELF_TST.CPP $
 * Path:                 $Logfile: /NMRA/SRC/SEND/SELF_TST.CPP $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 8 $
 * Last Modified on:     $Modtime: 7/24/19 10:01p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 7/24/19 10:15p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	self_tst.cpp	-	Dcc sender board self test methods.
 *
 *****************************************************************************/

#include "self_tst.h"
#include "send_reg.h"

#include <zlog.h>

#if SEND_VERSION < 4
 #include <dos.h>
 #include <PDS601x.H>
#endif
#include <stdio.h>

static const char sccsid[]      = "@(#) $Workfile: SELF_TST.CPP $$ $Revision: 8 $$";
static const char sccsid_h[]    = SELF_TST_H_DECLARED;

// Func to eliminate bogus sccsid and sccsid_h declared but never used warning.
inline void dummy(const char *, const char *) {}

#if SEND_VERSION >= 4
extern "C"
{
	#include "port.h"

	extern void OUT_PC(unsigned char out_pos, unsigned char value);
	extern uint8_t inportb(unsigned char port);
	extern void outportb(unsigned char port, unsigned char value);

	extern int GetTime(RTC_TimeTypeDef* time, RTC_DateTypeDef* date);

};
#endif

const u_long	TEST_ZERO_MIN	=	90;		// Min ZERO in 100ths of sec.
const u_long	TEST_ZERO_MAX	=	110;	// Max ZERO in 100ths of sec.
const u_long	TEST_ONE_MIN	=	90;		// Min ONE in 100ths of sec.
const u_long	TEST_ONE_MAX	=	110;	// Max ONE in 100ths of sec.


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		print_stats()							-	Print self test stats.
 *
 *	RETURN VALUE
 *
 *		None
  *
 *	DESCRIPTION
 *
 *		print_stats() prints out the present self test statistics.
 */
/*--------------------------------------------------------------------------*/

void
Self_tst::print_stats( void ) const
{
	const char	*my_name = "Self_tst::print_stats";

    // Eliminate bogus sccsid and sccsid_h declared but never used warning.
    dummy( sccsid, sccsid_h );

	if ( get_tst_cnt() > 0L )
	{
		ERRPRINT( my_name, LOG_INFO,
			"Passed %lu test cycles, Failed %lu test cycles, test count %lu",
			get_tst_pass(), get_tst_fail(), get_tst_cnt() );
		ERRPRINT( my_name, LOG_INFO,
			"Failed %lu vector, %lu dynamic, %lu both, last vector %d",
			get_v_tst_fail(),get_d_tst_fail(),get_vd_tst_fail(),p_vect-1 );
		ERRPRINT( my_name, LOG_INFO,
			"Dynamic 0 - min %lu, avg %lu, max %lu",
			get_min_0(), get_avg_0(), get_max_0() );
		ERRPRINT( my_name, LOG_INFO,
			"Dynamic 1 - min %lu, avg %lu, max %lu",
			get_min_1(), get_avg_1(), get_max_1() );
		printf(
			"\nPassed %lu test cycles, Failed %lu test cycles, test count %lu\n",
			get_tst_pass(), get_tst_fail(), get_tst_cnt() );
		printf(
			"Failed %lu vector, %lu dynamic, %lu both, last vector %d\n",
			get_v_tst_fail(),get_d_tst_fail(),get_vd_tst_fail(),p_vect-1 );
		printf(
			"Dynamic 0 - min %lu, avg %lu, max %lu\n",
			get_min_0(), get_avg_0(), get_max_0() );
		printf(
			"Dynamic 1 - min %lu, avg %lu, max %lu\n",
			get_min_1(), get_avg_1(), get_max_1() );
	}
	else
	{
		ERRPRINT( my_name, LOG_INFO,
			"No self tests completed" );
		printf(
			"\nNo self tests completed\n" );
	}
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		test_send()						   -	Perform self tests.
 *
 *	RETURN VALUE
 *
 *		OK		-	All tests successful.
 *		FAIL	-	Test error.
 *
 *	DESCRIPTION
 *
 *		test_send() performs the Sender self tests.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Self_tst::test_send( void )
{
	const char	*my_name = "Self_tst::test_send";
	Rslt_t		retval = OK;				// Return value.

	++tst_cnt;

	if ( vector_test() !=  OK )
	{
		ERRPRINT( my_name, LOG_ERR, "Vector self test FAILED" );
		retval	=	FAIL;
	}
	else
	{
		++v_tst_pass;
	}

	if ( dynamic_test() != OK )
	{
		ERRPRINT( my_name, LOG_ERR, "Dynamic self test FAILED" );
		if ( retval == FAIL )
		{
			++vd_tst_fail;
		}
		retval	=	FAIL;
	}
	else
	{
		++d_tst_pass;
	}

	if ( retval != FAIL )
	{
		++tst_pass;
	}

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		vector_test()						   -	Perform vector self tests.
 *
 *	RETURN VALUE
 *
 *		OK		-	All tests successful.
 *		FAIL	-	Test error.
 *
 *	DESCRIPTION
 *
 *		vector_test() performs the Sender vector self tests.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Self_tst::vector_test( void )
{
#if SEND_VERSION < 4
	int			i;							// Index variable.
	int			t_vect;						// Temp vector index.

	p_vect	=	0;

	/*
	 *	Do the steps of dcc_reg.init_send(), testing
	 *	the output after each step.
	 */

	// (0) First reset test.
	dcc_reg.rst_8255();						// Reset board.
	if ( do_test() != OK )
	{
		return ( FAIL );
	}

	// (1) Set 8255 test.
	dcc_reg.init_8255();					// Init 8255.
	if ( do_test() != OK )
	{
		return ( FAIL );
	}

	// (2) Set 8254 test.
	dcc_reg.init_8254();					// Init 8254.
	if ( do_test() != OK )
	{
		return ( FAIL );
	}

	// (3) Cnt 0T test.
	OUT_PC( PC_POS_CPUCLKL, 1 );			// Toggle clock to load 0T.
	OUT_PC( PC_POS_CPUCLKL, 0 );
	if ( do_test() != OK )
	{
		return ( FAIL );
	}

	// (4) Second reset test.
	dcc_reg.rst_8255();						// Reset board.
	if ( do_test() != OK )
	{
		return ( FAIL );
	}

	// (5) First init_send() test.
	dcc_reg.init_send();					// Initialize board.

	if ( do_test() != OK )
	{
		return ( FAIL );
	}


	// (6-7) Underflow clear test.
	OUT_PC( PC_POS_UNDERCLRL, 0 );			// Clear underflow.
	if ( do_test() != OK )
	{
		return ( FAIL );
	}
	OUT_PC( PC_POS_UNDERCLRL, 1 );			// Set underflow clear inactive.
	if ( do_test() != OK )
	{
		return ( FAIL );
	}

	// (8-9) Initial CNT0T CNT0H test.
	clk_bits	=   CT_TCNT0H | CT_TCNT1T | CT_DEC0;
	tcnt0h		=	dcc_reg.get_cnt0h();
	tcnt1t		=	dcc_reg.get_cnt1t();
	if ( clk_test( 1 ) != OK )
	{
		return ( FAIL );
	}

	// (10-11) CNT0T first 1 countdown test.
	clk_bits	=   CT_TCNT0H | CT_TCNT1T | CT_DEC0 | CT_REPEAT;
	if ( clk_test( tcnt0t/2 - 1 ) != OK )
	{
		return ( FAIL );
	}

	// (12-13) CNT0T first rollover 1-0 test.
	clk_bits	=   CT_TCNT0H | CT_TCNT1T | CT_DEC0;
	tcnt0h		-=	tcnt0t/2 - 1;
	if ( clk_test( 1 ) != OK )
	{
		return ( FAIL );
	}

	// (14-15) CNT0T 0 countdown test.
	clk_bits	=   CT_TCNT0H | CT_TCNT1T | CT_DEC0 | CT_REPEAT;
	if ( clk_test( tcnt0t/2 - 1 ) != OK )
	{
		return ( FAIL );
	}

	// (16-19) CNT0T first rollover 0-1 test.
	clk_bits	=   CT_TCNT0H | CT_TCNT1T | CT_DEC0;
	tcnt0h		-=	tcnt0t/2 - 1;
	if ( clk_test( 2 ) != OK )
	{
		return ( FAIL );
	}

	// (20-21) CNT0H reset test.  This resets CNT0H.
	treg.set_vm( p_vect );
	treg.set_cnt0h( tcnt0h );				// Set vector to present CNT0H.
	OUT_PC( PC_POS_CPUCLKL, 1 );			// Set -CLK high.
	dcc_reg.update();
	if ( treg.test( dcc_reg ) != OK )
	{
		return ( FAIL );
	}
	++p_vect;

	// CNT0H should reset now.
	OUT_PC( PC_POS_CPUCLKL, 0 );			// Clock CNT0T, CNT0H.
	if ( do_test() != OK )
	{
		return ( FAIL );
	}

	// (22-23) CNT0H first countdown test.
	tcnt0t		=	dcc_reg.get_cnt0t();	// Get present CNT0T.
	clk_bits	=   CT_TCNT1T | CT_DEC0 | CT_REPEAT;
	if ( clk_test( tcnt0t/2 - 1 ) != OK )
	{
		return ( FAIL );
	}

	// (24-27) CNT0T rollover 1-0 test.
	clk_bits	=   CT_TCNT1T | CT_DEC0;
	if ( clk_test( 2 ) != OK )
	{
		return ( FAIL );
	}

	// (28-29) CNT0T 0 countdown test.
	tcnt0t		=	dcc_reg.get_cnt0t();	// Get present CNT0T.
	clk_bits	=   CT_TCNT1T | CT_DEC0 | CT_REPEAT;
	if ( clk_test( tcnt0t/2 - 1 ) != OK )
	{
		return ( FAIL );
	}

	/*
	 *	Loop through remaining bits in 0 bits, stopping
	 *	at one count from rollover so carry won't activate.
	 */
	t_vect	=	p_vect;
	for ( i = 1; i <= 13; i++ )
	{
		p_vect	= t_vect;

		if ( clk_test_00() != OK )
		{
			printf( "      Failed for 0 repeat cycle %d\n", i );
			return ( FAIL );
		}
	}

	// Test for -SHOLD active.
	if ( clk_test_00() != OK )
	{
		return ( FAIL );
	}

	// Test for UNDERFLOW start.
	if ( clk_test_00() != OK )
	{
		return ( FAIL );
	}

	// Test for UNDERFLOW continue.
	if ( clk_test_00() != OK )
	{
		return ( FAIL );
	}

	// Test for loading a BYTE & setting SCOPE trigger active.
	// (86) Send first 0 bit.
	outportb( PA, 0x55 );
	OUT_PC( PC_POS_SCOPENL, 0 );

	if ( clk_test_00() != OK )
	{
		return ( FAIL );
	}

	// Turn off SCOPE trigger signal.
	OUT_PC( PC_POS_SCOPENL, 1 );

	// Test for sending first 1 bit.
	// (100-101) Rollover 0-1.
	OUT_PC( PC_POS_UNDERCLRL, 0 );
	OUT_PC( PC_POS_UNDERCLRL, 1 );
	clk_bits	=   CT_TCNT1T | CT_DEC0;
	if ( clk_test( 1 ) != OK )
	{
		return ( FAIL );
	}

	// (102) First CNT1T load.
	treg.set_vm( p_vect );
	treg.set_cnt1t( dcc_reg.get_cnt1t() );	// Set CNT1T to its present value.
	OUT_PC( PC_POS_CPUCLKL, 1 );			// Set -CLK high.
	dcc_reg.update();
	if ( treg.test( dcc_reg ) != OK )
	{
		return ( FAIL );
	}
	++p_vect;

	// (103) CNT1T should initialize now.
	OUT_PC( PC_POS_CPUCLKL, 0 );			// Set -CLK low.
	if ( do_test() != OK )
	{
		return ( FAIL );
	}

	// (104-105) First CNT1T 1 countdown, output high
	tcnt1t		=	dcc_reg.get_cnt1t();	// Get present CNT1T.
	clk_bits	=   CT_DEC1 | CT_REPEAT;
	if ( clk_test( tcnt1t/2 - 1 ) != OK )
	{
		return ( FAIL );
	}

	// (106-109) CNT1T rollover 1-0 test.
	clk_bits	=   CT_DEC1;
	if ( clk_test( 2 ) != OK )
	{
		return ( FAIL );
	}

	// (110-111) First CNT1T 0 countdown, output low
	tcnt1t		=	dcc_reg.get_cnt1t();	// Get present CNT1T.
	clk_bits	=   CT_DEC1 | CT_REPEAT;
	if ( clk_test( tcnt1t/2 - 1 ) != OK )
	{
		return ( FAIL );
	}

	// (112-125) Send second 0 bit.
	if ( clk_test_10() != OK )
	{
		return ( FAIL );
	}

	/*
	 *	Loop through remaining bits in 1-0 bit pairs.
	 */
	outportb( PA, 0xff );	// Set up all 1s Byte for next cycle.
	t_vect	=	p_vect;
	for ( i = 2; i <= 5; i++ )
	{
		p_vect	= t_vect;

		// (126-137) Send a 1 bit.
		if ( clk_test_01() != OK )
		{
			printf(  "     Failed for 1 bit at position %d\n", i );
			return ( FAIL );
		}

		++i;

		// (138-151) Send a 0 bit.
		if ( clk_test_10() != OK )
		{
			printf(  "     Failed for 0 bit at position %d\n", i );
			return ( FAIL );
		}
	}

	/*
	 *	Send last 1 bit.
	 */
	 // (152-163) Send a 1 bit.
	 if ( clk_test_01() != OK )
	 {
	 printf(  "     Failed for 1 bit at position 6\n" );
	 return ( FAIL );
	 }

	 /*
	  *	Send a Byte containing all 1s.
	  */
	 // (164-175) Send first 1 bit of second Byte.
	 if ( clk_test_11() != OK )
	 {
		printf(  "     Failed for 1 bit at position 0\n" );
		return ( FAIL );
	}

	t_vect	=	p_vect;
	for ( i = 1; i <= 6; i++ )
	{
		p_vect	= t_vect;

		// (176-187) Send remaining 1 bits of second Byte.
		if ( clk_test_11() != OK )
		{
			printf(  "     Failed for 1 bit at position %d\n", i );
			return ( FAIL );
		}
	}

	// (188-199) Send remaining 1 bits of second Byte.
	if ( clk_test_11() != OK )
	{
		printf(  "     Failed for 1 bit at position 7\n" );
		return ( FAIL );
	}
#endif

	return ( OK );
}



/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		do_tick()							-	Send software ticks.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		do_tick() sends 'cnt' ticks to the sender hardware.  It assumes
 *		the hardware is in software clock mode and the software clock is
 *		in the 0 state on entry.
 */
/*--------------------------------------------------------------------------*/

void
Self_tst::do_tick(
	int			cnt )						// Count of ticks (optional).
{
	while ( cnt-- > 0 )
	{
		OUT_PC( PC_POS_CPUCLKL, 1 );
		OUT_PC( PC_POS_CPUCLKL, 0 );
	}
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		do_test()							-	Test hardware state.
 *
 *	RETURN VALUE
 *
 *		OK		-	Hardware values match active vector state values.
 *		FAIL	-	Test error.
 *
 *	DESCRIPTION
 *
 *		do_test() compares each hardware register against the corresponding
 *		value in the present test vector.  Only values that are not masked
 *		in the test vector are tested.  It returns OK and updates 'p_vect'
 *		to point to the next vector if all values match.  It returns FAIL
 *		and leaves 'p_vect' unchanged if any test fails.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Self_tst::do_test( void )
{
	dcc_reg.update();

	if ( treg.set_test( dcc_reg, p_vect ) )
	{
		return ( FAIL );
	}
	else
	{
		++p_vect;							// Point to next vector.
		return ( OK );
	}
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		clk_test()							-	Do clocked tests.
 *
 *	RETURN VALUE
 *
 *		OK		-	Hardware values match active vector state values.
 *		FAIL	-	Test error.
 *
 *	DESCRIPTION
 *
 *		clk_test() performs 'cnt' number of tests where the software clock
 *		is set high and low for each test cycle.  Note that each test tick
 *		consumes two test vectors, one for the clock high state and one
 *		for the clock low state.  As such, 2 * 'cnt' test vectors are
 *		required.  It returns OK and updates 'p_vect' if all test cycles
 *		pass.  It returns FAIL and leaves 'p_vect' pointing at the failed
 *		vector if a test fails.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Self_tst::clk_test(
	int			cnt )						// Count of ticks (optional).
{
	int			cycle;						// Cycle count.

	treg.set_vm( p_vect );					// Initial clock values.

	if ( !(clk_bits & CT_TCNT0T) )			// Use vector cnt0t.
	{
		tcnt0t	=	treg.get_cnt0t();
	}
	if ( !(clk_bits & CT_TCNT0H) )			// Use vector cnt0h.
	{
		tcnt0h	=	treg.get_cnt0h();
	}
	if ( !(clk_bits & CT_TCNT1T) )			// Use vector cnt1t
	{
		tcnt1t	=	treg.get_cnt1t();
	}

	for ( cycle = 0; cnt-- > 0; cycle++ )
	{
		treg.set_vm( p_vect );		   		// Set up 1 vector.
		treg.set_cnt0t( tcnt0t );
		treg.set_cnt0h( tcnt0h );
		treg.set_cnt1t( tcnt1t );

		OUT_PC( PC_POS_CPUCLKL, 1 );		// -CLK set high.
		dcc_reg.update();
		if ( treg.test( dcc_reg ) != OK )
		{
			printf( "      FAILED clk_tst high for cycle %d\n", cycle );
			return ( FAIL );
		}
		++p_vect;

		if ( clk_bits & CT_DEC0 )			// Decrement 0 clock.
		{
			if ( tcnt0t <= 2 )
			{
				tcnt0t	=	icnt0t;
			}
			else
			{
				tcnt0t	-=	2;
			}
		}
		if ( clk_bits & CT_DEC1 )			// Decrement 1 clock.
		{
			if ( tcnt1t <= 2 )
			{
				tcnt1t	=	icnt1t;
			}
			else
			{
				tcnt1t	-=	2;
			}
		}

		--tcnt0h;

		treg.set_vm( p_vect );				// Set up 0 vector.
		treg.set_cnt0t( tcnt0t );
		treg.set_cnt0h( tcnt0h );
		treg.set_cnt1t( tcnt1t );
		OUT_PC( PC_POS_CPUCLKL, 0 );		// -CLK set low.
		dcc_reg.update();
		if ( treg.test( dcc_reg ) != OK )
		{
			printf( "      FAILED clk_tst low for cycle %d\n", cycle );
			return ( FAIL );
		}

		if ( clk_bits & CT_REPEAT )
		{
			--p_vect;
		}
		else
		{
			++p_vect;
		}
	}

	if ( clk_bits & CT_REPEAT )
	{
		p_vect += 2;
    }

	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		clk_test_00()	 					-	Do 0 0 clock cycle.
 *
 *	RETURN VALUE
 *
 *		OK		-	Hardware values match active vector state values.
 *		FAIL	-	Test error.
 *
 *	DESCRIPTION
 *
 *		clk_test_00() runs through the 15 clock states necessary to sequence
 *		through a 0 clock pulse preceded by a 0 clock pulse.
 *		It returns a FAIL on the first FAIL.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Self_tst::clk_test_00( void )
{
	// (0R00-0R03) CNT0T rollover 0-1 test.
	clk_bits	=   CT_TCNT1T | CT_DEC0;
	if ( clk_test( 2 ) != OK )
	{
		return ( FAIL );
	}

	// (0R04-0R05) CNT0H reset test.  This resets CNT0H.
	treg.set_vm( p_vect );
	OUT_PC( PC_POS_CPUCLKL, 1 );			// Set -CLK high.
	dcc_reg.update();
	if ( treg.test( dcc_reg ) != OK )
	{
		return ( FAIL );
	}
	++p_vect;

	// CNT0H should reset now.
	OUT_PC( PC_POS_CPUCLKL, 0 );			// Clock CNT0T, CNT0H.
	if ( do_test() != OK )
	{
		return ( FAIL );
	}

	// (0R06-0R07) CNT0H countdown test.
	tcnt0t		=	dcc_reg.get_cnt0t();	// Get present CNT0T.
	clk_bits	=   CT_TCNT1T | CT_DEC0 | CT_REPEAT;
	if ( clk_test( tcnt0t/2 - 1 ) != OK )
	{
		return ( FAIL );
	}

	// (0R08-0R12) CNT0T rollover 1-0 test.
	clk_bits	=   CT_TCNT1T | CT_DEC0;
	if ( clk_test( 2 ) != OK )
	{
		return ( FAIL );
	}

	// (0R13-0R14) CNT0T 0 countdown test.
	tcnt0t		=	dcc_reg.get_cnt0t();	// Get present CNT0T.
	clk_bits	=   CT_TCNT1T | CT_DEC0 | CT_REPEAT;
	if ( clk_test( tcnt0t/2 - 1 ) != OK )
	{
		return ( FAIL );
	}

	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		clk_test_10()	 					-	Do 0 0 clock cycle.
 *
 *	RETURN VALUE
 *
 *		OK		-	Hardware values match active vector state values.
 *		FAIL	-	Test error.
 *
 *	DESCRIPTION
 *
 *		clk_test_10() runs through the 15 clock states necessary to sequence
 *		through a 0 clock pulse preceded by a 1 clock pulse.
 *		It returns a FAIL on the first FAIL.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Self_tst::clk_test_10( void )
{
	// (0R00-0R01) CNT0T rollover 0-1 test.
	clk_bits	=   CT_DEC1;
	if ( clk_test( 1 ) != OK )
	{
		return ( FAIL );
	}

	// (0R02-0R03) CNT0T rollover 0-1 test.
	clk_bits	=   CT_DEC0;
	if ( clk_test( 1 ) != OK )
	{
		return ( FAIL );
	}

	// (0R04-0R05) CNT0H reset test.  This resets CNT0H.
	treg.set_vm( p_vect );
	OUT_PC( PC_POS_CPUCLKL, 1 );			// Set -CLK high.
	dcc_reg.update();
	if ( treg.test( dcc_reg ) != OK )
	{
		return ( FAIL );
	}
	++p_vect;

	// CNT0H should reset now.
	OUT_PC( PC_POS_CPUCLKL, 0 );			// Clock CNT0T, CNT0H.
	if ( do_test() != OK )
	{
		return ( FAIL );
	}

	// (0R06-0R07) CNT0H countdown test.
	tcnt0t		=	dcc_reg.get_cnt0t();	// Get present CNT0T.
	clk_bits	=   CT_TCNT1T | CT_DEC0 | CT_REPEAT;
	if ( clk_test( tcnt0t/2 - 1 ) != OK )
	{
		return ( FAIL );
	}

	// (0R08-0R12) CNT0T rollover 1-0 test.
	clk_bits	=   CT_TCNT1T | CT_DEC0;
	if ( clk_test( 2 ) != OK )
	{
		return ( FAIL );
	}

	// (0R13-0R14) CNT0T 0 countdown test.
	tcnt0t		=	dcc_reg.get_cnt0t();	// Get present CNT0T.
	clk_bits	=   CT_TCNT1T | CT_DEC0 | CT_REPEAT;
	if ( clk_test( tcnt0t/2 - 1 ) != OK )
	{
		return ( FAIL );
	}

	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		clk_test_01()	 					-	Do 1 clock cycle.
 *
 *	RETURN VALUE
 *
 *		OK		-	Hardware values match active vector state values.
 *		FAIL	-	Test error.
 *
 *	DESCRIPTION
 *
 *		clk_test_01() runs through the 8 clock states necessary to sequence
 *		through a 1 clock pulse preceded by a 0 pulse.
 *		It returns a FAIL on the first FAIL.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Self_tst::clk_test_01( void )
{
	// (1R00-1R01) CNT0T rollover 0-1 test.
	clk_bits	=   CT_DEC0;
	if ( clk_test( 1 ) != OK )
	{
		return ( FAIL );
	}

	// (1R02-1R03) First 1 count.
	clk_bits	=   CT_DEC1;
	if ( clk_test( 1 ) != OK )
	{
		return ( FAIL );
	}

	// (1R04-1R05) CNT1T countdown test.
	tcnt1t		=	dcc_reg.get_cnt1t();	// Get present CNT1T.
	clk_bits	=   CT_DEC1 | CT_REPEAT;
	if ( clk_test( tcnt1t/2 - 1 ) != OK )
	{
		return ( FAIL );
	}

	// (1R06-1R09) CNT1T rollover 1-0 test.
	clk_bits	=   CT_DEC1;
	if ( clk_test( 2 ) != OK )
	{
		return ( FAIL );
	}

	// (1R10-1R11) CNT1T 0 countdown test.
	tcnt1t		=	dcc_reg.get_cnt1t();	// Get present CNT1T.
	clk_bits	=   CT_DEC1 | CT_REPEAT;
	if ( clk_test( tcnt1t/2 - 1 ) != OK )
	{
		return ( FAIL );
	}

	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		clk_test_11()	 					-	Do 1 clock cycle.
 *
 *	RETURN VALUE
 *
 *		OK		-	Hardware values match active vector state values.
 *		FAIL	-	Test error.
 *
 *	DESCRIPTION
 *
 *		clk_test_11() runs through the 8 clock states necessary to sequence
 *		through a 1 clock pulse preceded by a 1 pulse.
 *		It returns a FAIL on the first FAIL.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Self_tst::clk_test_11( void )
{
	/*
	 *	Get present CNT0H for this cycle.
	 */
	tcnt0h		=	dcc_reg.get_cnt0h();	// Get present CNT0H.
	// (1R00-1R01) CNT0T rollover 0-1 test.
	clk_bits	=   CT_TCNT0H | CT_DEC1;
	if ( clk_test( 1 ) != OK )
	{
		return ( FAIL );
	}

	// (1R02-1R03) First 1 count.
	clk_bits	=   CT_TCNT0H | CT_DEC1;
	if ( clk_test( 1 ) != OK )
	{
		return ( FAIL );
	}

	// (1R04-1R05) CNT1T countdown test.
	tcnt1t		=	dcc_reg.get_cnt1t();	// Get present CNT1T.
	clk_bits	=   CT_TCNT0H | CT_DEC1 | CT_REPEAT;
	if ( clk_test( tcnt1t/2 - 1 ) != OK )
	{
		return ( FAIL );
	}

	// (1R06-1R09) CNT1T rollover 1-0 test.
	clk_bits	=   CT_TCNT0H | CT_DEC1;
	if ( clk_test( 2 ) != OK )
	{
		return ( FAIL );
	}

	// (1R10-1R11) CNT1T 0 countdown test.
	tcnt1t		=	dcc_reg.get_cnt1t();	// Get present CNT1T.
	clk_bits	=   CT_TCNT0H | CT_DEC1 | CT_REPEAT;
	if ( clk_test( tcnt1t/2 - 1 ) != OK )
	{
		return ( FAIL );
	}

	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		dynamic_test()	 					-	Test oscillator clock.
 *
 *	RETURN VALUE
 *
 *		OK		-	Test passed.
 *		FAIL	-	Test error.
 *
 *	DESCRIPTION
 *
 *		dynamic_test() runs the board via the oscillator and verifies that
 *		the time to send a 1 and 0 is correct.
 *		It returns a FAIL on the first FAIL.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Self_tst::dynamic_test( void )
{
#if SEND_VERSION >= 4
	RTC_TimeTypeDef sw_time;					// Test start time.
#else
	struct time	sw_time;					// Test start time.
#endif
	u_long		d_time;						// Time the test ran.

	dcc_reg.init_send();					// Initialize the hardware.
	dcc_reg.start_clk();					// Start oscillator clock.
	OUT_PC( PC_POS_UNDERCLRL, 0 );			// Clear underflow.

	start_watch( &sw_time );
	if ( dcc_reg.send_bytes( ZERO_SEC_NOM, 0x00, "Self test 0 check." ) != OK )
	{
		dcc_reg.stop_clk();
		printf( "Fail for send_bytes( 0x00 )\n" );
		return ( FAIL );
	}

	d_time	=	stop_watch( &sw_time );
	sum_0	+=	d_time;
	if ( d_time < min_0 )
	{
		min_0	=	d_time;
	}
	if ( d_time > max_0 )
	{
		max_0	=	d_time;
	}
	if ( d_time < TEST_ZERO_MIN || d_time > TEST_ZERO_MAX )
	{
		dcc_reg.stop_clk();
		printf( "\nFailed 0 test, d_time %lu out of range\n", d_time );
		return ( FAIL );
	}
	
	OUT_PC( PC_POS_UNDERCLRL, 0 );			// Clear underflow.

	start_watch( &sw_time );
	if ( dcc_reg.send_bytes( ONE_SEC_NOM, 0xff, "Self test 1 check." ) != OK )
	{
		dcc_reg.stop_clk();
		printf( "Fail for send_bytes( 0xff )\n" );
		return ( FAIL );
	}
	d_time	=	stop_watch( &sw_time );
	sum_1	+=	d_time;
	if ( d_time < min_1 )
	{
		min_1	=	d_time;
	}
	if ( d_time > max_1 )
	{
		max_1	=	d_time;
	}
	if ( d_time < TEST_ONE_MIN || d_time > TEST_ONE_MAX )
	{
		dcc_reg.stop_clk();
		printf( "\nFailed 1 test, d_time %lu out of range\n", d_time );
		return ( FAIL );
	}

	dcc_reg.stop_clk();					// Stop oscillator clock.
	return ( OK );
}

#if SEND_VERSION >= 4

/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		stop_watch()	 					-	Find elapsed time.
 *
 *	RETURN VALUE
 *
 *		delta	-	Elapsed time in 100ths of secodnd.
 *
 *	DESCRIPTION
 *
 *		stop_watch() gets the present time, subtracts the passed in 'sw_time'
 *		from it, and returns the elapsed time in 100ths of a second.
 */
/*--------------------------------------------------------------------------*/
u_long
Self_tst::stop_watch(
const RTC_TimeTypeDef* sw_time ) const
{
	RTC_TimeTypeDef	p_time;						// Present time.
	u_long		start_time;					// Start in 100ths of a second.
	u_long		end_time;					// End in 100ths of a second.
	long		delta_time;					// Delta time in 100ths of a second.
	RTC_DateTypeDef date;

	GetTime ( &p_time, &date );


//uint8_t Hours;            /*!< Specifies the RTC Time Hour.
//uint8_t Minutes;          /*!< Specifies the RTC Time Minutes.
//uint8_t Seconds;          /*!< Specifies the RTC Time Seconds.
//uint8_t TimeFormat;       /*!< Specifies the RTC AM/PM Time.
//uint32_t SubSeconds;      /*!< Specifies the RTC_SSR RTC Sub Second register content.
//uint32_t SecondFraction;  /*!< Specifies the range or granularity of Sub Second register content

	/*
	 *	Find the start time in 100ths of a second.
	 */
	start_time	=		((u_long)sw_time->SubSeconds)
					+	((u_long)sw_time->Seconds   	* 100L)
					+	((u_long)sw_time->Minutes	* 100L * 60L)
					+	((u_long)sw_time->Hours		* 100L * 60L * 60L );

	/*
	 *	Find the end time in 100ths of a second.
	 */
	end_time	=		((u_long)p_time.SubSeconds)
					+	((u_long)p_time.Seconds		* 100L)
					+	((u_long)p_time.Minutes		* 100L * 60L)
					+	((u_long)p_time.Hours		* 100L * 60L * 60L );

	/*
	 *	Handle the day rollover case.
	 */
	if ( ( delta_time = end_time - start_time ) < 0 )
	{
		delta_time += 100L * 60L * 60L * 24L;
	}

	return ( (u_long)delta_time );
}

void
Self_tst::start_watch( RTC_TimeTypeDef* sw_time )
{
	RTC_DateTypeDef p_date;

	GetTime ( sw_time, &p_date );
}

#else

/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		stop_watch()	 					-	Find elapsed time.
 *
 *	RETURN VALUE
 *
 *		delta	-	Elapsed time in 100ths of secodnd.
 *
 *	DESCRIPTION
 *
 *		stop_watch() gets the present time, subtracts the passed in 'sw_time'
 *		from it, and returns the elapsed time in 100ths of a second.
 */
/*--------------------------------------------------------------------------*/

u_long
Self_tst::stop_watch(
const time		&sw_time ) const
{
	struct time	p_time;						// Present time.
	u_long		start_time;					// Start in 100ths of a second.
	u_long		end_time;					// End in 100ths of a second.
	long		delta_time;					// Delta time in 100ths of a second.

	gettime ( &p_time );

	/*
	 *	Find the start time in 100ths of a second.
	 */
	start_time	=		((u_long)sw_time.ti_hund)
					+	((u_long)sw_time.ti_sec   	* 100L)
					+	((u_long)sw_time.ti_min		* 100L * 60L)
					+	((u_long)sw_time.ti_hour	* 100L * 60L * 60L );

	/*
	 *	Find the end time in 100ths of a second.
	 */
	end_time	=		((u_long)p_time.ti_hund)
					+	((u_long)p_time.ti_sec		* 100L)
					+	((u_long)p_time.ti_min		* 100L * 60L)
					+	((u_long)p_time.ti_hour		* 100L * 60L * 60L );

	/*
	 *	Handle the day rollover case.
	 */
	if ( ( delta_time = end_time - start_time ) < 0 )
	{
		delta_time += 100L * 60L * 60L * 24L;
	}

	return ( (u_long)delta_time );
}
#endif


/*****************************************************************************
 * $History: SELF_TST.CPP $
 * 
 * *****************  Version 8  *****************
 * User: Kenw         Date: 7/24/19    Time: 10:15p
 * Updated in $/NMRA/SRC/SEND
 * Removed spurious sccsid warning.
 * 
 * *****************  Version 7  *****************
 * User: Kenw         Date: 4/28/17    Time: 8:49p
 * Updated in $/NMRA/SRC/SEND
 * Shortened @(#)  Revision string to just Workfile and Revision.
 * 
 * *****************  Version 4  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:57p
 * Updated in $/NMRA/SRC/SEND
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 3  *****************
 * User: Kenw         Date: 6/28/98    Time: 9:38a
 * Updated in $/NMRA/SRC/SEND
 * Changed to abort program at LOG_ERR. Rolling to X.2.12.
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 10/29/97   Time: 4:26p
 * Updated in $/NMRA/SRC/SEND
 * X.2.8 - Added packet log command.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 3:57p
 * Created in $/NMRA/SRC/SEND
 * Methods for 'class Self_tst'.
 *
 *****************************************************************************/

