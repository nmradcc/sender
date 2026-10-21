/*****************************************************************************
 *
 * DESCRIPTION:
 *
 *	zlog.cpp	-	Error and Logging routines.
 *
 *****************************************************************************/
 #include <zlog.h>

#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>		// Needed for abort()

/*
 *	Declare DBH_ON to set dbgon true in the constructor.
 *	Do not declare it to set dbgon false in the constructor.
 */
//#define			DBG_ON	// If defined, set dbgon true.
	

const char		SEPCHARU				= '/';	 	// Unix directory separator.
const char		SEPCHARM				= '\\';		// DOS directory separator.
const int		LOG_STRING_SIZE			= 512;		// Max message size.
const int		TIME_STR				= 32;		// Time string length.
const int		ERR_T_STR_SIZE			= 8;		// Max length of type.
const int		LOG_ABORT_PRIORITY		= LOG_ERR;	// Level to abort program.

/*
 *	Build "<time> LABEL cmd: func - message\n" in a bounded buffer.
 *	Over-long messages are truncated but always end in a newline.
 */
static void
format_message(
	char			*mess,					// Output buffer.
	size_t			size,					// Output buffer size.
	const char		*label,					// Message type label.
	const char		*cmd,					// Command name, may be empty.
	const char		*func,					// Calling function, may be null.
	const char		*fmt,					// vprintf(3) style format string.
	va_list			ap )					// vprintf(3) arguments.
{
	time_t			raw_time;				// Seconds from epoch
	char			t_buf[TIME_STR];		// Buffer for time string
	bool			has_cmd = ( *cmd != '\0' );
	bool			has_func = ( func != nullptr && *func != '\0' );
	int				hdr_len;				// Header length.
	size_t			len;					// Message length.

	(void)time( &raw_time );
	snprintf( t_buf, sizeof( t_buf ), "%.24s", ctime( &raw_time ) );

	hdr_len = snprintf( mess, size, "<%s> %-*s%s%s%s%s",
		t_buf, ERR_T_STR_SIZE, label,
		has_cmd ? cmd : "", has_cmd ? ": " : "",
		has_func ? func : "", has_func ? " - " : "" );

	if ( hdr_len < 0 || static_cast<size_t>( hdr_len ) >= size )
	{
		hdr_len = static_cast<int>( size ) - 1;
	}

	(void)vsnprintf( mess + hdr_len, size - hdr_len, fmt, ap );

	len = strlen( mess );
	if ( len > size - 2 )
	{
		len = size - 2;
	}
	mess[len]		= '\n';
	mess[len + 1]	= '\0';
}

/*
 *	Names for each priority from <syslog.h>.
 *	WARNING!!!! Make sure to modify this table
 *	if any of the <syslog.h> priorities change.
 */
const char		*Zlog::err_pri_str[PRI_SIZE]	=
{
	"EMERG",
	"ALERT",
	"CRIT",
	"ERROR",
	"WARNING",
	"NOTICE",
	"INFO",
	"DEBUG"
};

/*
 *	Default error log.
 */
Zlog	Deflog;

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		Zlog()							-	 Constructor.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		Zlog() constructs a new Zlog object.
 */
/*--------------------------------------------------------------------------*/

Zlog::Zlog( void )
{
	cmd_name[0]		=	'\0';
	fp_log			=	(FILE *)0;
	fp_stat			=	(FILE *)0;
	no_abort_flag	=	false;
	stderr_too		=	true;
	dbg_on			=	false;
	abort_handler	= nullptr;

}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		~Zlog()							-	Destructor.
 *
 *	RETURN VALUE
 *
 *		NONE
 *
 *	DESCRIPTION
 *
 *		~Zlog() is the destructor for Zlog.
 */
/*--------------------------------------------------------------------------*/

