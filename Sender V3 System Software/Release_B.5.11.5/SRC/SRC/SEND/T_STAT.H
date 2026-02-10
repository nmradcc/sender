/*****************************************************************************
 *
 * File:                 $Workfile: T_STAT.H $
 * Path:                 $Logfile: /NMRA/SRC/SEND/T_STAT.H $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 2 $
 * Last Modified on:     $Modtime: 4/28/96 5:08p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 4/30/96 8:39p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	t_stat.h	-	This file defines extern interface to the
 *					test statistic object.
 *
 *****************************************************************************/

#ifndef T_STAT_H_DECLARED
#define T_STAT_H_DECLARED	"@(#) $Workfile: T_STAT.H $$ $Revision: 2 $$"

#include <ztypes.h>

class T_stat
{
  public:
	/*Method section */
	T_stat( void )
	{
		reset();
	}

	~T_stat() {}

	void
	reset( void )
	{
		t_cnt	=	0L;
		f_cnt	=	0L;
	}

	u_long get_t_cnt() const { return ( t_cnt ); }

	u_long get_f_cnt() const { return ( f_cnt ); }

	u_long get_p_cnt() const { return ( t_cnt - f_cnt ); }

	u_long
	get_percent() const
	{
		if ( t_cnt == 0L )
		{
			return ( 0 );
		}
		else
		{
			return ( (get_p_cnt()*100)/t_cnt );
		}
	}

	u_long incr_t_cnt() { return ( ++t_cnt ); }

	u_long incr_f_cnt() { return ( ++f_cnt ); }

  protected:
	/* Data section */
	u_long		t_cnt;						// Test count.
	u_long		f_cnt;						// Fail count.
};

#endif /* T_STAT_H_DECLARED */


/*****************************************************************************
 * $History: T_STAT.H $
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 4/30/96    Time: 8:39p
 * Updated in $/NMRA/SRC/SEND
 * Changed return from get_percent() to u_long to prevent narrowing
 * warning.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 3:54p
 * Created in $/NMRA/SRC/SEND
 * Header file for 'class T_stat' that stores and prints test statistics
 * for each
 * decoder test.
 *
 *****************************************************************************/

