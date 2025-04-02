/*****************************************************************************
 *
 * File:                 $Workfile: SELF_TST.H $
 * Path:                 $Logfile: /NMRA/SRC/SEND/SELF_TST.H $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 2 $
 * Last Modified on:     $Modtime: 8/28/00 8:08p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 8/31/00 7:57p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	self_tst.h	-	Defines the Self_tst class which performs a self test
 *					of the sender hardware.
 *
 *****************************************************************************/

#ifndef SELF_TST_H_DECLARED
#define SELF_TST_H_DECLARED	"@(#) $Workfile: SELF_TST.H $$ $Revision: 2 $$"

#include "pds601.h"

#include "test_reg.h"

#include <z_core.h>

#include <limits.h>

#if SEND_VERSION >= 4
 #include "main.h"
#endif

struct	Test_reg_vm;						// Defined in test_reg.h.
class	Send_reg;							// Defined in send_reg.h.

/*
 *	clk_test() control flag values.
 */
const Bits_t	CT_TCNT0T		=	l_bit( 0 );		// Use tcnt0t.
const Bits_t	CT_TCNT0H		=	l_bit( 1 );		// Use tcnt0h.
const Bits_t	CT_TCNT1T		=	l_bit( 2 );		// Use tcnt1t.
const Bits_t	CT_DEC0			=	l_bit( 8 );		// Decrement 0.
const Bits_t	CT_DEC1			=	l_bit( 9 );		// Decrement 1.
const Bits_t	CT_REPEAT		=	l_bit( 16 );	// Repeat vectors.


#if SEND_VERSION >= 4
 #define CNT_0T_INIT		(116/2)
 #define CNT_0H_INIT		116
 #define CNT_1T_INIT		240
#endif

/*
 *	Self_tst object.
 */
class Self_tst : public Z_core
{
  public:
	/* Method section */
	Self_tst(
	Send_reg		&idcc_reg,				// Hardware register object.
	Test_reg_vm		*ivect,					// Test vectors.
	int				ivect_size )			// Size of test vector array.
	:	dcc_reg( idcc_reg ), treg( ivect, ivect_size ),
		p_vect( 0 ), clk_bits( 0L ),
		icnt0t(CNT_0T_INIT), icnt0h(CNT_0H_INIT), icnt1t(CNT_1T_INIT)
	{
		rst_stats();
	}

	~Self_tst() {}

	/* Simple get methods */
	u_long	get_tst_cnt( void ) const { return ( tst_cnt ); }
	u_long	get_tst_pass( void ) const { return ( tst_pass ); }
	u_long	get_tst_fail( void ) const { return ( tst_cnt - tst_pass ); }
	u_long	get_v_tst_pass( void ) const { return ( v_tst_pass ); }
	u_long	get_v_tst_fail( void ) const { return ( tst_cnt - v_tst_pass ); }
	u_long	get_d_tst_pass( void ) const { return ( d_tst_pass ); }
	u_long	get_d_tst_fail( void ) const { return ( tst_cnt - d_tst_pass ); }
	u_long	get_vd_tst_fail( void ) const { return ( vd_tst_fail ); }
	u_long	get_min_0( void ) const { return ( min_0 ); }
	u_long	get_avg_0( void ) const
	{
		return ( get_tst_cnt() ==  0 ? 0L : sum_0 / get_tst_cnt() );
	}
	u_long	get_max_0( void ) const { return ( max_0 ); }
	u_long	get_min_1( void ) const { return ( min_1 ); }
	u_long	get_avg_1( void ) const
	{
		return ( get_tst_cnt() ==  0 ? 0L : sum_1 / get_tst_cnt() );
	}
	u_long	get_max_1( void ) const { return ( max_1 ); }

	void	print_stats( void ) const;
	void	rst_stats( void )
	{
		tst_cnt		=	0L;
		tst_pass	=	0L;
		v_tst_pass	=	0L;
		d_tst_pass	=	0L;
		vd_tst_fail	=	0L;
		min_0		=	ULONG_MAX;
		min_1		=	ULONG_MAX;
		max_0		=	0L;
		max_1		=	0L;
		sum_0		=	0L;
		sum_1		=	0L;
	}
	
	Rslt_t	test_send( void );
	Rslt_t	do_test( void );
	void	do_tick( int cnt = 1 );
	Rslt_t	clk_test( int cnt = 1 );
	Rslt_t	clk_test_00( void );
	Rslt_t	clk_test_10( void );
	Rslt_t	clk_test_01( void );
	Rslt_t	clk_test_11( void );

  protected:
	/* Data section */
	Send_reg			&dcc_reg;			// Hardware register object.
	int					p_vect;				// Present test vector index.
	Test_reg			treg;				// Test compare object.
	u_short				tcnt0t;				// Temp CNT0T value.
	u_short				tcnt0h;				// Temp CNT0H value.
	u_short				tcnt1t;				// Temp CNT1T value.
	Bits_t				clk_bits;			// Clock test control bits.
	u_short				icnt0t;				// Initial CNT0T value.
	u_short				icnt0h;				// Initial CNT0H value.
	u_short				icnt1t;				// Initial CNT1T value.
	u_long				tst_cnt;			// Count of tests executed.
	u_long				tst_pass;			// Count of tests passed.
	u_long				v_tst_pass;			// Vector tests passed.
	u_long				d_tst_pass;			// Dynamic tests passed.
	u_long				vd_tst_fail;		// Vec. & Dyn. tests failed.
	u_long				min_0;				// Min 0 time.
	u_long				min_1;				// Min 1 time.
	u_long				max_0;				// Max 0 time.
	u_long				max_1;				// Max 1 time.
	u_long				sum_0;				// Sum of 0 times.
	u_long				sum_1;				// Sum of 1 times.

	/* Method secion */
	Rslt_t	vector_test( void );
	Rslt_t	dynamic_test( void );

#if SEND_VERSION >= 4
	void
	start_watch( RTC_TimeTypeDef* sw_time );

	u_long
	stop_watch( const RTC_TimeTypeDef* sw_time ) const;
#else
	void
	start_watch( struct time &sw_time )
	{
		gettime( &sw_time );
	}

	u_long
	stop_watch( const struct time &sw_time ) const;
#endif
};

#endif /* SELF_TST_H_DECLARED */


/*****************************************************************************
 * $History: SELF_TST.H $
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:57p
 * Updated in $/NMRA/SRC/SEND
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 3:48p
 * Created in $/NMRA/SRC/SEND
 * Header file for 'class Self_tst'  which performs the self tests on the
 * sender
 * board hardware prior to running decoder tests.
 *
 *****************************************************************************/