Zlog::~Zlog()
{
	close_log();
	close_stat();
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		set_cmd_name()		  			-	Set a new command name.
 *
 *	RETURN VALUE
 *
 *		None
 *
 *	DESCRIPTION
 *
 *		set_cmd_name() sets a new process command name.  Passing a NULL or
 *		'\0'  will clear the command name.
 */
/*--------------------------------------------------------------------------*/

void
Zlog::set_cmd_name(
	const char			*icmd )			 // New command name.
{
	int					i;				// Index variable.
	const char			*sptr;				// Pointer to command part of string.
	if ( !icmd )
	{
		cmd_name[0]	=	'\0';
		return;
	}

	for ( sptr = icmd; *icmd != '\0'; icmd++ )
	{
		if ( *icmd == SEPCHARU || *icmd == SEPCHARM )
		{
			sptr	=	icmd + 1;
		}
	}

	for ( i = 0; *sptr != '\0'  && i < MAX_ROOT && *sptr != '.'; i++,sptr++ )
	{
		cmd_name[i]	=	tolower(*sptr);
	}

	cmd_name[i]	=	'\0';
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		open_log()						  -	Open alternate log file.
 *
 *	RETURN VALUE
 *
 *		OK		-	File opened succesfully.
 *		FAIL	-	Problem opening file.
 *
 *	DESCRIPTION
 *
 *		open_log() opens an alternate logging file.
 *		It will clear the 'stderr_too'
 *		flag if the file is opened successfully.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Zlog::open_log(
	const char		*fname )			 // Alternate log file name.
{
	const char		*my_name = "Zlog::open_log";

	if ( !fname || !fname[0] )
	{
		errprint( my_name, LOG_ERR,
			"Open  FAILED, invalid file name\n",
			fname );
		return ( FAIL );
	}

	if ( fp_log )
	{
		errprint( my_name, LOG_ERR,
			"Open of log file name <%s> FAILED, another file already open\n",
			fname );
		return ( FAIL );
	}

	fp_log	=	fopen( fname, "a+" );
	if ( !fp_log )
	{
		errprint( my_name, LOG_ERR,
			"Open of log file name <%s> FAILED\n", fname );
		return ( FAIL );
	}

	stderr_too	=	false;
	return ( OK );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		close_log()						-	Close alternate log file.
 *
 *	RETURN VALUE
 *
 *		None.
 *
 *	DESCRIPTION
 *
 *		close_log() closes an alternate logging file if it is open.
 *		It will set the 'stderr_too' flag after closing the alternate file.
 */
/*--------------------------------------------------------------------------*/

void
Zlog::close_log( void )
{
	stderr_too	=	true;

	if ( fp_log )
	{
		fclose( fp_log );
	}

	fp_log	=	(FILE *)0;
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		open_stat()						  -	Open statistics only file.
 *
 *	RETURN VALUE
 *
 *		OK		-	File opened succesfully.
 *		FAIL	-	Problem opening file.
 *
 *	DESCRIPTION
 *
 *		open_stat() opens optional statistics only file.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Zlog::open_stat(
	const char		*fname )			 // Stat only file name.
{
	const char		*my_name = "Zlog::open_stat";

	if ( !fname || !fname[0] )
	{
		errprint( my_name, LOG_ERR,
			"Open  FAILED, invalid stat file name\n",
			fname );
		return ( FAIL );
	}

	if ( fp_stat )
	{
		errprint( my_name, LOG_ERR,
			"Open of stat file name <%s> FAILED, another file already open\n",
			fname );
		return ( FAIL );
	}

	fp_stat	=	fopen( fname, "a+" );
	if ( !fp_stat )
	{
		errprint( my_name, LOG_ERR,
			"Open of stat file name <%s> FAILED\n", fname );
		return ( FAIL );
	}

	return ( OK );
}

 
/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		close_stat()		   			-	Close statistics only log file.
 *
 *	RETURN VALUE
 *
 *		None.
 *
 *	DESCRIPTION
 *
 *		close_stat() closes the statistics only file if it is open.
 */
/*--------------------------------------------------------------------------*/

void
Zlog::close_stat( void )
{
	if ( fp_stat )
	{
		fclose( fp_stat );
	}

	fp_stat	=	(FILE *)0;
}

 
// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			errprint()					-	Format and print the error msg.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			errprint() writes the error message to the error log (usually
 *			stderr).  It first prints the command name if Cmd is set, followed
 *			by the error message given by the fmt string and its arguments.
 *
 */

void
Zlog::errprint(
	const char		*func,					// name of calling function
	const char		*fmt,					// printf(3) style format string
	... )									// printf(3) style arguments
{
	va_list			ap;						// Variable argument list

	va_start( ap, fmt );

	verrprint( func, LOG_ERR, fmt, ap );

	va_end( ap );
}

 
// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			errprint()					-	Format and print leveled error.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			errprint() writes the error message to the error log (usually
 *			stderr).  It first prints the command name if Cmd is set, followed
 *			by the priority, followed by the error message given by the fmt
 *			string and its arguments.
 *
 */

void
Zlog::errprint(
	const char		*func,				// name of calling function.
	const Zlog_pri	priority,			// Priority of error.
	const char		*fmt,				// printf(3) style format string.
	... )								// printf(3) style arguments.
{
	va_list			ap;					// Variable argument list.

	va_start( ap, fmt );

	verrprint( func, priority, fmt, ap );

	va_end( ap );
}

 
// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			verrprint()						-	Format and print leveled error.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			verrprint() writes the error message to the error log (usually
 *			stderr).  It first prints the command name if Cmd is set, followed
 *			by the priority, followed by the error message given by the fmt
 *			string and its arguments.
 *
 */

void
Zlog::verrprint(
	const char		*func,					// name of calling function.
	const Zlog_pri	priority,				// Priority of error.
	const char		*fmt,					// vprintf(3) style format string.
	va_list			ap )					// vprintf(3) arguments.
{
	char			mess [LOG_STRING_SIZE];	// message buffer.

	format_message( mess, sizeof( mess ), get_err_pri_str( priority ),
		cmd_name, func, fmt, ap );

	if ( fp_log )
	{
		fputs( mess, fp_log );
		fflush( fp_log );
	}

	if ( stderr_too )
	{
		fputs( mess, stderr );
	}

	/*
	 *	Cause a core dump if the priority is bad enough.
	 */
	if ( !no_abort_flag && (priority <= LOG_ABORT_PRIORITY) )
	{
		if ( abort_handler != nullptr )
		{
			abort_handler();
		}

		set_no_abort_flag( true );	// Don't spiral trying to end.
		ERRPRINT( "errprint", LOG_ERR,
			"Aborting program - error priority <%s>",
			get_err_pri_str( priority ) );
		exit( 1 );
	}
}
 
// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			statprint()						-	Format and print the status msg.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			statprint() writes a status message to the error log (usually
 *			stderr).  It first prints the command name if Cmd is set, followed
 *			by the status message given by the fmt string and its arguments.
 *
 */

