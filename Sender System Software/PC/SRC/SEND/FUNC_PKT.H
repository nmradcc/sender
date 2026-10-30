/*****************************************************************************
 *
 * DESCRIPTION:
 *
 *	func_pkt.h	-	Function decoder packet builder.  Builds the loco and/or
 *					function group packets used to preset and trigger
 *					function decoders, using the Bits class.
 *
 *****************************************************************************/

#ifndef FUNC_PKT_H_DECLARED
#define FUNC_PKT_H_DECLARED

#include <bits.h>

class Send_reg;								// Defined in send_reg.h

/*
 *	Options that control how function decoder packets are built.
 */
struct Func_pkt_cfg
{
	bool	trig_rev	= false;				// Use reverse as trigger cmd.
	bool	loco_first	= false;				// Put loco pkt + func pkt.
	bool	lamp_rear	= false;				// Use rear lamp.
	BYTE	func_mask	= 0x00;					// Function group bit mask.
};

/*
 *	A function decoder sequence is one or two complete packets: a loco
 *	speed 0 packet and/or a function group packet.  Each one is sent as its
 *	own packet with the host side packet delay between them.
 */
class Func_pkt
{
  public:
	// Number of packets in the sequence (1 or 2).
	static u_int
	pkt_count( const Func_pkt_cfg &cfg )
	{
		return ( cfg.loco_first ? 2 : 1 );
	}

	// Append packet number 'index' (0 based) of the sequence to 'bits'.
	static Bits &
	put_pkt(
		Bits				&bits,
		const Func_pkt_cfg	&cfg,
		u_int				index,
		u_short				address,
		enum Func_Grps		func_grp,
		BYTE				func_bits,
		u_int				pre_bits = PRE_BITS );

	// Append only the last packet of the sequence to 'bits'.
	static Bits &
	put_final_pkt(
		Bits				&bits,
		const Func_pkt_cfg	&cfg,
		u_short				address,
		enum Func_Grps		func_grp,
		BYTE				func_bits,
		u_int				pre_bits = PRE_BITS )
	{
		return ( put_pkt( bits, cfg, pkt_count( cfg ) - 1, address,
						  func_grp, func_bits, pre_bits ) );
	}

	// Build each packet in 'bits' (used as scratch) and send it, waiting
	// the packet delay between packets.
	static bool
	send(
		Send_reg			&reg,
		Bits				&bits,
		const Func_pkt_cfg	&cfg,
		u_short				address,
		enum Func_Grps		func_grp,
		BYTE				func_bits,
		const char			*info,
		u_int				pre_bits = PRE_BITS );
};

#endif /* FUNC_PKT_H_DECLARED */
