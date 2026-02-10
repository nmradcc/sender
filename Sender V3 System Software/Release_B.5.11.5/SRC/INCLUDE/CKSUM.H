/*****************************************************************************
 *
 * File:                 $Workfile: CKSUM.H $
 * Path:                 $Logfile: /NMRA/INCLUDE/CKSUM.H $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 1 $
 * Last Modified on:     $Modtime: 1/26/04 10:19p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 2/01/04 6:19p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	cksum.h	-	Function to open a file and calculate its checksum.
 *
 *****************************************************************************/

#ifndef CKSUM_H_DECLARED
#define CKSUM_H_DECLARED	"@(#) $Workfile: CKSUM.H $$ $Revision: 1 $$"

int
nCheckSumFile(
	const char			*pszDirectory,		// Pointer to optional directory name.
	const char			*pszFile,			// Pointer to returned file name.
	unsigned long		*pnCRC,				// Pointer to place to return CRC.
	unsigned long		*pnLength );		// Pointer to place to return file length.


#endif /* DCC_H_DECLARED */


/*****************************************************************************
 * $History: CKSUM.H $
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 2/01/04    Time: 6:19p
 * Created in $/NMRA/INCLUDE
 * Header file for CKSUM.CPP.  Posix 2 checksum calulator.
 * 
  *****************************************************************************/
