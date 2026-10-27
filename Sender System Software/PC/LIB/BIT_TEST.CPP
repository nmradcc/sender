/*****************************************************************************
 *
 * DESCRIPTION:
 *
 *	bit_test.cpp	-	Unit test program for Bits class.
 *
 *****************************************************************************/

#include <bits.h>

#include <stdio.h>
#include <stdlib.h>

const int		BITS_SIZE		= 256;		// Size of Bits array.
const int		FLIP_TST_SIZE	=	2;		// Size of flip test array.

struct Dcc_vector
{
	int			v_size;						// Size of vector in Bytes.
	BYTE		v_array[64];				// Pointer to vector.
};

static Dcc_vector Dcc_vects[]	=
{
	{ 5, {0xff, 0xf0, 0x00, 0x00, 0x01}},		// ( 0) Reset pkt.
	{ 5, {0xff, 0xf7, 0xf8, 0x01, 0xff}},		// ( 1) Idle pkt.
	{ 5, {0xff, 0xf0, 0x19, 0x9c, 0xc9}},		// ( 2) Base 14 pkt.
	{ 5, {0xff, 0xf0, 0x19, 0x9c, 0xc9}},		// ( 3) Base 14 pkt 2.
	{ 5, {0xff, 0xf0, 0x00, 0x00, 0x01}},		// ( 4) Reset pkt.
	{ 5, {0xff, 0xf7, 0xf8, 0x01, 0xff}},		// ( 5) Idle pkt.
	{ 5, {0xff, 0xf0, 0x19, 0x9c, 0xc9}},		// ( 6) Base 14 pkt.
	{ 5, {0xff, 0xf0, 0x19, 0xdc, 0xe9}},		// ( 7) Base 14 pkt, lamp on.
	{ 5, {0xff, 0xf0, 0x19, 0x80, 0xc7}},		// ( 8) SP_MIN Base 14 pkt.
	{ 5, {0xff, 0xf0, 0x19, 0xbc, 0xd9}},		// ( 9) SP_MAX_14 Base 14 pkt.
	{ 5, {0xff, 0xf0, 0x19, 0x84, 0xc5}},		// (10) SP_E_STOP Base 14 pkt.
	{ 5, {0xff, 0xf0, 0x19, 0xd0, 0xef}},		// (11) Base 28 pkt.
	{ 5, {0xff, 0xf0, 0x19, 0x80, 0xc7}},		// (12) SP_MIN Base 28 pkt.
	{ 5, {0xff, 0xf0, 0x19, 0xfc, 0xf9}},		// (13) SP_MAX_28 Base 28 pkt.
	{ 5, {0xff, 0xf0, 0x19, 0x84, 0xc5}},		// (14) SP_E_STOP Base 28 pkt.
	{ 5, {0xff, 0xf0, 0x19, 0xc4, 0xe5}},		// (15) SP_E_STOP_I Base 28 pkt.
	{ 5, {0xff, 0xf0, 0x19, 0x88, 0xc3}},		// (16) Odd speed 1 Base 28 pkt.
	{ 5, {0xff, 0xf0, 0x19, 0xa4, 0xd5}},		// (17) Odd speed 15 Base 28 pkt.
	{ 6, {0xff, 0xfe, 0x03, 0x3a, 0x1d, 0xe0}},	// (18) 15 pre Base pkt.
	{ 5, {0xff, 0xf4, 0x13, 0xe4, 0xf7}},		// (19) Accessory pkt.
	{ 5, {0xff, 0xf5, 0xfa, 0x3c, 0x61}},		// (20) Accessory pkt.
	{ 5, {0xff, 0xf4, 0x12, 0x24, 0x17}},		// (21) Accessory pkt.
	{ 5, {0xff, 0xf4, 0x12, 0x04, 0x07}},		// (22) Accessory pkt.
    { 5, {0xff, 0xf0, 0x1a, 0x7d, 0x39}},		// (23) Func Grp 1 pkt.
    { 5, {0xff, 0xf0, 0x1a, 0xfd, 0x79}},		// (24) Func Grp 2 pkt.
    { 7, {0xff, 0xf4, 0x09, 0xc4, 0x00, 0xf0, 0x80}},	// 25: Sig pkt.
    { 7, {0xff, 0xf4, 0x09, 0xcc, 0x14, 0xf8, 0x80}},	// 26: Sig pkt.
    { 7, {0xff, 0xf4, 0x09, 0xdc, 0x3e, 0xe9, 0x80}},	// 27: Sig pkt.
    { 7, {0xff, 0xf4, 0x11, 0xc4, 0x3e, 0xec, 0x80}},	// 28: Sig pkt.
    { 7, {0xff, 0xf5, 0xf8, 0x14, 0x2a, 0xaf, 0x80}},	// 29: Sig pkt.
    { 7, {0xff, 0xf4, 0x09, 0xc4, 0x3e, 0xef, 0x80}},	// 30: Sig pkt.
    { 7, {0xff, 0xf4, 0x09, 0xc4, 0x3e, 0xef, 0x80}},	// 31: Sig pkt.
};
const	int		DCC_VECTS_SIZE	=	sizeof( Dcc_vects )/sizeof( Dcc_vector );

