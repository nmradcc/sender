/*****************************************************************************
 *
 * File:                 $Workfile: ZLOG.H $
 * Path:                 $Logfile: /NMRA/INCLUDE/ZLOG.H $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 10 $
 * Last Modified on:     $Modtime: 9/12/23 5:34p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 9/12/23 6:10p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	zlog.h	-	Error and Logging routines.  These functions log status
 *				and error messages.
 *
 *****************************************************************************/

#ifndef ZLOG_H_DECLARED
#define ZLOG_H_DECLARED	"@(#) $Workfile: ZLOG.H $$ $Revision: 10 $$"

#include <stdarg.h>
#include <ztypes.h>							// Define Rslt_t, etc.
#include <stdio.h>

/*--------------------------------------------------------------------------*/
/*
 * Defines
 */

#ifndef LOG_MASK_INIT
#define LOG_MASK_INIT	((Bits_t)0L)		// Initial logging mask.
#endif

/*
 *	Constants
 */
const int		MAX_INDENT		= 31;		// Maximum logdump indent level.
const int		MAX_ROOT		= 31;		// Maximum command name.
const char		DFLT_HDR		= '>';		// Default header char after '!'.

/*
 *	Error log priority values.
 */
enum Zlog_pri
{
	LOG_EMERG,
	LOG_ALERT,
	LOG_CRIT,
	LOG_ERR,
	LOG_WARNING,
	LOG_NOTICE,
	LOG_INFO,
	LOG_DEBUG
};
const int	PRI_SIZE	=	LOG_DEBUG+1;	// Number of priorities

/*
 *	Debug print header character after '!' character.
 */
enum Dbg_hdr
{	DBG_DFLT	=	'>', // Default header.
	DBG_PKT		=	'P', // Packet display header.
	DBG_LOG		=	'L', // Send to log only header.
};
	

/*
 * Log bits reserved for library logging using EITHER logprint or lib_logprint.
 * These bits should NOT be used for new code.  New code should use the bits
 * reserved for lib_logprint.  Do not use these bits !!!
 */
#define ZLIB_LOG_TRACE		l_bit( 28 )		// Library trace logging
#define ZLIB_LOG_ACTION		l_bit( 29 )		// Library action logging
#define ZLIB_LOG_IO			l_bit( 30 )		// Library I/O logging
#define ZLIB_LOG_MISC		l_bit( 31 )		// Library misc. logging
#define ZLIB_LOG_ANY	( ZLIB_LOG_TRACE | ZLIB_LOG_ACTION | ZLIB_LOG_IO | \
						ZLIB_LOG_MISC )

/*
 * Log bits for use with lib_logprint ONLY.  Do NOT use these bits for
 * logprint.  lib_logprint should be used only in libraries.
 */
#define LIB_Z_LOG_TRACE		l_bit( 0 )		// Trace Logging in libz
#define LIB_Z_LOG_ACTION	l_bit( 1 )		// Action Logging in libz

class Zlog
{
  public:
	/* Method section */
	Zlog( void );

	~Zlog();

	/* Simple get methods */
	const char	*get_cmd_name( void ) const { return ( cmd_name ); }
	Bits_t		get_log_mask( void ) const { return ( log_mask ); }
	Bits_t		get_lib_log_mask( void ) const { return ( lib_log_mask ); }
	bool		get_no_abort_flag( void ) const { return ( no_abort_flag ); }
	bool		get_stderr_too( void ) const { return ( stderr_too ); }
	const char	*
	get_err_pri_str(
		Zlog_pri			ipri ) const
	{
		return ( err_pri_str[ ipri ] );
	}
	FILE *get_fp_log( void ) const { return ( fp_log ); }
	FILE *get_fp_stat( void ) const { return ( fp_stat ); }
	bool  get_dbg_on( void) { return dbg_on; } 

	/* Simple set methods */
	void set_stderr_too( bool istderr_too ) { stderr_too = istderr_too; }
	void set_log_mask( Bits_t mask ) { log_mask = mask; }
	void set_lib_log_mask( Bits_t imask ) { lib_log_mask = imask; }
	void set_no_abort_flag( bool iabort ) { no_abort_flag = iabort; }
	void set_dbg_on( bool idbg) { dbg_on = idbg;} 

	void
	set_cmd_name(
		const char			*icmd );

	Rslt_t
	open_log(
		const char			*fname );

	Rslt_t
	open_stat(
		const char			*fname );

	void
	close_log( void );

	void
	close_stat( void );

	void
	errprint(
		const char	 		*func,
		const char			*fmt
		... );

	void
	errprint(
		const char		 	*func ,
		const Zlog_pri		priority ,
		const char		  	*fmt,
		... );

	void
	verrprint(
		const char			*func ,
		const Zlog_pri		priority ,
		const char			*fmt ,
		va_list				ap );

	void
	statprint(
		const char			*fmt,
		... );

	void
	logprint(
		const char			*func,
		Bits_t				mask,
		const char			*fmt,
		... );

