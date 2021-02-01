/*****************************************************************************
 *
 * File:                 $Workfile: ZLOG.CPP $
 * Path:                 $Logfile: /NMRA/LIB/ZLOG.CPP $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 6 $
 * Last Modified on:     $Modtime: 9/18/17 7:28p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 9/18/17 7:31p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	zlog.cc	-	Error and Logging routines.
 *
 *****************************************************************************/

#include <zlog.h>

#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>		// Needed for abort()

#if SEND_VERSION >= 4
  #include "main.h"
  #include "ff.h"

	extern "C"
	{
		extern void GetCTime(char* time_buf);
	}

#endif

static const char sccsid[]			= "@(#) $Workfile: ZLOG.CPP $$ $Revision: 6 $$";
static const char sccsid_h[]		= ZLOG_H_DECLARED;

// Func to eliminate bogus sccsid and sccsid_h declared but never used warning.
inline void dummy(const char *, const char *) {}

const char		SEPCHARU				= '/';	 	// Unix directory separator.
const char		SEPCHARM				= '\\';		// DOS directory separator.
const int		LOG_STRING_SIZE			= 512;		// Max message size.
const int		TIME_STR				= 32;		// Time string length.
const int		ERR_T_STR_SIZE			= 8;		// Max length of type.
const int		LOG_ABORT_PRIORITY		= LOG_ERR;	// Level to abort program.

const char		Zlog::tab_array[MAX_INDENT+1] =
{
	"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
};

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
	log_mask		=	LOG_MASK_INIT;
	lib_log_mask	=	LOG_MASK_INIT;
	no_abort_flag	=	false;
	stderr_too		=	true;

    // Eliminate bogus sccsid and sccsid_h declared but never used warning.
    dummy( sccsid, sccsid_h );
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
	register int		i;				// Index variable.
	register const char	*sptr;			// Pointer to command part of string.
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
	char			*tptr;					// Temp string pointer.
	#if SEND_VERSION < 4
		time_t          raw_time;               // Seconds from epoch
	#endif
	char            t_buf[TIME_STR];        // Buffer for time string

#if SEND_VERSION >= 4
	GetCTime(t_buf);
#else
	(void)time( &raw_time );
	(void)strcpy( t_buf, ctime( &raw_time ) );
	t_buf[strlen( t_buf ) - 1] = '\0';      // Kill trailing '\n'
#endif

	sprintf(mess, "<%s> %-*s",
		t_buf, ERR_T_STR_SIZE, get_err_pri_str( priority) );

	if ( *cmd_name != '\0' )
	{
		(void)strcat( mess, cmd_name );
		(void)strcat( mess, ": " );
	}

	if ( func != (char *)NULL && *func != '\0' )
	{
		(void)strcat( mess, func );
		(void)strcat( mess, " - " );
	}

	tptr	=	mess + strlen( mess );

	(void)vsprintf( tptr, fmt, ap );

	strcat(tptr, "\n");

	if ( fp_log )
	{
		fputs( mess, fp_log );
		fflush( fp_log );
	}

	if ( stderr_too )
	{
		#if SEND_VERSION >= 4
			puts(mess);
		#else
			fputs( mess, stderr );
		#endif
	}

	/*
	 *	Cause a core dump if the priority is bad enough.
	 */
	if ( !no_abort_flag && (priority <= LOG_ABORT_PRIORITY) )
	{
		set_no_abort_flag( true );	// Don't spiral trying to end.
		ERRPRINT( "errprint", LOG_ERR,
			"Aborting program - error priority <%s>",
			get_err_pri_str( priority ) );
		#if SEND_VERSION < 4
			exit( 1 );
		#endif
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
	char			*tptr;					// Temp string pointer.
	#if SEND_VERSION < 4
		time_t          raw_time;               // Seconds from epoch
	#endif
	char            t_buf[TIME_STR];        // Buffer for time string

	va_start( ap, fmt );

	#if SEND_VERSION >= 4
		GetCTime(t_buf);
	#else
		(void)time( &raw_time );
		(void)strcpy( t_buf, ctime( &raw_time ) );
		t_buf[strlen( t_buf ) - 1] = '\0';      // Kill trailing '\n'
	#endif

	sprintf(mess, "<%s> %-*s",
		t_buf, ERR_T_STR_SIZE, "STATUS" );

	if ( *cmd_name != '\0' )
	{
		(void)strcat( mess, cmd_name );
		(void)strcat( mess, ": " );
	}

	tptr	=	mess + strlen( mess );

	(void)vsprintf( tptr, fmt, ap );

	strcat(tptr, "\n");

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
		#if SEND_VERSION >= 4
			puts(mess);
		#else
			fputs( mess, stderr );
		#endif
	}
}


// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			logprint()						- Log a message.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			logprint() prints a message to the log if logging is enabled
 *			by the bits in the log mask.
 *
 */

void
Zlog::logprint(
	const char			*func,			// Name of calling function
	Bits_t				mask,			// Print mask
	const char			*fmt,			// printf(3) style format string
	... )							   	// printf(3) arguments
{
	va_list			  	ap;			   	// Variable argument list

	if ( !(mask & log_mask) )
	{
		return;
	}

	#if SEND_VERSION >= 4
		(void)puts( "Log msg " );

		if ( *cmd_name != '\0' )
		{
			(void)printf( "%s: ", cmd_name );
		}

		if ( func != (char *)NULL && *func != '\0' )
		{
			(void)printf( "%s - ", func );
		}

		(void)printf( fmt, ap );

		(void)printf( "\n" );
	#else
		(void)fputs( "Log msg ", stderr );

		if ( *cmd_name != '\0' )
		{
			(void)fprintf( stderr, "%s: ", cmd_name );
		}

		if ( func != (char *)NULL && *func != '\0' )
		{
			(void)fprintf( stderr, "%s - ", func );
		}

		va_start( ap, fmt );

		(void)vfprintf( stderr, fmt, ap );

		va_end( ap );

		(void)putc( '\n', stderr );
	#endif
}


// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			vlogprint()						- Log a message.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			vlogprint() prints a message to the log if logging is enabled
 *			by the bits in the log mask.
 *
 */

void
Zlog::vlogprint(
	const char			*func,			// Name of calling function
	Bits_t				mask,			// Print mask
	const char			*fmt,			// vprintf(3) style format string
	va_list				ap )			// vprintf(3) arguments.
{
	if ( !(mask & log_mask) )
	{
		return;
	}

	#if SEND_VERSION >= 4
		(void)puts( "Log msg " );

		if ( *cmd_name != '\0' )
		{
			(void)printf( "%s: ", cmd_name );
		}

		if ( func != (char *)NULL && *func != '\0' )
		{
			(void)printf( "%s - ", func );
		}

		(void)printf( fmt, ap );

		(void)printf( "\n" );
	#else
		(void)fputs( "Log msg ", stderr );

		if ( *cmd_name != '\0' )
		{
			(void)fprintf( stderr, "%s: ", cmd_name );
		}

		if ( func != (char *)NULL && *func != '\0' )
		{
			(void)fprintf( stderr, "%s - ", func );
		}

		(void)vfprintf( stderr, fmt, ap );

		(void)putc( '\n', stderr );
	#endif
}


// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			lib_logprint()					- Log a message from a library.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			lib_logprint() prints a message to the log if logging is enabled
 *			by the bits in the log mask.
 *
 */

