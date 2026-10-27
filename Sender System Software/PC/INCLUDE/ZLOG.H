/*****************************************************************************
 *
 * DESCRIPTION:
 *
 *	zlog.h	-	Error and Logging routines.  These functions log status
 *				and error messages.
 *
 *****************************************************************************/

#ifndef ZLOG_H_DECLARED
#define ZLOG_H_DECLARED

#include <stdarg.h>
#include <ztypes.h>							// Define Rslt_t, etc.
#include <stdio.h>

/*--------------------------------------------------------------------------*/
/*
 * Defines
 */

typedef void (*Zlog_abort_handler)( void );

/*
 *	Constants
 */
const int		MAX_ROOT		= 31;		// Maximum command name.

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
	

class Zlog
{
  public:
	/* Method section */
	Zlog( void );

	~Zlog();

	/* Simple get methods */
	const char	*get_cmd_name( void ) const noexcept { return ( cmd_name ); }
	bool		get_no_abort_flag( void ) const noexcept { return ( no_abort_flag ); }
	bool		get_stderr_too( void ) const noexcept { return ( stderr_too ); }
	const char	*
	get_err_pri_str(
		Zlog_pri			ipri ) const noexcept
	{
		return ( err_pri_str[ ipri ] );
	}
	FILE *get_fp_log( void ) const noexcept { return ( fp_log ); }
	FILE *get_fp_stat( void ) const noexcept { return ( fp_stat ); }
	bool  get_dbg_on( void ) const noexcept { return dbg_on; }

	/* Simple set methods */
	void set_stderr_too( bool istderr_too ) { stderr_too = istderr_too; }
	void set_no_abort_flag( bool iabort ) { no_abort_flag = iabort; }
	void set_dbg_on( bool idbg) { dbg_on = idbg;} 
	void set_abort_handler( Zlog_abort_handler handler )
		{ abort_handler = handler; }

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
	static const char	*err_pri_str[PRI_SIZE];		// Priority strings.
	char				cmd_name[MAX_ROOT+1];	  	// Command name.
	FILE				*fp_log;				 	// Log file pointer.
	FILE				*fp_stat;					// Stat only file pointer.
	bool				no_abort_flag;				// If true, don't abort.
	bool				stderr_too;					// Log to stderr also
	bool				dbg_on;						// If true, log dbg msgs.
	Zlog_abort_handler		abort_handler;				// Called before aborting.
};

extern Zlog		Deflog;						// Default error log.

#define	ERRPRINT		(Deflog.errprint)
#define VERRPRINT		(Deflog.verrprint)
#define STATPRINT		(Deflog.statprint)
#define DBGPRINT		(Deflog.dbgprint)
#define DBGALL			(Deflog.dbgall)
#define DBGLOG			(Deflog.dbglog)
#define DBGPKT			(Deflog.dbgpkt)
#define TO_LOG			(Deflog.to_log)
#define TO_DUMP			(Deflog.to_dump)
#define TO_STAT			(Deflog.to_stat)

#endif /* ZLOG_H_DECLARED */

