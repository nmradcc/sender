/*****************************************************************************
 *
 * DESCRIPTION:
 *
 *	send.h	-	Public defines for main() Sender test program in send.cpp.
 *
 *****************************************************************************/

#ifndef SEND_H_DECLARED
#define SEND_H_DECLARED

#include <ztypes.h>

/*
 *	Possible version release levels.
 */
enum Rel_levels
{
	VER_DEB						= 'D',			// Debug only.
	VER_EXP						= 'X', 			// Prerelease experimenatal.
	VER_BETA					= 'B',	 		// Prerelease beta.
	VER_REL						= 'R'	 		// Released.
};

/*
 *	Types of decoders.
 */
enum Dec_types
{
	DEC_LOCO					= 'L',	 		// Locomotive decoder.
	DEC_ACC						= 'A',	 		// Accessory decoder.
    DEC_SIG						= 'S',			// Signal decoder.
    DEC_FUNC					= 'F'			// Function decoder.
};

/*
 *	Version information.
 */
const Rel_levels	Ver_rel		= VER_BETA;		// Release level.
const u_int			Ver_maj		=  7;			// Major version.
const u_int			Ver_min		=  0;			// Minor version.
const u_int			Ver_bld		=  0;			// Build version.

/*
 *	Forward references.
 */
class				Send_reg;					// Defined in send_reg.h

/*
 *	Global copy of Send board registers.
 */
extern Send_reg 	Dcc_reg;

/*
 * Macro to clear the line.
 */
#define CLR_LINE	printf( \
"                                                                            \r" )

#endif /* SEND_H_DECLARED */
