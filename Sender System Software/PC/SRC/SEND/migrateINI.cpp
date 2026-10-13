/*---------------------------------------------------------------------------
 * migrateINI - convert legacy SEND.INI files into SEND.CFG files.
 *
 * Usage: migrateINI [-f] [folder]
 *
 * Every *.ini file in the folder (default: the current folder) is converted
 * to a .cfg file with the same base name, using the send.cfg located beside
 * migrateINI.exe as the template. Each active (not ';' or '#' prefixed) line in the
 * .ini replaces the matching line in the baseline. An active LOG_PKTS is
 * migrated to LOG 0x00000007. The .ini files are never modified. Existing
 * .cfg files are skipped unless -f is given; the baseline send.cfg itself is
 * never overwritten.
 *-------------------------------------------------------------------------*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include <vector>

#define LOG_PKTS_MASK	"0x00000007"
#define BODY_WIDTH		24

struct IniEntry
{
	std::string key;
	std::string val;
};

static std::string
Trim( const std::string &s )
{
	size_t b = s.find_first_not_of( " \t\r\n" );
	if ( b == std::string::npos )
	{
		return "";
	}
	size_t e = s.find_last_not_of( " \t\r\n" );
	return s.substr( b, e - b + 1 );
}

static bool
ReadLines( const std::string &path, std::vector<std::string> &lines )
{
	FILE *fp = fopen( path.c_str(), "r" );
	if ( fp == NULL )
	{
		return false;
	}
	char buf[512];
	while ( fgets( buf, sizeof( buf ), fp ) )
	{
		std::string s( buf );
		while ( !s.empty() && ( s.back() == '\n' || s.back() == '\r' ) )
		{
			s.pop_back();
		}
		lines.push_back( s );
	}
	fclose( fp );
	return true;
}

/* Key/value of a line in the first column sense: leading ';' optional. */
static bool
ParseLine( const std::string &line, bool &active, std::string &key,
	std::string &val, std::string &comment )
{
	std::string s = Trim( line );
	if ( s.empty() || s[0] == '#' )
	{
		return false;
	}
	active = ( s[0] != ';' );
	if ( !active )
	{
		s = s.substr( 1 );
		if ( s.empty() || s[0] == ' ' || s[0] == '\t' || s[0] == ';' ||
			s[0] == '*' )
		{
			return false;
		}
	}

	size_t hash = s.find( '#' );
	comment = ( hash == std::string::npos ) ? "" : s.substr( hash );
	if ( hash != std::string::npos )
	{
		s = s.substr( 0, hash );
	}
	s = Trim( s );

	size_t sp = s.find_first_of( " \t" );
	if ( sp == std::string::npos )
	{
		key = s;
		val = "";
	}
	else
	{
		key = s.substr( 0, sp );
		val = Trim( s.substr( sp ) );
	}
	return !key.empty();
}

static bool
KeyEq( const std::string &a, const std::string &b )
{
	return _stricmp( a.c_str(), b.c_str() ) == 0;
}

/* Rebuild a baseline line as active, with the given value (empty = flag). */
static std::string
BuildLine( const std::string &key, const std::string &val,
	const std::string &comment )
{
	std::string body = key;
	if ( !val.empty() )
	{
		int start = 20 - (int)val.size();
		if ( start < (int)key.size() + 1 )
		{
			start = (int)key.size() + 1;
		}
		body.append( start - key.size(), ' ' );
		body += val;
	}
	if ( !comment.empty() )
	{
		while ( body.size() < BODY_WIDTH )
		{
			body += ' ';
		}
		body += comment;
	}
	return body;
}

