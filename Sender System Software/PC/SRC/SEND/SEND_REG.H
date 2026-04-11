/*****************************************************************************
 *
 * File:                 $Workfile: SEND_REG.H $
 * Path:                 $Logfile: /NMRA/SRC/SEND/SEND_REG.H $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 13 $
 * Last Modified on:     $Modtime: 9/14/23 4:03p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 9/14/23 4:12p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	send_reg.h	-	This file defines the Send_reg class which handles
 *					interractions with the sender hardware.
 *
 *****************************************************************************/

#ifndef SEND_REG_H_DECLARED
#define SEND_REG_H_DECLARED	"@(#) $Workfile: SEND_REG.H $$ $Revision: 13 $$"

#include <zlog.h>
#include <dcc.h>
#include "sr_core.h"

const u_short	DCC_CLK_HOLD	= 0;		// Hold present clock value.
const u_short	DCC_CLK_ERR		= 1;		// Clock has an error.
const u_short	DCC_CLK_MIN		= 10;		// Minimum valid clock value.
const u_int		PKT_SIZE		= 5;		// Size of basic packets.
/*
 *	Count of 1s and 0s added to fill out 1,2, or 4 39 bit packets
 *	up the the next Byte boundary.
 */
const u_int		FILL_1_0S       = 0;		// 1 packet 0 filler.
const u_int		FILL_1_1S		= 1;		// 1 packet 1 filler.
const u_int		FILL_2_0S		= 1;		// 2 packet 0 filler.
const u_int		FILL_2_1S		= 1;		// 2 packet 1 filler.
const u_int		FILL_4_0S		= 3;		// 4 packet 0 filler.
const u_int		FILL_4_1S		= 1;		// 4 packet 1 filler.
const u_short	PKT_REP_MIN		= 10;		// Min. pkt repeat count.


const u_int		PC_DELAY_INIT	= 5;		// Initial 1 usec delay count.

enum Scope_states
{
	SCOPE_IDLE,								// Scope trigger idle.
	SCOPE_END,								// End Scope trigger.
	SCOPE_START								// Start scope trigger.
};

class Bits;									// Defined in bits.h
class Args_obj;								// Defined in args.h
class Sender_hw_if;

class Send_reg : public Sr_core
{
  public:
  
	/* Static method section */
	
	/*
	 * Calculate the number of repeats usings 1 idle packet.
	 * 1 39 bit packet plus filler requires 5 Bytes.
	 */
	static u_short fill_idles (		u_long  ifill_usec = USEC_PER_SEC,
									u_short iclk0t     = DECODER_0T_NOM,
									u_short iclk1t     = DECODER_1T_NOM);
	/*
	 * Calculate the number of repeats usings 1 command + idle packet pair.
	 * 2 39 bit packets plus filler require 10 Bytes.
	 */
	static u_short fill_cmds_1 (	u_long  ifill_usec = USEC_PER_SEC,
									u_short iclk0t     = DECODER_0T_NOM,
									u_short iclk1t     = DECODER_1T_NOM);
									
	/*
	 * Calculate the number of repeats usings 2 command + idle packet pairs.
	 * 4 39 bit packets plus filler require 20 Bytes.
	 */
	static u_short fill_cmds_2 (	u_long  ifill_usec = USEC_PER_SEC,
									u_short iclk0t     = DECODER_0T_NOM,
									u_short iclk1t     = DECODER_1T_NOM);

	/* Method section */
	
	Send_reg( void )
	:	running( false ), scope( SCOPE_IDLE ),
		clk0t( DCC_CLK_ERR ), clk0h( DCC_CLK_ERR ), clk1t( DCC_CLK_ERR ),
		err_cnt( 0 ), m_pc_delay_1usec( PC_DELAY_INIT ),
		m_swap_0_1( false ), m_log_pkts( false ), m_hw_if( NULL )
	{
		rst_stats();
	}

	/* Simple get methods */
	bool	get_running( void ) const { return ( running ); }
	u_long	get_b_cnt( void ) const { return ( b_cnt ); }
	u_long	get_p_cnt( void ) const { return ( p_cnt ); }
	u_int	get_scope( void ) const { return ( scope ); }
	u_short	get_clk0t( void ) const { return ( clk0t ); }
	u_short	get_clk0h( void ) const { return ( clk0h ); }
	u_short	get_clk1t( void ) const { return ( clk1t ); }
	bool	get_swap_0_1( void ) const { return ( m_swap_0_1 ); }
	bool	get_log_pkts( void ) const { return ( m_log_pkts ); }