void
Zlog::statprint(
	const char *fmt,						// printf(3) style format string
	... )							 		// printf(3) arguments
{
	va_list			ap;						// Variable argument list
	char			mess [LOG_STRING_SIZE];	// message buffer

	va_start( ap, fmt );

	format_message( mess, sizeof( mess ), "STATUS", cmd_name, nullptr, fmt, ap );

	va_end( ap );

	if ( fp_log )
	{
		fputs( mess, fp_log );
		fflush( fp_log );
	}

	if ( fp_stat )
	{
		fputs( mess, fp_stat );
		fflush( fp_stat );
	}

	if ( stderr_too )
	{
		fputs( mess, stderr );
	}
}

 
// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			dbgprint()					-	Print debug msg.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			dbprint() are four overloaded methods that write a debug message
 *			to the log. It begins the message with the '!' character followed
 *			by either the hdr member variable if present or the DBG_DFLT character. 
 *			It ends the message with the '\n' char.
 *			The dbgprint( do_print, fmt, ... ) prints the message if
 *			do_print is true. dbpprint( fmt, ''' ) prints the message if
 *			the dbg_on member variable is true. Both methoids use
 *			vfdbgprint() to print the message.
 */

void
Zlog::dbgprint(
	bool				do_print,		// If true, print the message.
	const Dbg_hdr		hdr,			// Header following '!' char.
	const char			*fmt,		 	// printf(3) style format string
	... )							  	// printf(3) arguments.
{
	va_list			ap;					// Variable argument list.

	va_start( ap, fmt );

	vdbgprint( do_print, hdr, fmt, ap );

	va_end( ap );
}

void
Zlog::dbgprint(
	bool				do_print,		// If true, print the message.
	const char			*fmt,		 	// printf(3) style format string
	... )							  	// printf(3) arguments.
{
	va_list			ap;					// Variable argument list.

	va_start( ap, fmt );

	vdbgprint( do_print, DBG_DFLT, fmt, ap );

	va_end( ap );
}

void
Zlog::dbgprint(
	const Dbg_hdr		hdr,			// Header following '!' char.
	const char			*fmt,		 	// printf(3) style format string
	... )							  	// printf(3) arguments.
{
	va_list			ap;					// Variable argument list.

	va_start( ap, fmt );

	vdbgprint( dbg_on, hdr, fmt, ap );

	va_end( ap );
}

void
Zlog::dbgprint(
	const char			*fmt,		 	// printf(3) style format string
	... )							  	// printf(3) arguments.
{
	va_list			ap;					// Variable argument list.

	va_start( ap, fmt );

	vdbgprint( dbg_on, DBG_DFLT, fmt, ap );

	va_end( ap );
}
	
 
// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			dbgall()					-	Always print debug msg
 *                                 			using the DBG_DFLT header.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			dbgall() always writes a debug message to the log.
 *			It begins the message with the '!' character followed
 *			by either the hdr member variable if present or the DBG_DFLT character. 
 *			It ends the message with the '\n' char.
 *			It uses vdbgprint() to print the message.
 */

void
Zlog::dbgall(
	const Dbg_hdr		hdr,			// Header following '!' char.
	const char			*fmt,		 	// printf(3) style format string
	... )							  	// printf(3) arguments.
{
	va_list			ap;					// Variable argument list.

	va_start( ap, fmt );

	vdbgprint( true, hdr, fmt, ap );

	va_end( ap );
}

void
Zlog::dbgall(
	const char			*fmt,		 	// printf(3) style format string
	... )							  	// printf(3) arguments.
{
	va_list			ap;					// Variable argument list.

	va_start( ap, fmt );

	vdbgprint( true, DBG_DFLT, fmt, ap );

	va_end( ap );
}

 
// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			dbglog()					-	Always print debug msg
 *                                 			using the DBG_LOG header.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			dbglog() always writes a debug message to the log.
 *			It begins the message with the '!' character followed
 *			by the DBG_LOG character. 
 *			It ends the message with the '\n' char.
 *			It uses vdbgprint() to print the message.
 */

void
Zlog::dbglog(
	const char			*fmt,		 	// printf(3) style format string
	... )							  	// printf(3) arguments.
{
	va_list			ap;					// Variable argument list.

	va_start( ap, fmt );

	vdbgprint( true, DBG_LOG, fmt, ap );

	va_end( ap );
}

 
// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			dbgpkt()					-	Always print debug msg
 *                                 			using the DBG_PKT header.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			dbgpkt() always writes a debug message to the log.
 *			It begins the message with the '!' character followed
 *			by the DBG_PKT character. 
 *			It ends the message with the '\n' char.
 *			It uses vdbgprint() to print the message.
 */

void
Zlog::dbgpkt(
	const char			*fmt,		 	// printf(3) style format string
	... )							  	// printf(3) arguments.
{
	va_list			ap;					// Variable argument list.

	va_start( ap, fmt );

	vdbgprint( true, DBG_PKT, fmt, ap );

	va_end( ap );
}
	
 
// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			vdbgprint()					-	Print log msg with "!>" heaser.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			vdbgprint() writes a debug message to the error log (usually
 *			stderr). It prints the "!> " string at the head of the message
 *			and ends the message with the '\n' character. This is the
 *			core debug message used by dbgprint().
 */

void
Zlog::vdbgprint(
	bool				do_print,		// If true, print the message.
	const Dbg_hdr		hdr,			// Debug header after '!' char.
	const char			*fmt,		 	// printf(3) style format string
	va_list				ap )			// printf(3) arguments.
{
	if ( !do_print )
	{
		return;
	}
	
	if ( fp_log )
	{
		(void)fprintf(  fp_log, "!%c ", static_cast<char>( hdr ) );
		(void)vfprintf( fp_log, fmt, ap );
		(void)fputc( '\n', fp_log );
		fflush( fp_log );
	}

	if ( stderr_too )
	{
		(void)fprintf(  stderr, "!%c ", static_cast<char>( hdr ) );
		(void)vfprintf( stderr, fmt, ap );
		(void)fputc( '\n', stderr );
		fflush( stderr );
	}	
}

 
// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			to_log()					-	Print log msg w/o a header.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			to_log() writes a dump message to the error log (usually
 *			stderr). It does not print any header informaton prior to the
 *			message.
 */

void
Zlog::to_log(
	const char			*fmt,		 	// printf(3) style format string
	... )							 	// printf(3) arguments
{
	va_list				ap;			 	// Variable argument list

	va_start( ap, fmt );

	if ( fp_log )
	{
		(void)vfprintf( fp_log, fmt, ap );
		fflush( fp_log );
	}

	if ( stderr_too )
	{
		(void)vfprintf( stderr, fmt, ap );
	}

	va_end( ap );
}

 
// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			to_stat()		   		-	Print stat only msg w/o a header.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			to_stat() writes a dump message to the stat only file.
 *			It does not print any header informaton prior to the
 *			message.
 */

void
Zlog::to_stat(
	const char			*fmt,		 	// printf(3) style format string
	... )							 	// printf(3) arguments
{
	va_list				ap;			 	// Variable argument list

	va_start( ap, fmt );

	if ( fp_log )
	{
		(void)vfprintf( fp_log, fmt, ap );
		fflush( fp_log );
	}

	if ( stderr_too )
	{
		(void)vfprintf( stderr, fmt, ap );
	}

	if ( fp_stat )
	{
		(void)vfprintf( fp_stat, fmt, ap );
		fflush( fp_stat );
	}

	va_end( ap );
}

 
// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			to_dump()					-	Print dump msg w/o a header.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			dump_no_hdr() writes a dump message to the error log (usually
 *			stderr). It does not print any header informaton prior to the
 *			message.
 */

void
Zlog::to_dump(
	const char			*fmt,		 	// printf(3) style format string
	... )							 	// printf(3) arguments
{
	va_list				ap;			 	// Variable argument list

	va_start( ap, fmt );

	(void)vfprintf( stderr, fmt, ap );

	va_end( ap );
}
