/*****************************************************************************
 *
 * File:                 $Workfile: BITS.H $
 * Path:                 $Logfile: /NMRA/INCLUDE/BITS.H $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 13 $
 * Last Modified on:     $Modtime: 8/06/23 3:00p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 8/06/23 4:11p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	bits.h	-	Bit array handling class.  This class stores and manipulates
 *				an array of bits.
 *
 *****************************************************************************/

#ifndef BITS_H_DECLARED
#define BITS_H_DECLARED	"@(#) $Workfile: BITS.H $$ $Revision: 13 $$"

#include <z_core.h>							// Define Z_core class.
#include <dcc.h>							// Fundamental DCC constants.

/*
 *	Possible decoder commands for Accesory command type 2 packets
 */
enum Acc2_states
{
	A2_RESET,								// Reset device.
	A2_INCR,								// Increment device 1 step.
	A2_DECR,								// Decrement device 1 step.
	A2_HOLD									// Hold present state.
};

enum Func_Grps
{
	GRP_1	=	0,							// Function Group 1.
    GRP_2	=	1							// Function Group 2.
};

inline u_int
bits_to_bytes(
	u_int			ibits )
{
	return ( ibits == 0 ? 0 : ((ibits - 1)/BITS_IN_BYTE) + 1);
}

class Bits : public Z_core
{
  public:
  	/* Constants section */
    static const BYTE	ver_major;					// Major library version.
    static const BYTE	ver_minor;					// Minor library version.
    static const BYTE	ver_build;					// Build library version.

	/*
	 *	Static method section.
	 */
	 
	// Get static variables at runtime.
	static bool
	get_trig_rev( void )
	{
		return ( trig_rev );
	}
	
	static bool
	get_loco_first( void )
	{
		return ( loco_first );
	}
	
	static bool
	get_lamp_rear( void )
	{
		return ( lamp_rear );
	}
	
	static BYTE
	get_func_mask( void )
	{
		return ( func_mask );
	}

	// Set the static variables at runtime.
	static void
	set_static_variables( bool itrg, bool iloco_first,
						  bool ilamp, BYTE ifuncs )
	{
    	trig_rev	=	itrg;
		loco_first	=	iloco_first;
        lamp_rear	=	ilamp;
		func_mask	=	ifuncs;
	}

	/*
	 *	Method section
	 */

	Bits(
		u_int	isize );

	~Bits();

	Rslt_t
	print( void ) const;

	u_int
	get_isize( void ) const
	{
		return ( (u_int)(last_byte - bytes)+1);
	}

	u_int
	get_bit_size( void ) const
	{
		return ( (u_int)	(	(in_byte - bytes) * BITS_IN_BYTE )
							+	MAX_BIT_POS - in_bit );
	}

	const BYTE *
	get_byte_array( void ) const
	{
		return ( bytes );
	}

	u_int
	get_byte_size( void ) const
	{
		return ( (u_int)(in_byte - bytes) );
	}

	Rslt_t
	get_byte(
		BYTE	&obyte );

	BYTE
	get_check() const
	{
		return ( check_byte );
	}
	
	void
	rst_out( void )
	{
		out_byte	=	bytes;
	}

	Bits &
	clr_in( void );

	Bits &
	put_byte(
		BYTE	ibyte );

	Bits &
	put_cmd_14(
		bool	forward,
		bool	lamp,
        int		speed );

	Bits &
	put_cmd_28(
		bool	forward,
        int		speed );

	Bits &
	put_check( void )
	{
		put_byte( check_byte );
		check_byte	=	CHECK_INIT;
		return ( *this );
	}

	Bits &
	clr_check( void )
	{
		check_byte	=	CHECK_INIT;
		return ( *this );
	}

	Bits &
	put_1s(
		u_int	count );

	Bits &
	put_0s(
		u_int	count );

	Bits &
	put_fsoc( void );

	Bits &
	put_reset_pkt(
		u_int	packets = 1,
		u_int	pre_bits = PRE_BITS );

	Bits &
	put_idle_pkt(
		u_int	packets = 1,
		u_int	pre_bits = PRE_BITS );

	Bits &
	put_cmd_pkt_14(
		BYTE	address,
		bool	forward,
		bool	lamp,
		int		speed,
		u_int	pre_bits = PRE_BITS )
	{
		clr_check();
		put_1s( pre_bits ).put_0s( 1 ).put_byte( address ).put_0s( 1 );
		put_cmd_14( forward, lamp, speed ).put_0s( 1 ).put_check();
		return ( *this );
	}

	Bits &
	put_cmd_pkt_28(
		BYTE	address,
		bool	forward,
		int		speed,
		u_int	pre_bits = PRE_BITS )
	{
		clr_check();
		put_1s( pre_bits ).put_0s( 1 ).put_byte( address ).put_0s( 1 );
		put_cmd_28( forward, speed ).put_0s( 1 ).put_check();
		return ( *this );
	}

	Bits &
	put_acc_pkt(
		u_short		address,
		bool		active,
		BYTE		out_id,
		u_int		pre_bits = PRE_BITS );