	BYTE	get_gen( void ) const;

	/* Simple set methods */
	void	set_swap_0_1( bool iswap ) { m_swap_0_1 =  iswap; }
	void	set_log_pkts( bool ilog_pkts ) { m_log_pkts =  ilog_pkts; }
	void	set_hw_if( Sender_hw_if *ihw_if ) { m_hw_if = ihw_if; }

	/* Board initialization and reset routines */
	Rslt_t	set_clk( u_short iclk0t, u_short iclk0h, u_short iclk1t );
	Rslt_t	set_pc_delay_1usec( void );

	void	set_scope( bool scope_on );

	/* Packet sending routines */
	void	clr_under( void );
	void	clr_err_cnt( void ) { err_cnt = 0; }
	Rslt_t	start_clk( void );
	Rslt_t	stop_clk( void );
	Rslt_t	send_rst( void )
	{ return ( send_pkt(rst_bytes,PKT_SIZE,"Reset") ); }
	Rslt_t	send_hard_rst( void )
	{ return ( send_pkt(rst_hard_bytes,PKT_SIZE,"Reset") ); }
	Rslt_t	send_idle( void )
	{ return ( send_pkt(idle_bytes,PKT_SIZE,"Idle") ); }
	Rslt_t	send_base( void )
	{ return ( send_pkt(base_bytes,PKT_SIZE,"Baseline") ); }
	Rslt_t	send_bytes( u_int icnt, BYTE ibyte, const char *info );
	Rslt_t	send_stretched_byte(
									u_short iclk0t, u_short iclk0h,
									BYTE ibyte, const char *into );
	Rslt_t	send_1_ambig_bit(
									u_short iclk0t, u_short iclk0h,
									BYTE ibyte, const char *into );
	Rslt_t	send_2_ambig_bits(
									u_short iclk0t1, u_short iclk0h1,
									u_short iclk0t2, u_short iclk0h2,
									BYTE ibyte, const char *into );
	Rslt_t	send_pkt( Bits &ibits, const char *info );
	Rslt_t	send_pkt(	const BYTE *ibytes, u_int isize,
						const char *info );

	// Send the kick start packet sequence to the decoder.
	Rslt_t kick_start( Bits & ibits, Args_obj & iargs );

	bool errprint_ok( void );

	/* Packet statistics routines */
	void	print_stats( void );
	void
	rst_stats( void )
	{
		b_cnt	=	0L;
		p_cnt	=	0L;
	}

	void	update( void );					// Get hardware state.

	void	up_print( bool pr_hdr_flag = false );

	void
	gen_print( void ) const
	{
		print_nibble( get_gen() );
	}

  protected:
	
	/* Data section */
	
	static const BYTE	rst_bytes[PKT_SIZE];	// Reset Bytes.
	static const BYTE	rst_hard_bytes[PKT_SIZE];	// Hard reset Bytes.
	static const BYTE	idle_bytes[PKT_SIZE];	// Idle Bytes.
	static const BYTE	base_bytes[PKT_SIZE];	// Baseline Bytes.
	bool				running;				// true if running.
	u_long				p_cnt;					// Count of packets sent.
	u_long				b_cnt;					// Count of Bytes sent.
	Scope_states		scope;					// Scope trigger state.
	u_short				clk0t;					// 0T duration in usec.
	u_short				clk0h;					// 0H duration in usec.
	u_short				clk1t;					// 1T duration in usec.
	u_int				err_cnt;				// Count of contiguous errors.
	u_int				m_pc_delay_1usec;	   	// Loop count for 1 usec. delay.
	bool				m_swap_0_1;				// Swap 0 & 1 if true.
	bool				m_log_pkts;				// true to log pkts.
	Sender_hw_if		*m_hw_if;			// Optional MCU backend interface.

	/* Method section */
	
	void
	start_crit( void )
	{
		return;
	}

	u_int	pc_delay_high( register u_int idelay );
	u_int	pc_delay_low( register u_int idelay );

	void	print_pkt_log( const BYTE *ibytes, u_int isize );
};

#endif /* SEND_REG_H_DECLARED */
