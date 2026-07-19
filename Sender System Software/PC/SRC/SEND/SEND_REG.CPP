/*****************************************************************************
 *
 * File:                 $Workfile: SEND_REG.CPP $
 * Path:                 $Logfile: /NMRA/SRC/SEND/SEND_REG.CPP $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 28 $
 * Last Modified on:     $Modtime: 8/30/23 6:55p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 9/12/23 6:44p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	send_reg.cpp	-	Send_reg class methods.
 *
 *****************************************************************************/

#include "send_reg.h"

#include <bits.h>
#include "args.h"
#include "sender_hw_if.h"

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
	if ( m_log_pkts )
	{
		return ( 0 );
	}

	if ( m_hw_if != NULL )
	{
		BYTE gen1 = 0;
		BYTE gen2 = 0;
		if ( m_hw_if->get_gen_in( gen1, gen2 ) )
		{
			return ( gen1 );
		}
	}

	return ( 0 );
}

void
Send_reg::set_scope(
	bool		scope_on )
{
	if ( m_log_pkts )
	{
		scope	=	scope_on ? SCOPE_START : SCOPE_IDLE;
		return;
	}

	if ( scope_on )
	{
		scope	=	SCOPE_START;
	}
	else
	{
		if ( m_hw_if != NULL )
		{
			m_hw_if->set_scope( false );
		}
		scope	=	SCOPE_IDLE;
	}
}