void
Zlog::lib_logprint(
	const char			*func,			// Name of calling function
	Bits_t				mask,			// Print mask
	const char			*fmt,			// printf(3) style format string
	... )							 	// printf(3) arguments
{
	va_list			 	ap;				// Variable argument list

	if ( !(mask & lib_log_mask) )
	{
		return;
	}

	#if SEND_VERSION >= 4
		(void)puts( "Lib msg " );

		if ( *cmd_name != '\0' )
		{
			(void)printf( "%s: ", cmd_name );
		}

		if ( func != (char *)NULL && *func != '\0' )
		{
			(void)printf( "%s - ", func );
		}

		(void)printf( fmt, ap );

		(void)printf( "\n" );
	#else
		(void)fputs( "Lib msg ", stderr );

		if ( *cmd_name != '\0' )
		{
			(void)fprintf( stderr, "%s: ", cmd_name );
		}

		if ( func != (char *)NULL && *func != '\0' )
		{
			(void)fprintf( stderr, "%s - ", func );
		}

		va_start( ap, fmt );

		(void)vfprintf( stderr, fmt, ap );

		va_end( ap );

		(void)putc( '\n', stderr );
	#endif
}


// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			vlib_logprint()					- Log a message from a library.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			vlib_logprint() prints a message to the log if logging is enabled
 *			by the bits in the log mask.
 */

void
Zlog::vlib_logprint(
	const char			*func,			// Name of calling function
	Bits_t				mask,			// Print mask
	const char			*fmt,			// vprintf(3) style format string
	va_list				ap )			// vprintf(3) arguments.
{
	if ( !(mask & lib_log_mask) )
	{
		return;
	}

	#if SEND_VERSION >= 4
		(void)puts( "Lib msg " );

		if ( *cmd_name != '\0' )
		{
			(void)printf( "%s: ", cmd_name );
		}

		if ( func != (char *)NULL && *func != '\0' )
		{
			(void)printf( "%s - ", func );
		}

		(void)printf( fmt, ap );

		(void)printf( "\n" );
	#else
		(void)fputs( "Lib msg ", stderr );

		if ( *cmd_name != '\0' )
		{
			(void)fprintf( stderr, "%s: ", cmd_name );
		}

		if ( func != (char *)NULL && *func != '\0' )
		{
			(void)fprintf( stderr, "%s - ", func );
		}

		(void)vfprintf( stderr, fmt, ap );

		(void)putc( '\n', stderr );
	#endif
}


// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			logdump()						-	Format and print the dump msg.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			logdump() writes a dump message to the error log (usually
 *			stderr).  It first prints the command name if cmd_name is set,
 *			followed by the dump message given by the fmt string and its
 *			arguments.
 */

void
Zlog::logdump(
	const char		*func,		  		// name of calling function.
	const char	  	*fmt,		 		// printf(3) style fmt string.
	... )							 		// printf(3) arguments.
{
	va_list			ap;					// Variable argument list.

	va_start( ap, fmt );

	vlogdump( 0, func, fmt, ap );

	va_end( ap );
}


// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			logdump()						-	Format and print the dump msg.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			logdump() writes a dump message to the error log (usually
 *			stderr).  It first prints the command name if cmd_name is set,
 *			followed by the dump message given by the fmt string and its
 *			arguments.
 */

void
Zlog::logdump(
	int				indent,				// Indent level.
	const char		*func,		  		// name of calling function.
	const char	  	*fmt,		 		// printf(3) style fmt string.
	... )							  	// printf(3) arguments.
{
	va_list			ap;					// Variable argument list.

	va_start( ap, fmt );

	vlogdump( indent, func, fmt, ap );

	va_end( ap );
}


// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			vlogdump()						-	Format and print the dump msg.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			vlogdump() writes a dump message to the error log (usually
 *			stderr).  It first prints the command name if cmd_name is set,
 *			followed by the dump message given by the fmt string and its
 *			arguments.
 *
 */

void
Zlog::vlogdump(
	const char			*func,		   	// name of calling function.
	const char			*fmt,		   	// vprintf(3) style fmt string.
	va_list				ap )		   	// vprintf(3) arguments.
{
	vlogdump( 0, func, fmt, ap );
}


