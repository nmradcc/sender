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

static const char sccsid[]      = "@(#) $Workfile: SEND_REG.CPP $$ $Revision: 28 $$";
static const char sccsid_h[]    = SEND_REG_H_DECLARED;

// Func to eliminate bogus sccsid and sccsid_h declared but never used warning.
inline void dummy(const char *, const char *) {}

#if SEND_VERSION >= 4
extern "C"
{
	#include "Main.h"
	//#include "app_threadx.h"
	#include "cmsis_os.h"

	extern void OUT_PC(uint8_t mask, uint8_t value);
	extern uint8_t inportb(uint8_t port);
	extern void outportb(uint8_t port, uint8_t value);

	int PacketToTrack(const uint8_t* buf, uint32_t len, uint32_t clk1t, uint32_t clk0t, uint32_t clk0h);
	int PacketToTrackByte(const uint8_t packet_byte, uint32_t clk1t, uint32_t clk0t, uint32_t clk0h, uint32_t clk0ts);
	int PacketToTrackBytes(const uint8_t packet_byte, uint8_t count, uint32_t clk1t, uint32_t clk0t, uint32_t clk0h);
	int PacketToTrackAmbig1(const uint8_t packet_byte, uint16_t clk1t, uint32_t clk0t1, uint32_t clk0h1, uint32_t clk0t, uint32_t clk0h);
	int PacketToTrackAmbig2(const uint8_t packet_byte, uint16_t clk1t, uint32_t clk0t1, uint32_t clk0h1, uint32_t clk0t2, uint32_t clk0h2, uint32_t clk0t, uint32_t clk0h);
	int IsPacketAvailable(void);

	#include "Track.h"

	#define SEND_PACKET_TIMEOUT 10000
};
#endif
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

