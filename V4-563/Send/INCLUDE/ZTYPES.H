/*****************************************************************************
 *
 * File:                 $Workfile: ZTYPES.H $
 * Path:                 $Logfile: /NMRA/INCLUDE/ZTYPES.H $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 8 $
 * Last Modified on:     $Modtime: 8/21/23 5:55p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 8/21/23 5:57p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *  ztypes.h    -   Declares core system constants and types.
 *
 *****************************************************************************/

#ifndef ZTYPES_H_DECLARED
#define ZTYPES_H_DECLARED   "@(#) $Workfile: ZTYPES.H $$ $Revision: 8 $$"

/*
 *  Enum for a result type returning 2 possible results.
 *  Note:   OK and FAIL are redefined in this way to avoid
 *  conflicts with libraries that have already defined them.
 */
enum Rslt_t
{
	FAIL_       = -1,                           // Failure occurred.
	OK_         = 0                             // Success.
};
#ifdef FAIL
#undef FAIL
#undef OK
#endif
#define FAIL    (FAIL_)                         // Alias for FAIL_.
#define OK      (OK_)                           // Alias for OK_.

/*
 *  Enum for a result type returning 3 possible results.
 */
enum TRI_t
{
	TRI_ERR     = -1,                           // Error occurred.
	TRI_F       = 0,                            // No error, No action.
	TRI_T       = 1                             // No error, action.
};
#define ERR     (TRI_ERR)                       // Alias for TRI_ERR.
#define NO      (TRI_F)                         // Alias for TRI_F.
#define YES     (TRI_T)                         // Alias for TRI_T.

/*
 *  Typedefs of the basic data types.
 */
typedef unsigned char   BYTE;                   // Contains 8 bits of data.
typedef unsigned long   Bits_t;                 // Holds 32 bits for masks.
typedef unsigned char   u_char;
typedef unsigned int    u_int;
typedef unsigned short  u_short;
typedef unsigned long   u_long;

/*
 *  Process exit() values.
 */
const int       EXIT_OK             = 0;        // Exit() OK.
const int       EXIT_USER_ERROR     = 1;        // Exit() user error.
const int       EXIT_INTERNAL_ERROR = 2;        // Exit() internal error.

/*
 *  File descriptors.
 */
const int       STDIN               = 0;        // stdin file descriptor.
const int       STDOUT              = 1;        // stdout file descriptor.
const int       STDERR              = 2;        // stderr file descriptor.
const int       FD_INV              = -1;       // Invalid file descriptor.

/*
 *  Init_state indicates which state an object is in.
 *  An object may be initialized once and and terminated once.
 */
enum Init_state
{
	UNINITIALIZED,                              // Constructed only.
	INITIALIZING,                               // Being initialized.
	INITIALIZED,                                // Done initializing.
	TERMINATING,                                // Being terminated.
	TERMINATED                                  // Done terminating.
};

/*
 *  The following macro can be used in printf() statements to prevent
 *  NULL string pointers from causing core dumps.  It returns the string
 *  if it is not NULL, the string "(nil)" otherwise.
 */
#define NIL(str)    ( (str) ? (str) : "(nil)" )

/*
 *  Int and Long bit set macros
 */
#define i_bit( x )  ( 1U << (x) )
#define l_bit( x )  ( 1UL << (x) )

#endif /* ZTYPES_H_DECLARED */


/*****************************************************************************
 * $History: ZTYPES.H $
 * 
 * *****************  Version 8  *****************
 * User: Kenw         Date: 8/21/23    Time: 5:57p
 * Updated in $/NMRA/INCLUDE
 * Removed comment end sequence from version 7 comment.
 * 
 * *****************  Version 7  *****************
 * User: Kenw         Date: 8/21/23    Time: 5:50p
 * Updated in $/NMRA/INCLUDE
 * Removed duplicate $History keywords and added history
 * comment closing comment line.
 * 
 * *****************  Version 6  *****************
 * User: Kenw         Date: 8/21/23    Time: 5:04p
 * Updated in $/NMRA/INCLUDE
 * Move torf() from ZTYPES.H to TORF.H
 * 
 * *****************  Version 5  *****************
 * User: Kenw         Date: 6/28/23    Time: 9:56p
 * Updated in $/NMRA/INCLUDE
 * Added torf() method to rretturn "true" or "false" from bool arguement.
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:56p
 * Updated in $/NMRA/INCLUDE
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 1:20p
 * Created in $/NMRA/INCLUDE
 * Header file for fundamental system constants and macros.
 *
 *****************************************************************************/

