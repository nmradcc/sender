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
 * Current version:      $Revision: 12 $
 * Last Modified on:     $Modtime: 6/21/23 5:18p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 6/22/23 7:27p $
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
#define SEND_REG_H_DECLARED	"@(#) $Workfile: SEND_REG.H $$ $Revision: 12 $$"

#include <zlog.h>
#include "sr_core.h"

//k #include "track.h"
#include "DCC.h"
#include "pds601.h"

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
		do_crit( false ), err_cnt( 0 ), m_pc_delay_1usec( PC_DELAY_INIT ),
		m_swap_0_1( false ), m_log_pkts( false )
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
	bool	get_do_crit( void ) const { return ( do_crit ); }
	bool	get_swap_0_1( void ) const { return ( m_swap_0_1 ); }
	bool	get_log_pkts( void ) const { return ( m_log_pkts ); }

#if SEND_VERSION < 4
	BYTE	get_gen( void ) const { return m_log_pkts ? 0 : inportb( GEN ); }
#endif

	/* Simple set methods */
	void	set_do_crit( bool ido_crit ) { do_crit = ido_crit; }
	void	set_swap_0_1( bool iswap ) { m_swap_0_1 =  iswap; }
	void	set_log_pkts( bool ilog_pkts ) { m_log_pkts =  ilog_pkts; }

	/* Board initialization and reset routines */
	Rslt_t	init_8254( void );
	Rslt_t	init_8255( void );
	Rslt_t	rst_8255( void );
	Rslt_t	init_send( void );
	Rslt_t	set_clk( u_short iclk0t, u_short iclk0h, u_short iclk1t );
	Rslt_t	set_pc_delay_1usec( void );

	void
	set_scope(
		bool	scope_on )
	{
		if ( scope_on )
		{
			scope	=	SCOPE_START;
		}
		else
		{
			if ( !m_log_pkts )
			{
#if SEND_VERSION < 4
				OUT_PC( PC_POS_SCOPENL, 1 );
#endif
			}
			scope	=	SCOPE_IDLE;
		}
	}

	/* Packet sending routines */

	void	clr_under( void )
	{
#if SEND_VERSION < 4
		if ( !m_log_pkts )
		{
			OUT_PC( PC_POS_UNDERCLRL, 0 );
		}
		else
		{
			TO_LOG( "!Skipping hw underclr.\n" );
		}	
#endif
	}
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
//k		print_nibble( !m_log_pkts ? inportb( GEN ) : 0 );
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
	bool				do_crit;				// true to do crit regions.
	u_int				err_cnt;				// Count of contiguous errors.
	u_int				m_pc_delay_1usec;	   	// Loop count for 1 usec. delay.
	bool				m_swap_0_1;				// Swap 0 & 1 if true.
	bool				m_log_pkts;				// true to log pkts.

	/* Method section */
	
	void
	start_crit( void )
	{
		if ( do_crit )
		{
			if ( !m_log_pkts )
			{
#if SEND_VERSION < 4
				disable();
				OUT_PC( PC_POS_UNDERCLRL, 0 );
#endif
			}
		}
	}

	u_int	pc_delay_high( register u_int idelay );
	u_int	pc_delay_low( register u_int idelay );

	void	print_pkt_log( const BYTE *ibytes, u_int isize );
};

#endif /* SEND_REG_H_DECLARED */


/*****************************************************************************
 * $History: SEND_REG.H $
 * 
 * *****************  Version 12  *****************
 * User: Kenw         Date: 6/22/23    Time: 7:27p
 * Updated in $/NMRA/SRC/SEND
 * Added send_reg::kick_start() public method.
 * Verified is worked with a manual 'k' command.
 * 
 * *****************  Version 9  *****************
 * User: Kenw         Date: 12/06/04   Time: 7:15p
 * Updated in $/NMRA/SRC/SEND
 * First set of changes to support Ambiguous bit tests
 * 
 * *****************  Version 8  *****************
 * User: Kenw         Date: 5/15/04    Time: 8:32a
 * Updated in $/NMRA/SRC/SEND
 * Added 1 interior feedback bit test
 * 
 * *****************  Version 7  *****************
 * User: Kenw         Date: 1/11/04    Time: 12:30p
 * Updated in $/NMRA/SRC/SEND
 * Updated for Version 4.1 Beta.
 * Adds feedback test.
 * 
 * *****************  Version 6  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:57p
 * Updated in $/NMRA/SRC/SEND
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 5  *****************
 * User: Kenw         Date: 5/30/98    Time: 9:11p
 * Updated in $/NMRA/SRC/SEND
 * Added support for manual hard reset 'R' command.
 * Rolled to X.2.11
 * 
 * *****************  Version 4  *****************
 * User: Kenw         Date: 10/29/97   Time: 4:26p
 * Updated in $/NMRA/SRC/SEND
 * X.2.8 - Added packet log command.
 * 
 * *****************  Version 3  *****************
 * User: Kenw         Date: 7/26/96    Time: 5:46p
 * Updated in $/NMRA/SRC/SEND
 * Rolled to revision 2.3
 * Added support for 1 pulse duty cycle test.
 * Added Reset test to Address and Bad bit tests.
 * Did some text cleanup.
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 4/30/96    Time: 8:37p
 * Updated in $/NMRA/SRC/SEND
 * Added support for single stretched 0 test.
 *
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 3:49p
 * Created in $/NMRA/SRC/SEND
 * Header file for 'class Send_reg' which handles interraction with the
 * decoder
 * test hardware.
 *
 *****************************************************************************/