	Bits &
	put_sig_pkt(
		u_short		address,
		BYTE		aspect,
		u_int		pre_bits = PRE_BITS );

	Bits &
	put_func_grp_pkt(
		u_short			address,
		enum Func_Grps	func_grp,
		BYTE			func_bits,
		u_int			pre_bits = PRE_BITS );

// See ..\NMRAput_........_pktx Info.txt
	Bits &
	put_func_grp_pktx(
		u_short			address,
		enum Func_Grps	func_grp,
		BYTE			func_bits,
		u_int			pre_bits = PRE_BITS );

	Bits &
	done( void );

	Rslt_t
	set_flip(
		u_int	bit_pos = 0 );

	void
	clr_flip( void );

	Rslt_t
	clr_bit(
		u_int	bit_pos );

	Rslt_t
	truncate(
		u_int	bit_size );

  protected:
	/* Data section */
	static const BYTE	bit1_1st[BITS_IN_BYTE];		// 1s for first Byte.
	static const BYTE	bit1_2nd[BITS_IN_BYTE];		// 1s for second Byte.
	static const BYTE	bit0_1st[BITS_IN_BYTE];		// 0s for first Byte.
	static const BYTE	bit0_2nd[BITS_IN_BYTE];		// 0s for second Byte.
	static const BYTE	bit_flips[BITS_IN_BYTE];	// Flipped bit array.
	static bool			trig_rev;					// Use reverse as trigger cmd.
	static bool			loco_first;					// Put loco pkt + func pkt.
	static bool			lamp_rear;					// Use rear lamp.
	static BYTE			func_mask;					// Function group bit mask.
	BYTE				*bytes;						// Array of BYTES.
	BYTE				*last_byte;					// Ptr to last BYTE.
	BYTE				*in_byte;					// Next incoming byte.
	u_int				in_bit;						// Next bit position.
	BYTE				*out_byte;					// Next outgoing byte.
	BYTE				check_byte;					// Check byte.
	BYTE				*flip_byte;					// Byte with flipped bit.
	u_int				flip_bit;					// Bit to flip.
};

class Fsoc_bits : public Bits
{
  public:
	/* Method section */
	Fsoc_bits( void );
};

class Idle_bits : public Bits
{
  public:
	/* Method section */
	Idle_bits(
		u_int	packets = 1 );
};

#endif /* BITS_H_DECLARED */


/*****************************************************************************
 * $History: BITS.H $
 * 
 * *****************  Version 13  *****************
 * User: Kenw         Date: 8/06/23    Time: 4:11p
 * Updated in $/NMRA/INCLUDE
 * Added loco_first and func_mask variable.
 * Updated put_func_grp_pktx() to use them.
 * 
 * *****************  Version 12  *****************
 * User: Kenw         Date: 7/02/23    Time: 8:08p
 * Updated in $/NMRA/INCLUDE
 * Added put_func_grp_pktx().
 * Changed 'lamp_rear' and 'trig_rev' to static variables.
 * Changed get_lamp_rear() and get_trig_rev() to static methods.
 * Added static void set_static_variables() to set
 *     'lamp_rear' and 'trig_rev'.
 * 
 * *****************  Version 11  *****************
 * User: Kenw         Date: 6/24/23    Time: 10:36p
 * Updated in $/NMRA/INCLUDE
 * Added lamp_rear.
 * 
 * *****************  Version 10  *****************
 * User: Kenw         Date: 4/19/17    Time: 7:52p
 * Updated in $/NMRA/INCLUDE
 * Moving to 2.0.0
 *   - Added Bits::put_sig_pkt()
 *   - Added unit tests for Bits::put_sig_pkt() and
 * Bits::put_func_grp_pkt()
 * 
 * *****************  Version 7  *****************
 * User: Kenw         Date: 11/17/13   Time: 7:00p
 * Updated in $/NMRA/INCLUDE
 * Added put_func_grp_pkt() method send send Function commands.
 * 
 * *****************  Version 6  *****************
 * User: Kenw         Date: 7/01/04    Time: 7:31p
 * Updated in $/NMRA/INCLUDE
 * Updated to include testing feedback bits in all error Byte bits
 * 
 * *****************  Version 5  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:56p
 * Updated in $/NMRA/INCLUDE
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 4  *****************
 * User: Kenw         Date: 10/29/97   Time: 4:26p
 * Updated in $/NMRA/INCLUDE
 * X.2.8 - Added packet log command.
 * 
 * *****************  Version 3  *****************
 * User: Kenw         Date: 10/28/97   Time: 2:38p
 * Updated in $/NMRA/INCLUDE
 * Added Prior Packet Test & Idle manual command.
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 9/25/96    Time: 7:52p
 * Updated in $/NMRA/INCLUDE
 * Added truncate() command.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 12:52p
 * Created in $/NMRA/INCLUDE
 * Header file for 'class Bits'  bit array handling class
 *
 *****************************************************************************/

