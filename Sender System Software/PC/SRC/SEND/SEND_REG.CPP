/*****************************************************************************
 *
 * DESCRIPTION:
 *
 *	send_reg.cpp	-	Send_reg class methods.
 *
 *****************************************************************************/

#include "send_reg.h"

#include <bits.h>
#include <conio.h>
#include "args.h"
#include "sender_hw_if.h"

#include <string.h>
#include <chrono>
#include <thread>
#include <vector>

/*
 * Baseline packet arrays.
 */
const BYTE	Send_reg::rst_bytes[PKT_SIZE] =
	{ 0xff, 0xf0, 0x00, 0x00, 0x01 };			// Reset packet.
const BYTE	Send_reg::rst_hard_bytes[PKT_SIZE] =
	{ 0xff, 0xf0, 0x00, 0x04, 0x03 };			// Hard reset packet.
const BYTE	Send_reg::idle_bytes[PKT_SIZE] =
	{ 0xff, 0xf7, 0xf8, 0x01, 0xff };			// Idle packet.
const BYTE	Send_reg::base_bytes[PKT_SIZE] =
	{ 0xff, 0xf0, 0x19, 0xd0, 0xef };			// Baseline packet.

const u_long	SAN_CNT			= 0xFFFFFFUL; 	// Sanity timeout value.
const u_int		SHORT_SAN_CNT 	= 0xFFFF; 		// Short sanity timeout value.
const u_int		ERR_MAX			= 4;	  		// Max contig. errors to print.
const int		MAX_PKT_LINE	= 24;			// Max packet bytes per line.

BYTE
Send_reg::get_gen( void ) const
{
	if ( m_hw_if != nullptr )
	{
		BYTE gen1 = 0;
		BYTE gen2 = 0;

		/*
		 * The MCU queues bits well ahead of the track output (seconds for
		 * long 0 clocks), so wait for the queue to drain before sampling
		 * decoder feedback.  The status reply carries both queue_depth and
		 * the input pins, so the final status read supplies the inputs.
		 * After the queue empties, the last half period is still on the
		 * track, so re-read once it has finished.
		 */
		bool				have_gen		= false;
		{
			const int			MAX_DRAIN_MS	= 15000;
			const int			LAST_HP_MS		= 12;	// Longest half period.
			Sender_hw_status	hw_status;
			int					waited_ms		= 0;
			bool				saw_queued		= false;

			std::this_thread::sleep_for(
				std::chrono::milliseconds( LAST_HP_MS ) );

			while ( m_hw_if->get_status( hw_status ) )
			{
				if ( hw_status.queue_depth != 0 )
				{
					saw_queued = true;
					if ( waited_ms++ >= MAX_DRAIN_MS )
					{
						break;
					}
					std::this_thread::sleep_for(
						std::chrono::milliseconds( 1 ) );
				}
				else if ( saw_queued )
				{
					saw_queued = false;
					std::this_thread::sleep_for(
						std::chrono::milliseconds( LAST_HP_MS ) );
				}
				else
				{
					gen1		= hw_status.gen1;
					gen2		= hw_status.gen2;
					have_gen	= true;
					break;
				}
			}
		}

		if ( have_gen || m_hw_if->get_gen_in( gen1, gen2 ) )
		{
			if ( m_log_io )
			{
				DBGLOG( "Send_reg::get_gen() GEN1 %02X GEN2 %02X", gen1, gen2 );
			}
			return ( gen1 & 0x0F );
		}
	}

	// No decoder feedback wiring present; simulate a floating (pulled-up)
	// input so checks fail the same way a disconnected decoder would.
	return ( 0x03 );
}

bool
Send_reg::get_gen_fb(
	Dec_types	idec_type ) const
{
	if ( m_hw_if == nullptr )
	{
		return ( false );
	}

	BYTE	gen		=	get_gen();
	BYTE	mask	=	0;

	if ( idec_type == DEC_LOCO )
	{
		mask	=	GEN_MOTOR_P_BIT;
	}
	else
	{
		mask	=	GEN_LP_MASK ;
	}

	return ( ( gen & mask ) != 0 );
}

