/*****************************************************************************
 *
 * DESCRIPTION:
 *
 *	cksum.h	-	Function to open a file and calculate its checksum.
 *
 *****************************************************************************/

#ifndef CKSUM_H_DECLARED
#define CKSUM_H_DECLARED

int
nCheckSumFile(
	const char			*pszDirectory,		// Pointer to optional directory name.
	const char			*pszFile,			// Pointer to returned file name.
	unsigned long		*pnCRC,				// Pointer to place to return CRC.
	unsigned long		*pnLength );		// Pointer to place to return file length.


#endif /* DCC_H_DECLARED */

