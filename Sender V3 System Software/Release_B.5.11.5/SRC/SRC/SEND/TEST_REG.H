/*****************************************************************************
 *
 * File:                 $Workfile: TEST_REG.H $
 * Path:                 $Logfile: /NMRA/SRC/SEND/TEST_REG.H $
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
 *	test_reg.h	-	This file defines the Test_reg class which stores
 *					and compares self test vectors.
 *
 *****************************************************************************/

#ifndef TEST_REG_H_DECLARED
#define TEST_REG_H_DECLARED	"@(#) $Workfile: TEST_REG.H $$ $Revision: 2 $$"

#include "sr_core.h"

struct Test_reg_mask
{
	bool				pa_skip;			// 8255 port A bit skip flag.
	BYTE				pb_mask;			// 8255 port B bit skip mask.
	BYTE				pc_mask;			// 8255 port C bit skip mask.
	BYTE				s0t_mask;			// 8254 cnt 0T status skip mask.
	BYTE				s0h_mask;			// 8254 cnt 0H status skip mask.
	BYTE				s1t_mask;			// 8254 cnt 1T status skip mask.
	bool				cnt0t_skip;			// 8254 cnt 0T skip flag.
	bool				cnt0h_skip;			// 8254 cnt 0H skip flag.
	bool				cnt1t_skip;			// 8254 cnt 1T skip flag.
	BYTE				gen_mask;			// Generic input skip mask.
};

struct Test_reg_vm
{
	char			*msg;					// Vector ID message.
	Sr_core_vals	v;						// Register values.
	Test_reg_mask	m;						// Mask values.
};

class Test_reg : public Sr_core
{
  public:
	/* Method section */
	Test_reg( void )
		: vects((Test_reg_vm *)0), vsize(0)
	{
		init();
	}

	Test_reg(
		const Test_reg_vm	*ivects,
		int					ivsize )
	: vects(ivects), vsize(ivsize)
	{
		init();
	}

	/* Simple get methods */
	BYTE get_gen_mask( void ) const { return ( m.gen_mask ); }
	bool get_pa_skip( void ) const { return ( m.pa_skip ); }
	BYTE get_pb_mask( void ) const { return ( m.pb_mask ); }
	BYTE get_pc_mask( void ) const { return ( m.pc_mask ); }
	BYTE get_s0t_mask( void ) const { return ( m.s0t_mask ); }
	BYTE get_s0h_mask( void ) const { return ( m.s0h_mask ); }
	BYTE get_s1t_mask( void ) const { return ( m.s1t_mask ); }
	bool get_cnt0t_skip( void ) const { return ( m.cnt0t_skip ); }
	bool get_cnt0h_skip( void ) const { return ( m.cnt0h_skip ); }
	bool get_cnt1t_skip( void ) const { return ( m.cnt1t_skip ); }
	const Test_reg_mask *get_m_ptr( void ) const { return ( &m ); }
	const char *get_msg( void ) const { return ( msg ); }
	const Test_reg_vm *get_vects( void ) const { return ( vects ); }
	int get_vsize( void ) const { return ( vsize ); }

	/* Simple set methods */
	void set_gen_mask( BYTE igen_mask ) { m.gen_mask = igen_mask; }
	void set_pa_skip( bool ipa_skip ) { m.pa_skip = ipa_skip; }
	void set_pb_mask( BYTE ipb_mask ) { m.pb_mask = ipb_mask; }
	void set_pc_mask( BYTE ipc_mask ) { m.pc_mask = ipc_mask; }
	void set_s0t_mask( BYTE is0t_mask ) { m.s0t_mask = is0t_mask; }
	void set_s0h_mask( BYTE is0h_mask ) { m.s0h_mask = is0h_mask; }
	void set_s1t_mask( BYTE is1t_mask ) { m.s1t_mask = is1t_mask; }
	void set_cnt0t_skip( bool icnt0t_skip ) { m.cnt0t_skip = icnt0t_skip; }
	void set_cnt0h_skip( bool icnt0h_skip ) { m.cnt0h_skip = icnt0h_skip; }
	void set_cnt1t_skip( bool icnt1t_skip ) { m.cnt1t_skip = icnt1t_skip; }
	void set_m( const Test_reg_mask &im ) { m = im; }
	void set_msg( const char *imsg) { msg = imsg; }
	void set_vm( const Test_reg_vm &ivm )
	{
		v	=	ivm.v;
		m	=	ivm.m;
		msg	=	ivm.msg;
	}
	Rslt_t set_vm( int v_index );
	void set_vects(
		const Test_reg_vm	*ivects,
		int					ivsize )
	{
		vects	=	ivects;
		vsize	=	ivsize;
	}

	void print( bool pr_hdr_flag = false ) const;

	Rslt_t test( const Sr_core &ireg, int v_index ) const;
	Rslt_t test( const Sr_core &ireg ) const
	{
		return ( test( ireg, v, m, msg ) );
	}

	static Rslt_t
	test( const Sr_core &ireg, const Test_reg_vm &vect )
	{
		return ( test( ireg, vect.v, vect.m, vect.msg ) );
	}

	static Rslt_t
	test(
		const Sr_core		&ireg,
		const Sr_core_vals	&iv,
		const Test_reg_mask	&im,
		const char			*fmsg =  (const char *)0 );

	Rslt_t set_test( const Sr_core &ireg, const Test_reg_vm &vect )
	{
		set_vm( vect );
		return ( test( ireg ) );
	}

	Rslt_t set_test( const Sr_core &ireg, int v_index );

  protected:
	/* Data section */
	Test_reg_mask		m;					// Mask values.
	const char			*msg;				// Vector message.
	const Test_reg_vm	*vects;				// Ptr to test vector tbl.
	int					vsize;				// Size of vector tbl.

  /* Method section */

	void init();
};

#endif /* TEST_REG_H_DECLARED */


/*****************************************************************************
 * $History: TEST_REG.H $
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:57p
 * Updated in $/NMRA/SRC/SEND
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 3:55p
 * Created in $/NMRA/SRC/SEND
 * Header file for 'class Test_reg' that stores and compares self test
 * vectors.
 *
 *****************************************************************************/