// ----------------------------------------------------------------------------
/*
 *	NAME
 *
 *			vlogdump()						-	Format and print the dump msg.
 *
 *	RETURN VALUE
 *
 *			None.
 *
 *	DESCRIPTION
 *
 *			vlogdump() writes a dump message to the error log (usually
 *			stderr).  It first prints the command name if Cmd is set, followed
 *			by the dump message given by the fmt string and its arguments.
 *
 */

void
Zlog::vlogdump(
	int				indent,				// Indent level.
	const char		*func,				// name of calling function.
	const char		*fmt,				// vprintf(3) style fmt string.
	va_list			ap )				// vprintf(3) arguments.
{
	const char		*my_name = "vlogdump";
	#if SEND_VERSION < 4
		time_t         	raw_time;		 	// Seconds from epoch.
	#endif
	char           	t_buf[TIME_STR];  	// Buffer for time string.

	if ( indent < 0 || indent > MAX_INDENT )
	{
		errprint( my_name,
			"Called with out of range indent %d", indent );
		indent = 0;
	}

	#if SEND_VERSION >= 4
		if ( indent == 0 )
		{
			#if SEND_VERSION >= 4
				GetCTime(t_buf);
			#else
				(void)time( &raw_time );
				(void)strcpy( t_buf, ctime( &raw_time ) );
				t_buf[strlen( t_buf ) - 1] = '\0';      	// Kill trailing '\n'.
			#endif

			(void)printf( "<%s> %-*s", t_buf, ERR_T_STR_SIZE, "DUMP" );

			if ( *cmd_name != '\0' )
			{
				(void)printf( "%s: ", cmd_name );
			}
		}
		else
		{
			(void)printf( "%.*s", indent, tab_array );
		}

		if ( func != (char *)NULL && *func != '\0' )
		{
			(void)printf( "%s - ", func );
		}

		(void)printf( fmt, ap );

		(void)printf( "\n" );
	#else
		if ( indent == 0 )
		{
			(void)time( &raw_time );
			(void)strcpy( t_buf, ctime( &raw_time ) );
			t_buf[strlen( t_buf ) - 1] = '\0';      	// Kill trailing '\n'.

			(void)fprintf( stderr, "<%s> %-*s", t_buf, ERR_T_STR_SIZE, "DUMP" );

			if ( *cmd_name != '\0' )
			{
				(void)fprintf( stderr, "%s: ", cmd_name );
			}
		}
		else
		{
			(void)fprintf( stderr, "%.*s", indent, tab_array );
		}

		if ( func != (char *)NULL && *func != '\0' )
		{
			(void)fprintf( stderr, "%s - ", func );
		}

		(void)vfprintf( stderr, fmt, ap );

		(void)putc( '\n', stderr );
	#endif
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
		#if SEND_VERSION >= 4
			(void)printf( fmt, ap );
		#else
			(void)vfprintf( stderr, fmt, ap );
		#endif
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
		#if SEND_VERSION >= 4
			(void)printf( fmt, ap );
		#else
			(void)vfprintf( stderr, fmt, ap );
		#endif
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

	#if SEND_VERSION >= 4
		(void)printf( fmt, ap );
	#else
		(void)vfprintf( stderr, fmt, ap );
	#endif

	va_end( ap );
}


/*****************************************************************************
 * $History: ZLOG.CPP $
 * 
 * *****************  Version 6  *****************
 * User: Kenw         Date: 9/18/17    Time: 7:31p
 * Updated in $/NMRA/LIB
 * Added code to suppress sccsid[] warning.
 * 
 * *****************  Version 3  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:56p
 * Updated in $/NMRA/LIB
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 6/28/98    Time: 9:36a
 * Updated in $/NMRA/LIB
 * Changed to abort program at LOG_ERR.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 1:50p
 * Created in $/NMRA/LIB
 * Methods for 'class Zlog' error and status logging routines.
 *
 *****************************************************************************/


