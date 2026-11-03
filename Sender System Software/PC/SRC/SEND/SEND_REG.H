/*****************************************************************************
 *
 * DESCRIPTION:
 *
 *	send_reg.h	-	This file defines the Send_reg class which handles
 *					interractions with the sender hardware.
 *
 *****************************************************************************/

#ifndef SEND_REG_H_DECLARED
#define SEND_REG_H_DECLARED

#include <zlog.h>
#include <chrono>
#include <thread>
#include <dcc.h>
#include "send.h"

const u_short	DCC_CLK_HOLD	= 0;		// Hold present clock value.
const u_short	DCC_CLK_ERR		= 1;		// Clock has an error.
const u_short	DCC_CLK_MIN		= 40;		// Minimum valid clock value.
const u_int		PKT_SIZE		= 5;		// Size of basic packets.
/*
 *	Count of 1s and 0s added to fill out 4 39 bit packets
 *	up the the next Byte boundary.
 */
const u_int		FILL_4_0S		= 3;		// 4 packet 0 filler.
const u_int		FILL_4_1S		= 1;		// 4 packet 1 filler.
const u_short	PKT_REP_MIN		= 10;		// Min. pkt repeat count.

const BYTE		GEN_INPUT1_MASK		    = 0x0F;	// Valid GEN1 input bits.
const BYTE		GEN_MOTOR_FEEDBACK_MASK	= 0x03;	// Motor feedback mask bit.
const BYTE		GEN_MOTOR_P_BIT 		= 0x01; // Motor positive feedback bit.
const BYTE		GEN_MOTOR_N_BIT 		= 0x02; // Motor negative feedback bit.
const BYTE		GEN_FL_MASK	            = 0x04;	// Front Light mask bit.
const BYTE		GEN_FL_BIT 		        = 0x04; // Front Light bit.
const BYTE		GEN_RL_MASK	            = 0x08;	// Rear Light mask bit.
const BYTE		GEN_RL_BIT 		        = 0x08; // Rear Light bit.
const BYTE		GEN_LP_MASK 	        = 0x0C; // Light mask bit.
const BYTE		GEN_F1_BIT 		        = 0x10; // F1/AUX1 bit.
const BYTE		GEN_F1_MASK		        = 0x10; // F1/AUX1 mask bit.
const BYTE		GEN_F2_BIT 		        = 0x20; // F2/AUX2 bit.
const BYTE		GEN_F2_MASK		        = 0x20; // F2/AUX2 mask bit.
const BYTE		GEN_F3_BIT 		        = 0x40; // F3/AUX3 bit.
const BYTE		GEN_F3_MASK		        = 0x40; // F3/AUX3 mask bit.
const BYTE		GEN_F4_BIT 		        = 0x80; // F4/AUX4 bit.
const BYTE		GEN_F4_MASK		        = 0x80; // F4/AUX4 mask bit.

enum Scope_states
{
	SCOPE_IDLE,								// Scope trigger idle.
	SCOPE_END,								// End Scope trigger.
	SCOPE_START								// Start scope trigger.
};

class Bits;									// Defined in bits.h
class Args_obj;								// Defined in args.h
class Sender_hw_if;

class Send_reg
{
  public:
  
	/* Static method section */
	
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
		err_cnt( 0 ),
		m_swap_0_1( false ), m_no_hw_sim( false ), m_log_pkts( false ),
		m_log_data( false ), m_log_io( false ), m_hw_if( nullptr ),
		m_pkt_gap_msec( 100 )
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
	bool	get_no_hw_sim( void ) const { return ( m_no_hw_sim ); }
	bool	get_log_pkts( void ) const { return ( m_log_pkts ); }
	bool	get_log_data( void ) const { return ( m_log_data ); }

	BYTE	get_gen( void ) const;
	bool	get_gen_fb( Dec_types idec_type ) const;

	/* Simple set methods */
	void	set_swap_0_1( bool iswap ) { m_swap_0_1 =  iswap; }
	void	set_no_hw_sim( bool ino_hw_sim ) { m_no_hw_sim = ino_hw_sim; }
	void	set_log_pkts( bool ilog_pkts ) { m_log_pkts =  ilog_pkts; }
	void	set_log_data( bool ilog_data ) { m_log_data = ilog_data; }
	void	set_log_io( bool ilog_io ) { m_log_io = ilog_io; }
	void	set_hw_if( Sender_hw_if *ihw_if ) { m_hw_if = ihw_if; }
	void	set_pkt_gap_msec( u_int imsec ) { m_pkt_gap_msec = imsec; }

	// Sleep for the host side inter-packet delay (the Fill time).
	void	pkt_gap( void ) const
	{
		std::this_thread::sleep_for(
			std::chrono::milliseconds( m_pkt_gap_msec ) );
	}

	/* Board initialization and reset routines */
	Rslt_t	set_clk( u_short iclk0t, u_short iclk0h, u_short iclk1t );

	void	set_scope( bool scope_on );

	/* Packet sending routines */
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
	Rslt_t	send_raw_bytes_stretched( const BYTE *ibytes, u_int isize,
								u_int stretch_byte_index, u_short iclk0t,
								u_short iclk0h, const char *info );
	Rslt_t	send_raw_bytes_timed( const BYTE *ibytes, u_int isize,
								u_short bit_index1, u_short iclk0t1,
								u_short iclk0h1, u_short bit_index2,
								u_short iclk0t2, u_short iclk0h2, const char *info );
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
	Rslt_t	send_pkt( Bits &ibits, const char *info, bool add_gap = true );
	Rslt_t	send_pkt(	const BYTE *ibytes, u_int isize,
						const char *info, bool add_gap = true );

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

	// Print the 4 GEN input bits as 1s and 0s.
	void
	gen_print( void ) const
	{
		BYTE	gen = get_gen();

		for ( BYTE mask = 0x08; mask != 0; mask >>= 1 )
		{
			putchar( ( gen & mask ) ? '1' : '0' );
		}
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
	bool				m_swap_0_1;				// Swap 0 & 1 if true.
	bool				m_no_hw_sim;			// true to simulate with no MCU.
	bool				m_log_pkts;				// true to log pkts.
	bool				m_log_data;				// true to log packet data.
	bool				m_log_io;				// true to log IO data.
	Sender_hw_if		*m_hw_if;			// Optional MCU backend interface.
	u_int				m_pkt_gap_msec;		// Host delay between pkts (msec).

	/* Method section */
	void	print_pkt_log( const BYTE *ibytes, u_int isize );
};

#endif /* SEND_REG_H_DECLARED */
