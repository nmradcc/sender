/*****************************************************************************
 *
 * File:                 $Workfile: SR_CORE.H $
 * Path:                 $Logfile: /NMRA/SRC/SEND/SR_CORE.H $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 2 $
 * Last Modified on:     $Modtime: 8/27/00 1:04p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 8/31/00 7:57p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	sr_core	-	This file defines the Sr_core class which stores
 *				and prints out the status registers  It is a parent
 *				class for Send_reg and Test_reg.
 *
 *****************************************************************************/

#ifndef SR_CORE_H_DECLARED
#define SR_CORE_H_DECLARED	"@(#) $Workfile: SR_CORE.H $$ $Revision: 2 $$"

#include <z_core.h>

struct Sr_core_vals
{
	BYTE				pa;					// 8255 port A.
	BYTE				pb;					// 8255 port B.
	BYTE				pc;					// 8255 port C.
	BYTE				s0t;				// 8254 count 0T status.
	BYTE				s0h;				// 8254 count 0H status.
	BYTE				s1t;				// 8254 count 1T status.
	u_short				cnt0t;				// 8254 count 0T.
	u_short				cnt0h;				// 8254 count 0H.
	u_short				cnt1t;				// 8254 count 1T.
	BYTE				gen;				// Generic input port.
};

class Sr_core : public Z_core
{
  public:
	/* Method section */
	Sr_core( void ) { init(); }

	/* Simple get methods */
	BYTE get_gen( void ) const { return ( v.gen ); }
	BYTE get_pa( void ) const { return ( v.pa ); }
	BYTE get_pb( void ) const { return ( v.pb ); }
	BYTE get_pc( void ) const { return ( v.pc ); }
	BYTE get_s0t( void ) const { return ( v.s0t ); }
	BYTE get_s0h( void ) const { return ( v.s0h ); }
	BYTE get_s1t( void ) const { return ( v.s1t ); }
	u_short get_cnt0t( void ) const { return ( v.cnt0t ); }
	u_short get_cnt0h( void ) const { return ( v.cnt0h ); }
	u_short get_cnt1t( void ) const { return ( v.cnt1t ); }
	const Sr_core_vals	*get_v_ptr( void ) const { return ( &v ); }

	/* Simple set methods */
	void set_gen( BYTE igen_in ) { v.gen = igen_in; }
	void set_pa( BYTE ipa ) { v.pa = ipa; }
	void set_pb( BYTE ipb ) { v.pb = ipb; }
	void set_pc( BYTE ipc ) { v.pc = ipc; }
	void set_s0t( BYTE is0t ) { v.s0t = is0t; }
	void set_s0h( BYTE is0h ) { v.s0h = is0h;}
	void set_s1t( BYTE is1t ) { v.s1t = is1t; }
	void set_cnt0t( u_short icnt0t ) { v.cnt0t = icnt0t; }
	void set_cnt0h( u_short icnt0h ) { v.cnt0h = icnt0h; }
	void set_cnt1t( u_short icnt1t ) { v.cnt1t = icnt1t; }
	void set_v( const Sr_core_vals &iv ) { v =  iv; }

	void print_header( void ) const;
	void print( bool print_hdr = false ) const;

  protected:
	/* Data section */
	Sr_core_vals		v;					// Register values.

  /* Method section */
  void init( void );
  static void print_byte( BYTE val, BYTE skip = 0x00 );
  static void print_nibble( BYTE val, BYTE skip =  0x00 );
};

#endif /* SR_CORE_H_DECLARED */


/*****************************************************************************
 * $History: SR_CORE.H $
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:57p
 * Updated in $/NMRA/SRC/SEND
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 3:52p
 * Created in $/NMRA/SRC/SEND
 * Header file for 'class Sr_core' which is the parent class of Send_reg
 * and Test_reg.  It provides the common methods for both child classes.
 *
 *****************************************************************************/