void
Send_reg::set_scope(
	bool		scope_on )
{
	if ( scope_on )
	{
		scope	=	SCOPE_START;
	}
	else
	{
		if ( m_hw_if != nullptr )
		{
			m_hw_if->set_scope( false );
		}
		scope	=	SCOPE_IDLE;
	}
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		set_clk()							-	 Set DCC clocks.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem setting clock.
 *
 *	DESCRIPTION
 *
 *		set_clk() sets one or more of the DCC clocks to a new value.
 *		'iclk0t' sets the 0T time in microseconds.  'iclk0h' sets the
 *		0H time in microseconds.  'iclk1t' sets the 1T time in microseconds.
 *		Setting any of these value to DCC_CLK_HOLD will leave that value
 *		unchanged.  If the clock is not running it is started first; if it
 *		is running the new timing is sent on the fly without a stop/start.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::set_clk(
	u_short			iclk0t,					// 0T duration in usec.
	u_short			iclk0h,					// 0H duration in usec.
	u_short			iclk1t )				// 1T duration in usec.
{
	const char		*my_name = "Send_reg::set_clk";

	clr_err_cnt();							// Restart error counter.

	// Timing is changed on the fly; never stop the clock to do it.
	if ( !running )
	{
		if ( start_clk() != OK )
		{
			return ( FAIL );
		}
		clr_err_cnt();
	}

	if ( m_hw_if == nullptr )
	{
		if ( m_no_hw_sim )
		{
			if ( m_log_pkts )
			{
				DBGLOG( "%s() Clk 0T %hu, 0H %hu, 1T %hu.",
					my_name, iclk0t, iclk0h, iclk1t );
			}
			return ( OK );
		}

		ERRPRINT( my_name, LOG_ERR,
			"MCU hardware interface not initialized" );
		clk0t = clk0h = clk1t = DCC_CLK_ERR;
		return ( FAIL );
	}

	if ( iclk0t != DCC_CLK_HOLD )
	{
		if ( iclk0t < DCC_CLK_MIN )
		{
			ERRPRINT( my_name, LOG_ERR,
				"iclk0t value %u invalid", iclk0t );
		}
		else
		{
			clk0t = iclk0t;
		}
 	}

	if ( iclk0h != DCC_CLK_HOLD )
	{
		if ( iclk0h < ( DCC_CLK_MIN / 2 ) )
		{
			ERRPRINT( my_name, LOG_ERR,
				"iclk0h value %u invalid", iclk0h );
		}
		else
		{
			clk0h = iclk0h;
		}
 	}

	if ( iclk1t != DCC_CLK_HOLD )
	{
		if ( iclk1t < DCC_CLK_MIN )
		{
			ERRPRINT( my_name, LOG_ERR,
				"iclk1t value %u invalid", iclk1t );
		}
		else
		{
			clk1t = iclk1t;
		}
	}

	// The 0 bit low phase (0T - 0H) must also be a valid half period.
	if ( clk0h >= clk0t || ( clk0t - clk0h ) < ( DCC_CLK_MIN / 2 ) )
	{
		ERRPRINT( my_name, LOG_ERR,
			"0T %hu, 0H %hu leave an invalid 0 low phase", clk0t, clk0h );
		return ( FAIL );
	}

	if ( m_log_pkts )
	{
		DBGLOG( "%s() Clk 0T %hu, 0H %hu, 1T %hu.",
			my_name, clk0t, clk0h, clk1t );
	}

	if ( !m_hw_if->set_timing( clk0t, clk0h, clk1t ) )
	{
		ERRPRINT( my_name, LOG_ERR,
			"set_timing() FAILED" );
		clk0t = clk0h = clk1t = DCC_CLK_ERR;
		return ( FAIL );
	}

	return ( OK );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		start_clk()							-	 Start hardware clock.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem starting clock.
 *
 *	DESCRIPTION
 *
 *		start_clk() starts the hardware sender clock.  It does this
 *		in a glitch free way which guarantees that the first hardware
 *		clock pulse will be full width.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::start_clk( void )
{
	const char		*my_name = "Send_reg::start_clk";

	clr_err_cnt();							// Restart error counter.

	if ( running )
	{
		ERRPRINT( my_name, LOG_ERR,
			"Clock already running" );
		return ( FAIL );
	}

	if ( m_hw_if == nullptr )
	{
		if ( m_no_hw_sim )
		{
			if ( m_log_pkts )
			{
				DBGLOG( "%s() Clock started.", my_name );
			}
			scope	= 	SCOPE_IDLE;
			running	=	true;
			return ( OK );
		}

		ERRPRINT( my_name, LOG_ERR,
			"MCU hardware interface not initialized" );
		return ( FAIL );
	}

	if ( m_log_pkts )
	{
		DBGLOG( "%s() Clock started.", my_name );
	}

	if ( !m_hw_if->start_clock() )
	{
		ERRPRINT( my_name, LOG_ERR,
			"start_clock() FAILED" );
		return ( FAIL );
	}

	scope	=	SCOPE_IDLE;
	running	=	true;						// We're off and running.

	return ( OK );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		stop_clk()							-	 Stop hardware clock.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem stopping clock.
 *
 *	DESCRIPTION
 *
 *		stop_clk() stops the hardware sender clock.  It does this
 *		in a glitch free way which guarantees that the last hardware
 *		clock pulse will be full width.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::stop_clk( void )
{
	const char		*my_name = "Send_reg::stop_clk";

	clr_err_cnt();							// Restart error counter.

	if ( !running )
	{
		ERRPRINT( my_name, LOG_ERR,
			"Clock already stopped" );
		return ( FAIL );
	}

	if ( m_hw_if == nullptr )
	{
		if ( m_no_hw_sim )
		{
			if ( m_log_pkts )
			{
				DBGLOG( "%s() Clock stopped.", my_name );
			}
			scope	= 	SCOPE_IDLE;
			running	=	false;
			return ( OK );
		}

		ERRPRINT( my_name, LOG_ERR,
			"MCU hardware interface not initialized" );
		return ( FAIL );
	}

	if ( m_log_pkts )
	{
		DBGLOG( "%s() Clock stopped.", my_name );
	}


	if ( !m_hw_if->stop_clock() )
	{
		ERRPRINT( my_name, LOG_ERR,
			"stop_clock() FAILED" );
		return ( FAIL );
	}

	scope	=	SCOPE_IDLE;
	running	=	false;						// We're stopped.

	return ( OK );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		send_bytes()						-	 Send byte repeatedly.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem sending bytes.
 *
 *	DESCRIPTION
 *
 *		send_bytes() sends 'ibyte' 'icnt' times.  It returns FAIL if the
 *		INTRA line does not go inactive in a reasonable length of time.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::send_bytes(
	u_int			icnt,					// Bytes to send.
	BYTE			ibyte,					// Byte to repeat.
	const char		*info )					// Packet log info.
{
	const char		*my_name = "Send_reg::send_bytes";

	if ( icnt < 1 )
	{
		ERRPRINT( my_name, LOG_ERR,
			"0 icnt, b_cnt %lu, p_cnt %lu", b_cnt, p_cnt );
		return ( FAIL );
	}

	if ( m_hw_if == nullptr )
	{
		if ( m_no_hw_sim )
		{
			if ( m_log_pkts )
			{
				DBGLOG( "%s() %s.", my_name, info ? info : "" );
				if ( m_log_data )
				{
					DBGPKT( "%02x %4d %s", ibyte, icnt, icnt > 1 ? "times" : "time" );
				}
			}
			clr_err_cnt();
			b_cnt	+= icnt;
			++p_cnt;
			return ( OK );
		}

		ERRPRINT( my_name, LOG_ERR,
			"MCU hardware interface not initialized" );
		return ( FAIL );
	}

	if ( m_log_pkts )
	{
		DBGLOG( "%s() %s.", my_name, info ? info : "" );
		if ( m_log_data )
		{
			DBGPKT( "%02x %4d %s", ibyte, icnt, icnt > 1 ? "times" : "time" );
		}
	}

	switch ( scope )
	{
	case SCOPE_END:
		m_hw_if->set_scope( false );
		scope = SCOPE_IDLE;
		break;
	case SCOPE_START:
		m_hw_if->set_scope( true );
		scope = SCOPE_END;
		break;
	default:
		break;
	}

	BYTE out_byte = m_swap_0_1 ? static_cast<BYTE>( ~ibyte ) : ibyte;
	if ( !m_hw_if->send_bytes( static_cast<u_short>( icnt ), out_byte, info ) )
	{
		ERRPRINT( my_name, LOG_ERR,
			"send_bytes() FAILED, b_cnt %lu, p_cnt %lu", b_cnt, p_cnt );
		return ( FAIL );
	}

	clr_err_cnt();
	b_cnt	+= icnt;
	++p_cnt;
	return ( OK );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		send_stretched_byte()				-	 Send stretched Byte.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem sending byte.
 *
 *	DESCRIPTION
 *
 *		send_stretched byte() sends 'ibyte' with a stretched first 0.
 *		This routine is designed to stretch the interbyte 0 immediately
 *		following the preamble bits by loading clock values 'iclk0t' and
 *		'iclk0h' for the duration of the first 0 bit in 'ibyte'.
 *		It returns FAIL if a problem occurs.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::send_stretched_byte(
	u_short			iclk0t,					// Stretched 0T duration in usec.
	u_short			iclk0h,					// Stretched 0H duration in usec.
	BYTE			ibyte,					// Byte to send.
	const char		*info )					// Packet log info.
{
	const char		*my_name = "Send_reg::send_stretched_byte";
	u_long			san_cnt; 		   		// Sanity timeout.
	u_short			tclk0t;					// clk0t for stretched 0.

	if ( ibyte & 0x80 )
	{
		ERRPRINT( my_name, LOG_ERR,
			"ibyte must have bit 7 set to 0, its value is 0x%02x", ibyte );
		return ( FAIL );
	}


	if ( iclk0t ==  DCC_CLK_HOLD )
	{
		iclk0t	=	clk0t;
	}

	if ( iclk0h == DCC_CLK_HOLD )
	{
		iclk0h	=	clk0h;
	}

	if ( iclk0t < clk0t )
	{
		ERRPRINT( my_name, LOG_ERR,
			"iclk0t value %u invalid, must be >= %u", iclk0t, clk0t );
		return ( FAIL );
	}

	if ( iclk0h < clk0h )
	{
		ERRPRINT( my_name, LOG_ERR,
			"iclk0h value %u invalid, must be >= %u", iclk0h, clk0h );
		return ( FAIL );
	}

	if ( m_no_hw_sim && m_hw_if == nullptr )
	{
		if ( m_log_pkts )
		{
			DBGLOG( "%s() Clk 0T %hu, 0H %hu %s.",
					 my_name, iclk0t, iclk0h, info ? info : "" );
			if ( m_log_data )
			{
				DBGPKT( "%02x", ibyte );
			}
		}
		clr_err_cnt();
		++b_cnt;
		return ( OK );
	}

	if ( m_hw_if != nullptr )
	{
		if ( m_log_pkts )
		{
			DBGLOG( "%s() Clk 0T %hu, 0H %hu %s.",
				my_name, iclk0t, iclk0h, info ? info : "" );
			if ( m_log_data )
			{
				DBGPKT( "%02x", ibyte );
			}
		}

		switch ( scope )
		{
		case SCOPE_END:
			m_hw_if->set_scope( false );
			scope = SCOPE_IDLE;
			break;
		case SCOPE_START:
			m_hw_if->set_scope( true );
			scope = SCOPE_END;
			break;
		default:
			break;
		}

		BYTE out_byte = m_swap_0_1 ? static_cast<BYTE>( ~ibyte ) : ibyte;
		if ( !m_hw_if->send_stretched_byte( iclk0t, iclk0h, out_byte, info ) )
		{
			ERRPRINT( my_name, LOG_ERR,
				"send_stretched_byte() FAILED, b_cnt %lu, p_cnt %lu",
				b_cnt, p_cnt );
			return ( FAIL );
		}

		clr_err_cnt();
		++b_cnt;
		return ( OK );
	}

	ERRPRINT( my_name, LOG_ERR,
		"MCU hardware interface not initialized" );
	return ( FAIL );
}

Rslt_t
Send_reg::send_raw_bytes_stretched(
	const BYTE		*ibytes,
	u_int			isize,
	u_int			stretch_byte_index,
	u_short			iclk0t,
	u_short			iclk0h,
	const char		*info )
{
	const char	*my_name = "Send_reg::send_raw_bytes_stretched";

	if ( ibytes == nullptr || isize == 0 || stretch_byte_index >= isize )
	{
		ERRPRINT( my_name, LOG_ERR, "Invalid raw stretched-byte arguments" );
		return ( FAIL );
	}

	if ( m_no_hw_sim && m_hw_if == nullptr )
	{
		if ( m_log_pkts )
		{
			DBGLOG( "%s() Clk 0T %hu, 0H %hu %s.",
				my_name, iclk0t, iclk0h, info ? info : "" );
			print_pkt_log( ibytes, isize );
		}
		b_cnt += isize;
		return ( OK );
	}

	if ( m_hw_if == nullptr )
	{
		ERRPRINT( my_name, LOG_ERR, "MCU hardware interface not initialized" );
		return ( FAIL );
	}

	if ( m_log_pkts )
	{
		DBGLOG( "%s() Clk 0T %hu, 0H %hu %s.",
			my_name, iclk0t, iclk0h, info ? info : "" );
		print_pkt_log( ibytes, isize );
	}

	switch ( scope )
	{
	case SCOPE_END:
		m_hw_if->set_scope( false );
		scope = SCOPE_IDLE;
		break;
	case SCOPE_START:
		m_hw_if->set_scope( true );
		scope = SCOPE_END;
		break;
	default:
		break;
	}

	if ( !m_hw_if->send_raw_bytes_stretched( ibytes, isize,
		stretch_byte_index, iclk0t, iclk0h, info ) )
	{
		ERRPRINT( my_name, LOG_ERR,
			"send_raw_bytes_stretched() FAILED, b_cnt %lu, p_cnt %lu",
			b_cnt, p_cnt );
		return ( FAIL );
	}

	clr_err_cnt();
	b_cnt += isize;
	return ( OK );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		send_1_ambig_bit()				-	 Send ambiguous bit.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem sending byte.
 *
 *	DESCRIPTION
 *
 *		send_1_ambig_bit() sends 'ibyte' with a stretched 0.
 *		This routine is designed to provide an ambiguous sized 0 bit.  It does
 *		this by loading clock values 'iclk0t' and 'iclk0h' for the duration
 *		of the 0 bit in 'ibyte'.
 *		It returns FAIL if a problem occurs.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::send_1_ambig_bit(
	u_short			iclk0t,					// Stretched 0T in usec.
	u_short			iclk0h,					// Stretched 0H in usec.
	BYTE			ibyte,					// Byte to send.
	const char		*info )					// Packet log info.
{
	return ( send_stretched_byte( iclk0t, iclk0h, ibyte, info ) );
}

Rslt_t
Send_reg::send_raw_bytes_timed(
	const BYTE	*ibytes, u_int isize, u_short bit_index1, u_short iclk0t1,
	u_short		iclk0h1, u_short bit_index2, u_short iclk0t2,
	u_short		iclk0h2, const char *info )
{
	const char	*my_name = "Send_reg::send_raw_bytes_timed";

	if ( ibytes == nullptr || isize == 0 || bit_index1 >= isize * BITS_IN_BYTE ||
		 (bit_index2 != 0xFFFF && bit_index2 >= isize * BITS_IN_BYTE) )
	{
		ERRPRINT( my_name, LOG_ERR, "Invalid raw timed-bit arguments" );
		return ( FAIL );
	}

	if ( m_no_hw_sim && m_hw_if == nullptr )
	{
		if ( m_log_pkts )
		{
			DBGLOG( "%s() %s.", my_name, info ? info : "" );
			print_pkt_log( ibytes, isize );
		}
		b_cnt += isize;
		return ( OK );
	}

	if ( m_hw_if == nullptr )
	{
		ERRPRINT( my_name, LOG_ERR, "MCU hardware interface not initialized" );
		return ( FAIL );
	}

	if ( m_log_pkts )
	{
		DBGLOG( "%s() %s.", my_name, info ? info : "" );
		print_pkt_log( ibytes, isize );
	}

	if ( scope == SCOPE_START )
	{
		m_hw_if->set_scope( true );
		scope = SCOPE_END;
	}
	else if ( scope == SCOPE_END )
	{
		m_hw_if->set_scope( false );
		scope = SCOPE_IDLE;
	}

	if ( !m_hw_if->send_raw_bytes_timed( ibytes, isize, bit_index1,
		iclk0t1, iclk0h1, bit_index2, iclk0t2, iclk0h2, info ) )
	{
		ERRPRINT( my_name, LOG_ERR, "send_raw_bytes_timed() FAILED" );
		return ( FAIL );
	}

	clr_err_cnt();
	b_cnt += isize;
	return ( OK );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		send_2_ambig_bits()				-	 Send 2 ambiguous bits.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem sending byte.
 *
 *	DESCRIPTION
 *
 *		send_2_ambig_bits() sends 'ibyte' with a stretched first 0 and a
 *		different stretched second 0.  This routine is designed to provide
 *		sized 0 bits immediately after the stop bit to test proper operation
 *		with advanced feedback that occurs immedicately after the packet end
 *		bit.  It does this by loading clock values 'iclk0t1' and
 *		'iclk0h1' for the duration of the first 0 bit in 'ibyte' and loads
 *		'iclkoh2' and 'iclk0t2' for the second 0 bit in 'ibyte'.
 *		It returns FAIL if a problem occurs.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::send_2_ambig_bits(
	u_short			iclk0t1,				// 1st stretched 0T in usec.
	u_short			iclk0h1,				// 1st stretched 0H in usec.
	u_short			iclk0t2,				// 2nd stretched 0T in usec.
	u_short			iclk0h2,				// 2nd stretched 0H in usec.
	BYTE			ibyte,					// Byte to send.
	const char		*info )					// Packet log info.
{
	return ( send_raw_bytes_timed( &ibyte, 1, 0, iclk0t1, iclk0h1,
		1, iclk0t2, iclk0h2, info ) );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		send_pkt()							-	 Send array of BYTES.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem sending bytes.
 *
 *	DESCRIPTION
 *
 *		send_pkt() sends each BYTE in the 'ibytes' array.
 *		The host side inter-packet delay is appended only if 'add_gap'
 *		is true.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::send_pkt(
	const BYTE		*ibytes,				// Pointer to BYTE array.
	u_int			isize,					// Size of ibytes array.
	const char		*info,					// Packet log info.
	bool			add_gap )				// true to append the pkt gap.
{
	const char		*my_name = "Send_reg::send_pkt(BYTE)";
	u_int			count;					// Count of BYTES sent.

	if ( isize < 1 )
	{
		ERRPRINT( my_name, LOG_ERR,
			"0 isize, b_cnt %lu, p_cnt %lu", b_cnt, p_cnt );
		return ( FAIL );
	}

	if ( m_hw_if == nullptr )
	{
		if ( m_no_hw_sim )
		{
			if ( m_log_pkts )
			{
				DBGLOG( "%s %s.", my_name, info ? info : "" );
				print_pkt_log( ibytes, isize );
			}
			clr_err_cnt();
			b_cnt	+= isize;
			++p_cnt;
			if ( add_gap )
			{
				pkt_gap();
			}
			return ( OK );
		}

		ERRPRINT( my_name, LOG_ERR,
			"MCU hardware interface not initialized" );
		return ( FAIL );
	}

	if ( m_log_pkts )
	{
		DBGLOG( "%s %s.", my_name, info ? info : "" );
		print_pkt_log( ibytes, isize );
	}

	switch ( scope )
	{
	case SCOPE_END:
		m_hw_if->set_scope( false );
		scope = SCOPE_IDLE;
		break;
	case SCOPE_START:
		m_hw_if->set_scope( true );
		scope = SCOPE_END;
		break;
	default:
		break;
	}

	if ( !m_swap_0_1 )
	{
		if ( !m_hw_if->send_raw_bytes( ibytes, isize, info ) )
		{
			ERRPRINT( my_name, LOG_ERR,
				"send_raw_bytes() FAILED, b_cnt %lu, p_cnt %lu",
				b_cnt, p_cnt );
			return ( FAIL );
		}
	}
	else
	{
		std::vector<BYTE> swap_bytes( isize );

		for ( count = 0; count < isize; ++count )
		{
			swap_bytes[count] = static_cast<BYTE>( ~ibytes[count] );
		}

		if ( !m_hw_if->send_raw_bytes( swap_bytes.data(), isize, info ) )
		{
			ERRPRINT( my_name, LOG_ERR,
				"send_raw_bytes() FAILED in swap mode, b_cnt %lu, p_cnt %lu",
				b_cnt, p_cnt );
			return ( FAIL );
		}
	}

	clr_err_cnt();
	b_cnt	+= isize;
	++p_cnt;
	if ( add_gap )
	{
		pkt_gap();
	}
	return ( OK );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		send_pkt()							-	 Send Bits object contents.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem sending bytes.
 *
 *	DESCRIPTION
 *
 *		send_pkt() sends out all BYTES in a Bits object.
  *		The host side inter-packet delay is appended only if 'add_gap'
 *		is true.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::send_pkt(
	Bits			&ibits,					// Bits to send.
	const char		*info,					// Packet log info.
	bool			add_gap )				// true to append the pkt gap.
{
	const char		*my_name = "Send_reg::send_pkt(Bits)";
	u_int			count = 0;				// Count of BYTES sent.
	BYTE			pbyte;					// Present BYTE.

	if ( ibits.get_obj_errs() )
	{
		if ( errprint_ok() )
		{
			ERRPRINT( my_name, LOG_ERR,
			"Incoming Bits object has errors 0x%08lx",
			ibits.get_obj_errs() );
		}

		return ( FAIL );
	}

	const BYTE *pkt_bytes = ibits.get_byte_array();
	u_int pkt_size = ibits.get_byte_size();

	if ( m_hw_if == nullptr )
	{
		if ( m_no_hw_sim )
		{
			if ( m_log_pkts )
			{
				DBGLOG( "%s %s.", my_name, info ? info : "" );
				print_pkt_log( pkt_bytes, pkt_size );
			}
			clr_err_cnt();
			b_cnt	+= pkt_size;
			++p_cnt;
			if ( add_gap )
			{
				pkt_gap();
			}
			return ( OK );
		}

		ERRPRINT( my_name, LOG_ERR,
			"MCU hardware interface not initialized" );
		return ( FAIL );
	}

	if ( m_log_pkts )
	{
		DBGLOG( "%s %s.", my_name, info ? info : "" );
		print_pkt_log( pkt_bytes, pkt_size );
	}

	switch ( scope )
	{
	case SCOPE_END:
		m_hw_if->set_scope( false );
		scope = SCOPE_IDLE;
		break;
	case SCOPE_START:
		m_hw_if->set_scope( true );
		scope = SCOPE_END;
		break;
	default:
		break;
	}

	if ( pkt_size == 0 )
	{
		ERRPRINT( my_name, LOG_ERR,
			"0 Bytes sent, b_cnt %lu, p_cnt %lu", b_cnt, p_cnt );
		return ( FAIL );
	}

	if ( !m_swap_0_1 )
	{
		if ( !m_hw_if->send_raw_bytes( pkt_bytes, pkt_size, info ) )
		{
			ERRPRINT( my_name, LOG_ERR,
				"send_raw_bytes() FAILED, b_cnt %lu, p_cnt %lu",
				b_cnt, p_cnt );
			return ( FAIL );
		}
	}
	else
	{
		std::vector<BYTE> swap_bytes( pkt_size );

		for ( count = 0, ibits.rst_out();
				ibits.get_byte( pbyte ) == OK; ++count )
		{
			swap_bytes[count] = static_cast<BYTE>( ~pbyte );
		}

		if ( !m_hw_if->send_raw_bytes( swap_bytes.data(), pkt_size, info ) )
		{
			ERRPRINT( my_name, LOG_ERR,
				"send_raw_bytes() FAILED in swap mode, b_cnt %lu, p_cnt %lu",
				b_cnt, p_cnt );
			return ( FAIL );
		}
	}

	clr_err_cnt();
	b_cnt	+= pkt_size;
	++p_cnt;
	if ( add_gap )
	{
		pkt_gap();
	}
	return ( OK );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		errprint_ok()					   	-	 Check if OK to print error.
 *
 *	RETURN VALUE
 *
 *		true	-	OK to print error.
 *		false	-	Don't print the error.
 *
 *	DESCRIPTION
 *
 *		errprint_ok() return true if the number of contiguous errors
 *		given by 'err_cnt' has not been exceeded.  It also prints an
 *		error of its own when 'err_cnt' is just reached.
 */
/*--------------------------------------------------------------------------*/

bool
Send_reg::errprint_ok( void )
{
	const char		*my_name = "Send_reg::errprint_ok";

	if ( err_cnt == ERR_MAX )
	{
		ERRPRINT( my_name, LOG_ERR,
			"Contiguous error count of %u reached", err_cnt );
		++err_cnt;
	}
	else if ( err_cnt < ERR_MAX  )
	{
		++err_cnt;
		return ( true );
	}

	return ( false );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		print_stats()						-	 Print packet statistics.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		print_stats() prints packet sending statistics to stdout.
 */
/*--------------------------------------------------------------------------*/

void
Send_reg::print_stats( void )
{
	printf(
		"Packets sent %lu, Bytes sent %lu\n", p_cnt, b_cnt );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		kick_start()	-	Send kick start sequence to decoder.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem with kick start sequence.
 *
 *	DESCRIPTION
 *
 *		This method sends the following command sequence to power up and
 *      turn on the forward or reverse functions of the loco functions
 *		being tested.  Each step sends one loco speed packet and one function
 *		packet, then waits:
 *           Power up:        5 seconds of speed 0 with functions on.
 *           Loco start:      1 second of speed 1 to activate locomotive.
 *           function on:    10 seconds to get the functions turned on.
 *
 *      This command should only be run for a function decoder.
 *      This method assumes the clock values are set to the
 *      default values and the clock is started.
 *		'ibits' refers to a Bits buffer used to build DCC bit streams.
 *		'iargs' refers to the Args_obj that holds command line arguments.
 *		'ilog'  indicates when information should be logged.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::kick_start(
	Bits			&ibits,					// Bits object to use.
	Args_obj		&iargs )				// Arguments object to use.
{
	const char			*my_name 	= "Send_reg::kick_start()";
	u_short				addr	 	= 		// Loco address.
								iargs.get_decoder_address();
	bool				fwd		 	=	   	// true for forward direction.
								!iargs.get_lamp_rear();
	u_int				pre			= 		// Extra preambles.
								PRE_BITS + iargs.get_extra_preamble();
	BYTE				fmask		=		// Function mask.
								iargs.get_func_mask();

    if ( running == false )
	{
		fprintf( stderr,            "Clock must be running\n" );
		ERRPRINT( my_name, LOG_ERR, "Clock must be running" );

		return ( FAIL );
	}

	if ( !m_log_pkts )
	{
		if (   clk0t != DECODER_0T_NOM
			|| clk0h != DECODER_0H_NOM
			|| clk1t != DECODER_1T_NOM )
		{
			fprintf( stderr,
				"Clock error: clk0t %u, clk0h %u, clkit %u\n",
				clk0t, clk0h, clk1t );
			ERRPRINT( my_name, LOG_ERR,
				"Clock error: clk0t %u, clk0h %u, clkit %u",
            	clk0t, clk0h, clk1t );
			return ( FAIL );
		}
	}
	else
	{
		DBGLOG( "%s: Skipping DCC clk tests.", my_name );
	}

	/*
	 *	Send the loco speed packet and the function packet as separate
	 *	send_pkt() calls so each gets the post packet fill delay.  Repeat
	 *	the pair until the step time has elapsed or the user types the
	 *	break sequence '<ESC> q'.  Returns false if stopped by BREAK.
	 */
	char				lchar = '\0';		// Last key received.
	auto break_typed = [&]()
	{
		bool		got_break = false;

		while ( kbhit() )
		{
			char	tchar = lchar;

			lchar = static_cast<char>( getch() );
			if ( tchar == '\033' && lchar == 'q' )
			{
				got_break = true;
			}
		}

		return ( got_break );
	};

	auto run_phase = [&]( int speed, int seconds, const char *name )
	{
		const auto	end_time = std::chrono::steady_clock::now()
								+ std::chrono::seconds( seconds );

		STATPRINT( "%s %s for %d seconds, '<ESC> q' to stop.",   my_name, name, seconds );
		printf(    "%s %s for %d seconds, '<ESC> q' to stop.\n", my_name, name, seconds );

		do
		{
			ibits.clr_in().put_0s( 1 );
			ibits.put_cmd_pkt_28( addr, fwd, speed, pre ).done();
			Dcc_reg.send_pkt( ibits, name );

			ibits.clr_in().put_0s( 1 );
			ibits.put_func_grp_pkt( addr, GRP_1, fmask, pre ).done();
			Dcc_reg.send_pkt( ibits, name );

			if ( break_typed() )
			{
				printf( "\n<BREAK>\n" );
				ERRPRINT( my_name, LOG_WARNING, "Kick start stopped by BREAK" );
				return ( false );
			}
		}
		while ( std::chrono::steady_clock::now() < end_time );

		return ( true );
	};

	if (	!run_phase( 0, 5,  "Power up" )			// Speed 0, functions on.
		||	!run_phase( 1, 1,  "Loco start" )		// Speed 1 to activate the loco.
		||	!run_phase( 0, 10, "Function on" ) )	// Speed 0, functions on.
	{
		return ( FAIL );
	}

	return ( OK );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		print_pkt_log()	   		 		-	 Print BYTE array to log.
 *
 *	RETURN VALUE
 *
 *		NONE.
 *
 *	DESCRIPTION
 *
 *		print_pkt_log() formats and print the BYTE array to the log.
 */
/*--------------------------------------------------------------------------*/

void
Send_reg::print_pkt_log(
	const BYTE		*ibytes,			// BYTE array to log.
	u_int			isize )				// Size of BYTE array.
{
	const char		*my_name 	= "Send_reg::print_pkt_log()";

	if ( !m_log_data )
	{
		return;
	}
	int				lcnt;				// Count of BYTES on present line.

	if ( !ibytes )
	{
		ERRPRINT( my_name, LOG_ERR, "ERROR - ibytes is NULL." );
		return;
	}

	if ( isize == 0 )
	{
		ERRPRINT( my_name, LOG_ERR, "Bytes array empty." );
		return;
	}

	lcnt	=	0;
	while ( isize-- > 0 )
	{
		if ( lcnt++ == 0 )
		{
			TO_LOG( "!%c ", DBG_PKT );
		}

		TO_LOG( "%02x", *ibytes++ );

		if ( lcnt >= MAX_PKT_LINE || isize == 0 )
		{
			lcnt	=	0;
			TO_LOG( "\n" );
		}
		else
		{
			TO_LOG( " " );
		}
	}
}