#if SEND_VERSION >= 4
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		init_8254()							-	 Init 8254 timer.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem with initialization.
 *
 *	DESCRIPTION
 *
 *		init_8254() initializes the 8254 timer.  It sets it to the
 *		correct mode, load the default timer values.  and cycles
 *      CNT0H to force the DCC0 output to an initial low state.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::init_8254( void )
{

    // Eliminate bogus sccsid and sccsid_h declared but never used warning.
    dummy( sccsid, sccsid_h );

	if ( m_log_pkts )
	{
		DBGLOG( "Send_reg::init_8254() Done initializing 8254." );
	}

	clk0t	=	CNT_0T_INIT;
	clk0h	=	CNT_0H_INIT + 2;
	clk1t	=	CNT_1T_INIT;

	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		init_8255()							-	 Init 8255 I/O ports.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem with initialization.
 *
 *	DESCRIPTION
 *
 *		init_8255() initializes the 8255 I/O ports.  It sets port A
 *		as a strobed output, port B as a direct output, and port C
 *		as a direct input.  It also initializes port A and B to the
 *		correct initial values.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::init_8255( void )
{
	/*
	 *	Set 8255 to power up condition, all ports as inputs
	 */

	if ( m_log_pkts )
	{
		DBGLOG( "Send_reg::init_8255() Done initializing 8255." );
	}

	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		rst_8255()							-	 Reset 8255 to power up state.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem with reset.
 *
 *	DESCRIPTION
 *
 *		rst_8255() resets the 8255 I/O ports to the power on state.
 *		All ports are set to inputs.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::rst_8255( void )
{
	if ( m_log_pkts )
	{
		DBGLOG( "Send_reg::rst_8255() Done resetting 8255." );
	}

	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		init_send()							-	 Init sender board.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem with initialization.
 *
 *	DESCRIPTION
 *
 *		init_send() initializes the DCC sender board.  It sets the timers
 *		and I/O ports for normal operation.  It halts the hardware clock by
 *		placing the board in software clock mode.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::init_send( void )
{

	if ( m_log_pkts )
	{
		DBGLOG(	"Send_reg::init_send() Done initializing sender." );
	}

	clr_err_cnt();

	return ( OK );
}

#else
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		init_8254()							-	 Init 8254 timer.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem with initialization.
 *
 *	DESCRIPTION
 *
 *		init_8254() initializes the 8254 timer.  It sets it to the
 *		correct mode, load the default timer values.  and cycles
 *      CNT0H to force the DCC0 output to an initial low state.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::init_8254( void )
{

    // Eliminate bogus sccsid and sccsid_h declared but never used warning.
    dummy( sccsid, sccsid_h );
    
	if ( !m_log_pkts )
	{
		/*
		 *	Set 8254 CNT0H to mode 0 to force DCC output low.
		 */
		outportb( CNT_CNTRL, CNTRL_0H_RST );
		outportb( CNT_0H, CNT_LOWER( CNT_0H_INIT ) );
		outportb( CNT_0H, CNT_UPPER( CNT_0H_INIT ) );

		/*
		 *	Set 8254 CNT0T to normal mode and initialize count.
		 */
		outportb( CNT_CNTRL, CNTRL_0T_NORM );
		outportb( CNT_0T, CNT_LOWER( CNT_0T_INIT ) );
		outportb( CNT_0T, CNT_UPPER( CNT_0T_INIT ) );

		/*
		 *	Set 8254 CNT0H to normal mode and initialize count.
		 */
		outportb( CNT_CNTRL, CNTRL_0H_NORM );
		outportb( CNT_0H, CNT_LOWER( CNT_0H_INIT ) );
		outportb( CNT_0H, CNT_UPPER( CNT_0H_INIT ) );

		/*
		 *	Set 8254 CNT1T to normal mode and initialize count.
		 */
		outportb( CNT_CNTRL, CNTRL_1T_NORM );
		outportb( CNT_1T, CNT_LOWER( CNT_1T_INIT ) );
		outportb( CNT_1T, CNT_UPPER( CNT_1T_INIT ) );
	}
	else
	{
		DBGLOG( "Send_reg::init_8254() Done initializing 8254." );
	}

	clk0t	=	CNT_0T_INIT;
	clk0h	=	CNT_0H_INIT + 2;
	clk1t	=	CNT_1T_INIT;
	
	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		init_8255()							-	 Init 8255 I/O ports.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem with initialization.
 *
 *	DESCRIPTION
 *
 *		init_8255() initializes the 8255 I/O ports.  It sets port A
 *		as a strobed output, port B as a direct output, and port C
 *		as a direct input.  It also initializes port A and B to the
 *		correct initial values.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::init_8255( void )
{
	/*
	 *	Set 8255 to power up condition, all ports as inputs
	 */
	rst_8255();

	if ( !m_log_pkts )
	{
		/*
		 *	Set 8255 to normal mode.
		 */
		outportb( P_CNTRL, P_CNTRL_NORM );

		OUT_PC( PC_POS_BDRST, 1 );		   	// Reset board.
		OUT_PC( PC_POS_CPUEN, 1 );		   	// Enable software clock.
		OUT_PC( PC_POS_CPUCLKL, 0 );	   	// Set SW -CLK to 0.
		OUT_PC( PC_POS_INTRA, 1 );		   	// Set interrupt active.
		OUT_PC( PC_POS_UNDERCLRL, 1 );	   	// Set under clear inactive.
		OUT_PC( PC_POS_SCOPENL, 1 );	   	// Turn off scope signal.
		OUT_PC( PC_POS_ACKAL, 1 );		   	// Enable interrupt line.
		OUT_PC( PC_POS_OBFAL, 1 );		   	// Set -OBF to 1.

		OUT_PC( PC_POS_BDRST, 0 );		   	// Turn off board reset.
	}
	else
	{
		DBGLOG( "Send_reg::init_8255() Done initializing 8255." );
	}

	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		rst_8255()							-	 Reset 8255 to power up state.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem with reset.
 *
 *	DESCRIPTION
 *
 *		rst_8255() resets the 8255 I/O ports to the power on state.
 *		All ports are set to inputs.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::rst_8255( void )
{
	if ( !m_log_pkts )
	{
		/*
		 *	Set 8255 to power up condition, all ports as inputs
		 */
		outportb( P_CNTRL, P_CNTRL_RST );
	}
	else
	{
		DBGLOG( "Send_reg::rst_8255() Done resetting 8255." );
	}

	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		init_send()							-	 Init sender board.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem with initialization.
 *
 *	DESCRIPTION
 *
 *		init_send() initializes the DCC sender board.  It sets the timers
 *		and I/O ports for normal operation.  It halts the hardware clock by
 *		placing the board in software clock mode.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::init_send( void )
{
	init_8255();							// Init 8255.
	init_8254();							// Init 8254.

	if ( !m_log_pkts )
	{
		OUT_PC( PC_POS_CPUCLKL, 1 );		// Toggle clock to load 0T.
		OUT_PC( PC_POS_CPUCLKL, 0 );
	}
	else
	{
		DBGLOG(
			"Send_reg::init_send() Done initializing sender." );
	}

	clr_err_cnt();

	return ( OK );
}
#endif

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

#if SEND_VERSION < 4
	/*
	 *	Set 8254 CNT0T.
	 */
	if ( iclk0t != DCC_CLK_HOLD )
	{
		if ( iclk0t < DCC_CLK_MIN )
		{
			ERRPRINT( my_name, LOG_ERR,
				"iclk0t value %u invalid", iclk0t );
		}
		else
		{
			clk0t	=	iclk0t;
			if ( !m_log_pkts )
			{
				outportb( CNT_0T, CNT_LOWER( clk0t ) );
				outportb( CNT_0T, CNT_UPPER( clk0t ) );
			}
		}
	}

	/*
	 *	Set 8254 CNT0H.  Note - Correct for 2 count delay starting 0H.
	 */
	if ( iclk0h != DCC_CLK_HOLD )
	{
		if ( iclk0h < ( DCC_CLK_MIN / 2 ) )
		{
			ERRPRINT( my_name, LOG_ERR,
				"iclk0h value %u invalid", iclk0h );
		}
		else
		{
			clk0h	=	iclk0h;
			if ( !m_log_pkts )
			{
				outportb( CNT_0H, CNT_LOWER( clk0h - 2 ) );
				outportb( CNT_0H, CNT_UPPER( clk0h - 2 ) );
			}
		}
	}

	/*
	 *	Set 8254 CNT1T.
	 */
	if ( iclk1t != DCC_CLK_HOLD )
	{
		if ( iclk1t < DCC_CLK_MIN )
		{
			ERRPRINT( my_name, LOG_ERR,
				"iclk1t value %u invalid", iclk1t );
		}
		else
		{
			clk1t	=	iclk1t;
			if ( !m_log_pkts )
			{
				outportb( CNT_1T, CNT_LOWER( clk1t ) );
				outportb( CNT_1T, CNT_UPPER( clk1t ) );
			}
		}
	}

	/*
	 *	Start clock to flush change through.
	 */
	clr_under();
	start_clk();
	if ( send_bytes( 2, 0x55, "Flush clock change through hardware." ) != OK )
	{
		ERRPRINT( my_name, LOG_ERR,
			"send_bytes() FAILED" );
		stop_clk();
		clk0t = clk0h = clk1t = DCC_CLK_ERR;
		return ( FAIL );
	}
#endif

	/*
	 *	Reset to the initial run state.
	 */
	if ( !trunning )
	{
		stop_clk();
	}

	if ( m_log_pkts )
	{
		DBGLOG( "%s() Clk 0T %hu, 0H %hu, 1T %hu.",
			my_name, clk0t, clk0h, clk1t );
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

	if ( !m_log_pkts )
	{
		OUT_PC( PC_POS_CPUCLKL, 0 );	 	// Set SW clock to 0.
		OUT_PC( PC_POS_CPUEN, true );	 	// Stop the HW clock.
		OUT_PC( PC_POS_CPUEN, true );	 	// Delay so HW clock stops.
		OUT_PC( PC_POS_SCOPENL, 1 );	 	// Disable scope trigger.
		OUT_PC( PC_POS_CPUEN, false );	 	// Start glitch free HW clock.
	}
	else
	{
		DBGLOG( "%s() Clock started.", my_name );
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

	if ( !m_log_pkts )
	{
		OUT_PC( PC_POS_CPUCLKL, 0 );	  	// Set SW clock to 0.
		OUT_PC( PC_POS_CPUEN, true );	  	// Stop the HW clock w/o glitch.
		OUT_PC( PC_POS_CPUEN, true );	  	// Delay so HW clock stops.
		OUT_PC( PC_POS_SCOPENL, 1 );	  	// Disable scope trigger.
	}
	else
	{
		DBGLOG( "%s() Clock stopped.", my_name );
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
	#if SEND_VERSION < 4
		register u_int	count;			// Count of BYTES sent.
		register u_long	san_cnt;		// Sanity timeout.
	#endif

	if ( icnt < 1 )
	{
		ERRPRINT( my_name, LOG_ERR,
			"0 icnt, b_cnt %lu, p_cnt %lu", b_cnt, p_cnt );
		return ( FAIL );
	}

	if ( !m_log_pkts )	// Skip hardware interaction if just logging.
	{
#if SEND_VERSION >= 4

		for(int i = 0; i < SEND_PACKET_TIMEOUT; i++)
		{
			if(IsPacketAvailable() == 1)
			{
				if(PacketToTrackBytes(ibyte, icnt, clk1t, clk0t, clk0h) == 0)
				{
					return ( OK );
				}
			}
			osDelay(pdMS_TO_TICKS(1));
		}
		return ( FAIL );

#else
		start_crit();
		for ( count = 0; count < icnt; ++count )
		{
			for ( san_cnt =  SAN_CNT;
				san_cnt > 0 && !(inportb( PC ) & PC_MSK_INTRA);
				--san_cnt )
			{
				;
			}

			if ( san_cnt ==  0 )
			{
				enable();
				if ( errprint_ok() )
				{
					ERRPRINT( my_name, LOG_ERR,
						"Sanity timeout, sent %u, b_cnt %lu, p_cnt %lu",
						count, b_cnt, p_cnt );
				}
				SET_ERROR( NOT_READY_OBJ_WARN  );
				return ( FAIL );
			}

			switch ( scope )
			{
			case SCOPE_IDLE:					// Do nothing.
				break;

			case SCOPE_END:						// End scope trigger.
				OUT_PC( PC_POS_SCOPENL, 1 );
				scope	=	SCOPE_IDLE;
				break;

			case SCOPE_START:					// Start scope trigger.
				OUT_PC( PC_POS_SCOPENL, 0 );
				scope	=	SCOPE_END;
				break;
			}

			if ( inportb( PB ) & PB_MSK_UNDER )
			{
				enable();
				if ( errprint_ok() )
				{
					ERRPRINT( my_name, LOG_ERR,
						"Underflow, sent %u, b_cnt %lu, p_cnt %lu",
						count, b_cnt, p_cnt );
				}
				SET_ERROR( NOT_READY_OBJ_WARN  );
				return ( FAIL );
			}

			outportb( PA, m_swap_0_1 ? ~ibyte : ibyte );
			OUT_PC( PC_POS_UNDERCLRL, 1 );
		}

		enable();
#endif
	} // End of hardware interaction.
	else
	{
		DBGLOG( "%s() %s.", my_name, info ? info : "" );
		DBGPKT( "%02x %4d %s", ibyte, icnt, icnt > 1 ? "times" : "time" );
	}

	clr_err_cnt();							// Restart error counter.
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
	#if SEND_VERSION < 4
		register u_long	san_cnt;		   		// Sanity timeout.
	#endif
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

	if ( !m_log_pkts )	// Skip hardware interraction if just logging.
	{
#if SEND_VERSION >= 4

		/*
		 *	Calculate the CLK0T value to use for the stretched 0.  The
		 *	hardware will use this value for the first half of the total
		 *	period and the original value for the second half.  The CLK0T
         *	counter is reloaded half way through the square wave period.
		 */
		tclk0t	=	(2 * iclk0t) - clk0t;
		//tclk0t	=	12000;

		for(int i = 0; i < SEND_PACKET_TIMEOUT; i++)
		{
			if(IsPacketAvailable() == 1)
			{
				if(PacketToTrackByte(ibyte, clk1t, clk0t, clk0h, tclk0t) == 0)
				{
					return ( OK );
				}
			}
			osDelay(pdMS_TO_TICKS(1));
		}
		return ( FAIL );


#else
		if ( inportb( PA ) != 0xff )
		{
			ERRPRINT( my_name, LOG_ERR,
				"PA must be sending preamble bits, it is set to <%02x>",
				inportb( PA ) );
			return ( FAIL );
		}

		/*
		 *	Calculate the CLK0T value to use for the stretched 0.  The
		 *	hardware will use this value for the first half of the total
		 *	period and the original value for the second half.  The CLK0T
         *	counter is reloaded half way through the square wave period.
		 */
		tclk0t	=	(2 * iclk0t) - clk0t;

		start_crit();

		// Step 1: Wait to load 'ibyte' until board is ready.
		for (	san_cnt =  SAN_CNT;
				san_cnt > 0 && !(inportb( PC ) & PC_MSK_INTRA);
				--san_cnt )
		{
			;
		}

		if ( san_cnt ==  0 )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout before loading ibyte, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		switch ( scope )
		{
		case SCOPE_IDLE:			 		// Do nothing.
			break;

		case SCOPE_END:					 	// End scope trigger.
			OUT_PC( PC_POS_SCOPENL, 1 );
			scope	=	SCOPE_IDLE;
			break;

		case SCOPE_START:				 	// Start scope trigger.
			OUT_PC( PC_POS_SCOPENL, 0 );
			scope	=	SCOPE_END;
			break;
		}

		if ( inportb( PB ) & PB_MSK_UNDER )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
					"Underflow, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		outportb( PA, m_swap_0_1 ? ~ibyte : ibyte );
		OUT_PC( PC_POS_UNDERCLRL, 1 );

		// Step 2: Disable interrupts and set temporary new 0T and 0H times.
		disable();

		/*
		 *	Set stretched 8254 CNT0T.
         *	Bug ALERT: OKI 82C54-2 counters must have the control word
         *	set so that the temporary values load.  82C54-10 doesn't need
         *	this.  This is set shortly before the beginning of the beginning
         *	of the first 0 bit of the stretched packet.  This fix is needed
         *	only if the set occurs while a 1 is being sent.
		 */
		if ( inportb( PB ) & PB_MSK_DCCQH )
        {
			outportb( CNT_CNTRL, CNTRL_0T_NORM );
        }
        
		outportb( CNT_0T, CNT_LOWER( tclk0t ) );
		outportb( CNT_0T, CNT_UPPER( tclk0t ) );

		/*
		 *	Set stretched 8254 CNT0H.  Note - Correct for 2 count 0H delay.
		 */
		outportb( CNT_0H, CNT_LOWER( iclk0h - 2 ) );
		outportb( CNT_0H, CNT_UPPER( iclk0h - 2 ) );

		// Step 3: Wait for board to load 'ibyte'by monitoring INTRA.
		for (	san_cnt =  SAN_CNT;
				san_cnt > 0 && !(inportb( PC ) & PC_MSK_INTRA);
				--san_cnt )
		{
			;
		}

		if ( san_cnt ==  0 )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout waiting to load ibyte, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		/*
         *	Step 4: Wait for board to start sending 0 bit by monitoring DCCOUTD.
         *	This load will occur shortly after the start of the stretched 0
         *	bit.  Clk0T will be loaded half way through the stretched period.
         */
		for (	san_cnt =  SAN_CNT;
				san_cnt > 0 && !(inportb( PB ) & PB_MSK_DCCOUTD);
				--san_cnt )
		{
			;
		}

		if ( san_cnt ==  0 )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout before sending first 0, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		// Step 5: Kill at least 1 usec. while CNT0H gets loaded.
		pc_delay_high( m_pc_delay_1usec );

		// Step 6: Reload original CNT0T and CNT0H values.
		/*
		 *	Reset original 8254 CNT0T.
		 */
		outportb( CNT_0T, CNT_LOWER( clk0t ) );
		outportb( CNT_0T, CNT_UPPER( clk0t ) );

		/*
		 *	Reset original 8254 CNT0H.  Note - Correct for 2 count 0H delay.
		 */
		outportb( CNT_0H, CNT_LOWER( clk0h - 2 ) );
		outportb( CNT_0H, CNT_UPPER( clk0h - 2 ) );

		enable();
#endif
	} // End of hardware interaction.
	else
	{
		DBGLOG( "%s() Clk 0T %hu, 0H %hu %s.",
				 my_name, iclk0t, iclk0h, info ? info : "" );
		DBGPKT( "%02x", ibyte );
	}

	clr_err_cnt();							// Restart error counter.
	++b_cnt;

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
	const char		*my_name = "Send_reg::send_1_ambig_bit";
	#if SEND_VERSION >= 4
		u_short			tclk0t;					// clk0t for stretched 0.
	#else
		register u_long	san_cnt;		   		// Sanity timeout.
	#endif

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

	if ( iclk0t < AMBIG_0T_MIN )
	{
		ERRPRINT( my_name, LOG_ERR,
			"iclk0t value %u invalid, must be >= %u", iclk0t, AMBIG_0T_MIN );
		return ( FAIL );
	}

	if ( iclk0h < AMBIG_0H_MIN )
	{
		ERRPRINT( my_name, LOG_ERR,
			"iclk0h value %u invalid, must be >= %u", iclk0h, AMBIG_0H_MIN );
		return ( FAIL );
	}

	/*
	 *	Test that the scope trigger is set.
     *	This signal is used to synchronize the initial 0T and 0H clock load.
     */
	if ( scope != SCOPE_START )
	{
		ERRPRINT( my_name, LOG_ERR,
			"Scope trigger must be set for this command" );
		return ( FAIL );
	}

	if ( !m_log_pkts )	// Skip hardware interraction if just logging.
	{
#if SEND_VERSION >= 4

		tclk0t	=	12000;

		for(int i = 0; i < SEND_PACKET_TIMEOUT; i++)
		{
			if(IsPacketAvailable() == 1)
			{
				if(PacketToTrackByte(ibyte, clk1t, clk0t, clk0h, tclk0t) == 0)
				{
					return ( OK );
				}
			}
			osDelay(pdMS_TO_TICKS(1));
		}
		return ( FAIL );


#else
		start_crit();

		// Step 1: Wait to load 'ibyte' until board is ready.
		for (	san_cnt =  SAN_CNT;
				san_cnt > 0 && !(inportb( PC ) & PC_MSK_INTRA);
				--san_cnt )
		{
			;
		}

		if ( san_cnt ==  0 )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout before loading ibyte, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		switch ( scope )
		{
		case SCOPE_IDLE:			 		// Do nothing.
			break;

		case SCOPE_END:					 	// End scope trigger.
			OUT_PC( PC_POS_SCOPENL, 1 );
			scope	=	SCOPE_IDLE;
			break;

		case SCOPE_START:				 	// Start scope trigger.
			OUT_PC( PC_POS_SCOPENL, 0 );
			scope	=	SCOPE_END;
			break;
		}

		if ( inportb( PB ) & PB_MSK_UNDER )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
					"Underflow, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		outportb( PA, m_swap_0_1 ? ~ibyte : ibyte );
		OUT_PC( PC_POS_UNDERCLRL, 1 );

		// Step 2: Disable interrupts and set temporary new 0T and 0H times.
		disable();

        /*
		*	Step 3: Wait for board to get past half way point of last bit
        *	by monitoring 'SCOPE'.  This will go low high way through last
        *	bit.  It is safe to load new 0T and 0H times at this point.
        */
		for (	san_cnt =  SAN_CNT;
				(san_cnt > 0) && !(inportb( PB ) & PB_MSK_SCOPE);
				--san_cnt )
		{
			;
		}

		if ( san_cnt ==  0)
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout waiting to load 0 times, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		/*
		 *	Set ambiguous 8254 CNT0T.
         *	Bug ALERT: OKI 82C54-2 counters must have the control word
         *	set so that the temporary values load.  82C54-10 doesn't need
         *	this.  This is set shortly before the beginning of the beginning
         *	of the first 0 bit of the stretched packet.  This fix is needed
         *	only if the set occurs while a 1 is being sent.
		 */
		if ( inportb( PB ) & PB_MSK_DCCQH )
        {
			outportb( CNT_CNTRL, CNTRL_0T_NORM );
        }

		outportb( CNT_0T, CNT_LOWER( iclk0t ) );
		outportb( CNT_0T, CNT_UPPER( iclk0t ) );

		/*
		 *	Set ambiguous 8254 CNT0H.  Note - Correct for 2 count 0H delay.
		 */
		outportb( CNT_0H, CNT_LOWER( iclk0h - 2 ) );
		outportb( CNT_0H, CNT_UPPER( iclk0h - 2 ) );

		// Step 4: Wait for board to load 'ibyte'by monitoring INTRA.
		for (	san_cnt =  SAN_CNT;
				san_cnt > 0 && !(inportb( PC ) & PC_MSK_INTRA);
				--san_cnt )
		{
			;
		}

		if ( san_cnt ==  0 )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout waiting to load ibyte, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		/*
         *	Step 5: Wait for board to start sending 0 bit by monitoring DCCOUTD.
         *	This load will occur shortly after the start of the stretched 0
         *	bit.  Clk0T will be loaded half way through the stretched period.
         */
		for (	san_cnt =  SAN_CNT;
				san_cnt > 0 && !(inportb( PB ) & PB_MSK_DCCOUTD);
				--san_cnt )
		{
			;
		}

		if ( san_cnt ==  0 )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout before sending first 0, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		// Step 6: Wait for board to reach middle of 0T by monitoring OUT0.
		for (	san_cnt =  SAN_CNT, outportb( CNT_CNTRL, CNTRL_0T_STATUS );
				san_cnt > 0 && (inportb( CNT_0T ) & CNT_STATUS_OUTPUT );
				--san_cnt )
		{
			outportb( CNT_CNTRL, CNTRL_0T_STATUS );
		}

		if ( san_cnt ==  0 )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout reaching middle of 1st 0, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		// Step 7: Reload original CNT0T and CNT0H values.
		/*
		 *	Reset original 8254 CNT0T.
		 */
		outportb( CNT_0T, CNT_LOWER( clk0t ) );
		outportb( CNT_0T, CNT_UPPER( clk0t ) );

		/*
		 *	Reset original 8254 CNT0H.  Note - Correct for 2 count 0H delay.
		 */
		outportb( CNT_0H, CNT_LOWER( clk0h - 2 ) );
		outportb( CNT_0H, CNT_UPPER( clk0h - 2 ) );

		enable();
#endif
	} // End of hardware interaction.
	else
	{
		DBGLOG( "%s() Clk 0T %hu, 0H %hu: %s.",
				 my_name, iclk0t, iclk0h, info ? info : "" );
		DBGPKT( "%02x", ibyte );
	}

	clr_err_cnt();							// Restart error counter.
	++b_cnt;

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
	const char		*my_name = "Send_reg::send_2_ambig_bits";
	#if SEND_VERSION < 4
		register		u_long	san_cnt;		// Sanity timeout.
	#endif


	if ( ibyte & 0xC0 )
	{
		ERRPRINT( my_name, LOG_ERR,
			"ibyte must have bit 6 & 7 set to 0, its value is 0x%02x", ibyte );
		return ( FAIL );
	}

	if ( iclk0t1 ==  DCC_CLK_HOLD )
	{
		iclk0t1	=	clk0t;
	}

	if ( iclk0h1 == DCC_CLK_HOLD )
	{
		iclk0h1	=	clk0h;
	}

	if ( iclk0t2 ==  DCC_CLK_HOLD )
	{
		iclk0t1	=	clk0t;
	}

	if ( iclk0h2 == DCC_CLK_HOLD )
	{
		iclk0h1	=	clk0h;
	}

	if ( iclk0t1 < AMBIG_0T_MIN )
	{
		ERRPRINT( my_name, LOG_ERR,
			"iclk0t1 value %u invalid, must be >= %u", iclk0t1, AMBIG_0T_MIN );
		return ( FAIL );
	}

	if ( iclk0h1 < AMBIG_0H_MIN )
	{
		ERRPRINT( my_name, LOG_ERR,
			"iclk0h1 value %u invalid, must be >= %u", iclk0h1, AMBIG_0H_MIN );
		return ( FAIL );
	}

	if ( iclk0t2 < AMBIG_0T_MIN )
	{
		ERRPRINT( my_name, LOG_ERR,
			"iclk0t2 value %u invalid, must be >= %u", iclk0t2, AMBIG_0T_MIN );
		return ( FAIL );
	}

	if ( iclk0h2 < AMBIG_0H_MIN )
	{
		ERRPRINT( my_name, LOG_ERR,
			"iclk0h2 value %u invalid, must be >= %u", iclk0h2, AMBIG_0H_MIN );
		return ( FAIL );
	}

	/*
	 *	Test that the scope trigger is set.
     *	This signal is used to synchronize the initial 0T and 0H clock load.
     */
	if ( scope != SCOPE_START )
	{
		ERRPRINT( my_name, LOG_ERR,
			"Scope trigger must be set for this command" );
		return ( FAIL );
	}

	if ( !m_log_pkts )	// Skip hardware interraction if just logging.
	{
#if SEND_VERSION >= 4

		for(int i = 0; i < SEND_PACKET_TIMEOUT; i++)
		{
			if(IsPacketAvailable() == 1)
			{
				if(PacketToTrackAmbig2(ibyte, clk1t, iclk0t1, iclk0h1, iclk0t2, iclk0h2, clk0t, clk0h) == 0)
				{
					return ( OK );
				}
			}
			osDelay(pdMS_TO_TICKS(1));
		}
		return ( FAIL );


#else
		start_crit();

		// Step 1: Wait to load 'ibyte' until board is ready.
		for (	san_cnt =  SAN_CNT;
				san_cnt > 0 && !(inportb( PC ) & PC_MSK_INTRA);
				--san_cnt )
		{
			;
		}

		if ( san_cnt ==  0 )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout before loading ibyte, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		switch ( scope )
		{
		case SCOPE_IDLE:			 		// Do nothing.
			break;

		case SCOPE_END:					 	// End scope trigger.
			OUT_PC( PC_POS_SCOPENL, 1 );
			scope	=	SCOPE_IDLE;
			break;

		case SCOPE_START:				 	// Start scope trigger.
			OUT_PC( PC_POS_SCOPENL, 0 );
			scope	=	SCOPE_END;
			break;
		}

		if ( inportb( PB ) & PB_MSK_UNDER )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
					"Underflow, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		outportb( PA, m_swap_0_1 ? ~ibyte : ibyte );
		OUT_PC( PC_POS_UNDERCLRL, 1 );

		// Step 2: Disable interrupts and set temporary new 0T1 and 0H1 times.
		disable();


        /*
		*	Step 3: Wait for board to get past half way point of last bit
        *	by monitoring 'ACKAL'.  This will go low half way through last
        *	bit.  It is safe to load new 0T and 0H times at this point.
        */
		for (	san_cnt =  SAN_CNT;
				(san_cnt > 0) && !(inportb( PB ) & PB_MSK_SCOPE);
				--san_cnt )
		{
			;
		}

		if ( san_cnt ==  0 )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout waiting to load 0 times, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		/*
		 *	Set first ambiguous 8254 CNT0T.
         *	Bug ALERT: OKI 82C54-2 counters must have the control word
         *	set so that the temporary values load.  82C54-10 doesn't need
         *	this.  This is set shortly before the beginning of the beginning
         *	of the first 0 bit of the stretched packet.  This fix is needed
         *	only if the set occurs while a 1 is being sent.
		 */
		if ( inportb( PB ) & PB_MSK_DCCQH )
        {
			outportb( CNT_CNTRL, CNTRL_0T_NORM );
        }

		outportb( CNT_0T, CNT_LOWER( iclk0t1 ) );
		outportb( CNT_0T, CNT_UPPER( iclk0t1 ) );

		/*
		 *	Set first ambiguous 8254 CNT0H.  Note - Correct for 2 count 0H delay.
		 */
		outportb( CNT_0H, CNT_LOWER( iclk0h1 - 2 ) );
		outportb( CNT_0H, CNT_UPPER( iclk0h1 - 2 ) );

		// Step 4: Wait for board to load 'ibyte'by monitoring INTRA.
		for (	san_cnt =  SAN_CNT;
				san_cnt > 0 && !(inportb( PC ) & PC_MSK_INTRA);
				--san_cnt )
		{
			;
		}

		if ( san_cnt ==  0 )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout waiting to load ibyte, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		// Step 5: Wait for board to start 1st 0 bit by monitoring DCCOUTD.
		for (	san_cnt =  SAN_CNT;
				san_cnt > 0 && !(inportb( PB ) & PB_MSK_DCCOUTD);
				--san_cnt )
		{
			;
		}

		if ( san_cnt ==  0 )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout before sending 1st 0, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		// Step 6: Wait for board to reach middle of 0T by monitoring OUT0.
		for (	san_cnt =  SAN_CNT, outportb( CNT_CNTRL, CNTRL_0T_STATUS );
				san_cnt > 0 && (inportb( CNT_0T ) & CNT_STATUS_OUTPUT );
				--san_cnt )
		{
			outportb( CNT_CNTRL, CNTRL_0T_STATUS );
		}

		if ( san_cnt ==  0 )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout reaching middle of 1st 0, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		// Step 7: Load 2nd ambiguous CNT0T and CNT0H values.
		/*
		 *	Set 2nd ambiguous 8254 CNT0T.
		 */
		outportb( CNT_0T, CNT_LOWER( iclk0t2 ) );
		outportb( CNT_0T, CNT_UPPER( iclk0t2 ) );

		/*
		 *	Set 2nd ambiguous 8254 CNT0H.  Note - Correct for 2 count 0H delay.
		 */
		outportb( CNT_0H, CNT_LOWER( iclk0h2 - 2 ) );
		outportb( CNT_0H, CNT_UPPER( iclk0h2 - 2 ) );

		// Step 8: Wait for board to start 2nd 0 bit by monitoring OUT0.
      for (	san_cnt =  SAN_CNT, outportb( CNT_CNTRL, CNTRL_0T_STATUS );
				san_cnt > 0 && !(inportb( CNT_0T ) & CNT_STATUS_OUTPUT );
				--san_cnt )
		{
			outportb( CNT_CNTRL, CNTRL_0T_STATUS );
		}

		if ( san_cnt ==  0 )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout before sending 2nd 0, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}


		// Step 9: Wait for board to reach middle of 0T by monitoring OUT0.
		for (	san_cnt =  SAN_CNT, outportb( CNT_CNTRL, CNTRL_0T_STATUS );
				san_cnt > 0 && (inportb( CNT_0T ) & CNT_STATUS_OUTPUT );
				--san_cnt )
		{
			outportb( CNT_CNTRL, CNTRL_0T_STATUS );
		}

		if ( san_cnt ==  0 )
		{
			enable();
			if ( errprint_ok() )
			{
				ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout reaching middle of 2nd 0, b_cnt %lu, p_cnt %lu",
					b_cnt, p_cnt );
			}
			SET_ERROR( NOT_READY_OBJ_WARN  );
			return ( FAIL );
		}

		// Step 10: Reload original CNT0T and CNT0H values.
		/*
		 *	Reset original 8254 CNT0T.
		 */
		outportb( CNT_0T, CNT_LOWER( clk0t ) );
		outportb( CNT_0T, CNT_UPPER( clk0t ) );

		/*
		 *	Reset original 8254 CNT0H.  Note - Correct for 2 count 0H delay.
		 */
		outportb( CNT_0H, CNT_LOWER( clk0h - 2 ) );
		outportb( CNT_0H, CNT_UPPER( clk0h - 2 ) );

		enable();
#endif
	} // End of hardware interaction.
	else
	{
		DBGLOG( "%s() Clk 0T1 %hu, 0H1 %hu, 0T2 %hu, 0H2 %hu: %s.",
				my_name, iclk0t1, iclk0h1, iclk0t2, iclk0h2,
				info ? info : "", ibyte );
		DBGPKT( "%02x", ibyte );
	}

	clr_err_cnt();							// Restart error counter.
	++b_cnt;

	return ( OK );
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
	#if SEND_VERSION < 4
		register u_int	count;					// Count of BYTES sent.
		register u_long	san_cnt;		   		// Sanity timeout.
	#endif

	if ( isize < 1 )
	{
		ERRPRINT( my_name, LOG_ERR,
			"0 isize, b_cnt %lu, p_cnt %lu", b_cnt, p_cnt );
		return ( FAIL );
	}


	if ( !m_log_pkts )	// Skip hardware interraction if just logging.
	{
#if SEND_VERSION >= 4

		for(int i = 0; i < SEND_PACKET_TIMEOUT; i++)
		{
			if(IsPacketAvailable() == 1)
			{
				if(PacketToTrack(ibytes, isize, clk1t, clk0t, clk0h) == 0)
				{
					return ( OK );
				}
			}
			osDelay(pdMS_TO_TICKS(1));
		}
		return ( FAIL );

#else
		/* Send first BYTE, then activate underflow warning */
		start_crit();
		for ( count = 0; count < isize; ++ibytes, ++count )
		{
			for ( san_cnt =  SAN_CNT;
				san_cnt > 0 && !(inportb( PC ) & PC_MSK_INTRA);
				--san_cnt )
			{
				;
			}

			if ( san_cnt ==  0 )
			{
				enable();
				if ( errprint_ok() )
				{
					ERRPRINT( my_name, LOG_ERR,
					"Sanity timeout, sent %u, b_cnt %lu, p_cnt %lu",
						count, b_cnt, p_cnt );
				}
				SET_ERROR( NOT_READY_OBJ_WARN  );
				return ( FAIL );
			}

			switch ( scope )
			{
			case SCOPE_IDLE:		  			// Do nothing.
				break;

			case SCOPE_END:						// End scope trigger.
				OUT_PC( PC_POS_SCOPENL, 1 );
				scope	=	SCOPE_IDLE;
				break;

			case SCOPE_START:					// Start scope trigger.
				OUT_PC( PC_POS_SCOPENL, 0 );
				scope	=	SCOPE_END;
				break;
			}

			if ( inportb( PB ) & PB_MSK_UNDER )
			{
				enable();
				if ( errprint_ok()  )
				{
					ERRPRINT( my_name, LOG_ERR,
					"Underflow, sent %u, b_cnt %lu, p_cnt %lu",
						count, b_cnt, p_cnt );
				}
				SET_ERROR( NOT_READY_OBJ_WARN  );
				return ( FAIL );
			}

			outportb( PA, m_swap_0_1 ? ~*ibytes : *ibytes );
			OUT_PC( PC_POS_UNDERCLRL, 1 );
		}

		enable();
#endif
	} // End of hardware interaction.
	else
	{
		DBGLOG( "%s %s.", my_name, info ? info : "" );
		print_pkt_log( ibytes, isize );
	}

	clr_err_cnt();							// Restart error counter.
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
	register u_int	count;					// Count of BYTES sent.
	#if SEND_VERSION < 4
		register u_long	san_cnt;	   		// Sanity timeout.
		BYTE			pbyte;					// Present BYTE.
	#endif

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

	if ( !m_log_pkts )	// Skip hardware interraction if just logging.
	{
#if SEND_VERSION >= 4
	for(int i = 0; i < SEND_PACKET_TIMEOUT; i++)
	{
		if(IsPacketAvailable() == 1)
		{
			if(PacketToTrack(ibits.get_byte_array(), ibits.get_byte_size(), clk1t, clk0t, clk0h) == 0)
			{
				count = ibits.get_byte_size();
				return ( OK );
			}
		}
		osDelay(pdMS_TO_TICKS(1));
	}
	return ( FAIL );

#else
		start_crit();
		for (	count = 0, ibits.rst_out();
				ibits.get_byte( pbyte ) == OK; count++ )
		{
			for ( san_cnt =  SAN_CNT;
				san_cnt > 0 && !(inportb( PC ) & PC_MSK_INTRA);
				--san_cnt )
			{
				;
			}

			if ( san_cnt ==  0 )
			{
				enable();
				if ( errprint_ok() )
				{
					ERRPRINT( my_name, LOG_ERR,
					"Sanity timeout, sent %u, b_cnt %lu, p_cnt %lu",
						count, b_cnt, p_cnt );
				}
				SET_ERROR( NOT_READY_OBJ_WARN  );
				return ( FAIL );
			}

			switch ( scope )
			{
			case SCOPE_IDLE:		  			// Do nothing.
				break;

			case SCOPE_END:						// End scope trigger.
				OUT_PC( PC_POS_SCOPENL, 1 );
				scope	=	SCOPE_IDLE;
				break;

			case SCOPE_START:					// Start scope trigger.
				OUT_PC( PC_POS_SCOPENL, 0 );
				scope	=	SCOPE_END;
				break;
			}

			if ( inportb( PB ) & PB_MSK_UNDER )
			{
				enable();
				if ( errprint_ok() )
				{
					ERRPRINT( my_name, LOG_ERR,
					"Underflow, sent %u, b_cnt %lu, p_cnt %lu",
						count, b_cnt, p_cnt );
				}
				SET_ERROR( NOT_READY_OBJ_WARN  );
				return ( FAIL );
			}

			outportb( PA, m_swap_0_1 ? ~pbyte : pbyte );
			OUT_PC( PC_POS_UNDERCLRL, 1 );
		}

		enable();
		if ( count == 0 )
		{
			ERRPRINT( my_name, LOG_ERR,
				"0 Bytes sent, b_cnt %lu, p_cnt %lu", b_cnt, p_cnt );
			return ( FAIL );
		}
#endif
	} // End of hardware interaction.
	else
	{
		DBGLOG( "%s %s.", my_name, info ? info : "" );
		print_pkt_log( ibits.get_byte_array(), ibits.get_byte_size() );
		#if SEND_VERSION >= 4
			count = 0;		// to avoid the warning at "b_cnt	+= count;"
		#endif
	}

	clr_err_cnt();							// Restart error counter.
	b_cnt	+= count;
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

	/*
	 *	Skip this check if we are just logging packets.
	 */
	if ( m_log_pkts )
	{
		DBGLOG( "Send_reg::update() Skipping update." );
		m_pc_delay_1usec	=	1;
		return;
	}

#if SEND_VERSION < 4
	v.gen		=	inportb( GEN );			// Read generic inputs.

	v.pa 		=	inportb( PA );			// Read 8255 ports.
	v.pb 		=	inportb( PB );
	v.pc 		=	inportb( PC );

	outportb( CNT_CNTRL, (BYTE)0xCE );		// Latch counter data.
	v.s0t		=	inportb( CNT_0T );
	v.cnt0t		=	inportb( CNT_0T );
	v.cnt0t		+=	inportb( CNT_0T ) << 8;
	v.s0h		=	inportb( CNT_0H );
	v.cnt0h		=	inportb( CNT_0H );
	v.cnt0h		+=	inportb( CNT_0H ) << 8;
	v.s1t		=	inportb( CNT_1T );
	v.cnt1t		=	inportb( CNT_1T );
	v.cnt1t		+=	inportb( CNT_1T ) << 8;
#endif
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
 *		set_pc_delay_1usec()	   	 	-	 Set PC 1 usec delay value.
 *
 *	RETURN VALUE
 *
 *		OK		-	Sucess.
 *		FAIL	-	Problem setting m_pc_delay_1usec.
 *
 *	DESCRIPTION
 *
 *		set_pc_delay_1usec tests the PC 1 usec delay time and sets
 *		'm_pc_delay_1usec' accordingly.  It returns FAIL if a problem
 *		occurs.
 *      Note: set_clk() is called to set nominal clock values
 *            when this method is called.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Send_reg::set_pc_delay_1usec( void )
{
	const char		*my_name = "Send_reg::set_pc_delay_1usec";
	#if SEND_VERSION < 4
		register u_int	short_san_cnt;	   		// Short sanity timeout.
		register u_int	tmp_delay_high;	 		// pc_delay_high() return value.
		register u_int	tmp_delay_low;	 		// pc_delay_low() return value.
	#endif

	/*
	 *	Skip this check if we are just logging packets.
	 */
	if ( m_log_pkts )
	{
		DBGLOG( "%s() Skipping hardware test, setting delay to 1.",
			my_name );
		m_pc_delay_1usec	=	1;
		return ( OK );
	}

	/*
	 *	Initialize the hardware and start the clock at nominal values.
	 */
	init_send();
	if ( set_clk(	DECODER_0T_NOM,
					DECODER_0H_NOM,
					DECODER_1T_NOM ) != OK )
	{
		ERRPRINT( my_name, LOG_ERR, "set_clk() FAILED" );
		return ( FAIL );
	}
	start_clk();

	if ( send_bytes( 1, 0x00, "Send single 0." ) != OK )
	{
		ERRPRINT( my_name, LOG_ERR, "send_bytes() FAILED" );
		stop_clk();
		return ( FAIL );
	}

#if SEND_VERSION < 4
	// Step 1: Disable interrupts.
	disable();

	// Step 2: Wait for board to load 'ibyte'by monitoring INTRA.
	for (	short_san_cnt =  SHORT_SAN_CNT;
			short_san_cnt > 0 && !(inportb( PC ) & PC_MSK_INTRA);
			--short_san_cnt )
	{
		;
	}

	if ( short_san_cnt ==  0 )
	{
		enable();
		if ( errprint_ok() )
		{
			ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout waiting to load ibyte" );
		}
		SET_ERROR( NOT_READY_OBJ_WARN  );
		return ( FAIL );
	}

	// Step 3: Wait for board to start sending 0 bit by monitoring DCCOUTD.
	for (	short_san_cnt =  SHORT_SAN_CNT;
			short_san_cnt > 0 && !(inportb( PB ) & PB_MSK_DCCOUTD);
			--short_san_cnt )
	{
		;
	}

	if ( short_san_cnt ==  0 )
	{
		enable();
		if ( errprint_ok() )
		{
			ERRPRINT( my_name, LOG_ERR,
				"Sanity timeout before sending first 0" );
		}
		SET_ERROR( NOT_READY_OBJ_WARN  );
		return ( FAIL );
	}

	// Step 4: Time the high period of DCCOUTD.
	tmp_delay_high	=	pc_delay_high( SHORT_SAN_CNT );

	// Step 5: Time the low period of DCCOUTD.
	tmp_delay_low	=	pc_delay_low( SHORT_SAN_CNT );

	// Step 6: enable interrupts, stop the hardware, and evaluate the results.
	enable();
	stop_clk();

	STATPRINT(	"PC high %4u(%4u), low %4u(%4u)",
		clk0h, SHORT_SAN_CNT - tmp_delay_high,
		clk0t - clk0h, SHORT_SAN_CNT - tmp_delay_low );
	printf(		"PC high %4u(%4u), low %4u(%4u)\n",
		clk0h, SHORT_SAN_CNT - tmp_delay_high,
		clk0t - clk0h, SHORT_SAN_CNT - tmp_delay_low );

	m_pc_delay_1usec = ((SHORT_SAN_CNT - tmp_delay_high)/DECODER_0H_NOM) + 1;
	STATPRINT(	"PC 1 usec delay %u", m_pc_delay_1usec );
	printf(		"PC 1 usec delay %u\n", m_pc_delay_1usec );
#endif

	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		pc_delay_high()	   		 		-	 PC driven delay for DCCOUT high.
 *
 *	RETURN VALUE
 *
 *		idelay	-	Time remaining, if any.
 *
 *	DESCRIPTION
 *
 *		pc_delay_high() can test or generate a PC driven delay.
 *		This method assumes that interrupts are disabled
 *		and that DCCOUTD is high prior to the call.  This allows the same
 *		method to be called from set_pc_delay_1usec() to find the correct
 *		'idelay' value and then to be called from other methods to
 *		generate the delay.
 */
/*--------------------------------------------------------------------------*/

u_int
Send_reg::pc_delay_high(
	register u_int	idelay )			// Delay count.
{
	while ( idelay > 0 && (inportb( PB ) & PB_MSK_DCCOUTD) )
	{
		idelay--;
	}
	return ( idelay );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		pc_delay_low()	   		 		-	 PC driven delay for DCCOUT low.
 *
 *	RETURN VALUE
 *
 *		idelay	-	Time remaining, if any.
 *
 *	DESCRIPTION
 *
 *		pc_delay_low() can test or generate a PC driven delay.
 *		This method assumes that interrupts are disabled
 *		and that DCCOUTD is low prior to the call.  This allows the same
 *		method to be called from set_pc_delay_1usec() to find the correct
 *		'idelay' value and then to be called from other methods to
 *		generate the delay.
 */
/*--------------------------------------------------------------------------*/

u_int
Send_reg::pc_delay_low(
	register u_int	idelay )			// Delay count.
{
	while ( idelay > 0 && !(inportb( PB ) & PB_MSK_DCCOUTD) )
	{
		idelay--;
	}
	return ( idelay );
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


/*****************************************************************************
 * $History: SEND_REG.CPP $
 * 
 * *****************  Version 28  *****************
 * User: Kenw         Date: 9/12/23    Time: 6:44p
 * Updated in $/NMRA/SRC/SEND
 * Replaced LOG_PRINT() with DBGLOG() for all log messages
 * that occur when the program is set to log the packet data only
 * 
 * *****************  Version 27  *****************
 * User: Kenw         Date: 8/21/23    Time: 8:02p
 * Updated in $/NMRA/SRC/SEND
 * Changed Byte messages to begin with "!B" rather than "!>".
 * 
 * *****************  Version 26  *****************
 * User: Kenw         Date: 6/24/23    Time: 9:03p
 * Updated in $/NMRA/SRC/SEND
 * Change kick_start() to turn functions on rather than off.
 * 
 * *****************  Version 25  *****************
 * User: Kenw         Date: 6/23/23    Time: 7:22p
 * Updated in $/NMRA/SRC/SEND
 * Changed kick_start() to turn off rather than on functions.
 * 
 * *****************  Version 24  *****************
 * User: Kenw         Date: 6/22/23    Time: 7:27p
 * Updated in $/NMRA/SRC/SEND
 * Added send_reg::kick_start() public method.
 * Verified is worked with a manual 'k' command.
 * 
 * *****************  Version 23  *****************
 * User: Kenw         Date: 5/29/23    Time: 5:56p
 * Updated in $/NMRA/SRC/SEND
 * Added note to set_pc_delay() header to indicate
 * setclock() is called to set nominal clock values
 * when this method is called.
 * 
 * *****************  Version 22  *****************
 * User: Kenw         Date: 7/24/19    Time: 10:15p
 * Updated in $/NMRA/SRC/SEND
 * Removed spurious sccsid warning.
 * 
 * *****************  Version 19  *****************
 * User: Kenw         Date: 5/02/14    Time: 3:53p
 * Updated in $/NMRA/SRC/SEND
 * Fixed bug in send_2_ambig_bits().
 * Step 8 must test DCC0 because DCCOUTD may go to 0 before
 * DCC0 reaches the half way point. The old routine loaded 0T2 & 0H2
 * and then immediately loaded the normal 0T & 0H values. This made
 * the second 0 bit a normal 0 rather than the second Ambig2 bit.
 * The old routine failed if the first Ambig 2 bit had a 0H1 vaue of
 * <=207us for a 0T1 time of 400us.
 * 
 * *****************  Version 18  *****************
 * User: Kenw         Date: 12/15/04   Time: 2:44p
 * Updated in $/NMRA/SRC/SEND
 * Forced early scope trigger for ambig 1 and ambig 2 tests.
 * Reduced stack size for messages to try and fix underflows
 * on slow machines.
 * Some cosmetic cleanup
 * 
 * *****************  Version 17  *****************
 * User: Kenw         Date: 12/15/04   Time: 12:30p
 * Updated in $/NMRA/SRC/SEND
 * Updated for Version 5.1 Experimental
 * 
 * *****************  Version 15  *****************
 * User: Kenw         Date: 11/01/04   Time: 7:15p
 * Updated in $/NMRA/SRC/SEND
 * Added very long (13000 usec) stretched 0 test.
 * 
 * *****************  Version 14  *****************
 * User: Kenw         Date: 5/15/04    Time: 8:32a
 * Updated in $/NMRA/SRC/SEND
 * Added 1 interior feedback bit test
 * 
 * *****************  Version 13  *****************
 * User: Kenw         Date: 1/11/04    Time: 12:30p
 * Updated in $/NMRA/SRC/SEND
 * Updated for Version 4.1 Beta.
 * Adds feedback test.
 * 
 * *****************  Version 12  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:57p
 * Updated in $/NMRA/SRC/SEND
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 10  *****************
 * User: Kenw         Date: 6/28/98    Time: 9:39a
 * Updated in $/NMRA/SRC/SEND
 * Changed to abort program at LOG_ERR. Rolling to X.2.12.
 * 
 * *****************  Version 9  *****************
 * User: Kenw         Date: 5/30/98    Time: 9:11p
 * Updated in $/NMRA/SRC/SEND
 * Added support for manual hard reset 'R' command.
 * Rolled to X.2.11
 * 
 * *****************  Version 8  *****************
 * User: Kenw         Date: 10/30/97   Time: 5:53p
 * Updated in $/NMRA/SRC/SEND
 * X.2.9 - Format clean ups only.
 * 
 * *****************  Version 7  *****************
 * User: Kenw         Date: 10/29/97   Time: 4:26p
 * Updated in $/NMRA/SRC/SEND
 * X.2.8 - Added packet log command.
 *
 * *****************  Version 6  *****************
 * User: Kenw         Date: 1/21/97    Time: 7:41p
 * Updated in $/NMRA/SRC/SEND
 * Increased Bits buffer size to 256.  Added error message if Bits objects
 * have
 * errors.
 *
 * *****************  Version 5  *****************
 * User: Kenw         Date: 7/26/96    Time: 5:46p
 * Updated in $/NMRA/SRC/SEND
 * Rolled to revision 2.3
 * Added support for 1 pulse duty cycle test.
 * Added Reset test to Address and Bad bit tests.
 * Did some text cleanup.
 * 
 * *****************  Version 4  *****************
 * User: Kenw         Date: 7/26/96    Time: 9:04a
 * Updated in $/NMRA/SRC/SEND
 * Changes for version 2.2
 * Added support for 'o' oscilloscope setup  command.
 * Renamed 1/2 fast and 1/2 slow to command station max/min
 * 
 * *****************  Version 3  *****************
 * User: Kenw         Date: 5/10/96    Time: 9:20a
 * Updated in $/NMRA/SRC/SEND
 * Cleaned up formatting and comments after review of code.
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 4/30/96    Time: 8:31p
 * Updated in $/NMRA/SRC/SEND
 * Changed to support sending of single stretched 0.
 *
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 3:58p
 * Created in $/NMRA/SRC/SEND
 * Methods for 'class Send_reg'.
 *
 *****************************************************************************/

