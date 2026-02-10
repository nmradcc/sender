/*****************************************************************************
 *
 * File:                 $Workfile: ST_VECT.H $
 * Path:                 $Logfile: /NMRA/SRC/SEND/ST_VECT.H $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 1 $
 * Last Modified on:     $Modtime: 4/21/96 2:56p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 4/21/96 3:53p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	st_vect.h	-	This file defines extern interface to the
 *					self test vectors used to test the Sender hardware.
 *
 *****************************************************************************/

#ifndef ST_VECT_H_DECLARED
#define ST_VECT_H_DECLARED	"@(#) $Workfile: ST_VECT.H $$ $Revision: 1 $$"

struct Test_reg_vm;						// Defined in test_reg.h.

extern Test_reg_vm	St_vect[];			// Self test vector array.
extern const int	St_vect_size;		// Size of vector array.

#endif /* ST_VECT_H_DECLARED */


/*****************************************************************************
 * $History: ST_VECT.H $
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 3:53p
 * Created in $/NMRA/SRC/SEND
 * Header file that provides the interface to self test vector file in
 * ST_VECT.CPP.
 *
 *****************************************************************************/