void
Send_reg::clr_under( void )
{
	if ( m_log_pkts )
	{
		DBGLOG( "Skipping hw underclr." );
		return;
	}

	if ( m_hw_if != NULL )
	{
		m_hw_if->clear_underflow();
	}
	else
	{
		DBGLOG( "Skipping hw underclr." );
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
 *		unchanged.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::set_clk(
	u_short			iclk0t,					// 0T duration in usec.
	u_short			iclk0h,					// 0H duration in usec.
	u_short			iclk1t )				// 1T duration in usec.
{
	const char		*my_name = "Send_reg::set_clk";
	bool			trunning = running;		// Temp running state.

	clr_err_cnt();							// Restart error counter.

	if ( running )							// Stop the clock if it is running.
	{
		stop_clk();
	}

	if ( m_hw_if == NULL )
	{
		if ( m_log_pkts )
		{
			DBGLOG( "%s() Clk 0T %hu, 0H %hu, 1T %hu.",
				my_name, clk0t, clk0h, clk1t );
			return ( OK );
		}

		ERRPRINT( my_name, LOG_ERR,
			"MCU hardware interface not initialized" );
		clk0t = clk0h = clk1t = DCC_CLK_ERR;
		return ( FAIL );
	}

	if ( m_log_pkts )
	{
		DBGLOG( "%s() Clk 0T %hu, 0H %hu, 1T %hu.",
			my_name, clk0t, clk0h, clk1t );
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

	if ( !m_hw_if->set_timing( clk0t, clk0h, clk1t ) )
	{
		ERRPRINT( my_name, LOG_ERR,
			"set_timing() FAILED" );
		clk0t = clk0h = clk1t = DCC_CLK_ERR;
		return ( FAIL );
	}

	if ( trunning )
	{
		if ( start_clk() != OK )
		{
			return ( FAIL );
		}
	}
	else
	{
		running = false;
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

	if ( m_hw_if == NULL )
	{
		if ( m_log_pkts )
		{
			DBGLOG( "%s() Clock started.", my_name );
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

	if ( m_hw_if == NULL )
	{
		if ( m_log_pkts )
		{
			DBGLOG( "%s() Clock stopped.", my_name );
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
 *		INTRA line does not go inactive in a reasonable length of time or
 *		if the underflow flag gets set.
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

	if ( m_hw_if == NULL )
	{
		if ( m_log_pkts )
		{
			DBGLOG( "%s() %s.", my_name, info ? info : "" );
			DBGPKT( "%02x %4d %s", ibyte, icnt, icnt > 1 ? "times" : "time" );
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
		DBGPKT( "%02x %4d %s", ibyte, icnt, icnt > 1 ? "times" : "time" );
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

	BYTE out_byte = m_swap_0_1 ? (BYTE)~ibyte : ibyte;
	if ( !m_hw_if->send_bytes( (u_short)icnt, out_byte, info ) )
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
	register u_long	san_cnt;		   		// Sanity timeout.
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

	if ( m_log_pkts )
	{
		DBGLOG( "%s() Clk 0T %hu, 0H %hu %s.",
				 my_name, iclk0t, iclk0h, info ? info : "" );
		DBGPKT( "%02x", ibyte );
		clr_err_cnt();
		++b_cnt;
		return ( OK );
	}

	if ( m_hw_if != NULL )
	{
//		DBGLOG( "%s() Clk 0T %hu, 0H %hu %s.",
//				 my_name, iclk0t, iclk0h, info ? info : "" );
//		DBGPKT( "%02x", ibyte );

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

		BYTE out_byte = m_swap_0_1 ? (BYTE)~ibyte : ibyte;
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
	const char		*my_name = "Send_reg::send_2_ambig_bits";
	if ( m_log_pkts )
	{
		DBGLOG( "%s() Clk 0T1 %hu, 0H1 %hu, 0T2 %hu, 0H2 %hu: %s.",
			my_name, iclk0t1, iclk0h1, iclk0t2, iclk0h2, info ? info : "" );
		DBGPKT( "%02x", ibyte );
		clr_err_cnt();
		++b_cnt;
		return ( OK );
	}

	ERRPRINT( my_name, LOG_ERR,
		"send_2_ambig_bits() is not supported in MCU mode" );
	return ( FAIL );
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
 *		It returns FAIL if the INTRA line does not go inactive in a
 *		reasonable length of time or if the underflow flag gets set.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::send_pkt(
	const BYTE		*ibytes,				// Pointer to BYTE array.
	u_int			isize,					// Size of ibytes array.
	const char		*info )					// Packet log info.
{
	const char		*my_name = "Send_reg::send_pkt(BYTE)";
	register u_int	count;					// Count of BYTES sent.

	if ( isize < 1 )
	{
		ERRPRINT( my_name, LOG_ERR,
			"0 isize, b_cnt %lu, p_cnt %lu", b_cnt, p_cnt );
		return ( FAIL );
	}

	if ( m_hw_if == NULL )
	{
		if ( m_log_pkts )
		{
			DBGLOG( "%s %s.", my_name, info ? info : "" );
			print_pkt_log( ibytes, isize );
			clr_err_cnt();
			b_cnt	+= isize;
			++p_cnt;
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
		for ( count = 0; count < isize; ++count )
		{
			if ( !m_hw_if->send_bytes( 1, (BYTE)~ibytes[count], info ) )
			{
				ERRPRINT( my_name, LOG_ERR,
					"send_bytes() FAILED, sent %u, b_cnt %lu, p_cnt %lu",
					count, b_cnt, p_cnt );
				return ( FAIL );
			}
		}
	}

	clr_err_cnt();
	b_cnt	+= isize;
	++p_cnt;
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
 *		It returns FAIL if the INTRA line does not go inactive in a
 *		reasonable length of time or if the underflow flag gets set.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::send_pkt(
	Bits			&ibits,					// Bits to send.
	const char		*info )					// Packet log info.
{
	const char		*my_name = "Send_reg::send_pkt(Bits)";
	register u_int	count = 0;				// Count of BYTES sent.
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

	if ( m_hw_if == NULL )
	{
		if ( m_log_pkts )
		{
			DBGLOG( "%s %s.", my_name, info ? info : "" );
			print_pkt_log( pkt_bytes, pkt_size );
			clr_err_cnt();
			b_cnt	+= pkt_size;
			++p_cnt;
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
		for ( count = 0, ibits.rst_out();
				ibits.get_byte( pbyte ) == OK; ++count )
		{
			if ( !m_hw_if->send_bytes( 1, (BYTE)~pbyte, info ) )
			{
				ERRPRINT( my_name, LOG_ERR,
					"send_bytes() FAILED, sent %u, b_cnt %lu, p_cnt %lu",
					count, b_cnt, p_cnt );
				return ( FAIL );
			}
		}
	}

	clr_err_cnt();
	b_cnt	+= pkt_size;
	++p_cnt;
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
 *		update()						-	 Read hardware state.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		update() reads the hardware data values into the Send_reg
 *		class variables.
 */
/*--------------------------------------------------------------------------*/

void
Send_reg::update( void )
{
	if ( m_log_pkts || m_hw_if == NULL )
	{
		DBGLOG( "Send_reg::update() Skipping update." );
		m_pc_delay_1usec	=	1;
		return;
	}

	BYTE gen1 = 0;
	BYTE gen2 = 0;
	Sender_hw_status status;

	if ( m_hw_if->get_gen_in( gen1, gen2 ) )
	{
		v.gen = gen1;
	}

	if ( m_hw_if->get_status( status ) )
	{
		running = status.running;
		v.pb = status.underflow ? 0x01 : 0x00;
	}

	v.pa = 0;
	v.pc = 0;
	v.s0t = 0;
	v.s0h = 0;
	v.s1t = 0;
	v.cnt0t = clk0t;
	v.cnt0h = clk0h;
	v.cnt1t = clk1t;
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		up_print()						-	 Update & print statistics.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		up_print() first updates the Send_reg class variables using update()
 *		and then prints it out using print().
 */
/*--------------------------------------------------------------------------*/

void
Send_reg::up_print(
	bool		pr_hdr_flag )				// If true, print header.
{
	update();
	print( pr_hdr_flag );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		fill_idles()	-	Calc number of reps. for idles.
 *		fill_cmds_1{}	-	Calc number of reps. for 1 cmd and 1 idle.
 *		fill_cmds_2{}	-	Calc number of reps. for 2 cmds and 2 idles.
 *
 *	RETURN VALUE
 *
 *		u_reps	-	Reps to fill time required for packet sequence.
 *
 *	DESCRIPTION
 *
 *		These methods calculate the 'u_reps' of the given packet sequence
 *		requited to filll the delay time of 'ifill_usec' using the given
 *		packet sequence wih the 'iclk0t' and 'iclk1t' packet times.
 *		Note: The command delay times are approximate since generic
 *		cmd packets are used rather than the actual cmd packets.
 */
/*--------------------------------------------------------------------------*/

/*
 * Calculate the number of repeats usings 1 idle packet.
 * 1 39 bit packet plus filler requires 5 Bytes.
 */
u_short
Send_reg::fill_idles (
	u_long			ifill_usec,				// Fill time in usec.
	u_short			iclk0t,					// Total clock 0 time.
	u_short 		iclk1t )			   	// Total clock 1 time.
{
	u_short			reps;					// Reps to delay calculated time.

	reps	=	(int)( ifill_usec /
						(	(u_long)iclk0t * (u_long)(		IDLE_0S
														+	FILL_1_0S )
						+ 	(u_long)iclk1t * (u_long)(		IDLE_1S
														+	FILL_1_1S ) ) );
	if ( reps < PKT_REP_MIN )
	{
		reps	=	PKT_REP_MIN;
	}

	return ( reps );
}

/*
 * Calculate the number of repeats usings 1 command + idle packet pair.
 * 2 39 bit packets plus filler require 10 Bytes.
 */
u_short
Send_reg::fill_cmds_1 (
	u_long			ifill_usec,				// Fill time in usec.
	u_short			iclk0t,					// Total clock 0 time.
	u_short 		iclk1t )			   	// Total clock 1 time.
{
	u_short			reps;					// Reps to delay calculated time.

	reps		=	(int)( ifill_usec /
						(	(u_long)iclk0t * (u_long)(		IDLE_0S
														+	CMD_0S
														+	FILL_2_0S )
						+ 	(u_long)iclk1t * (u_long)(		IDLE_1S
														+	CMD_1S
														+	FILL_2_1S ) ) );
	if ( reps < PKT_REP_MIN )
	{
		reps	=	PKT_REP_MIN;
	}

	return ( reps );
}

/*
 * Calculate the number of repeats usings 2 command + idle packet pairs.
 * 4 39 bit packets plus filler require 20 Bytes.
 */
u_short
Send_reg::fill_cmds_2 (
	u_long			ifill_usec,				// Fill time in usec.
	u_short			iclk0t,					// Total clock 0 time.
	u_short 		iclk1t )			   	// Total clock 1 time.
{
	u_short			reps;					// Reps to delay calculated time.

	reps		=	(int)( ifill_usec /
						(	(u_long)iclk0t * (u_long)(		( 2 * IDLE_0S )
														+	( 2 * CMD_0S )
														+	FILL_4_0S )
						+ 	(u_long)iclk1t * (u_long)(		( 2 * IDLE_1S )
														+	( 2 * CMD_1S )
														+	FILL_4_1S ) ) );
	if ( reps < PKT_REP_MIN )
	{
		reps	=	PKT_REP_MIN;
	}

	return ( reps );
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
 *		being tested:
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
	u_short				dots		=		// Dot time 4 pkt sequences.
								fill_cmds_2() / 2;
	u_short				reps;				// Number of command repeats.
    register u_short	i;					// Index variabble.

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
	 * Run 5 second power up sequence.
	 */
	 
	 // Set up loco speed, idle, function, and idle packets.
	ibits.clr_in().put_0s( 1 );
	ibits.put_cmd_pkt_28( addr, fwd, 0, pre ).put_idle_pkt( 1 );
	ibits.put_func_grp_pkt( addr, GRP_1, fmask, pre ).put_idle_pkt( 1 );
	ibits.put_1s( 1 ).done();
	
	// Get the number of reps.
	reps = fill_cmds_2( 5 * USEC_PER_SEC, DECODER_0T_NOM, DECODER_1T_NOM );
	STATPRINT( "%s Sending %3u power up pkts.",   my_name, reps );
	printf(    "%s Sending %3u power up pkts.\n", my_name, reps );
	clr_under();
	for ( i = 0; i < reps; i++ )
	{
		Dcc_reg.set_scope( true );
		Dcc_reg.send_pkt( ibits, "SEND power up pkts." );
	   	if ( (i % dots) == 0 )
	   	{
	   		putchar( '.' );
			clr_under();
	   	}
	}
	putchar( '\n' );
	
	/*
	 * Run 1 second loco start sequence.
	 */
	 
	 // Set up loco speed, idle, function, and idle packets.
	ibits.clr_in().put_0s( 1 );
	ibits.put_cmd_pkt_28( addr, fwd, 1, pre ).put_idle_pkt( 1 );
	ibits.put_func_grp_pkt( addr, GRP_1, fmask, pre ).put_idle_pkt( 1 );
	ibits.put_1s( 1 ).done();
	
	// Get the number of reps.
	reps = fill_cmds_2( 1 * USEC_PER_SEC, DECODER_0T_NOM, DECODER_1T_NOM );
	STATPRINT( "%s Sending %3u loco start pkts.",   my_name, reps );
	printf(    "%s Sending %3u loco start pkts.\n", my_name, reps );
	clr_under();
	for ( i = 0; i < reps; i++ )
	{
		Dcc_reg.set_scope( true );
		Dcc_reg.send_pkt( ibits, "SEND loco start pkts." );
	   	if ( (i % dots) == 0 )
	   	{
	   		putchar( '.' );
			clr_under();
	   	}
	}
	putchar( '\n' );
	
	/*
	 * Run 10 second power up sequence.
	 */
	 
	 // Set up loco speed, idle, function, and idle packets.
	ibits.clr_in().put_0s( 1 );
	ibits.put_cmd_pkt_28( addr, fwd, 0, pre ).put_idle_pkt( 1 );
	ibits.put_func_grp_pkt( addr, GRP_1, fmask, pre ).put_idle_pkt( 1 );
	ibits.put_1s( 1 ).done();
	
	// Get the number of reps.
	reps = fill_cmds_2( 10 * USEC_PER_SEC, DECODER_0T_NOM, DECODER_1T_NOM );
	STATPRINT( "%s Sending %3u function on pkts.",   my_name, reps );
	printf(    "%s Sending %3u function on pkts.\n", my_name, reps );
	clr_under();
	for ( i = 0; i < reps; i++ )
	{
		Dcc_reg.set_scope( true );
		Dcc_reg.send_pkt( ibits, "SEND function on pkts." );
	   	if ( (i % dots) == 0 )
	   	{
	   		putchar( '.' );
			clr_under();
	   	}
	}
	putchar( '\n' );

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