static bool
Migrate( const std::string &iniPath, const std::string &cfgPath,
	const std::vector<std::string> &baseline )
{
	std::vector<std::string> iniLines;
	if ( !ReadLines( iniPath, iniLines ) )
	{
		fprintf( stderr, "  cannot read %s\n", iniPath.c_str() );
		return false;
	}

	std::vector<IniEntry> entries;
	for ( size_t i = 0; i < iniLines.size(); i++ )
	{
		bool active;
		std::string key, val, comment;
		if ( !ParseLine( iniLines[i], active, key, val, comment ) || !active )
		{
			continue;
		}
		IniEntry e;
		e.key = key;
		e.val = val;
		if ( KeyEq( key, "TYPE" ) )
		{
			for ( size_t c = 0; c < e.val.size(); c++ )
			{
				e.val[c] = (char)toupper( (unsigned char)e.val[c] );
			}
		}
		if ( KeyEq( key, "LOG_PKTS" ) )
		{
			e.key = "LOG";
			e.val = LOG_PKTS_MASK;
		}
		entries.push_back( e );
	}

	std::vector<std::string> output = baseline;
	for ( size_t n = 0; n < entries.size(); n++ )
	{
		bool found = false;
		for ( size_t li = 0; li < output.size(); li++ )
		{
			bool active;
			std::string key, val, comment;
			if ( ParseLine( output[li], active, key, val, comment ) &&
				KeyEq( key, entries[n].key ) )
			{
				output[li] = BuildLine( key, entries[n].val, comment );
				found = true;
				break;
			}
		}
		if ( !found )
		{
			fprintf( stderr, "  warning: key <%s> not in send.cfg, skipped\n",
				entries[n].key.c_str() );
		}
	}

	FILE *fp = fopen( cfgPath.c_str(), "w" );
	if ( fp == NULL )
	{
		fprintf( stderr, "  cannot write %s\n", cfgPath.c_str() );
		return false;
	}
	for ( size_t i = 0; i < output.size(); i++ )
	{
		fprintf( fp, "%s\n", output[i].c_str() );
	}
	fclose( fp );
	return true;
}

int
main( int argc, char *argv[] )
{
	bool force = false;
	std::string folder = ".";

	for ( int i = 1; i < argc; i++ )
	{
		if ( _stricmp( argv[i], "-f" ) == 0 )
		{
			force = true;
		}
		else
		{
			folder = argv[i];
		}
	}
	while ( folder.size() > 1 &&
		( folder.back() == '\\' || folder.back() == '/' ) )
	{
		folder.pop_back();
	}

	/* Template is always the send.cfg beside migrateINI.exe, read only. */
	char exePath[MAX_PATH];
	GetModuleFileNameA( NULL, exePath, MAX_PATH );
	char *exeSlash = strrchr( exePath, '\\' );
	if ( exeSlash != NULL )
	{
		exeSlash[1] = '\0';
	}
	else
	{
		exePath[0] = '\0';
	}
	std::string basePath = std::string( exePath ) + "send.cfg";
	std::vector<std::string> baseline;
	if ( !ReadLines( basePath, baseline ) )
	{
		fprintf( stderr, "ERROR: template <%s> not found\n", basePath.c_str() );
		return 1;
	}

	WIN32_FIND_DATAA fd;
	HANDLE h = FindFirstFileA( ( folder + "\\*.ini" ).c_str(), &fd );
	if ( h == INVALID_HANDLE_VALUE )
	{
		printf( "No .ini files found in <%s>\n", folder.c_str() );
		return 0;
	}

	int converted = 0, skipped = 0, failed = 0;
	do
	{
		if ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			continue;
		}
		std::string name = fd.cFileName;
		if ( name.size() < 4 ||
			_stricmp( name.c_str() + name.size() - 4, ".ini" ) != 0 )
		{
			continue;
		}
		std::string cfgName = name.substr( 0, name.size() - 4 ) + ".cfg";
		if ( _stricmp( cfgName.c_str(), "send.cfg" ) == 0 )
		{
			/* Never overwrite the baseline send.cfg. */
			cfgName = "migrated_send.cfg";
		}
		std::string iniPath = folder + "\\" + name;
		std::string cfgPath = folder + "\\" + cfgName;

		printf( "%s -> %s\n", name.c_str(), cfgName.c_str() );

		if ( !force && GetFileAttributesA( cfgPath.c_str() ) !=
			INVALID_FILE_ATTRIBUTES )
		{
			printf( "  skipped: %s exists (use -f to overwrite)\n",
				cfgName.c_str() );
			skipped++;
			continue;
		}
		if ( Migrate( iniPath, cfgPath, baseline ) )
		{
			converted++;
		}
		else
		{
			failed++;
		}
	} while ( FindNextFileA( h, &fd ) );
	FindClose( h );

	printf( "%d converted, %d skipped, %d failed\n", converted, skipped, failed );
	return failed ? 1 : 0;
}
