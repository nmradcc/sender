/*****************************************************************************
 *
 * File:                 $Workfile: BITS.CPP $
 * Path:                 $Logfile: /NMRA/LIB/BITS.CPP $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 15 $
 * Last Modified on:     $Modtime: 8/07/23 5:33p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 8/12/23 8:23p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	bits.cpp	-	Methods for Bits class.
 *
 *****************************************************************************/

#include <bits.h>
#include <zlog.h>

#include <stdio.h>


// Define DEBUG_LIB to print debug information to stdio.
//#define DEBUG_LIB

static const char sccsid[]      = "@(#) $Workfile: BITS.CPP $$ $Revision: 15 $$";
static const char sccsid_h[]    = BITS_H_DECLARED;

// Func to eliminate bogus sccsid and sccsid_h declared but never used warning.
inline void dummy(const char *, const char *) {}

/*
 *	Library version information.
 */
const BYTE	Bits::ver_major		= 2;		// Major library version.
const BYTE	Bits::ver_minor		= 1;		// Minor library version.
const BYTE	Bits::ver_build		= 1;		// Build library version.

/*
 *	Initialize the static member variables.
 */
bool		Bits::trig_rev		= false;	// Use reverse as trigger cmd.
bool		Bits::loco_first	= false;	// Put loco pkt + func pkt.
bool		Bits::lamp_rear		= false;	// Use rear lamp.
BYTE		Bits::func_mask		= 0x00;		// Function group bit mask.

/*
 *	Table of initial shift positions
 */
const BYTE Bits::bit1_1st[BITS_IN_BYTE] =
	{ 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };
const BYTE Bits::bit1_2nd[BITS_IN_BYTE] =
	{ 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };
const BYTE Bits::bit0_1st[BITS_IN_BYTE] =
	{ 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00 };
const BYTE Bits::bit0_2nd[BITS_IN_BYTE] =
	{ 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };

