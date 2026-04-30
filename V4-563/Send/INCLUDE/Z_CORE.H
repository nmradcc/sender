/*****************************************************************************
 *
 * File:                 $Workfile: Z_CORE.H $
 * Path:                 $Logfile: /NMRA/INCLUDE/Z_CORE.H $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 1 $
 * Last Modified on:     $Modtime: 4/21/96 1:11p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 4/21/96 1:19p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	z_core.h	-	Class description for Z_core.  This abstract
 *					class forms the core for all Z_core types.  It
 *					defines only the 'obj_errs' variable and methods.
 *					It defines no virtual functions to lower overhead.
 *
 *****************************************************************************
 *
 * CLASS VARIABLES:
 *
 *	obj_errs		-	Bit mask of object construction and init errors.
 *						By convention, fatal errors occupy bit positions
 *						0-15 and nonfatal errors occupy bit positions
 *						16-31.	Bit 0-7 and 16-23 are reserved for use by
 *						Z_core and Config_src.  All other bits can be used
 *						by subclasses.
 *
 *****************************************************************************
 *
 * PUBLIC CLASS METHODS:
 *
 *	z_core			-	Constructor.
 *	get_obj_errs	-	Returns obj_errs.
 *	get_fatal_errs	-	Returns only fatal obj_errs.
 *
 *****************************************************************************/

#ifndef Z_CORE_H_DECLARED
#define Z_CORE_H_DECLARED	"@(#) $Workfile: Z_CORE.H $$ $Revision: 1 $$"

#include <ztypes.h>

/*
 *	Object error type.
 */
typedef	unsigned long	Z_obj_err_t;

/*
 *	Macros to set and test obj_errs bits
 */
#define SET_ERROR( x )	( obj_errs |= (Z_obj_err_t)(x) )
#define GET_ERROR( x )	( obj_errs & (Z_obj_err_t)(x) )

#define CLEAR_WARN( x )	( obj_errs &= ~( (Z_obj_err_t)(x) & ~FATAL_ERR ) )

/*
 *	Error bits for obj_errs.  Setting any of these bits implies that
 *	the object is fatally flawed and execution should not continue.
 *	Bits 0-7 are reserved for use by Z_core.  Bits 8-15 are available
 *	for other objects although you should use bits 0-7 whenever possible.
 */

/* Fatal object errors */
const Z_obj_err_t	CONSTRUCTOR_OBJ_ERR	= l_bit( 0);	// Constructor error.
const Z_obj_err_t	SET_OBJ_ERR			= l_bit( 1);	// Set error.
const Z_obj_err_t	DESTROY_OBJ_ERR		= l_bit( 2);	// Destroy error.
const Z_obj_err_t	MISC_OBJ_ERR		= l_bit( 7);	// Miscellaneous error.

/*
 *	Nonfatal object warnings warn of some nonfatal problem with the object.
 *	Execution can continue but something funny happened.
 *	Bits 16-23 are reserved for use by Z_object_core and Config_src.  Bits
 *	24-31 are available for other objects although you should use bits 16-23
 *	whenever possible.
 */

/* Nonfatal object errors */
const Z_obj_err_t	NOT_READY_OBJ_WARN	= l_bit(16);	// can't do action at
														//		this time.
const Z_obj_err_t	INV_OBJ_WARN		= l_bit(17);	// Got invalid request.
const Z_obj_err_t	RANGE_OBJ_WARN		= l_bit(18);	// Out of range.
const Z_obj_err_t	MISC_OBJ_WARN		= l_bit(19);	// Misc. warning.

/* Config_src warnings used by the lex routines */
const Z_obj_err_t	NULL_LEX_WARN		= l_bit(20);	// No valid input.
const Z_obj_err_t	PARSE_LEX_WARN		= l_bit(21);	// Parse problem.
const Z_obj_err_t	RANGE_LEX_WARN		= l_bit(22);	// Value out of range.
const Z_obj_err_t	EOL_LEX_WARN		= l_bit(23);	// End of line or
														// comment seen.

/* Mask of all fatal errors */
const Z_obj_err_t	FATAL_ERR			= ( 	0xffffL 				);

/* Mask of all warning errors */
const Z_obj_err_t	WARN_ERR			= (		0xffff0000L				);

/* Mask of all object warnings */
const Z_obj_err_t	OBJ_WARN			= (		NOT_READY_OBJ_WARN
											|	INV_OBJ_WARN
											|	RANGE_OBJ_WARN
											|	MISC_OBJ_WARN			);

/* Mask of all Config_src lex warnings */
const Z_obj_err_t	LEX_WARN			= (		NULL_LEX_WARN
											|	PARSE_LEX_WARN
											|	RANGE_LEX_WARN
											|	EOL_LEX_WARN			);


class Z_core
{
  public:
	/* Method section */
	Z_core( void ) :
		obj_errs( 0L ) {}

	Z_core(
		const Z_core		&icore )
		: obj_errs( icore.obj_errs ) {}

	Z_core &
	operator =( const Z_core &icore )
	{
		obj_errs = icore.obj_errs;

		return ( *this );
	}

	/* Simple get methods */

	const Z_obj_err_t
	get_obj_errs() const
	{
		return ( obj_errs );
	}

	const Z_obj_err_t
	get_fatal_errs( void ) const
	{
		return ( obj_errs & FATAL_ERR );
	}

	void
	clear_warn( void )
	{
		CLEAR_WARN( WARN_ERR );
	}

  protected:
	/* Data section */
	Z_obj_err_t			obj_errs;			// Error bits for this object
};

#endif /* Z_CORE_H_DECLARED */


/*****************************************************************************
 * $History: Z_CORE.H $
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 1:19p
 * Created in $/NMRA/INCLUDE
 * Header file for 'class Z_core' the base class of the class tree.
 *
 *****************************************************************************/