static int		g_checks		= 0;		// Checks run.
static int		g_failures		= 0;		// Checks failed.

#define RECORD_FAIL()	( ++g_checks, ++g_failures )

#define CHECK( cond )														\
	do																		\
	{																		\
		++g_checks;															\
		if ( !(cond) )														\
		{																	\
			++g_failures;													\
			printf( "FAIL line %d: %s\n", __LINE__, #cond );				\
		}																	\
	} while ( 0 )

// Value of the bit at 'pos' in a Bits array, where bit 0 is the MSB of Byte 0.
static bool
bit_at(
	const Bits		&b,
	u_int			pos )
{
	return ( ((b.get_byte_array()[pos / BITS_IN_BYTE]
				>> (MAX_BIT_POS - (pos % BITS_IN_BYTE))) & 1) != 0 );
}

// Fill a Bits object with 0s or 1s and verify the contents and range checks.
static void
test_fill(
	Bits			&b,
	bool			ones )
{
	const BYTE		expect	= ones ? 0xff : 0x00;
	BYTE			obyte	= 0;

	b.clr_in();

	if ( ones )
	{
		b.put_1s( BITS_SIZE * BITS_IN_BYTE );
	}
	else
	{
		b.put_0s( BITS_SIZE * BITS_IN_BYTE );
	}

	CHECK( b.get_obj_errs() == 0 );
	CHECK( b.get_bit_size() == static_cast<u_int>(BITS_SIZE * BITS_IN_BYTE) );
	CHECK( b.get_byte_size() == static_cast<u_int>(BITS_SIZE) );

	for ( int i = 0; i < BITS_SIZE; i++ )
	{
		CHECK( b.get_byte( obyte ) == OK );
		CHECK( obyte == expect );
	}

	CHECK( b.get_byte( obyte ) == FAIL );

	b.put_byte( expect );
	CHECK( b.get_obj_errs() != 0 );

	b.clr_in();
	CHECK( b.get_obj_errs() == 0 );
	CHECK( b.get_bit_size() == 0 );
}

static void
test_misc(
	Bits			&b )
{
	BYTE			obyte	= 0;

	b.clr_in().clr_check();
	CHECK( b.get_check() == CHECK_INIT );

	b.put_byte( 0x0f ).put_byte( 0xf3 );
	CHECK( b.get_check() == 0xfc );
	b.clr_check();
	CHECK( b.get_check() == CHECK_INIT );

	CHECK( b.get_byte( obyte ) == OK && obyte == 0x0f );
	CHECK( b.get_byte( obyte ) == OK && obyte == 0xf3 );
	CHECK( b.get_byte( obyte ) == FAIL );
	b.rst_out();
	CHECK( b.get_byte( obyte ) == OK && obyte == 0x0f );

	b.clr_in().put_1s( 16 );
	CHECK( b.clr_bit( 3 ) == OK );
	CHECK( b.clr_bit( 15 ) == OK );
	CHECK( b.clr_bit( 16 ) == FAIL );

	for ( u_int pos = 0; pos < 16; pos++ )
	{
		CHECK( bit_at( b, pos ) == (pos != 3 && pos != 15) );
	}

	b.clr_in();
}
Fsoc_bits	Fsoc;
Idle_bits	Idle( 2 );

int
main( void )
{
	int				i,j;					// Index variables.
	int				ftype;					// Type of fill bit (0,1).
	BYTE			obyte;					// Test Byte.
	Bits			my_bits( BITS_SIZE );
	Bits			flip_tst( FLIP_TST_SIZE );
    bool			ignoreError;		  	// Ignore expected object error.

	printf( "Begin Bits test. Bits version %u.%u.%u\n",
    									Bits::ver_major,
                                		Bits::ver_minor,
                                		Bits::ver_build );

	printf( "Check of constants\n" );
	printf( "  USEC_PER_SEC %lu\n", USEC_PER_SEC );
	printf(
		"  DECODER_0T_MIN %u, DECODER_0T_NOM %u, DECODER_0T_MAX %u\n",
		DECODER_0T_MIN, DECODER_0T_NOM, DECODER_0T_MAX );
	printf(
		"  DECODER_0H_MIN %u, DECODER_0H_NOM %u, DECODER_0H_MAX %u\n",
		DECODER_0H_MIN, DECODER_0H_NOM, DECODER_0H_MAX );
	printf(
		"  DECODER_1T_MIN %u, DECODER_1T_NOM %u, DECODER_1T_MAX %u\n",
		DECODER_1T_MIN, DECODER_1T_NOM, DECODER_1T_MAX );
	printf(
		"  DECODER_AVG_MIN %u, DECODER_AVG_NOM %u, DECODER_AVG_MAX %u\n",
		DECODER_AVG_MIN, DECODER_AVG_NOM, DECODER_AVG_MAX );
	printf(
		"  ZERO_SEC_MIN %u, ZERO_SEC_NOM %u, ZERO_SEC_MAX %u\n",
		ZERO_SEC_MIN, ZERO_SEC_NOM, ZERO_SEC_MAX );
	printf(
		"  ONE_SEC_MIN %u, ONE_SEC_NOM %u, ONE_SEC_MAX %u\n",
		ONE_SEC_MIN, ONE_SEC_NOM, ONE_SEC_MAX );
	printf(
		"  AVG_SEC_MIN %u, AVG_SEC_NOM %u, AVG_SEC_MAX %u\n",
		AVG_SEC_MIN, AVG_SEC_NOM, AVG_SEC_MAX );
	printf(
		"  BASE_SEC_MIN %u, BASE_SEC_NOM %u, BASE_SEC_MAX %u\n",
		BASE_SEC_MIN, BASE_SEC_NOM, BASE_SEC_MAX );
	printf(
		"  RESET_SEC_MIN %u, RESET_SEC_NOM %u, RESET_SEC_MAX %u\n",
		RESET_SEC_MIN, RESET_SEC_NOM, RESET_SEC_MAX );
	printf(
		"  IDLE_SEC_MIN %u, IDLE_SEC_NOM %u, IDLE_SEC_MAX %u\n",
		IDLE_SEC_MIN, IDLE_SEC_NOM, IDLE_SEC_MAX );
	printf(
		"  THREE_BITS %u, FOUR_BITS %u, FIVE_BITS %u SIX_BITS %u\n",
		THREE_BITS, FOUR_BITS, FIVE_BITS, SIX_BITS );
	printf(
		"  PRE_BITS %u, BASE_BITS %u, SIG_BITS %u\n",
		PRE_BITS, BASE_BITS, SIG_BITS );

	CHECK( Fsoc.get_obj_errs() == 0 );
	CHECK( Fsoc.get_bit_size() > 0 );

	printf( "Fsoc: size %u Bytes, %u bits, array -\n",
		Fsoc.get_isize(), Fsoc.get_bit_size() );
	Fsoc.print();

	CHECK( Idle.get_obj_errs() == 0 );
	CHECK( Idle.get_bit_size() > 0 );

	printf( "Idle: size %u Bytes, %u bits, array -\n",
		Idle.get_isize(), Idle.get_bit_size() );
	Idle.print();

	CHECK( my_bits.get_obj_errs() == 0 );
	CHECK( my_bits.get_isize() == static_cast<u_int>(BITS_SIZE) );
	CHECK( my_bits.get_bit_size() == 0 );

	printf( "my_bits: size %u Bytes, %u bits\n",
		my_bits.get_isize(), my_bits.get_bit_size() );

	CHECK( flip_tst.get_obj_errs() == 0 );
	CHECK( flip_tst.get_isize() == static_cast<u_int>(FLIP_TST_SIZE) );

	printf( "flip_tst: size %u\n", flip_tst.get_isize() );

	printf( "Test put_0s()\n" );
	test_fill( my_bits, false );

	printf( "Test put_1s()\n" );
	test_fill( my_bits, true );

	printf( "Test get_check(), rst_out(), clr_bit()\n" );
	test_misc( my_bits );

	printf( "Begin test vectors\n" );

	for ( i = 0; i < DCC_VECTS_SIZE; i++ )
	{
		my_bits.clr_in();
        ignoreError	=	false;

		switch ( i )
		{
		case  0:	// Low level Reset
			my_bits.put_1s(PRE_BITS).put_0s(1).put_byte(0x00).put_0s(1);
			my_bits.put_byte(0x00).put_0s(1).put_check().put_1s(1).done();
			break;

		case  1:	// Low level Idle
			my_bits.put_1s(PRE_BITS).put_0s(1).put_byte(0xff).put_0s(1);
			my_bits.put_byte(0x00).put_0s(1).put_check().put_1s(1).done();
			break;

		case  2:	// Low level 14 step Baseline
			my_bits.put_1s(PRE_BITS).put_0s(1).put_byte(0x03).put_0s(1);
			my_bits.put_byte(0x67).put_0s(1).put_check().put_1s(1).done();
			break;

		case  3:	// Low level 14 step Baseline with put_cmd()
			my_bits.put_1s(PRE_BITS).put_0s(1).put_byte(0x03).put_0s(1);
			my_bits.put_cmd_14(true, false, 6 );
			my_bits.put_0s(1).put_check().put_1s(1).done();
			break;

		case  4:	// High level Reset
			my_bits.put_reset_pkt( 1 ).put_1s( 1 ).done();
			break;

		case  5:	// High level Idle
			my_bits.put_idle_pkt( 1 ).put_1s( 1 ).done();
			break;

		case  6:	// High level 14 step Baseline
			my_bits.put_cmd_pkt_14( 0x03, true, false, 6 ).put_1s( 1 ).done();
			break;

		case  7:	// High level 14 step Baseline
			my_bits.put_cmd_pkt_14( 0x03, true, true, 6 ).put_1s( 1 ).done();
			break;

		case  8:	// SP_MIN 14 step Baseline
			my_bits.put_cmd_pkt_14( 0x03, true, false, SP_MIN );
			my_bits.put_1s( 1 ).done();
			break;

		case  9:	// SP_MAX 14 step Baseline
			my_bits.put_cmd_pkt_14( 0x03, true, false, SP_14_MAX );
			my_bits.put_1s( 1 ).done();
			break;

		case  10:	// SP_E_STOP 14 step Baseline
			my_bits.put_cmd_pkt_14( 0x03, true, false, SP_E_STOP );
			my_bits.put_1s( 1 ).done();
			break;

		case  11:	// High level 28 step Baseline
			my_bits.put_cmd_pkt_28( 0x03, true, 6 ).put_1s( 1 ).done();
			break;

		case  12:	// SP_MIN 28 step Baseline
			my_bits.put_cmd_pkt_28( 0x03, true, SP_MIN );
			my_bits.put_1s( 1 ).done();
			break;

		case  13:	// SP_MAX 28 step Baseline
			my_bits.put_cmd_pkt_28( 0x03, true, SP_28_MAX );
			my_bits.put_1s( 1 ).done();
			break;

		case  14:	// SP_E_STOP 28 step Baseline
			my_bits.put_cmd_pkt_28( 0x03, true, SP_E_STOP );
			my_bits.put_1s( 1 ).done();
			break;

		case  15:	// SP_E_STOP_I 28 step Baseline
			my_bits.put_cmd_pkt_28( 0x03, true, SP_E_STOP_I );
			my_bits.put_1s( 1 ).done();
			break;

		case  16:	// Odd speed step 1 28 step Baseline
			my_bits.put_cmd_pkt_28( 0x03, true, 1 );
			my_bits.put_1s( 1 ).done();
			break;

		case  17:	// Odd speed step 15 28 step Baseline
			my_bits.put_cmd_pkt_28( 0x03, true, 15 );
			my_bits.put_1s( 1 ).done();
			break;

		case 18:	// 15 Preamble High level Baseline
			my_bits.put_cmd_pkt_28( 0x03, true, 6, 15 ).put_1s( 1 );
			my_bits.done();
			break;

		case 19:	// Accessory packet, Addr 0x02, act. true, Out. 1.
			my_bits.put_acc_pkt( 0x02, true, 0x01 ).put_1s( 1 ).done();
			break;

		case 20:	// Accessory packet, Addr 0xffff, act. true, Out. 0xff.
			my_bits.put_acc_pkt( 0xffff, true, 0xff ).put_1s( 1 ).done();
			break;

		case 21:	// Accessory packet, Addr 450, act. true, Out. 1.
			my_bits.put_acc_pkt( 450, true, 1 ).put_1s( 1 ).done();
			break;

		case 22:	// Accessory packet, Addr 450, act. false, Out. 1.
			my_bits.put_acc_pkt( 450, false, 1 ).put_1s( 1 ).done();
			break;

		case 23:	// Func Grp 1, Addr 3, function 0x1f.
			my_bits.put_func_grp_pkt( 3, GRP_1, 0x1f ).put_1s( 1 ).done();
			break;

		case 24:	// Func Grp 2, Addr 3, function 0x1f.
			my_bits.put_func_grp_pkt( 3, GRP_2, 0x1f ).put_1s( 1 ).done();
			break;

		case 25:	// Signal packet, Addr 1, aspect 0.
			my_bits.put_sig_pkt( 1, 0 ).put_1s( 1 ).done();
			break;

		case 26:	// Signal packet, Addr 2, aspect 0x0a.
			my_bits.put_sig_pkt( 2, 0x0a ).put_1s( 1 ).done();
			break;

		case 27:	// Signal packet, 4, aspect 0x1f.
			my_bits.put_sig_pkt( 4, 0x1f ).put_1s( 1 ).done();
			break;

		case 28:	// Signal packet, 5, aspect 0x1f.
			my_bits.put_sig_pkt( 5, 0x1f ).put_1s( 1 ).done();
			break;

		case 29:	// Signal packet, Addr SIG_ADDR_MAX, aspect 0x15.
			my_bits.put_sig_pkt( SIG_ADDR_MAX, 0x15 ).put_1s( 1 ).done();
			break;

		case 30:	// Signal packet, Addr too low, aspect too big.
			my_bits.put_sig_pkt( SIG_ADDR_MIN - 1, 0xff ).put_1s( 1 ).done();
            ignoreError = true;
			break;

		case 31:	// Signal packet, Addr too high, aspect too big.
			my_bits.put_sig_pkt( SIG_ADDR_MAX + 1, 0xff ).put_1s( 1 ).done();
            ignoreError = true;
			break;

		default:
			printf( "UNEXPECTED test index %d\n", i );
			RECORD_FAIL();
			continue;
		}

		if ( my_bits.get_obj_errs() )
		{
			if ( ignoreError )
			{
				printf( "Test %3d: Ignoring expected obj error 0x%08lx\n",
											i, my_bits.get_obj_errs() );
				my_bits.clear_warn();
			}
			else
			{
				printf( "UNEXPECTED obj error 0x%08lx, Test %d\n",
					my_bits.get_obj_errs(), i );
				RECORD_FAIL();
			}
		}
		else if ( ignoreError )
		{
			printf( "UNEXPECTED Test %d did not set expected obj error\n", i );
			RECORD_FAIL();
		}

		printf( "Test %3d: Bytes - ", i );

		my_bits.print();

		CHECK( my_bits.get_byte_size() == static_cast<u_int>(Dcc_vects[i].v_size) );

		for ( j = 0; j < Dcc_vects[i].v_size; j++ )
		{
			if ( my_bits.get_byte( obyte ) != OK )
			{
				printf( "UNEXPECTED Test %d get_byte() FAIL, Byte %d\n", i, j );
				RECORD_FAIL();
				break;
			}

			if ( obyte != Dcc_vects[i].v_array[j] )
			{
				printf(
					"UNEXPECTED Test %d, Byte %d, "
					"Bits 0x%02x != expected 0x%02x\n",
					i, j, obyte, Dcc_vects[i].v_array[j] );
			}

			CHECK( obyte == Dcc_vects[i].v_array[j] );
		}

		CHECK( my_bits.get_byte( obyte ) == FAIL );
	}

	printf( "Completed all %d test vectors\n", DCC_VECTS_SIZE );

	for ( ftype = 0; ftype < 2; ftype++ )
	{
		printf( "Begin flipped %d bit tests\n", ftype );

		for ( i = 0; i <= BITS_IN_BYTE * FLIP_TST_SIZE; i++ )
		{
			const bool	fill	= (ftype != 0);	// Fill bit value.

			flip_tst.clr_in();
			CHECK( flip_tst.get_obj_errs() == 0 );

			if ( fill )
			{
				flip_tst.put_1s( i );
			}
			else
			{
				flip_tst.put_0s( i );
			}

			CHECK( flip_tst.get_obj_errs() == 0 );
			CHECK( flip_tst.get_bit_size() == static_cast<u_int>(i) );

			for ( j = 0; j < i; j++ )
			{
				CHECK( flip_tst.set_flip( j ) == OK );

				printf( "%d flip test, bit length %2d, shift %2d - ",
					ftype, i, j );
				flip_tst.print();

				// Only bit j differs from the fill value.
				for ( int k = 0; k < i; k++ )
				{
					CHECK( bit_at( flip_tst, k ) == ( (k == j) ? !fill : fill ) );
				}
			}

			CHECK( flip_tst.set_flip( i ) == FAIL );

			flip_tst.clr_flip();

			for ( int k = 0; k < i; k++ )
			{
				CHECK( bit_at( flip_tst, k ) == fill );
			}
		}
	}

	// Starting truncate() tests.
	printf( "Starting truncated packet tests.\n" );
	my_bits.clr_in().put_1s( BASE_BITS );

	CHECK( my_bits.get_bit_size() == BASE_BITS );
	CHECK( my_bits.truncate( my_bits.get_bit_size() + 1 ) == FAIL );

	printf( "my_bits: size %u Bytes, %u bits, array -\n",
		my_bits.get_isize(), my_bits.get_bit_size() );

	// Finish up packet and print it out.
	my_bits.done().print();

	for ( i = BASE_BITS; i >= 0; i-- )
	{
		my_bits.clr_in().put_1s( BASE_BITS );

		CHECK( my_bits.truncate( i ) == OK );
		CHECK( my_bits.get_bit_size() == static_cast<u_int>(i) );

		printf( "%3u bits - ", my_bits.get_bit_size() );

		// Finish up packet and print it out.
		my_bits.done().print();

		CHECK( my_bits.get_byte_size() == bits_to_bytes( i ) );

		// Kept bits are 1s and done() pads the last Byte with 0s.
		for ( j = 0; j < static_cast<int>(my_bits.get_byte_size()) * BITS_IN_BYTE; j++ )
		{
			CHECK( bit_at( my_bits, j ) == ( j < i ) );
		}
	}

	printf( "bit_test: %d checks, %d failures\n", g_checks, g_failures );

	return ( g_failures == 0 ? 0 : 1 );
}
