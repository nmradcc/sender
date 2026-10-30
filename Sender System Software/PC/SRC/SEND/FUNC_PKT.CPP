/*****************************************************************************
 *
 * DESCRIPTION:
 *
 *	func_pkt.cpp	-	Methods for Func_pkt class.
 *
 *****************************************************************************/

#include <func_pkt.h>
#include "send_reg.h"

/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		put_pkt()		   					-	Add one packet of the sequence.
 *
 *	RETURN VALUE
 *
 *		bits	-	Reference of the Bits object.
 *
 *	DESCRIPTION
 *
 *		put_pkt() appends packet 'index' of the sequence to 'bits'.
 *		With 'cfg.trig_rev' the sequence is a loco speed 0 packet in the
 *		'func_bits' direction, then (if 'cfg.loco_first') a function packet
 *		using 'cfg.func_mask'. Otherwise it is (if 'cfg.loco_first') a loco
 *		speed 0 packet in the !lamp_rear direction, then a function packet
 *		using 'func_bits'. 'cfg.lamp_rear' reverses the loco direction.
 *		The packet is complete (includes its end bit).
 */
/*--------------------------------------------------------------------------*/

Bits &
Func_pkt::put_pkt(
	Bits				&bits,				// Bits object to append to.
	const Func_pkt_cfg	&cfg,				// Packet build options.
	u_int				index,				// Packet number (0 based).
	u_short				address,			// Mobile decoder address.
	enum Func_Grps		func_grp,			// Function group number.
	BYTE				func_bits,			// Function bits.
	u_int				pre_bits )			// Preamble bits (default PRE_BITS)
{
	if ( cfg.trig_rev == true )
	{
		if ( index == 0 )
		{
			bits.put_cmd_pkt_28( address, (func_bits != 0) != cfg.lamp_rear,
								 0, pre_bits );
		}
		else
		{
			bits.put_func_grp_pkt( address, func_grp, cfg.func_mask, pre_bits );
		}
	}
	else if ( cfg.loco_first == true && index == 0 )
	{
		bits.put_cmd_pkt_28( address, !cfg.lamp_rear, 0, pre_bits );
	}
	else
	{
		bits.put_func_grp_pkt( address, func_grp, func_bits, pre_bits );
	}

	return ( bits );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		send()		   						-	Send the packet sequence.
 *
 *	RETURN VALUE
 *
 *		true	-	All packets sent.
 *		false	-	Problem sending a packet.
 *
 *	DESCRIPTION
 *
 *		send() builds each packet of the sequence in 'bits' and sends it
 *		with 'reg'. send_pkt() supplies the packet gap after each packet.
 */
/*--------------------------------------------------------------------------*/

bool
Func_pkt::send(
	Send_reg			&reg,				// Sender to transmit with.
	Bits				&bits,				// Scratch Bits object.
	const Func_pkt_cfg	&cfg,				// Packet build options.
	u_short				address,			// Mobile decoder address.
	enum Func_Grps		func_grp,			// Function group number.
	BYTE				func_bits,			// Function bits.
	const char			*info,				// Packet log info.
	u_int				pre_bits )			// Preamble bits (default PRE_BITS)
{
	const u_int		count = pkt_count( cfg );

	for ( u_int i = 0; i < count; i++ )
	{
		bits.clr_in();
		put_pkt( bits, cfg, i, address, func_grp, func_bits, pre_bits );
		bits.done();

		if ( reg.send_pkt( bits, info ) != OK )
		{
			return ( false );
		}
	}

	return ( true );
}