	void
	vlogprint(
		const char			*func,
		Bits_t				mask,
		const char			*fmt,
		va_list				ap );

	void
	lib_logprint(
		const char			*func,
		Bits_t				mask,
		const char			*fmt,
		... );

	void
	vlib_logprint(
		const char			*func,
		Bits_t				mask,
		const char			*fmt,
		va_list		 		ap );

	void
	dbgprint(
		bool				do_print,
		Dbg_hdr				hdr,
		const char			*fmt,
		... );

	void
	dbgprint(
		bool				do_print,
		const char			*fmt,
		... );

	void
	dbgprint(
		Dbg_hdr				hdr,
		const char			*fmt,
		... );

	void
	dbgprint(
		const char			*fmt,
		... );

	void
	dbgall(
		Dbg_hdr				hdr,
		const char			*fmt,
		... );

	void
	dbgall(
		const char			*fmt,
		... );

	void
	dbglog(
		const char			*fmt,
		... );

	void
	dbgpkt(
		const char			*fmt,
		... );

		void
	vdbgprint(
		bool				print_msg,
		const Dbg_hdr		hdr,
		const char			*fmt,
		va_list				ap );

	void
	logdump(
		const char			*func,
		const char			*fmt,
		... );

	void
	logdump(
		int					indent,
		const char			*func,
		const char			*fmt,
		... );

	void
	vlogdump(
		const char			*func,
		const char			*fmt,
		va_list		  		ap );

	void
	vlogdump(
		int					indent,
		const char			*func,
		const char			*fmt,
		va_list				ap );

	void
	to_dump(
		const char			*fmt,
		... );

	void
	to_log(
		const char			*fmt,
		... );

	void
	to_stat(
		const char			*fmt,
		... );

  protected:
	/* Data section */
	static const char	tab_array[MAX_INDENT+1];	// Array of \t chars.
	static const char	*err_pri_str[PRI_SIZE];		// Priority strings.
	char				cmd_name[MAX_ROOT+1];	  	// Command name.
	FILE				*fp_log;				 	// Log file pointer.
	FILE				*fp_stat;					// Stat only file pointer.
	Bits_t				log_mask;					// Logging mask.
	Bits_t				lib_log_mask;				// Lib logging mask.
	bool				no_abort_flag;				// If true, don't abort.
	bool				stderr_too;					// Log to stderr also
	bool				dbg_on;						// If true, log dbg msgs.
};

extern Zlog		Deflog;						// Default error log.

#define	ERRPRINT		(Deflog.errprint)
#define VERRPRINT		(Deflog.verrprint)
#define STATPRINT		(Deflog.statprint)
#define LOGPRINT		(Deflog.logprint)
#define DBGPRINT		(Deflog.dbgprint)
#define DBGALL			(Deflog.dbgall)
#define DBGLOG			(Deflog.dbglog)
#define DBGPKT			(Deflog.dbgpkt)
#define VLOGPRINT		(Deflog.vlogprint)
#define LIB_LOGPRINT	(Deflog.lib_logprint)
#define LIB_VLOGPRINT	(Deflog.lib_vlogprint)
#define LOGDUMP			(Deflog.logdump)
#define VLOGDUMP		(Deflog.vlogdump)
#define TO_LOG			(Deflog.to_log)
#define TO_DUMP			(Deflog.to_dump)
#define TO_STAT			(Deflog.to_stat)

#endif /* ZLOG_H_DECLARED */


/*****************************************************************************
 * $History: ZLOG.H $
 * 
 * *****************  Version 10  *****************
 * User: Kenw         Date: 9/12/23    Time: 6:10p
 * Updated in $/NMRA/INCLUDE
 * Reordered dbgprint() methods to a more loogical order.
 * 
 * *****************  Version 9  *****************
 * User: Kenw         Date: 8/29/23    Time: 6:14p
 * Updated in $/NMRA/INCLUDE
 * Added vdbgprint(), dbgprint(), dbgall(), dbglog() and dbgpkt().
 * 
 * *****************  Version 8  *****************
 * User: Kenw         Date: 8/27/23    Time: 6:10p
 * Updated in $/NMRA/INCLUDE
 * Changed dbg_on to always be set to false in the constructor.
 * 
 * *****************  Version 7  *****************
 * User: Kenw         Date: 8/20/23    Time: 8:51p
 * Updated in $/NMRA/INCLUDE
 * Added dbgall() and DBGALL.
 * 
 * *****************  Version 6  *****************
 * User: Kenw         Date: 8/20/23    Time: 2:47p
 * Updated in $/NMRA/INCLUDE
 * Added dbgprint() and vfdbgprint().
 * 
 * *****************  Version 3  *****************
 * User: Kenw         Date: 2/01/04    Time: 6:26p
 * Updated in $/NMRA/INCLUDE
 * Updated to expose log and statistics file pointers.
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
 * Header file for 'class Zlog' the error and status logging class.
 *
 *****************************************************************************/

