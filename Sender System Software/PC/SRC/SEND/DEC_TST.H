/*****************************************************************************
 *
 * File:                 $Workfile: DEC_TST.H $
 * Path:                 $Logfile: /NMRA/SRC/SEND/DEC_TST.H $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 23 $
 * Last Modified on:     $Modtime: 8/14/23 2:42p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 8/16/23 3:40p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	dec_tst.h	-	Defines the Dec_tst class which performs the decoder
 *					tests.
 *
 *****************************************************************************/

#ifndef DEC_TST_H_DECLARED
#define DEC_TST_H_DECLARED	"@(#) $Workfile: DEC_TST.H $$ $Revision: 23 $$"

#include "t_stat.h"
#include "args.h"

#include <bits.h>

#include <stdio.h>

/*
 *	List of clock values to use for decoder tests.
 */
struct Dec_clk
{
	u_short			clk0t;			  			// 0T clock in microseconds.
	u_short			clk0h;		  	   			// 0H clock in microseconds.
	u_short			clk1t;		  	   			// 1T clock in microseconds.
	char	  		*msg;			   			// Comment on clock.
};

/*
 *	Types of clock stretches.
 */
enum Stretch_types
{
	S_NONE			=  0,			  			// No stretch.
	S_POS			=  1,			  			// Positive stretch.
	S_NEG			= -1			  			// Negative stretch.
};

/*
 *	List of stretched 0 clock values to use for decoder tests.
 */
struct Str0_clk
{
	u_short			clk0t_delta;		   		// 0T clock delta in usecs.
	Stretch_types	s_type;	  					// Type of stretch.
	char	  		*msg;						// Comment on clock.
};

struct Ames_type
{
	u_int			pre_cnt;					// Preamble bits to use.
	u_int			idle_cnt;				 	// Idles to use.
};

struct Ambig1_type
{
	u_short			ambig1_0t;					// Ambiguous 0T time.
    u_short			ambig1_0h;					// Ambiguous 0H time.
};

struct Ambig2_type
{
	u_short			ambig2_0t1;					// Ambiguous 0T1 time.
    u_short			ambig2_0h1;					// Ambiguous 0H1 time.
	u_short			ambig2_0t2;					// Ambiguous 0T2 time.
    u_short			ambig2_0h2;					// Ambiguous 0H2 time.
};
/*
 *	Dec_tst object.
 */
class Dec_tst : public Z_core
{
  public:
	/* Method section */
	Dec_tst( void );

	~Dec_tst();

    /* Set methods */

	void print_user_docs( FILE *ofp );

	void set_decoder_type( Dec_types itype )  { decoder_type = itype; }
	void set_run_mask( Bits_t irun_mask )     { run_mask = irun_mask; }
	void set_clk_mask( Bits_t iclk_mask )     { clk_mask = iclk_mask; }
	void set_trig_rev( bool itrig_rev = false );
	void set_fill_msec( u_long ifill_msec = MSEC_PER_SEC )
	{
		fill_usec	=	ifill_msec * 1000UL;
	}

	Rslt_t	decoder_test( Rslt_t &tst_rslt );

  protected:
  
	/* Method section */
	void debug_gen( const char *msg = "" );
	void fsoc_send( const char *msg );

	/* Data section */
	static const Dec_clk		dclk_tbl[];	   		// Decoder test clk array.
	static const u_int  		dclk_size;		   	// Size of clock array.
	static const Str0_clk   	str0_tbl[];			// Stretch 0 test clk array.
	static const u_int			str0_size;			// Size of stretch 0 array.
	static const Ames_type		ames_tbl[];         // Ames test parameters.
	static const u_int			ames_tbl_size;	   	// Size of Ames table.
    static const Ambig1_type	ambig1_tbl[];	   	// Ambiguous 1 test params.
	static const u_int			ambig1_tbl_size;	// Size of Ambig 1 table.
	static const Dec_clk		ambig2_dclk_tbl[];	// Decoder test clk array.
	static const u_int  		ambig2_dclk_size;  	// Size of clock array.
    static const Ambig2_type	ambig2_tbl[];		// Ambiguous 2 test params.
	static const u_int			ambig2_tbl_size;	// Size of Ambig 2 table.
	T_stat						t_stat;		   	  	// Test statistics.
	u_long						tst_cnt;	   	  	// Present test count.
	Bits_t						run_mask;	   	   	// Tests to run.
	Bits_t						clk_mask;			// Clocks to try.
	Bits						dcc_bits;	   	   	// Packet scratchpad.
	Bits						dcc_bits2;	   	   	// Packet scratchpad.
	Bits						dcc_bits3;			// Packet scratchpad.
	Fsoc_bits					fsoc;		   	   	// Fail safe packet.
	int							trig_cmd_speed;		// Trigger speed command.
	u_long						fill_usec;			// Fill time in usec.
	u_short						filler_idles;  	   	// Count of filler idles.
	u_short						pkt_rep_cnt;		// Test packet repeat count.
    char						m_tst_name[128];	// Buffer for tst_name.
	bool						trig_rev;			// Handle reverse differently.
	Dec_types					decoder_type;	  	// Decoder type.

	/* Method secion */
	bool	get_test_break();
	Rslt_t	decoder_ramp( Rslt_t &tst_rslt );
	Rslt_t	acc_ramp( Rslt_t &tst_rslt );
	Rslt_t	func_ramp( Rslt_t &tst_rslt );
	Rslt_t	sig_ramp( Rslt_t &tst_rslt );
	Rslt_t	decoder_ames( Rslt_t &tst_rslt, u_int pre_cnt, u_int idle_cnt );
	Rslt_t	decoder_bad_addr( Rslt_t &tst_rslt );
	Rslt_t	decoder_bad_bit( Rslt_t &tst_rslt );
	Rslt_t	decoder_margin_1( void );
	Rslt_t	decoder_duty_1( void );
	Rslt_t	quick_ames(	u_int &f_cnt, const char *tst_name, u_short tclk0t,
						u_short tclk0h, u_short tclk1t, u_int margin_pre,
						bool swap_0_1 );
  	Rslt_t	decoder_truncate( Rslt_t &tst_rslt );
	Rslt_t	decoder_prior( Rslt_t &tst_rslt );
	Rslt_t	decoder_6_byte( Rslt_t &tst_rslt );
    Rslt_t	decoder_ambig1( Rslt_t &tst_rslt );
    Rslt_t	decoder_ambig2( Rslt_t &tst_rslt );
	Rslt_t	decoder_str0_ames(	Rslt_t &tst_rslt,
								u_short iclk0t, u_short iclk0h );
	void	calc_filler( u_short clk0t, u_short clk1t );
	Rslt_t	send_filler( void );
	const char	*err_phrase(
		bool pre_fail, bool trig_fail, bool rst_fail = false );
	const char	*min_phrase( u_short t_min, u_short out_of_range );
	const char	*max_phrase( u_short t_max, u_short out_of_range );
	const char	*min_duty_phrase(
		u_short t_nom, u_short t_min, u_short out_of_range );
	const char	*max_duty_phrase(
		u_short t_nom, u_short t_max, u_short out_of_range );
    void	print_test_rslt(
    	Rslt_t	tst_rslt )
    {
        if ( tst_rslt == OK )
     	{
    		printf(
	        	"All clock tests have currently passed\n" );
    	}
    	else
    	{
    		printf(
        		"One or more clock tests have currently failed\n" );
    	}
    }

};

#endif /* DEC_TST_H_DECLARED */