const BYTE Bits::bit_flips[BITS_IN_BYTE] =
	{ 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

const BYTE	LAMP_BIT		= 0x10;			// Lamp bit mask.
const BYTE	IGNORE_BIT		= 0x10;			// Ignore direction bit.
const BYTE	LSB_28_BIT		= 0x10;			// 28 step LSB bit.
const BYTE	FORWARD_BIT		= 0x20;			// Forward bit mask.
const BYTE	CMD_BITS		= 0x40;			// Command bits.
const BYTE	SDCC_E_STOP		= 0x01;			// Emergency stop speed.
const BYTE	SDCC_STOP		= 0x00;			// Normal stop.


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		Bits()							-	 Constructor.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		Bits() constructs a new Bits object. it gets space for a
 *		isize size BYTE arrary and then calls reset() to initialize
 *		things.
 */
/*--------------------------------------------------------------------------*/

Bits::Bits(
	u_int			isize )					// Size of bytes array.
{
	if ( (bytes = new BYTE[ isize ]) == (BYTE *)0 )
	{
		SET_ERROR( CONSTRUCTOR_OBJ_ERR );
	}

	last_byte	=	&bytes[ isize - 1];

	clr_in();

    // Eliminate bogus sccsid and sccsid_h declared but never used warning.
    dummy( sccsid, sccsid_h );
}



/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		~Bits()							-	Destructor.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		~Bits() is the destructor for Bits.
 */
/*--------------------------------------------------------------------------*/

Bits::~Bits()
{
	delete [] bytes;
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		print()							-	Print Bytes to stdout.
 *
 *	RETURN VALUE
 *
 *		OK		-	Print successful.
 *		FAIL	-	Error printing.
 *
 *	DESCRIPTION
 *
 *		print() prints each Byte to stdout as a hex number.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Bits::print( void ) const
{
	BYTE			*pbyte;					  // Byte to print.

	if ( get_obj_errs() )
	{
		fprintf( stderr, "Bits object has errors 0x%08lx\n",
			get_obj_errs() );
		return ( FAIL );
	}

	if ( get_bit_size() == 0 )
	{
		printf( "<EMPTY>\n" );
		fflush( stdout );
		return ( OK );
	}

	for ( pbyte = bytes;; )
	{
		printf( "%02x", (u_int)*pbyte );

		if ( ++pbyte < in_byte )
		{
			putchar( ' ' );
		}
		else if ( pbyte == in_byte && in_bit < MAX_BIT_POS )
		{
			putchar( '.' );
		}
		else
		{
			break;
		}
	}

	putchar( '\n' );
	fflush( stdout );
	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		clr_in()							-	Clear out bit array.
 *
 *	RETURN VALUE
 *
 *		this	-	Reference of calling object.
 *
 *	DESCRIPTION
 *
 *		clr_in() clears a Bits object to its initial empty state.
 */
/*--------------------------------------------------------------------------*/

Bits &
Bits::clr_in()
{
	*bytes		=	0x00;
	in_byte		=	bytes;
	in_bit		=	MAX_BIT_POS;
	out_byte	=	bytes;
	check_byte	=	CHECK_INIT;
	clr_flip();
	CLEAR_WARN( WARN_ERR );

	return ( *this );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		get_byte()							-	Get next Byte for output.
 *
 *	RETURN VALUE
 *
 *		OK		-	Next byte obtained.
 *		FAIL	-	Next byte not available.
 *
 *	DESCRIPTION
 *
 *		get_byte() get the next available output Byte from a Bits object.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Bits::get_byte(
	BYTE			&obyte )				// Place to store Byte.
{
	if ( out_byte >= in_byte )
	{
		return ( FAIL );
	}

	obyte	=	*out_byte++;

	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		put_cmd_14()			 	  -	Add 14 step baseline cmd to array.
 *
 *	RETURN VALUE
 *
 *		this	-	Reference of calling object.
 *
 *	DESCRIPTION
 *
 *		put_cmd() adds a 14 step baseline command to the bit array.
 */
/*--------------------------------------------------------------------------*/

Bits &
Bits::put_cmd_14(
	bool			forward,				// If true, go forward.
	bool			lamp,					// If true, turn on lamp.
	int				speed )					// Speed value.
{
	BYTE			cmd = 0;				// Command BYTE.

	if ( speed ==  SP_E_STOP )
	{
		cmd	=	SDCC_E_STOP;
	}
	else if ( speed == SP_MIN )
	{
		cmd	=	SDCC_STOP;
	}
	else if ( speed < SP_MIN || speed > SP_14_MAX  )
	{
		SET_ERROR( RANGE_OBJ_WARN );
		return ( *this );
	}
	else
	{
		cmd	=	speed +  SDCC_E_STOP;
	}

	if ( forward )
	{
		cmd	|=	FORWARD_BIT;
	}

	if ( lamp )
	{
		cmd	|=	LAMP_BIT;
	}

	cmd	|= CMD_BITS;

	put_byte( cmd );

	return ( *this );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		put_cmd_28()				-	Add 28 step baseline cmd to array.
 *
 *	RETURN VALUE
 *
 *		this	-	Reference of calling object.
 *
 *	DESCRIPTION
 *
 *		put_cmd() adds a 28 step baseline command to the bit array.
 */
/*--------------------------------------------------------------------------*/

Bits &
Bits::put_cmd_28(
	bool			forward,				// If true, go forward.
	int				speed )					// Speed value.
{
	BYTE			cmd = 0;				// Command BYTE.

	if ( speed ==  SP_E_STOP )
	{
		cmd	=	SDCC_E_STOP;
	}
	else if ( speed ==  SP_E_STOP_I )
	{
		cmd	=	SDCC_E_STOP | IGNORE_BIT;
	}
	else if ( speed == SP_MIN )
	{
		cmd	=	SDCC_STOP;
	}
	else if ( speed == SP_STOP_I )
	{
		cmd	=	SDCC_STOP | IGNORE_BIT;
	}
	else if ( speed < SP_MIN || speed > SP_28_MAX  )
	{
		SET_ERROR( RANGE_OBJ_WARN );
		return ( *this );
	}
	else
	{
        cmd		=	(speed + 3) >> 1;

        if ( !(speed & 0x01) )
        {
        	cmd	|=	LSB_28_BIT;
        }
	}

	if ( forward )
	{
		cmd	|=	FORWARD_BIT;
	}

	cmd	|= CMD_BITS;

	put_byte( cmd );

#ifdef DEBUG_LIB
    printf( ">> put_cmd_28: cmd %02x.\n", cmd );
#endif // DEBUG_LIB

	return ( *this );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		put_byte()							-	Add Byte to bit array.
 *
 *	RETURN VALUE
 *
 *		this	-	Reference of calling object.
 *
 *	DESCRIPTION
 *
 *		put_byte() adds 'ibyte' to the bit array.
 */
/*--------------------------------------------------------------------------*/

Bits &
Bits::put_byte(
	BYTE			ibyte )					// Byte to add.
{
	BYTE			tbyte;					// Temporary Byte.

	if ( in_byte > last_byte )
	{
		SET_ERROR( RANGE_OBJ_WARN );
		return ( *this );
	}

	if ( in_bit	==	MAX_BIT_POS )			// At Byte boundary.
	{
		*in_byte++	=	ibyte;
	}
	else									// Between Byte boundarys.
	{
		*in_byte	&=	bit0_1st[ in_bit ];	// Clear out old bits.
		tbyte		=	ibyte >> (MAX_BIT_POS - in_bit);
		*in_byte++	|=	tbyte;				// Load new bits.

		if ( in_byte > last_byte )
		{
			SET_ERROR( RANGE_OBJ_WARN );
			return ( *this );
		}
		else
		{
			*in_byte	=	ibyte << (in_bit + 1);
		}
	}

	check_byte	^=	ibyte;

	return ( *this );
}

/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		put_1s()							-	Add 1s to Byte array.
 *
 *	RETURN VALUE
 *
 *		this	-	Reference of calling object.
 *
 *	DESCRIPTION
 *
 *		put_1s() adds 'count' 1s to the bit array.
 */
/*--------------------------------------------------------------------------*/

Bits &
Bits::put_1s(
	u_int			count )					// Count of 1 bits to add.
{
	/*
	 *	Fill in any bits in present Byte.
	 */
	*in_byte	|=	bit1_1st[ in_bit ];

	if ( count < (in_bit + 1) )				// We're done.
	{
		in_bit	-=	count;
		return ( *this );
	}
	else									// More bits to do.
	{
		count	-=	in_bit + 1;
		in_bit	=	MAX_BIT_POS;
		++in_byte;
	}

	/*
	 *	Handle full BYTES in middle of run.
	 */
	for ( ; count >= BITS_IN_BYTE; count -= BITS_IN_BYTE )
	{
		if ( in_byte > last_byte )
		{
			SET_ERROR( RANGE_OBJ_WARN );
			return ( *this );
		}
		else
		{
			*in_byte++	=	0xff;
		}
	}

	if ( count > 0 )
	{
		/*
	 	 *	Fill in any bits in last byte.
	 	 */
		if ( in_byte > last_byte )
		{
			SET_ERROR( RANGE_OBJ_WARN );
			return ( *this );
		}

		*in_byte	=	bit1_2nd[ in_bit ];
		in_bit		=	MAX_BIT_POS - count;
	}

	return ( *this );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		put_0s()							-	Add 0s to Byte array.
 *
 *	RETURN VALUE
 *
 *		this	-	Reference of calling object.
 *
 *	DESCRIPTION
 *
 *		put_0s() adds 'count' 0s to the bit array.
 */
/*--------------------------------------------------------------------------*/

Bits &
Bits::put_0s(
	u_int			count )					// Count of 0 bits to add.
{
	/*
	 *	Clear any bits in present Byte.
	 */
	*in_byte	&=	bit0_1st[ in_bit ];

	if ( count < (in_bit + 1) )				// We're done.
	{
		in_bit	-=	count;
		return ( *this );
	}
	else									// More bits to do.
	{
		count	-=	in_bit + 1;
		in_bit	=	MAX_BIT_POS;
		++in_byte;
	}

	/*
	 *	Handle full BYTES in middle of run.
	 */
	for ( ; count >= BITS_IN_BYTE; count -= BITS_IN_BYTE )
	{
		if ( in_byte > last_byte )
		{
			SET_ERROR( RANGE_OBJ_WARN );
			return ( *this );
		}
		else
		{
			*in_byte++	=	0x00;
		}
	}


	if ( count > 0 )
	{
		/*
		 *	Clear any bits in last byte.
		 */
		if ( in_byte > last_byte )
		{
			SET_ERROR( RANGE_OBJ_WARN );
			return ( *this );
		}

		*in_byte	=	0x00;
		in_bit		=	MAX_BIT_POS - count;
	}

	return ( *this );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		put_fsoc_pkt()						-	Add fail safe packet sequence.
 *
 *	RETURN VALUE
 *
 *		this	-	Reference of calling object.
 *
 *	DESCRIPTION
 *
 *		put_fsoc() adds the fail safe operation sequence to the bit array.
 *		It does NOT add the trailing 1 which may be part of the next command.
 */
/*--------------------------------------------------------------------------*/

Bits &
Bits::put_fsoc( void )
{
	put_reset_pkt( 20 ).put_idle_pkt( 10 );
	return ( *this );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		put_reset_pkt()						-	Add reset packets.
 *
 *	RETURN VALUE
 *
 *		this	-	Reference of calling object.
 *
 *	DESCRIPTION
 *
 *		put_reset_pkt() adds 'packets' number of reset packets to the bit
 *		array.
 *		It does NOT add the trailing 1 which may be part of the next command.
 */
/*--------------------------------------------------------------------------*/

Bits &
Bits::put_reset_pkt(
	u_int			packets,				// Count of reset packets to add.
	u_int			pre_bits )				// Preamble bits (default PRE_BITS)
{
	while ( packets-- > 0 )
	{
		put_1s( pre_bits ).put_0s( BASE_BITS - PRE_BITS );
	}

	return ( *this );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		put_idle_pkt()						-	Add idle packets.
 *
 *	RETURN VALUE
 *
 *		this	-	Reference of calling object.
 *
 *	DESCRIPTION
 *
 *		put_idle_pkt() adds 'packets' number of idle packets to the bit
 *		array.
 *		It does NOT add the trailing 1 which may be part of the next command.
 */
/*--------------------------------------------------------------------------*/

Bits &
Bits::put_idle_pkt(
	u_int			packets,				// Count of idle packets to add.
	u_int			pre_bits )				// Preamble bits (default PRE_BITS)
{
	while ( packets-- > 0 )
	{
		put_1s( pre_bits ).put_0s( 1 ).put_byte( 0xff ).put_0s( 10 );
		put_byte( 0xff );
	}

	return ( *this );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		put_acc_pkt()						-	Add accessory packet.
 *
 *	RETURN VALUE
 *
 *		this	-	Reference of calling object.
 *
 *	DESCRIPTION
 *
 *		put_acc_pkt() adds an accessory command packet to the bit
 *		array.
 *		It does NOT add the trailing 1 which may be part of the next command.
 */
/*--------------------------------------------------------------------------*/

Bits &
Bits::put_acc_pkt(
	u_short			address,				// Accessory decoder address.
	bool			active,					// true to activate output.
	BYTE			out_id,					// Device output (0-7).
	u_int			pre_bits )				// Preamble bits (default PRE_BITS)
{
	BYTE			byte1;					// First command byte.
	BYTE			byte2;					// Second command byte.

	// First byte - 10 <address 5-0>
	byte1	=	0x80 | (BYTE)( 0x3f & address );
	// Second byte - <1> ~<address 8-6> <dev_state> <out_id>
	byte2	=   	(BYTE) 0x80
				|	(BYTE)( 0x70 & ~( address >> 2 ) )
				|	(BYTE)(active ? 0x8 : 0x00 )
				|	(BYTE)( 0x07 & out_id );

	clr_check();
	put_1s( pre_bits ).put_0s( 1 ).put_byte( byte1 ).put_0s( 1 );
	put_byte( byte2 ).put_0s( 1 ).put_check();

	return ( *this );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		put_sig_pkt()						-	Add signal packet.
 *
 *	RETURN VALUE
 *
 *		this	-	Reference of calling object.
 *
 *	DESCRIPTION
 *
 *		put_sig_pkt() adds an Extended Accessory (signal) packet to the bit
 *		array.
 *		It does NOT add the trailing 1 which may be part of the next command.
 */
/*--------------------------------------------------------------------------*/

Bits &
Bits::put_sig_pkt(
	u_short			address,				// Signal decoder address.
	BYTE			aspect,					// 5 bit aspect number.
	u_int			pre_bits )				// Preamble bits (default PRE_BITS)
{
	BYTE			byte1;					// First command byte.
	BYTE			byte2;					// Second command byte.
    BYTE			byte3;					// Third command byte.

    if ( address < SIG_ADDR_MIN || address > SIG_ADDR_MAX )
    {
		SET_ERROR( RANGE_OBJ_WARN );
    	address = SIG_ADDR_MIN;
    }

    // Add 3 to address so min address of 1 sets A2 to 1 to match
    // Basic Address requirement.
    address += 3;

	// First byte - <10> <address 7-2 starting with 1>
	byte1	=	0x80 | (BYTE)( 0x3f & ( address >> 2 ) );
	// Second byte - <0> ~<address 10-8> <0> <adress 1-0> <1>
	byte2	=   	(BYTE)  0x01
				|	(BYTE)( 0x70 & ~( address >> 4 ) )
				|	(BYTE)( 0x06 &  ( address << 1 ) );
	// Third byte - 5 bit aspect number
	byte3	=	aspect & 0x1f;

	clr_check();
	put_1s( pre_bits ).put_0s( 1 ).put_byte( byte1 ).put_0s( 1 );
	put_byte( byte2  ).put_0s( 1 ).put_byte( byte3 ).put_0s( 1 ).put_check();

#ifdef DEBUG_LIB
    printf( ">> put_sig_pkt: Bytes %02x, %02x, %02x, %02x.\n",
    						byte1, byte2, byte3, byte1 ^ byte2 ^ byte3 );
#endif // DEBUG_LIB

	return ( *this );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		put_func_grp_pkt()		   			-	Add function group packet.
 *
 *	RETURN VALUE
 *
 *		this	-	Reference of calling object.
 *
 *	DESCRIPTION
 *
 *		put_func_grp_pkt() adds a function group command packet to the bit
 *		array.
 *		It does NOT add the trailing 1 which may be part of the next command.
 */
/*--------------------------------------------------------------------------*/
Bits &
Bits::put_func_grp_pkt(
	u_short			address,				// Mobile decoder address.
    enum Func_Grps	func_grp,				// Function group number.
    BYTE			func_bits,				// Function bits.
    u_int			pre_bits )				// Preamble bits (default PRE_BITS)
{
	BYTE			byte1;					// First command byte.

    // Command byte <cmd> <func_bits>
    if ( func_grp == GRP_1 )
    {
    	byte1	=		(BYTE) 0x80
        			|	(BYTE) ( 0x1f & func_bits );
    }
    else if ( func_grp == GRP_2 )
    {
    	byte1	=		(BYTE) 0xA0
        			|	(BYTE) ( 0x1f & func_bits );
    }
    else
    {
    	SET_ERROR( RANGE_OBJ_WARN );
		return ( *this );
    }

	clr_check();
    put_1s( pre_bits ).put_0s( 1 ).put_byte( address ).put_0s( 1 );
	put_byte( byte1 ).put_0s( 1 ).put_check();

	return ( *this );
}

// ..\See NMRA\put_........_pktx Info.txt

/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		put_func_grp_pktx()		   			-	Add a loco or function pkt.
 *
 *	RETURN VALUE
 *
 *		this	-	Reference of calling object.
 *
 *	DESCRIPTION
 *
 *		put_func_grp_pktx() adds a loco speed 0 packet to the bit
 *		array if class variable 'trig_rev' is true or adds a
 *      function group packet otherwise. Clss variable 'lamp_rear'
 *		reverses the direction of the speed command if true.
 *		It does NOT add the trailing 1 which may be part of the next command.
 */
/*--------------------------------------------------------------------------*/
Bits &
Bits::put_func_grp_pktx(
	u_short			address,				// Mobile decoder address.
    enum Func_Grps	func_grp,				// Function group number.
    BYTE			func_bits,				// Function bits.
    u_int			pre_bits )				// Preamble bits (default PRE_BITS)
{
	if ( trig_rev == true )
    {
		// First put a loco 0 speed packet in the func_bits direction.
		put_cmd_pkt_28( address, (func_bits != 0) != lamp_rear,
    					0, pre_bits );
		// Add a function packet with function bits on.
		if ( loco_first == true )
        {
			put_1s( 1 ).put_byte( 0x00 );	// Align next packet on Byte boundary.											
			put_idle_pkt( 1 );
			put_1s( 1 ).put_byte( 0x00 );	// Align next packet on Byte boundary.
			// Then add a function packet with funcs set by func_mask.
			put_func_grp_pkt( address, func_grp, func_mask, pre_bits );
        }
    }
    else
    {
		if ( loco_first == true )
		{
			// First put a loco 0 speed packet in the !lamp_rear direction.
			put_cmd_pkt_28( address, !lamp_rear,
							0, pre_bits );
			put_1s( 1 ).put_byte( 0x00 );	// Align next packet on Byte boundary.											
			put_idle_pkt( 1 );
			put_1s( 1 ).put_byte( 0x00 );	// Align next packet on Byte boundary.
		}
    	// Then add a function packet with funcs set by func_bits.
		put_func_grp_pkt( address, func_grp, func_bits, pre_bits );
    }

	return *this;
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		done()								-	Fill out last byte with 0s.
 *
 *	RETURN VALUE
 *
 *		this	-	Reference of calling object.
 *
 *	DESCRIPTION
 *
 *		done() should be called when all bits are entered. It fills out
 *		the last byte with 0s.
 */
/*--------------------------------------------------------------------------*/

Bits &
Bits::done( void )
{
	if ( (in_byte > last_byte) || (in_bit == MAX_BIT_POS) )
	{
		return ( *this );
	}

	*in_byte	&=	bit0_1st[ in_bit ];
	in_bit		=	MAX_BIT_POS;
	++in_byte;

	return ( *this );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		set_flip()							-	Set new flipped bit.
 *
 *	RETURN VALUE
 *
 *		OK		-	New position set without error.
 *		FAIL	-	Problem setting new position.
 *
 *	DESCRIPTION
 *
 *		set_flip() moves the flipped bit position to 'bit_pos'.
 *		Calling set_flip() with no parameters sets 'bit_pos' to 0.
 *		In operation, set_flip() first sets the present flipped bit, if
 *		any, normal, and then flips the bit at the new location.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Bits::set_flip(
	u_int		bit_pos )					// New flipped bit pos. (optional).
{
	BYTE		*tmp_flip_byte;				// Temporary new flip_byte.
	u_int		tmp_flip_bit;				// Temparary new flip_bit.

	tmp_flip_byte	=	bytes + (bit_pos / BITS_IN_BYTE);
	tmp_flip_bit	=	MAX_BIT_POS - (bit_pos % BITS_IN_BYTE);

	if (	(tmp_flip_byte > in_byte)
		||	( (tmp_flip_byte == in_byte) && (tmp_flip_bit <= in_bit) ) )
	{
		return ( FAIL );
	}

	if ( flip_byte )					// Reset present flipped bit.
	{
		*flip_byte	^=	bit_flips[ flip_bit ];
	}

	/*
	 *	Flip new bit.
	 */
	*tmp_flip_byte	^=	bit_flips[ tmp_flip_bit ];

	flip_byte	=	tmp_flip_byte;
	flip_bit	=	tmp_flip_bit;

	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		clr_flip()							-	Clear flipped bit.
 *
 *	RETURN VALUE
 *
 *		NONE.
 *
 *	DESCRIPTION
 *
 *		clr_flip() clears the flipped bit, if any.
 */
/*--------------------------------------------------------------------------*/

void
Bits::clr_flip( void )
{
	if ( flip_byte )					// Reset present flipped bit.
	{
		*flip_byte	^=	bit_flips[ flip_bit ];
	}

	flip_byte	=	(BYTE *)0;
	flip_bit	=	MAX_BIT_POS;
}



/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		clr_bit()							-	Clear specified bit.
 *
 *	RETURN VALUE
 *
 *		OK		-	Bit cleared without error.
 *		FAIL	-	Problem clearing bit.
 *
 *	DESCRIPTION
 *
 *		clr_bit() clears the bit at 'bit_pos' to 0.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Bits::clr_bit(
	u_int		bit_pos )					// Bit position to blear.
{
	BYTE		*tmp_clr_byte;				// Temporary Byte to clear.
	u_int		tmp_clr_bit;				// Temparary bit to clear.

	tmp_clr_byte	=	bytes + (bit_pos / BITS_IN_BYTE);
	tmp_clr_bit		=	MAX_BIT_POS - (bit_pos % BITS_IN_BYTE);

    // Check to make sure bit is in range.
	if (	(tmp_clr_byte > in_byte)
		||	( (tmp_clr_byte == in_byte) && (tmp_clr_bit <= in_bit) ) )
	{
		return ( FAIL );
	}

	/*
	 *	Clear selected bit.
	 */
	*tmp_clr_byte	&=	~bit_flips[ tmp_clr_bit ];

	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		truncate()							-	Truncate byte array.
 *
 *	RETURN VALUE
 *
 *		OK		-	New position set without error.
 *		FAIL	-	'bit_size' is out of range.
 *
 *	DESCRIPTION
 *
 *		truncate() shortens the byte array to 'bit_size' bits.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Bits::truncate(
	u_int		bit_size )					// New array size in bits.
{
	if ( bit_size > get_bit_size() )
	{
		return ( FAIL );
	}

	in_byte	=	bytes + (bit_size / BITS_IN_BYTE);
	in_bit	=	MAX_BIT_POS - (bit_size % BITS_IN_BYTE);

	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		Fsoc_bits()							-	 Constructor.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		Fsoc_bits() constructs a new Fsoc_bits object. it gets space for a
 *		BYTE arrary large enough to hold the fail safe packet sequence
 *		and then fills it with the fail safe packet sequence.
 */
/*--------------------------------------------------------------------------*/

Fsoc_bits::Fsoc_bits( void )
: Bits( bits_to_bytes( (30 * BASE_BITS) + 1 ) )
{
	put_fsoc().put_1s( 1 ).done();

	if ( get_obj_errs() )
	{
		SET_ERROR( CONSTRUCTOR_OBJ_ERR );
	}
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		Idle_bits()							-	 Put idle packet bits.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		Idle_bits() constructs a new Idle_bits object. it gets space for a
 *		BYTE arrary large enough to hold 'packets' idle packets
 *		and then fills it with the idle packets.
 */
/*--------------------------------------------------------------------------*/

Idle_bits::Idle_bits(
	u_int			packets )				// Count of idle packets.
: Bits( bits_to_bytes( (packets * BASE_BITS) + 1 ) )
{
	put_idle_pkt( packets ).put_1s( 1 ).done();

	if ( get_obj_errs() )
	{
		SET_ERROR( CONSTRUCTOR_OBJ_ERR );
	}
}


/*****************************************************************************
 * $History: BITS.CPP $
 * 
 * *****************  Version 15  *****************
 * User: Kenw         Date: 8/12/23    Time: 8:23p
 * Updated in $/NMRA/LIB
 * Removed log messages from put_func_grp_pktx().
 * 
 * *****************  Version 14  *****************
 * User: Kenw         Date: 8/06/23    Time: 4:10p
 * Updated in $/NMRA/LIB
 * Added loco_first and func_mask variable.
 * Updated put_func_grp_pktx() to use them.
 * 
 * *****************  Version 13  *****************
 * User: Kenw         Date: 7/02/23    Time: 8:08p
 * Updated in $/NMRA/LIB
 * Added put_func_grp_pktx().
 * Changed 'lamp_rear' and 'trig_rev' to static variables.
 * Changed get_lamp_rear() and get_trig_rev() to static methods.
 * Added static void set_static_variables() to set
 *     'lamp_rear' and 'trig_rev'.
 * 
 * .
 * 
 * *****************  Version 12  *****************
 * User: Kenw         Date: 6/24/23    Time: 10:38p
 * Updated in $/NMRA/LIB
 * Added lamp_rear.
 * 
 * *****************  Version 11  *****************
 * User: Kenw         Date: 9/19/17    Time: 1:01p
 * Updated in $/NMRA/LIB
 * Fices bug in put_cmd_28() where odd speeds were encoded incorrectly.
 * 
 * *****************  Version 10  *****************
 * User: Kenw         Date: 4/19/17    Time: 7:52p
 * Updated in $/NMRA/LIB
 * Moving to 2.0.0
 *   - Added Bits::put_sig_pkt()
 *   - Added unit tests for Bits::put_sig_pkt() and
 * Bits::put_func_grp_pkt()
 * 
 * *****************  Version 7  *****************
 * User: Kenw         Date: 11/17/13   Time: 7:02p
 * Updated in $/NMRA/LIB
 * Added put_func_grp_pkt() method send send Function commands.
 * 
 * *****************  Version 6  *****************
 * User: Kenw         Date: 7/01/04    Time: 7:31p
 * Updated in $/NMRA/LIB
 * Updated to include testing feedback bits in all error Byte bits
 * 
 * *****************  Version 5  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:56p
 * Updated in $/NMRA/LIB
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 4  *****************
 * User: Kenw         Date: 9/25/96    Time: 7:53p
 * Updated in $/NMRA/LIB
 * Added truncate() method
 * 
 * *****************  Version 3  *****************
 * User: Kenw         Date: 4/30/96    Time: 8:54p
 * Updated in $/NMRA/LIB
 * Changed Fail Safe packet to use 10 idles to reflect latest version
 * of RP 9.2.4
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 4/28/96    Time: 9:46a
 * Updated in $/NMRA/LIB
 * Fixed bug in put_reset_pkt().  It sent a proper RESET packet only if
 * 'pre_bits'
 * were set to the preamble minimum of 10 preamble bits.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 1:31p
 * Created in $/NMRA/LIB
 * Methods for 'class Bits'
 *
 *****************************************************************************/


