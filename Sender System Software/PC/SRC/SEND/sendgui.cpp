/*****************************************************************************
 *
 * sendgui.cpp - Native Win32 GUI editor for the SEND.CFG configuration file
 *               used by the "send" DCC conformance test program.
 *
 * Lets the user view/modify the test masks (TESTS, CLOCKS, FUNCS) and the
 * boolean option flags normally hand-edited in SEND.CFG, then load/save the
 * file without needing to remember the key names or bit layouts.
 *
 *****************************************************************************/

#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include "send.h"
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "comctl32.lib")

/*---------------------------------------------------------------------------
 * Control identifiers.
 *-------------------------------------------------------------------------*/
enum
{
	ID_EDIT_ADDRESS = 1000,
	ID_COMBO_TYPE,
	ID_EDIT_PRESET,
	ID_EDIT_TRIGGER,
	ID_EDIT_EXTRA_PRE,
	ID_EDIT_FILL_MSEC,
	ID_EDIT_TEST_REPS,
	ID_EDIT_ACC_PAIR,
	ID_EDIT_EXTRA_TRG,
	ID_EDIT_MCU_PORT,
	ID_EDIT_MCU_BAUD,
	ID_EDIT_TESTS,
	ID_EDIT_CLOCKS,
	ID_EDIT_FUNCS,
	ID_EDIT_TESTS_GENERAL,
	ID_EDIT_CLOCKS_GENERAL,
	ID_EDIT_FUNCS_GENERAL,
	ID_EDIT_LOG,
	ID_EDIT_LOG_GENERAL,

	ID_CHK_MANUAL,
	ID_CHK_LAMP,
	ID_CHK_REPEAT,
	ID_CHK_TRIG_REV,
	ID_CHK_LOCO_FIRST,
	ID_CHK_NO_ABORT,
	ID_CHK_LATE_SCOPE,
	ID_CHK_FRAGMENT,
	ID_CHK_SAME_AMBIG_ADDR,
	ID_CHK_KICK_START,
	ID_CHK_MCU_BYPASS_TST,

	ID_BTN_LOAD,
	ID_BTN_SAVE,
	ID_BTN_SAVEAS,
	ID_BTN_DEFAULTS,
	ID_BTN_LAUNCH,
	ID_BTN_MCU_TEST,

	ID_STATIC_PATH,
	ID_STATIC_MCU_STATUS,

	ID_TAB,

	ID_CHK_TESTBIT_BASE,			// 32 sequential ids follow (bits 0..31).
	ID_BTN_TESTBITS_ALL = ID_CHK_TESTBIT_BASE + 32,
	ID_BTN_TESTBITS_NONE,
	ID_BTN_TESTBITS_DEFAULT,

	ID_CHK_CLOCKBIT_BASE,			// 32 sequential ids follow (bits 0..31).
	ID_BTN_CLOCKBITS_ALL = ID_CHK_CLOCKBIT_BASE + 32,
	ID_BTN_CLOCKBITS_NONE,
	ID_BTN_CLOCKBITS_DEFAULT,

	ID_CHK_FUNCBIT_BASE,
	ID_BTN_FUNCBITS_ALL = ID_CHK_FUNCBIT_BASE + 5,
	ID_BTN_FUNCBITS_NONE,
	ID_BTN_FUNCBITS_DEFAULT,

	ID_CHK_LOGBIT_BASE,
	ID_BTN_LOGBITS_ALL = ID_CHK_LOGBIT_BASE + 6,
	ID_BTN_LOGBITS_NONE,
	ID_BTN_LOGBITS_DEFAULT
};

/* Number of individual TESTS mask bit checkboxes. */
static const int TESTBIT_COUNT = 32;

/* Short description of what each TESTS mask bit selects (see DEC_TST.CPP
 * print_user_docs() for the authoritative bit ordering). Valid bits are 0-19;
 * bits 20-31 are reserved and must stay clear. */
static const char *g_testBitLabels[TESTBIT_COUNT] =
{
	"1T margin test",
	"1H duty cycle test",
	"Ramp test",
	"Packet accept: Pre 12 Idle 1",
	"Packet accept: Pre 12 Idle 2",
	"Packet accept: Pre 13 Idle 1",
	"Packet accept: Pre 15 Idle 1",
	"Packet accept: Pre 15 Idle 2",
	"Bad address test",
	"Bad bit test",
	"Stretched 0: No stretch",
	"Stretched 0: Long negative",
	"Stretched 0: Long positive",
	"Stretched 0: Max negative",
	"Stretched 0: Max positive",
	"Truncated packet test",
	"Prior packet test",
	"6 prior byte test",
	"1 ambiguous bit test",
	"2 ambiguous bits test",
	"Reserved bit 20",
	"Reserved bit 21",
	"Reserved bit 22",
	"Reserved bit 23",
	"Reserved bit 24",
	"Reserved bit 25",
	"Reserved bit 26",
	"Reserved bit 27",
	"Reserved bit 28",
	"Reserved bit 29",
	"Reserved bit 30",
	"Reserved bit 31",
};

/* Number of individual CLOCKS mask bit checkboxes. */
static const int CLOCKBIT_COUNT = 32;

/* Name of each CLOCKS mask bit's clock timing variant (see DEC_TST.CPP
 * Dec_tst::dclk_tbl[] for the authoritative bit ordering). Valid bits are 0-16;
 * bits 17-31 are reserved and must stay clear. */
static const char *g_clockBitLabels[CLOCKBIT_COUNT] =
{
	"All nominal",
	"All 1/4 fast",
	"Command station min",
	"Minimum + 2",
	"Minimum + 1",
	"Decoder minimum",
	"All 1/4 slow",
	"Command station max",
	"Maximum - 2",
	"Maximum - 1",
	"Decoder maximum",
	"Negative stretched 0",
	"Positive stretched 0",
	"Very negative 0",
	"Very positive 0",
	"Max Decoder Neg 0",
	"Max Decoder Pos 0",
	"Reserved bit 17",
	"Reserved bit 18",
	"Reserved bit 19",
	"Reserved bit 20",
	"Reserved bit 21",
	"Reserved bit 22",
	"Reserved bit 23",
	"Reserved bit 24",
	"Reserved bit 25",
	"Reserved bit 26",
	"Reserved bit 27",
	"Reserved bit 28",
	"Reserved bit 29",
	"Reserved bit 30",
	"Reserved bit 31",
};

static const int FUNCBIT_COUNT = 5;

static const char *g_funcBitLabels[FUNCBIT_COUNT] =
{
	"AUX1",
	"AUX2",
	"AUX3",
	"AUX4",
	"FL",
};

static const int LOGBIT_COUNT = 6;

static const char *g_logBitLabels[LOGBIT_COUNT] =
{
	"Log file",
	"Command logging",
	"Packet data logging",
	"IO data logging",
	"Squelch idle packets",
	"Reduce pass count (5)",
};

/*---------------------------------------------------------------------------
 * Global state.
 *-------------------------------------------------------------------------*/
static HWND	g_hMain;
static char	g_cfgPath[MAX_PATH] = "";
static HANDLE	g_mcuTestProcess = NULL;
static HANDLE	g_mcuTestOutputRead = NULL;
static std::string	g_mcuTestOutput;
static const UINT_PTR MCU_TEST_TIMER_ID = 1;
static std::vector<std::string>	g_origLines;	// Raw lines of the last loaded/saved cfg file, preserved across saves.
static bool	g_iniSource = false;	// True when the last loaded file was an .ini; restricts Save As to .ini.
static std::vector<HWND>	g_page0Ctrls;	// Controls shown on the "General" tab.
static std::vector<HWND>	g_page1Ctrls;	// Controls shown on the "Test Mask Bits" tab.
static std::vector<HWND>	g_page2Ctrls;	// Controls shown on the "Clock Mask Bits" tab.
static std::vector<HWND>	g_page3Ctrls;	// Controls shown on the "Function Mask Bits" tab.
static std::vector<HWND>	g_page4Ctrls;	// Controls shown on the "Log Mask Bits" tab.

struct FlagCtrl
{
	int			ctrlId;		// Checkbox control id.
	const char	*key;		// SEND.CFG key name.
};

static const FlagCtrl g_flagCtrls[] =
{
	{ ID_CHK_MANUAL,          "MANUAL" },
	{ ID_CHK_LAMP,            "LAMP" },
	{ ID_CHK_REPEAT,          "REPEAT" },
	{ ID_CHK_TRIG_REV,        "TRIG_REV" },
	{ ID_CHK_LOCO_FIRST,      "LOCO_FIRST" },
	{ ID_CHK_NO_ABORT,        "NO_ABORT" },
	{ ID_CHK_LATE_SCOPE,      "LATE_SCOPE" },
	{ ID_CHK_FRAGMENT,        "FRAGMENT" },
	{ ID_CHK_SAME_AMBIG_ADDR, "SAME_AMBIG_ADDR" },
	{ ID_CHK_KICK_START,      "KICK_START" },
	{ ID_CHK_MCU_BYPASS_TST,  "MCU_BYPASS_TST" },
};

/*---------------------------------------------------------------------------
 * Small helpers.
 *-------------------------------------------------------------------------*/
static HWND
MakeStatic( HWND parent, const char *text, int x, int y, int w, int h )
{
	return CreateWindowEx( 0, "STATIC", text, WS_CHILD | WS_VISIBLE,
		x, y, w, h, parent, NULL, NULL, NULL );
}

static HWND
MakeEdit( HWND parent, int id, int x, int y, int w, int h )
{
	return CreateWindowEx( WS_EX_CLIENTEDGE, "EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
		x, y, w, h, parent, (HMENU)(INT_PTR)id, NULL, NULL );
}

static HWND
MakeCheck( HWND parent, int id, const char *text, int x, int y, int w, int h )
{
	return CreateWindowEx( 0, "BUTTON", text,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
		x, y, w, h, parent, (HMENU)(INT_PTR)id, NULL, NULL );
}

static HWND
MakeButton( HWND parent, int id, const char *text, int x, int y, int w, int h )
{
	return CreateWindowEx( 0, "BUTTON", text,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
		x, y, w, h, parent, (HMENU)(INT_PTR)id, NULL, NULL );
}

static void
SetEditText( int id, const char *text )
{
	SetDlgItemText( g_hMain, id, text );
}

static void
SetEditUInt( int id, unsigned long value, bool hex )
{
	char buf[32];
	if ( hex )
	{
		sprintf( buf, "0x%08lX", value );
	}
	else
	{
		sprintf( buf, "%lu", value );
	}
	SetEditText( id, buf );
}

static void
SetMaskText( int pageId, int generalId, const char *text )
{
	SetEditText( pageId, text );
	SetEditText( generalId, text );
}

static void
SetMaskUInt( int pageId, int generalId, unsigned long value )
{
	SetEditUInt( pageId, value, true );
	SetEditUInt( generalId, value, true );
}

static std::string
GetEditText( int id )
{
	char buf[256];
	GetDlgItemText( g_hMain, id, buf, sizeof( buf ) );
	return std::string( buf );
}

static unsigned long
GetEditULong( int id )
{
	std::string s = GetEditText( id );
	return strtoul( s.c_str(), NULL, 0 );
}

static void
SetChecked( int id, bool checked )
{
	CheckDlgButton( g_hMain, id, checked ? BST_CHECKED : BST_UNCHECKED );
}

static bool
IsChecked( int id )
{
	return IsDlgButtonChecked( g_hMain, id ) == BST_CHECKED;
}

static void
Trim( std::string &s )
{
	size_t a = s.find_first_not_of( " \t\r\n" );
	size_t b = s.find_last_not_of( " \t\r\n" );
	if ( a == std::string::npos )
	{
		s.clear();
	}
	else
	{
		s = s.substr( a, b - a + 1 );
	}
}

/* Pull the leading key token out of a cfg line, whether or not it is
 * commented out with ';' or '#', so a saved field can be matched back to
 * the line it originally came from. Returns false if the line has no key
 * (blank or free-form comment text). */
static bool
ExtractLineKey( const std::string &rawLine, std::string &keyOut )
{
	std::string s( rawLine );
	Trim( s );
	if ( !s.empty() && ( s[0] == ';' || s[0] == '#' ) )
	{
		s = s.substr( 1 );
		Trim( s );
	}
	if ( s.empty() )
	{
		return false;
	}
	size_t sp = s.find_first_of( " \t" );
	keyOut = ( sp == std::string::npos ) ? s : s.substr( 0, sp );
	return !keyOut.empty();
}

static std::string
PreserveHashComment( const std::string &updatedLine, const std::string &originalLine )
{
	size_t hashPos = originalLine.find( '#' );
	if ( hashPos == std::string::npos )
	{
		return updatedLine;
	}

	size_t firstText = originalLine.find_first_not_of( " \t" );
	if ( firstText != std::string::npos && originalLine[firstText] == '#' )
	{
		return updatedLine;
	}

	std::string line( updatedLine );
	size_t updatedHashPos = line.find( '#' );
	if ( updatedHashPos != std::string::npos )
	{
		line.erase( updatedHashPos );
	}
	while ( !line.empty() && ( line.back() == ' ' || line.back() == '\t' ) )
	{
		line.pop_back();
	}
	if ( line.size() < 24 )
	{
		line.append( 24 - line.size(), ' ' );
	}
	return line + originalLine.substr( hashPos );
}

/*---------------------------------------------------------------------------
 * Enumerate the serial ports currently present on the PC (registry list of
 * ports Windows has assigned a COMx name to).
 *-------------------------------------------------------------------------*/
static std::vector<std::string>
EnumComPorts( void )
{
	std::vector<std::string> ports;
	HKEY hKey;

	if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM",
		0, KEY_READ, &hKey ) == ERROR_SUCCESS )
	{
		for ( DWORD index = 0; ; index++ )
		{
			char valueName[256];
			char data[256];
			DWORD valueNameSize = sizeof( valueName );
			DWORD dataSize = sizeof( data );
			DWORD type;

			if ( RegEnumValue( hKey, index, valueName, &valueNameSize, NULL,
				&type, (BYTE *)data, &dataSize ) != ERROR_SUCCESS )
			{
				break;
			}

			if ( type == REG_SZ )
			{
				ports.push_back( std::string( data ) );
			}
		}
		RegCloseKey( hKey );
	}

	std::sort( ports.begin(), ports.end(), []( const std::string &a, const std::string &b )
	{
		return atoi( a.c_str() + 3 ) < atoi( b.c_str() + 3 );
	} );

	return ports;
}

/*---------------------------------------------------------------------------
 * Keep the TESTS mask hex field and the per-bit checkboxes in sync.
 *-------------------------------------------------------------------------*/
static void
SyncBitsFromTests( void )
{
	unsigned long mask = GetEditULong( ID_EDIT_TESTS );
	for ( int i = 0; i < TESTBIT_COUNT; i++ )
	{
		SetChecked( ID_CHK_TESTBIT_BASE + i, ( mask & ( 1UL << i ) ) != 0 );
	}
}

static void
SyncTestsFromBits( void )
{
	unsigned long mask = 0;
	for ( int i = 0; i < TESTBIT_COUNT; i++ )
	{
		if ( IsChecked( ID_CHK_TESTBIT_BASE + i ) )
		{
			mask |= ( 1UL << i );
		}
	}
	SetMaskUInt( ID_EDIT_TESTS, ID_EDIT_TESTS_GENERAL, mask );
}

/*---------------------------------------------------------------------------
 * Keep the CLOCKS mask hex field and the per-bit checkboxes in sync.
 *-------------------------------------------------------------------------*/
static void
SyncBitsFromClocks( void )
{
	unsigned long mask = GetEditULong( ID_EDIT_CLOCKS );
	for ( int i = 0; i < CLOCKBIT_COUNT; i++ )
	{
		SetChecked( ID_CHK_CLOCKBIT_BASE + i, ( mask & ( 1UL << i ) ) != 0 );
	}
}

static void
SyncClocksFromBits( void )
{
	unsigned long mask = 0;
	for ( int i = 0; i < CLOCKBIT_COUNT; i++ )
	{
		if ( IsChecked( ID_CHK_CLOCKBIT_BASE + i ) )
		{
			mask |= ( 1UL << i );
		}
	}
	SetMaskUInt( ID_EDIT_CLOCKS, ID_EDIT_CLOCKS_GENERAL, mask );
}

/*---------------------------------------------------------------------------
 * Keep the FUNCS mask hex field and the per-bit checkboxes in sync.
 *-------------------------------------------------------------------------*/
static void
SyncBitsFromFuncs( void )
{
	unsigned long mask = GetEditULong( ID_EDIT_FUNCS );
	for ( int i = 0; i < FUNCBIT_COUNT; i++ )
	{
		SetChecked( ID_CHK_FUNCBIT_BASE + i, ( mask & ( 1UL << i ) ) != 0 );
	}
}

static void
SyncFuncsFromBits( void )
{
	unsigned long mask = 0;
	for ( int i = 0; i < FUNCBIT_COUNT; i++ )
	{
		if ( IsChecked( ID_CHK_FUNCBIT_BASE + i ) )
		{
			mask |= ( 1UL << i );
		}
	}
	SetMaskUInt( ID_EDIT_FUNCS, ID_EDIT_FUNCS_GENERAL, mask );
}

static void
SyncBitsFromLog( void )
{
	unsigned long mask = GetEditULong( ID_EDIT_LOG );
	for ( int i = 0; i < LOGBIT_COUNT; i++ )
	{
		SetChecked( ID_CHK_LOGBIT_BASE + i, ( mask & ( 1UL << i ) ) != 0 );
	}
}

static void
SyncLogFromBits( void )
{
	unsigned long mask = 0;
	for ( int i = 0; i < LOGBIT_COUNT; i++ )
	{
		if ( IsChecked( ID_CHK_LOGBIT_BASE + i ) )
		{
			mask |= ( 1UL << i );
		}
	}
	SetMaskUInt( ID_EDIT_LOG, ID_EDIT_LOG_GENERAL, mask );
}

/* Default masks: TESTS valid bits are 0-19 (20-31 reserved), CLOCKS valid bits are 0-16 (17-31 reserved). */
static const unsigned long DEF_TESTS_MASK = 0x000FFFFFUL;
static const unsigned long DEF_CLOCKS_MASK = 0xFFFFFFFFUL & ~0x8 & ~0x10 & ~0x100 & ~0x200 & 0x0001FFFFUL;

static void
ResetTestsMaskDefault( void )
{
	SetMaskUInt( ID_EDIT_TESTS, ID_EDIT_TESTS_GENERAL, DEF_TESTS_MASK );
	SyncBitsFromTests();
}

static void
ResetClocksMaskDefault( void )
{
	SetMaskUInt( ID_EDIT_CLOCKS, ID_EDIT_CLOCKS_GENERAL, DEF_CLOCKS_MASK );
	SyncBitsFromClocks();
}

static void
ResetFuncsMaskDefault( void )
{
	SetMaskUInt( ID_EDIT_FUNCS, ID_EDIT_FUNCS_GENERAL, 0x1F );
	SyncBitsFromFuncs();
}

static void
ResetLogMaskDefault( void )
{
	SetMaskUInt( ID_EDIT_LOG, ID_EDIT_LOG_GENERAL, 0x01 );
	SyncBitsFromLog();
}

/*---------------------------------------------------------------------------
 * Default program-defined values (mirrors ARGS.CPP Args_obj defaults).
 *-------------------------------------------------------------------------*/
static void
LoadDefaults( void )
{
	SetEditUInt( ID_EDIT_ADDRESS, 3, false );
	SendDlgItemMessage( g_hMain, ID_COMBO_TYPE, CB_SETCURSEL, 0, 0 ); // LOCO
	SetEditUInt( ID_EDIT_PRESET, 0, false );
	SetEditUInt( ID_EDIT_TRIGGER, 8, false );
	SetEditUInt( ID_EDIT_EXTRA_PRE, 0, false );
	SetEditUInt( ID_EDIT_FILL_MSEC, 1000, false );
	SetEditUInt( ID_EDIT_TEST_REPS, 2, false );
	SetEditUInt( ID_EDIT_ACC_PAIR, 1, false );
	SetEditUInt( ID_EDIT_EXTRA_TRG, 0, false );
	SetEditText( ID_EDIT_MCU_PORT, "COM3" );
	SetEditUInt( ID_EDIT_MCU_BAUD, 115200, false );
	SetMaskUInt( ID_EDIT_TESTS, ID_EDIT_TESTS_GENERAL, DEF_TESTS_MASK );
	SetMaskUInt( ID_EDIT_CLOCKS, ID_EDIT_CLOCKS_GENERAL, DEF_CLOCKS_MASK );
	SetMaskUInt( ID_EDIT_FUNCS, ID_EDIT_FUNCS_GENERAL, 0x1F );
	SetMaskUInt( ID_EDIT_LOG, ID_EDIT_LOG_GENERAL, 0x01 );

	for ( size_t i = 0; i < _countof( g_flagCtrls ); i++ )
	{
		SetChecked( g_flagCtrls[i].ctrlId, false );
	}

	SyncBitsFromTests();
	SyncBitsFromClocks();
	SyncBitsFromFuncs();
	SyncBitsFromLog();
}

/* True for legacy .ini files, which have no LOG mask of their own. */
static bool
IsIniPath( const char *path )
{
	const char *dot = strrchr( path, '.' );
	return dot != NULL && _stricmp( dot, ".ini" ) == 0;
}

/*---------------------------------------------------------------------------
 * Load/parse a SEND.CFG style file into the controls.
 *-------------------------------------------------------------------------*/
static bool
LoadCfgFile( const char *path )
{
	FILE *fp = fopen( path, "r" );
	if ( fp == NULL )
	{
		return false;
	}

	/* Keep the raw lines around so a later Save can patch just the changed
	 * fields back into this file's original layout/comments. */
	g_origLines.clear();
	g_iniSource = IsIniPath( path );
	char line[256];
	while ( fgets( line, sizeof( line ), fp ) )
	{
		std::string raw( line );
		while ( !raw.empty() && ( raw.back() == '\n' || raw.back() == '\r' ) )
		{
			raw.pop_back();
		}
		g_origLines.push_back( raw );
	}
	fclose( fp );

	LoadDefaults();
	bool logPkts = false;

	for ( size_t li = 0; li < g_origLines.size(); li++ )
	{
		std::string s( g_origLines[li] );
		Trim( s );

		if ( s.empty() || s[0] == '#' || s[0] == ';' )
		{
			continue;
		}

		size_t sp = s.find_first_of( " \t" );
		std::string key = ( sp == std::string::npos ) ? s : s.substr( 0, sp );
		std::string val;
		if ( sp != std::string::npos )
		{
			val = s.substr( sp + 1 );
			Trim( val );
			// Strip trailing inline comment.
			size_t cpos = val.find_first_of( "#;" );
			if ( cpos != std::string::npos )
			{
				val = val.substr( 0, cpos );
				Trim( val );
			}
		}

		bool matchedFlag = false;
		if ( _stricmp( key.c_str(), "LOG_PKTS" ) == 0 )
		{
			logPkts = val.empty() || ( _stricmp( val.c_str(), "0" ) != 0
				&& _stricmp( val.c_str(), "false" ) != 0
				&& _stricmp( val.c_str(), "off" ) != 0
				&& _stricmp( val.c_str(), "no" ) != 0 );
			continue;
		}
		for ( size_t i = 0; i < _countof( g_flagCtrls ); i++ )
		{
			if ( _stricmp( key.c_str(), g_flagCtrls[i].key ) == 0 )
			{
				SetChecked( g_flagCtrls[i].ctrlId, true );
				matchedFlag = true;
				break;
			}
		}
		if ( matchedFlag )
		{
			continue;
		}

		if ( _stricmp( key.c_str(), "ADDRESS" ) == 0 )
		{
			SetEditText( ID_EDIT_ADDRESS, val.c_str() );
		}
		else if ( _stricmp( key.c_str(), "TYPE" ) == 0 )
		{
			int sel = 0;
			if ( !val.empty() )
			{
				switch ( tolower( val[0] ) )
				{
					case 'l': sel = 0; break;
					case 'f': sel = 1; break;
					case 'a': sel = 2; break;
					case 's': sel = 3; break;
				}
			}
			SendDlgItemMessage( g_hMain, ID_COMBO_TYPE, CB_SETCURSEL, sel, 0 );
		}
		else if ( _stricmp( key.c_str(), "PRESET" ) == 0 )
		{
			SetEditText( ID_EDIT_PRESET, val.c_str() );
		}
		else if ( _stricmp( key.c_str(), "TRIGGER" ) == 0 )
		{
			SetEditText( ID_EDIT_TRIGGER, val.c_str() );
		}
		else if ( _stricmp( key.c_str(), "TESTS" ) == 0 )
		{
			SetMaskText( ID_EDIT_TESTS, ID_EDIT_TESTS_GENERAL, val.c_str() );
		}
		else if ( _stricmp( key.c_str(), "CLOCKS" ) == 0 )
		{
			SetMaskText( ID_EDIT_CLOCKS, ID_EDIT_CLOCKS_GENERAL, val.c_str() );
		}
		else if ( _stricmp( key.c_str(), "FUNCS" ) == 0 )
		{
			SetMaskText( ID_EDIT_FUNCS, ID_EDIT_FUNCS_GENERAL, val.c_str() );
		}
		else if ( _stricmp( key.c_str(), "LOG" ) == 0 )
		{
			unsigned long mask = val.empty() ? 0x01 : strtoul( val.c_str(), NULL, 0 );
			SetMaskUInt( ID_EDIT_LOG, ID_EDIT_LOG_GENERAL, mask );
		}
		else if ( _stricmp( key.c_str(), "EXTRA_PRE" ) == 0 )
		{
			SetEditText( ID_EDIT_EXTRA_PRE, val.c_str() );
		}
		else if ( _stricmp( key.c_str(), "FILL_MSEC" ) == 0 )
		{
			SetEditText( ID_EDIT_FILL_MSEC, val.c_str() );
		}
		else if ( _stricmp( key.c_str(), "TEST_REPS" ) == 0 )
		{
			SetEditText( ID_EDIT_TEST_REPS, val.c_str() );
		}
		else if ( _stricmp( key.c_str(), "ACC_PAIR" ) == 0 )
		{
			SetEditText( ID_EDIT_ACC_PAIR, val.c_str() );
		}
		else if ( _stricmp( key.c_str(), "EXTRA_TRG" ) == 0 )
		{
			SetEditText( ID_EDIT_EXTRA_TRG, val.c_str() );
		}
		else if ( _stricmp( key.c_str(), "MCU_PORT" ) == 0 )
		{
			SetEditText( ID_EDIT_MCU_PORT, val.c_str() );
		}
		else if ( _stricmp( key.c_str(), "MCU_BAUD" ) == 0 )
		{
			SetEditText( ID_EDIT_MCU_BAUD, val.c_str() );
		}
		/* Unknown keys are silently ignored so newer cfg files still load. */
	}

	/* Legacy .ini files have no LOG mask; derive it from LOG_PKTS. */
	if ( IsIniPath( path ) )
	{
		SetMaskUInt( ID_EDIT_LOG, ID_EDIT_LOG_GENERAL, logPkts ? 0x07 : 0x01 );
	}

	SyncBitsFromTests();
	SyncBitsFromClocks();
	SyncBitsFromFuncs();
	SyncBitsFromLog();
	return true;
}

/*---------------------------------------------------------------------------
 * Write the controls out in SEND.CFG format. If a cfg file was previously
 * loaded (or saved), its original lines/comments/ordering are preserved and
 * only the value lines for known keys are patched in place; otherwise a
 * fresh, fully formatted file is generated from scratch.
 *-------------------------------------------------------------------------*/
static bool
SaveCfgFile( const char *path )
{
	std::vector<std::pair<std::string, std::string>> fields;
	char buf[512];

	auto addFlag = [&]( const char *key, bool checked )
	{
		if ( checked )
		{
			snprintf( buf, sizeof( buf ), "%-16s", key );
		}
		else
		{
			snprintf( buf, sizeof( buf ), ";%-16s", key );
		}
		fields.push_back( { key, buf } );
	};

	auto addField = [&]( const char *key, const std::string &val )
	{
		int valueStart = 20 - (int)val.size();
		if ( valueStart < (int)strlen( key ) + 1 )
		{
			valueStart = (int)strlen( key ) + 1;
		}
		snprintf( buf, sizeof( buf ), "%s%*s%s", key,
			valueStart - (int)strlen( key ), "", val.c_str() );
		fields.push_back( { key, buf } );
	};

	auto addMaskField = [&]( const char *key, const std::string &val )
	{
		unsigned long mask = strtoul( val.c_str(), NULL, 16 );
		char maskText[32];
		snprintf( maskText, sizeof( maskText ), "0x%08lx", mask );
		int valueStart = 20 - (int)strlen( maskText );
		if ( valueStart < (int)strlen( key ) + 1 )
		{
			valueStart = (int)strlen( key ) + 1;
		}
		snprintf( buf, sizeof( buf ), "%s%*s%s", key,
			valueStart - (int)strlen( key ), "", maskText );
		fields.push_back( { key, buf } );
	};

	addFlag( "MANUAL", IsChecked( ID_CHK_MANUAL ) );

	int typeSel = (int)SendDlgItemMessage( g_hMain, ID_COMBO_TYPE, CB_GETCURSEL, 0, 0 );
	const char *typeChar = "L";
	switch ( typeSel )
	{
		case 0: typeChar = "L"; break;
		case 1: typeChar = "F"; break;
		case 2: typeChar = "A"; break;
		case 3: typeChar = "S"; break;
	}

	addField( "ADDRESS", GetEditText( ID_EDIT_ADDRESS ) );
	addField( "TYPE", typeChar );
	addFlag( "LAMP", IsChecked( ID_CHK_LAMP ) );
	addField( "PRESET", GetEditText( ID_EDIT_PRESET ) );
	addField( "TRIGGER", GetEditText( ID_EDIT_TRIGGER ) );
	addFlag( "REPEAT", IsChecked( ID_CHK_REPEAT ) );
	addMaskField( "TESTS", GetEditText( ID_EDIT_TESTS ) );
	addMaskField( "CLOCKS", GetEditText( ID_EDIT_CLOCKS ) );
	addMaskField( "FUNCS", GetEditText( ID_EDIT_FUNCS ) );
	addMaskField( "LOG", GetEditText( ID_EDIT_LOG ) );
	addField( "EXTRA_PRE", GetEditText( ID_EDIT_EXTRA_PRE ) );
	addFlag( "TRIG_REV", IsChecked( ID_CHK_TRIG_REV ) );
	addFlag( "LOCO_FIRST", IsChecked( ID_CHK_LOCO_FIRST ) );
	addField( "FILL_MSEC", GetEditText( ID_EDIT_FILL_MSEC ) );
	addField( "TEST_REPS", GetEditText( ID_EDIT_TEST_REPS ) );
	addFlag( "NO_ABORT", IsChecked( ID_CHK_NO_ABORT ) );
	addFlag( "LATE_SCOPE", IsChecked( ID_CHK_LATE_SCOPE ) );
	addFlag( "FRAGMENT", IsChecked( ID_CHK_FRAGMENT ) );
	addFlag( "SAME_AMBIG_ADDR", IsChecked( ID_CHK_SAME_AMBIG_ADDR ) );
	addField( "ACC_PAIR", GetEditText( ID_EDIT_ACC_PAIR ) );
	addFlag( "KICK_START", IsChecked( ID_CHK_KICK_START ) );
	addField( "EXTRA_TRG", GetEditText( ID_EDIT_EXTRA_TRG ) );
	addFlag( "MCU_BYPASS_TST", IsChecked( ID_CHK_MCU_BYPASS_TST ) );
	addField( "MCU_PORT", GetEditText( ID_EDIT_MCU_PORT ) );
	addField( "MCU_BAUD", GetEditText( ID_EDIT_MCU_BAUD ) );

	std::vector<std::string> output;

	if ( g_origLines.empty() )
	{
		/* No previously loaded/saved file to preserve - emit a fresh file. */
		output.push_back( "; SEND.CFG - generated by sendgui" );
		output.push_back( ";" );
		output.push_back( "; Flags below are active (uncommented) only when checked." );
		output.push_back( ";" );
		output.push_back( "" );
		for ( size_t i = 0; i < fields.size(); i++ )
		{
			output.push_back( fields[i].second );
		}
	}
	else
	{
		/* Patch each known key's line in place; leave everything else (other
		 * lines, comments, ordering) untouched. Keys not already present are
		 * appended at the end. */
		output = g_origLines;
		for ( size_t i = 0; i < fields.size(); i++ )
		{
			bool found = false;
			for ( size_t li = 0; li < output.size(); li++ )
			{
				std::string existingKey;
				if ( ExtractLineKey( output[li], existingKey ) &&
					_stricmp( existingKey.c_str(), fields[i].first.c_str() ) == 0 )
				{
					output[li] = PreserveHashComment( fields[i].second, output[li] );
					found = true;
					break;
				}
			}
			if ( !found )
			{
				output.push_back( fields[i].second );
			}
		}
		for ( size_t li = 0; li < output.size(); li++ )
		{
			std::string existingKey;
			if ( output[li].find( '#' ) != std::string::npos )
			{
				output[li] = PreserveHashComment( output[li], output[li] );
			}
		}
	}

	FILE *fp = fopen( path, "w" );
	if ( fp == NULL )
	{
		return false;
	}
	for ( size_t i = 0; i < output.size(); i++ )
	{
		fprintf( fp, "%s\n", output[i].c_str() );
	}
	fclose( fp );

	g_origLines = output;
	return true;
}

static void
UpdatePathLabel( void )
{
	std::string text = "Cfg file: ";
	text += ( g_cfgPath[0] != '\0' ) ? g_cfgPath : "(none - use Load or Save As)";
	SetDlgItemText( g_hMain, ID_STATIC_PATH, text.c_str() );
}

static bool
BrowseOpen( char *outPath, DWORD outSize )
{
	OPENFILENAME ofn;
	char file[MAX_PATH] = "SEND.CFG";

	ZeroMemory( &ofn, sizeof( ofn ) );
	ofn.lStructSize     = sizeof( ofn );
	ofn.hwndOwner       = g_hMain;
	ofn.lpstrFilter     = "Config Files (*.cfg;*.ini)\0*.cfg;*.ini\0All Files\0*.*\0";
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof( file );
	ofn.lpstrDefExt     = "cfg";
	ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if ( GetOpenFileName( &ofn ) )
	{
		strncpy( outPath, file, outSize - 1 );
		outPath[outSize - 1] = '\0';
		return true;
	}
	return false;
}

static bool
BrowseSave( char *outPath, DWORD outSize )
{
	OPENFILENAME ofn;
	char file[MAX_PATH];

	/* An .ini source may only be saved as .ini; anything else only as .cfg. */
	strcpy( file, g_iniSource ? "SEND.INI" : "SEND.CFG" );

	ZeroMemory( &ofn, sizeof( ofn ) );
	ofn.lStructSize     = sizeof( ofn );
	ofn.hwndOwner       = g_hMain;
	ofn.lpstrFilter     = g_iniSource
		? "Ini Files (*.ini)\0*.ini\0"
		: "Config Files (*.cfg)\0*.cfg\0All Files\0*.*\0";
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof( file );
	ofn.lpstrDefExt     = g_iniSource ? "ini" : "cfg";
	ofn.Flags           = OFN_OVERWRITEPROMPT;

	while ( GetSaveFileName( &ofn ) )
	{
		if ( IsIniPath( file ) != g_iniSource )
		{
			MessageBox( g_hMain, g_iniSource
				? "A configuration read from an .ini file can only be saved as an .ini file."
				: "Only a configuration read from an .ini file can be saved as an .ini file.",
				"sendgui", MB_ICONWARNING );
			continue;
		}
		strncpy( outPath, file, outSize - 1 );
		outPath[outSize - 1] = '\0';
		return true;
	}
	return false;
}

/*---------------------------------------------------------------------------
 * Migrate the current settings, read from an .ini file, to a .cfg file with
 * the same base name (send.ini becomes migrated_send.cfg). The send.cfg in
 * the program's home directory is the template; it is never overwritten.
 *-------------------------------------------------------------------------*/
static bool
MigrateIniToCfg( HWND hwnd )
{
	std::string iniPath = g_cfgPath;
	size_t slash = iniPath.find_last_of( "\\/" );
	std::string dir = ( slash == std::string::npos ) ? "" : iniPath.substr( 0, slash + 1 );
	std::string name = iniPath.substr( slash == std::string::npos ? 0 : slash + 1 );
	name = name.substr( 0, name.size() - 4 );
	if ( _stricmp( name.c_str(), "send" ) == 0 )
	{
		name = "migrated_send";
	}
	std::string cfgPath = dir + name + ".cfg";

	/* The template is always the send.cfg beside sendgui.exe; it is only read. */
	char exePath[MAX_PATH];
	GetModuleFileName( NULL, exePath, MAX_PATH );
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

	FILE *fp = fopen( basePath.c_str(), "r" );
	if ( fp == NULL )
	{
		std::string msg = "Baseline file not found:\n" + basePath;
		MessageBox( hwnd, msg.c_str(), "sendgui", MB_ICONERROR );
		return false;
	}
	std::vector<std::string> baseline;
	char line[256];
	while ( fgets( line, sizeof( line ), fp ) )
	{
		std::string raw( line );
		while ( !raw.empty() && ( raw.back() == '\n' || raw.back() == '\r' ) )
		{
			raw.pop_back();
		}
		baseline.push_back( raw );
	}
	fclose( fp );

	if ( GetFileAttributes( cfgPath.c_str() ) != INVALID_FILE_ATTRIBUTES )
	{
		std::string msg = cfgPath + " already exists.\nOverwrite it?";
		if ( MessageBox( hwnd, msg.c_str(), "sendgui",
			MB_YESNO | MB_ICONQUESTION ) != IDYES )
		{
			return false;
		}
	}

	std::vector<std::string> savedLines = g_origLines;
	g_origLines = baseline;
	if ( !SaveCfgFile( cfgPath.c_str() ) )
	{
		g_origLines = savedLines;
		MessageBox( hwnd, "Failed to save the file.", "sendgui", MB_ICONERROR );
		return false;
	}

	strncpy( g_cfgPath, cfgPath.c_str(), sizeof( g_cfgPath ) - 1 );
	g_cfgPath[sizeof( g_cfgPath ) - 1] = '\0';
	g_iniSource = false;
	UpdatePathLabel();
	return true;
}

/* Returns IDYES to save as .ini, IDNO to migrate to .cfg, else IDCANCEL. */
static int
AskIniSaveMode( HWND hwnd )
{
	return MessageBox( hwnd,
		"This configuration was read from an .ini file.\n\n"
		"Yes: save as the modified .ini file\n"
		"No: migrate to a .cfg file with the same name",
		"sendgui", MB_YESNOCANCEL | MB_ICONQUESTION );
}

/*---------------------------------------------------------------------------
 * Save the current settings (prompting for a path if needed) and launch
 * send.exe against them via the SEND_CFG environment variable, since send.exe
 * only auto-loads a literal "SEND.CFG" from its own directory otherwise.
 *-------------------------------------------------------------------------*/
static void
LaunchSendExe( HWND hwnd )
{
	if ( g_cfgPath[0] == '\0' )
	{
		char path[MAX_PATH];
		if ( !BrowseSave( path, sizeof( path ) ) )
		{
			return;
		}
		strncpy( g_cfgPath, path, sizeof( g_cfgPath ) - 1 );
		UpdatePathLabel();
	}

	if ( !SaveCfgFile( g_cfgPath ) )
	{
		MessageBox( hwnd, "Failed to save the file before launching.",
			"sendgui", MB_ICONERROR );
		return;
	}

	char exeDir[MAX_PATH];
	GetModuleFileName( NULL, exeDir, MAX_PATH );
	char *slash = strrchr( exeDir, '\\' );
	if ( slash != NULL )
	{
		slash[1] = '\0';
	}
	else
	{
		exeDir[0] = '\0';
	}

	std::string sendExe = std::string( exeDir ) + "send.exe";
	if ( GetFileAttributes( sendExe.c_str() ) == INVALID_FILE_ATTRIBUTES )
	{
		MessageBox( hwnd, "Could not find send.exe next to sendgui.exe.",
			"sendgui", MB_ICONERROR );
		return;
	}

	SetEnvironmentVariable( "SEND_CFG", g_cfgPath );

	char cmdLine[MAX_PATH + 16];
	sprintf( cmdLine, "\"%s\"", sendExe.c_str() );

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof( si ) );
	si.cb = sizeof( si );

	/* Run in the cfg file's directory so the .log/.sum files land beside it. */
	char workDir[MAX_PATH];
	strncpy( workDir, g_cfgPath, sizeof( workDir ) - 1 );
	workDir[sizeof( workDir ) - 1] = '\0';
	char *cfgSlash = strrchr( workDir, '\\' );
	if ( cfgSlash == NULL )
	{
		cfgSlash = strrchr( workDir, '/' );
	}
	if ( cfgSlash != NULL )
	{
		*cfgSlash = '\0';
	}
	else
	{
		strcpy( workDir, exeDir );
	}

	if ( CreateProcess( sendExe.c_str(), cmdLine, NULL, NULL, FALSE, 0,
		NULL, workDir, &si, &pi ) )
	{
		CloseHandle( pi.hThread );
		CloseHandle( pi.hProcess );
	}
	else
	{
		MessageBox( hwnd, "Failed to launch send.exe.", "sendgui", MB_ICONERROR );
	}
}

static void
ReadMcuSmokeTestOutput( void )
{
	if ( g_mcuTestOutputRead == NULL )
	{
		return;
	}

	DWORD bytesAvailable = 0;
	if ( !PeekNamedPipe( g_mcuTestOutputRead, NULL, 0, NULL,
		&bytesAvailable, NULL ) )
	{
		return;
	}

	while ( bytesAvailable > 0 )
	{
		char buffer[1024];
		DWORD bytesRead = 0;
		DWORD bytesToRead = bytesAvailable < sizeof( buffer )
			? bytesAvailable : sizeof( buffer );
		if ( !ReadFile( g_mcuTestOutputRead, buffer, bytesToRead,
			&bytesRead, NULL ) || bytesRead == 0 )
		{
			break;
		}

		if ( g_mcuTestOutput.size() < 8192 )
		{
			size_t keep = 8192 - g_mcuTestOutput.size();
			g_mcuTestOutput.append( buffer,
				bytesRead < keep ? bytesRead : keep );
		}

		if ( !PeekNamedPipe( g_mcuTestOutputRead, NULL, 0, NULL,
			&bytesAvailable, NULL ) )
		{
			break;
		}
	}
}

static std::string
GetMcuFirmwareVersion( void )
{
	const char *versionLabel = "mcu fw version";
	size_t labelPos = g_mcuTestOutput.find( versionLabel );
	if ( labelPos == std::string::npos )
	{
		return "";
	}

	size_t versionStart = g_mcuTestOutput.find( ':', labelPos );
	if ( versionStart == std::string::npos )
	{
		return "";
	}
	versionStart = g_mcuTestOutput.find_first_not_of( " \t",
		versionStart + 1 );
	if ( versionStart == std::string::npos )
	{
		return "";
	}

	size_t versionEnd = g_mcuTestOutput.find_first_of( " \r\n", versionStart );
	return g_mcuTestOutput.substr( versionStart, versionEnd - versionStart );
}

static void
FinishMcuSmokeTest( HWND hwnd, bool passed )
{
	if ( g_mcuTestProcess != NULL )
	{
		CloseHandle( g_mcuTestProcess );
		g_mcuTestProcess = NULL;
	}
	if ( g_mcuTestOutputRead != NULL )
	{
		CloseHandle( g_mcuTestOutputRead );
		g_mcuTestOutputRead = NULL;
	}
	KillTimer( hwnd, MCU_TEST_TIMER_ID );
	EnableWindow( GetDlgItem( hwnd, ID_BTN_MCU_TEST ), TRUE );
	if ( passed )
	{
		std::string version = GetMcuFirmwareVersion();
		std::string status = "MCU test: PASS";
		if ( version.empty() )
		{
			status += " - MCU version unavailable";
		}
		else
		{
			status += " - MCU ";
			status += version;
		}
		SetDlgItemText( hwnd, ID_STATIC_MCU_STATUS, status.c_str() );
	}
	else
	{
		SetDlgItemText( hwnd, ID_STATIC_MCU_STATUS, "MCU test: FAIL" );
	}
	g_mcuTestOutput.clear();
}

static void
LaunchMcuSmokeTest( HWND hwnd )
{
	if ( g_mcuTestProcess != NULL )
	{
		return;
	}

	char port[MAX_PATH];
	GetDlgItemText( hwnd, ID_EDIT_MCU_PORT, port, sizeof( port ) );
	std::string portName( port );
	Trim( portName );
	if ( portName.empty() || portName.find_first_of( "\"\r\n" ) != std::string::npos )
	{
		SetDlgItemText( hwnd, ID_STATIC_MCU_STATUS, "MCU test: FAIL (invalid port)" );
		return;
	}

	char exePath[MAX_PATH];
	DWORD pathLength = GetModuleFileName( NULL, exePath, MAX_PATH );
	if ( pathLength == 0 || pathLength >= MAX_PATH )
	{
		SetDlgItemText( hwnd, ID_STATIC_MCU_STATUS, "MCU test: FAIL (cannot locate executable)" );
		return;
	}

	char *slash = strrchr( exePath, '\\' );
	if ( slash == NULL )
	{
		SetDlgItemText( hwnd, ID_STATIC_MCU_STATUS, "MCU test: FAIL (cannot locate executable)" );
		return;
	}
	slash[1] = '\0';

	std::string exeDir( exePath );
	std::string smokeExe = exeDir + "mcu_smoke_test.exe";
	if ( GetFileAttributes( smokeExe.c_str() ) == INVALID_FILE_ATTRIBUTES )
	{
		SetDlgItemText( hwnd, ID_STATIC_MCU_STATUS,
			"MCU test: FAIL (mcu_smoke_test.exe not found)" );
		return;
	}

	std::string commandLine = "\"" + smokeExe + "\" \"" + portName + "\"";
	std::vector<char> mutableCommandLine( commandLine.begin(), commandLine.end() );
	mutableCommandLine.push_back( '\0' );

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	SECURITY_ATTRIBUTES pipeSecurity;
	ZeroMemory( &pipeSecurity, sizeof( pipeSecurity ) );
	pipeSecurity.nLength = sizeof( pipeSecurity );
	pipeSecurity.bInheritHandle = TRUE;

	HANDLE outputRead = NULL;
	HANDLE outputWrite = NULL;
	if ( !CreatePipe( &outputRead, &outputWrite, &pipeSecurity, 0 )
		|| !SetHandleInformation( outputRead, HANDLE_FLAG_INHERIT, 0 ) )
	{
		if ( outputRead != NULL )
		{
			CloseHandle( outputRead );
		}
		if ( outputWrite != NULL )
		{
			CloseHandle( outputWrite );
		}
		SetDlgItemText( hwnd, ID_STATIC_MCU_STATUS,
			"MCU test: FAIL (could not capture test output)" );
		return;
	}

	ZeroMemory( &si, sizeof( si ) );
	ZeroMemory( &pi, sizeof( pi ) );
	si.cb = sizeof( si );
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = GetStdHandle( STD_INPUT_HANDLE );
	si.hStdOutput = outputWrite;
	si.hStdError = outputWrite;

	if ( !CreateProcess( smokeExe.c_str(), mutableCommandLine.data(),
		NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, exeDir.c_str(), &si, &pi ) )
	{
		CloseHandle( outputRead );
		CloseHandle( outputWrite );
		SetDlgItemText( hwnd, ID_STATIC_MCU_STATUS, "MCU test: FAIL (launch failed)" );
		return;
	}

	CloseHandle( outputWrite );
	CloseHandle( pi.hThread );
	g_mcuTestProcess = pi.hProcess;
	g_mcuTestOutputRead = outputRead;
	g_mcuTestOutput.clear();
	SetDlgItemText( hwnd, ID_STATIC_MCU_STATUS, "MCU test: running..." );
	EnableWindow( GetDlgItem( hwnd, ID_BTN_MCU_TEST ), FALSE );

	if ( SetTimer( hwnd, MCU_TEST_TIMER_ID, 100, NULL ) == 0 )
	{
		ReadMcuSmokeTestOutput();
		DWORD waitResult;
		do
		{
			waitResult = WaitForSingleObject( g_mcuTestProcess, 50 );
			ReadMcuSmokeTestOutput();
		} while ( waitResult == WAIT_TIMEOUT );

		DWORD exitCode = 1;
		bool passed = waitResult == WAIT_OBJECT_0
			&& GetExitCodeProcess( g_mcuTestProcess, &exitCode )
			&& exitCode == 0;
		FinishMcuSmokeTest( hwnd, passed );
	}
}

/*---------------------------------------------------------------------------
 * Layout.
 *-------------------------------------------------------------------------*/
static void
CreateControls( HWND hwnd )
{
	const int labelW = 110;
	const int editW  = 150;
	const int rowH   = 26;
	int y;

	/* Tab strip switches between the general options page and the
	 * individual TESTS/CLOCKS mask bit pages. */
	CreateWindowEx( 0, WC_TABCONTROL, "", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		10, 10, 1020, 30, hwnd, (HMENU)(INT_PTR)ID_TAB, NULL, NULL );
	HWND tab = GetDlgItem( hwnd, ID_TAB );

	TCITEM tie;
	tie.mask = TCIF_TEXT;
	tie.pszText = (char *)"General";
	TabCtrl_InsertItem( tab, 0, &tie );
	tie.pszText = (char *)"Test Mask Bits";
	TabCtrl_InsertItem( tab, 1, &tie );
	tie.pszText = (char *)"Clock Mask Bits";
	TabCtrl_InsertItem( tab, 2, &tie );
	tie.pszText = (char *)"Function Mask Bits";
	TabCtrl_InsertItem( tab, 3, &tie );
	tie.pszText = (char *)"Log Mask Bits";
	TabCtrl_InsertItem( tab, 4, &tie );

	const int pageTop = 50;	// Below the tab strip.

	/* Left column: values / masks. */
	int xLabel = 20;
	int xEdit  = xLabel + labelW + 8;

	y = pageTop;
	g_page0Ctrls.push_back( MakeStatic( hwnd, "Address:", xLabel, y, labelW, 18 ) );
	g_page0Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_ADDRESS, xEdit, y - 2, editW, 22 ) );
	y += rowH;

	g_page0Ctrls.push_back( MakeStatic( hwnd, "Decoder Type:", xLabel, y, labelW, 18 ) );
	HWND combo = CreateWindowEx( WS_EX_CLIENTEDGE, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
		xEdit, y - 2, editW, 200, hwnd, (HMENU)(INT_PTR)ID_COMBO_TYPE, NULL, NULL );
	SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)"LOCO (L)" );
	SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)"FUNC (F)" );
	SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)"ACC (A)" );
	SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)"SIG (S)" );
	g_page0Ctrls.push_back( combo );
	y += rowH;

	g_page0Ctrls.push_back( MakeStatic( hwnd, "Sig Preset:", xLabel, y, labelW, 18 ) );
	g_page0Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_PRESET, xEdit, y - 2, editW, 22 ) );
	y += rowH;

	g_page0Ctrls.push_back( MakeStatic( hwnd, "Sig Trigger:", xLabel, y, labelW, 18 ) );
	g_page0Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_TRIGGER, xEdit, y - 2, editW, 22 ) );
	y += rowH;

	g_page0Ctrls.push_back( MakeStatic( hwnd, "Extra Preamble:", xLabel, y, labelW, 18 ) );
	g_page0Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_EXTRA_PRE, xEdit, y - 2, editW, 22 ) );
	y += rowH;

	g_page0Ctrls.push_back( MakeStatic( hwnd, "Fill (msec):", xLabel, y, labelW, 18 ) );
	g_page0Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_FILL_MSEC, xEdit, y - 2, editW, 22 ) );
	y += rowH;

	g_page0Ctrls.push_back( MakeStatic( hwnd, "Test Repeats:", xLabel, y, labelW, 18 ) );
	g_page0Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_TEST_REPS, xEdit, y - 2, editW, 22 ) );
	y += rowH;

	g_page0Ctrls.push_back( MakeStatic( hwnd, "Acc. Pair (1-4):", xLabel, y, labelW, 18 ) );
	g_page0Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_ACC_PAIR, xEdit, y - 2, editW, 22 ) );
	y += rowH;

	g_page0Ctrls.push_back( MakeStatic( hwnd, "Extra Trig Pkts:", xLabel, y, labelW, 18 ) );
	g_page0Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_EXTRA_TRG, xEdit, y - 2, editW, 22 ) );
	y += rowH;

	g_page0Ctrls.push_back( MakeStatic( hwnd, "MCU Port:", xLabel, y, labelW, 18 ) );
	HWND portCombo = CreateWindowEx( WS_EX_CLIENTEDGE, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWN | CBS_AUTOHSCROLL,
		xEdit, y - 2, editW, 200, hwnd, (HMENU)(INT_PTR)ID_EDIT_MCU_PORT, NULL, NULL );
	std::vector<std::string> comPorts = EnumComPorts();
	for ( size_t i = 0; i < comPorts.size(); i++ )
	{
		SendMessage( portCombo, CB_ADDSTRING, 0, (LPARAM)comPorts[i].c_str() );
	}
	g_page0Ctrls.push_back( portCombo );
	y += rowH;

	g_page0Ctrls.push_back( MakeStatic( hwnd, "MCU Baud:", xLabel, y, labelW, 18 ) );
	HWND baudCombo = CreateWindowEx( WS_EX_CLIENTEDGE, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWN | CBS_AUTOHSCROLL,
		xEdit, y - 2, editW, 200, hwnd, (HMENU)(INT_PTR)ID_EDIT_MCU_BAUD, NULL, NULL );
	static const char *commonBauds[] =
	{
		"9600", "19200", "38400", "57600", "115200",
		"230400", "460800", "921600", "1000000", "2000000", "3000000"
	};
	for ( size_t i = 0; i < _countof( commonBauds ); i++ )
	{
		SendMessage( baudCombo, CB_ADDSTRING, 0, (LPARAM)commonBauds[i] );
	}
	g_page0Ctrls.push_back( baudCombo );
	y += rowH + 6;

	g_page0Ctrls.push_back( MakeStatic( hwnd, "Tests Mask:", xLabel, y, labelW, 18 ) );
	g_page0Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_TESTS_GENERAL, xEdit, y - 2, editW, 22 ) );
	y += rowH;

	g_page0Ctrls.push_back( MakeStatic( hwnd, "Clocks Mask:", xLabel, y, labelW, 18 ) );
	g_page0Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_CLOCKS_GENERAL, xEdit, y - 2, editW, 22 ) );
	y += rowH;

	g_page0Ctrls.push_back( MakeStatic( hwnd, "Funcs Mask:", xLabel, y, labelW, 18 ) );
	g_page0Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_FUNCS_GENERAL, xEdit, y - 2, editW, 22 ) );
	y += rowH;

	g_page0Ctrls.push_back( MakeStatic( hwnd, "Log Mask:", xLabel, y, labelW, 18 ) );
	g_page0Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_LOG_GENERAL, xEdit, y - 2, editW, 22 ) );
	y += rowH;

	/* Right column: boolean flag checkboxes. */
	int xChk = xEdit + editW + 40;
	int yChk = pageTop;
	const int chkW = 340;

	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_MANUAL, "MANUAL - Don't run tests automatically", xChk, yChk, chkW, 20 ) ); yChk += rowH;
	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_LAMP, "LAMP - Use rear lamp for func tests", xChk, yChk, chkW, 20 ) ); yChk += rowH;
	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_REPEAT, "REPEAT - Continuously repeat tests", xChk, yChk, chkW, 20 ) ); yChk += rowH;
	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_TRIG_REV, "TRIG_REV - Use reverse as trigger cmd", xChk, yChk, chkW, 20 ) ); yChk += rowH;
	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_LOCO_FIRST, "LOCO_FIRST - Loco pkt before func pkt", xChk, yChk, chkW, 20 ) ); yChk += rowH;

	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_NO_ABORT, "NO_ABORT - Do not stop on error", xChk, yChk, chkW, 20 ) ); yChk += rowH;
	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_LATE_SCOPE, "LATE_SCOPE - Scope trigger after cmd", xChk, yChk, chkW, 20 ) ); yChk += rowH;
	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_FRAGMENT, "FRAGMENT - Test all fragment lengths", xChk, yChk, chkW, 20 ) ); yChk += rowH;
	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_SAME_AMBIG_ADDR, "SAME_AMBIG_ADDR - Same addr for ambig.", xChk, yChk, chkW, 20 ) ); yChk += rowH;
	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_KICK_START, "KICK_START - Kick start motor", xChk, yChk, chkW, 20 ) ); yChk += rowH;
	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_MCU_BYPASS_TST, "MCU_BYPASS_TST - Skip MCU self test", xChk, yChk, chkW, 20 ) ); yChk += rowH;

	/* Lowest content bottom seen so far, used to place the shared button bar
	 * below every page instead of just the General page. */
	int contentBottom = ( y > yChk ? y : yChk );

	/* Test Mask Bits page: one checkbox per TESTS mask bit, 2 columns. */
	{
		const int bitColW = 500;
		const int bitRowH = 24;
		const int bitsPerCol = ( TESTBIT_COUNT + 1 ) / 2;

		g_page1Ctrls.push_back( MakeStatic( hwnd, "Tests Mask (hex):",
			xLabel, pageTop, labelW, 18 ) );
		g_page1Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_TESTS,
			xEdit, pageTop - 2, 150, 22 ) );

		for ( int i = 0; i < TESTBIT_COUNT; i++ )
		{
			char text[64];
			sprintf( text, "Bit %2d (0x%08lX) - %s", i, 1UL << i, g_testBitLabels[i] );

			int col = i / bitsPerCol;
			int row = i % bitsPerCol;
			HWND chk = MakeCheck( hwnd, ID_CHK_TESTBIT_BASE + i, text,
				xLabel + col * bitColW, pageTop + 32 + row * bitRowH, bitColW - 10, 20 );
			g_page1Ctrls.push_back( chk );
		}

		int by = pageTop + 32 + ((CLOCKBIT_COUNT + 1) / 2) * bitRowH + 10;
		g_page1Ctrls.push_back(
			MakeButton( hwnd, ID_BTN_TESTBITS_ALL, "Select All", xLabel, by, 110, 26 ) );
		g_page1Ctrls.push_back(
			MakeButton( hwnd, ID_BTN_TESTBITS_NONE, "Select None", xLabel + 120, by, 110, 26 ) );
		g_page1Ctrls.push_back(
			MakeButton( hwnd, ID_BTN_TESTBITS_DEFAULT, "Reset Mask Defaults", xLabel + 240, by, 160, 26 ) );

		if ( by + 26 > contentBottom )
		{
			contentBottom = by + 26;
		}
	}

		/* Log Mask Bits page: independent file, command, and packet logging bits. */
		{
			const int bitColW = 500;
			const int bitRowH = 24;

			g_page4Ctrls.push_back( MakeStatic( hwnd, "Log Mask",
				xLabel, pageTop, labelW, 18 ) );
			g_page4Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_LOG,
				xEdit, pageTop - 2, 150, 22 ) );

			for ( int i = 0; i < LOGBIT_COUNT; i++ )
			{
				char text[64];
				sprintf( text, "Bit %2d (0x%02lX) - %s", i, 1UL << i, g_logBitLabels[i] );
				g_page4Ctrls.push_back( MakeCheck( hwnd, ID_CHK_LOGBIT_BASE + i, text,
					xLabel, pageTop + 32 + i * bitRowH, bitColW - 10, 20 ) );
			}

			int by = pageTop + 32 + ((CLOCKBIT_COUNT + 1) / 2) * bitRowH + 10;
			g_page4Ctrls.push_back(
				MakeButton( hwnd, ID_BTN_LOGBITS_ALL, "Select All", xLabel, by, 110, 26 ) );
			g_page4Ctrls.push_back(
				MakeButton( hwnd, ID_BTN_LOGBITS_NONE, "Select None", xLabel + 120, by, 110, 26 ) );
			g_page4Ctrls.push_back(
				MakeButton( hwnd, ID_BTN_LOGBITS_DEFAULT, "Reset Mask Defaults", xLabel + 240, by, 160, 26 ) );

			if ( by + 26 > contentBottom )
			{
				contentBottom = by + 26;
			}
		}

	/* Function Mask Bits page: the five bits select functions F0-F4. */
	{
		const int bitColW = 500;
		const int bitRowH = 24;

		g_page3Ctrls.push_back( MakeStatic( hwnd, "Funcs Mask (hex):",
			xLabel, pageTop, labelW, 18 ) );
		g_page3Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_FUNCS,
			xEdit, pageTop - 2, 150, 22 ) );

		for ( int i = 0; i < FUNCBIT_COUNT; i++ )
		{
			char text[64];
			sprintf( text, "Bit %2d (0x%02lX) - %s", i, 1UL << i, g_funcBitLabels[i] );

			HWND chk = MakeCheck( hwnd, ID_CHK_FUNCBIT_BASE + i, text,
				xLabel, pageTop + 32 + i * bitRowH,
				bitColW - 10, 20 );
			g_page3Ctrls.push_back( chk );
		}

		int by = pageTop + 32 + ((CLOCKBIT_COUNT + 1) / 2) * bitRowH + 10;
		g_page3Ctrls.push_back(
			MakeButton( hwnd, ID_BTN_FUNCBITS_ALL, "Select All", xLabel, by, 110, 26 ) );
		g_page3Ctrls.push_back(
			MakeButton( hwnd, ID_BTN_FUNCBITS_NONE, "Select None", xLabel + 120, by, 110, 26 ) );
		g_page3Ctrls.push_back(
			MakeButton( hwnd, ID_BTN_FUNCBITS_DEFAULT, "Reset Mask Defaults", xLabel + 240, by, 160, 26 ) );

		if ( by + 26 > contentBottom )
		{
			contentBottom = by + 26;
		}
	}

	/* Clock Mask Bits page: one checkbox per CLOCKS mask bit, 2 columns. */
	{
		const int bitColW = 500;
		const int bitRowH = 24;
		const int bitsPerCol = ( CLOCKBIT_COUNT + 1 ) / 2;

		g_page2Ctrls.push_back( MakeStatic( hwnd, "Clocks Mask (hex):",
			xLabel, pageTop, labelW, 18 ) );
		g_page2Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_CLOCKS,
			xEdit, pageTop - 2, 150, 22 ) );

		for ( int i = 0; i < CLOCKBIT_COUNT; i++ )
		{
			char text[64];
			sprintf( text, "Bit %2d (0x%08lX) - %s", i, 1UL << i, g_clockBitLabels[i] );

			int col = i / bitsPerCol;
			int row = i % bitsPerCol;
			HWND chk = MakeCheck( hwnd, ID_CHK_CLOCKBIT_BASE + i, text,
				xLabel + col * bitColW, pageTop + 32 + row * bitRowH, bitColW - 10, 20 );
			g_page2Ctrls.push_back( chk );
		}

		int by = pageTop + 32 + ((CLOCKBIT_COUNT + 1) / 2) * bitRowH + 10;
		g_page2Ctrls.push_back(
			MakeButton( hwnd, ID_BTN_CLOCKBITS_ALL, "Select All", xLabel, by, 110, 26 ) );
		g_page2Ctrls.push_back(
			MakeButton( hwnd, ID_BTN_CLOCKBITS_NONE, "Select None", xLabel + 120, by, 110, 26 ) );
		g_page2Ctrls.push_back(
			MakeButton( hwnd, ID_BTN_CLOCKBITS_DEFAULT, "Reset Mask Defaults", xLabel + 240, by, 160, 26 ) );

		if ( by + 26 > contentBottom )
		{
			contentBottom = by + 26;
		}
	}

	/* Bottom: buttons + path label (shared, always visible). */
	const int btnW = 110;
	int yLaunch = pageTop + 32 + ((CLOCKBIT_COUNT + 1) / 2) * 24 + 10;
	g_page0Ctrls.push_back(
		MakeButton( hwnd, ID_BTN_LAUNCH, "Launch send.exe", 20, yLaunch, btnW + 40, 26 ) );
	g_page0Ctrls.push_back(
		MakeButton( hwnd, ID_BTN_DEFAULTS, "Reset All Defaults", 20 + btnW + 50, yLaunch, btnW + 40, 26 ) );
	g_page0Ctrls.push_back(
		MakeButton( hwnd, ID_BTN_MCU_TEST, "MCU test", 20 + 2 * (btnW + 50), yLaunch, btnW, 26 ) );

	int yBtn = yLaunch + 36;
	int xBtn = 20;

	MakeButton( hwnd, ID_BTN_LOAD, "Load...", xBtn, yBtn, btnW, 26 ); xBtn += btnW + 10;
	MakeButton( hwnd, ID_BTN_SAVE, "Save", xBtn, yBtn, btnW, 26 ); xBtn += btnW + 10;
	MakeButton( hwnd, ID_BTN_SAVEAS, "Save As...", xBtn, yBtn, btnW, 26 );

	CreateWindowEx( 0, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_PATHELLIPSIS | SS_NOPREFIX,
		20, yBtn + 30, 1020, 20, hwnd, (HMENU)(INT_PTR)ID_STATIC_PATH, NULL, NULL );
	CreateWindowEx( 0, "STATIC", "MCU test: Not run", WS_CHILD | WS_VISIBLE | SS_NOPREFIX,
		20, yBtn + 52, 1020, 20, hwnd, (HMENU)(INT_PTR)ID_STATIC_MCU_STATUS, NULL, NULL );

	/* Start on the General page. */
	for ( size_t i = 0; i < g_page1Ctrls.size(); i++ )
	{
		ShowWindow( g_page1Ctrls[i], SW_HIDE );
	}
	for ( size_t i = 0; i < g_page2Ctrls.size(); i++ )
	{
		ShowWindow( g_page2Ctrls[i], SW_HIDE );
	}
	for ( size_t i = 0; i < g_page3Ctrls.size(); i++ )
	{
		ShowWindow( g_page3Ctrls[i], SW_HIDE );
	}
	for ( size_t i = 0; i < g_page4Ctrls.size(); i++ )
	{
		ShowWindow( g_page4Ctrls[i], SW_HIDE );
	}
}


/*---------------------------------------------------------------------------
 * Window procedure.
 *-------------------------------------------------------------------------*/
static LRESULT CALLBACK
WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_CREATE:
			CreateControls( hwnd );
			LoadDefaults();
			UpdatePathLabel();
			return 0;

		case WM_COMMAND:
			if ( HIWORD( wParam ) == EN_CHANGE )
			{
				static bool syncingMasks = false;
				if ( !syncingMasks )
				{
					int changedId = LOWORD( wParam );
					int sourceId = 0;
					int targetId = 0;
					if ( changedId == ID_EDIT_TESTS || changedId == ID_EDIT_TESTS_GENERAL )
					{
						sourceId = changedId;
						targetId = changedId == ID_EDIT_TESTS
							? ID_EDIT_TESTS_GENERAL : ID_EDIT_TESTS;
					}
					else if ( changedId == ID_EDIT_CLOCKS || changedId == ID_EDIT_CLOCKS_GENERAL )
					{
						sourceId = changedId;
						targetId = changedId == ID_EDIT_CLOCKS
							? ID_EDIT_CLOCKS_GENERAL : ID_EDIT_CLOCKS;
					}
					else if ( changedId == ID_EDIT_FUNCS || changedId == ID_EDIT_FUNCS_GENERAL )
					{
						sourceId = changedId;
						targetId = changedId == ID_EDIT_FUNCS
							? ID_EDIT_FUNCS_GENERAL : ID_EDIT_FUNCS;
					}
					else if ( changedId == ID_EDIT_LOG || changedId == ID_EDIT_LOG_GENERAL )
					{
						sourceId = changedId;
						targetId = changedId == ID_EDIT_LOG
							? ID_EDIT_LOG_GENERAL : ID_EDIT_LOG;
					}

					if ( targetId != 0 )
					{
						std::string value = GetEditText( sourceId );
						syncingMasks = true;
						SetEditText( targetId, value.c_str() );
						syncingMasks = false;
					}
				}
			}

			switch ( LOWORD( wParam ) )
			{
				case ID_BTN_LOAD:
				{
					char path[MAX_PATH];
					if ( BrowseOpen( path, sizeof( path ) ) )
					{
						if ( LoadCfgFile( path ) )
						{
							strncpy( g_cfgPath, path, sizeof( g_cfgPath ) - 1 );
							UpdatePathLabel();
						}
						else
						{
							MessageBox( hwnd, "Failed to open the selected file.",
								"sendgui", MB_ICONERROR );
						}
					}
					return 0;
				}

				case ID_BTN_SAVE:
				{
					if ( g_cfgPath[0] == '\0' )
					{
						PostMessage( hwnd, WM_COMMAND, ID_BTN_SAVEAS, 0 );
						return 0;
					}
					if ( g_iniSource )
					{
						int mode = AskIniSaveMode( hwnd );
						if ( mode == IDCANCEL )
						{
							return 0;
						}
						if ( mode == IDNO )
						{
							MigrateIniToCfg( hwnd );
							return 0;
						}
					}
					if ( !SaveCfgFile( g_cfgPath ) )
					{
						MessageBox( hwnd, "Failed to save the file.",
							"sendgui", MB_ICONERROR );
					}
					return 0;
				}

				case ID_BTN_SAVEAS:
				{
					char path[MAX_PATH];
					if ( g_iniSource && g_cfgPath[0] != '\0' )
					{
						int mode = AskIniSaveMode( hwnd );
						if ( mode == IDCANCEL )
						{
							return 0;
						}
						if ( mode == IDNO )
						{
							MigrateIniToCfg( hwnd );
							return 0;
						}
					}
					if ( BrowseSave( path, sizeof( path ) ) )
					{
						if ( SaveCfgFile( path ) )
						{
							strncpy( g_cfgPath, path, sizeof( g_cfgPath ) - 1 );
							UpdatePathLabel();
						}
						else
						{
							MessageBox( hwnd, "Failed to save the file.",
								"sendgui", MB_ICONERROR );
						}
					}
					return 0;
				}

				case ID_BTN_DEFAULTS:
					LoadDefaults();
					return 0;

				case ID_BTN_LAUNCH:
					LaunchSendExe( hwnd );
					return 0;

				case ID_BTN_MCU_TEST:
					LaunchMcuSmokeTest( hwnd );
					return 0;

				case ID_BTN_TESTBITS_ALL:
					for ( int i = 0; i < TESTBIT_COUNT; i++ )
					{
						SetChecked( ID_CHK_TESTBIT_BASE + i, true );
					}
					SyncTestsFromBits();
					return 0;

				case ID_BTN_TESTBITS_NONE:
					for ( int i = 0; i < TESTBIT_COUNT; i++ )
					{
						SetChecked( ID_CHK_TESTBIT_BASE + i, false );
					}
					SyncTestsFromBits();
					return 0;

				case ID_BTN_TESTBITS_DEFAULT:
					ResetTestsMaskDefault();
					return 0;

				case ID_BTN_CLOCKBITS_ALL:
					for ( int i = 0; i < CLOCKBIT_COUNT; i++ )
					{
						SetChecked( ID_CHK_CLOCKBIT_BASE + i, true );
					}
					SyncClocksFromBits();
					return 0;

				case ID_BTN_CLOCKBITS_NONE:
					for ( int i = 0; i < CLOCKBIT_COUNT; i++ )
					{
						SetChecked( ID_CHK_CLOCKBIT_BASE + i, false );
					}
					SyncClocksFromBits();
					return 0;

				case ID_BTN_CLOCKBITS_DEFAULT:
					ResetClocksMaskDefault();
					return 0;

				case ID_BTN_FUNCBITS_ALL:
					for ( int i = 0; i < FUNCBIT_COUNT; i++ )
					{
						SetChecked( ID_CHK_FUNCBIT_BASE + i, true );
					}
					SyncFuncsFromBits();
					return 0;

				case ID_BTN_FUNCBITS_NONE:
					for ( int i = 0; i < FUNCBIT_COUNT; i++ )
					{
						SetChecked( ID_CHK_FUNCBIT_BASE + i, false );
					}
					SyncFuncsFromBits();
					return 0;

				case ID_BTN_FUNCBITS_DEFAULT:
					ResetFuncsMaskDefault();
					return 0;

				case ID_BTN_LOGBITS_ALL:
					for ( int i = 0; i < LOGBIT_COUNT; i++ )
					{
						SetChecked( ID_CHK_LOGBIT_BASE + i, true );
					}
					SyncLogFromBits();
					return 0;

				case ID_BTN_LOGBITS_NONE:
					for ( int i = 0; i < LOGBIT_COUNT; i++ )
					{
						SetChecked( ID_CHK_LOGBIT_BASE + i, false );
					}
					SyncLogFromBits();
					return 0;

				case ID_BTN_LOGBITS_DEFAULT:
					ResetLogMaskDefault();
					return 0;
			}

			if ( LOWORD( wParam ) >= ID_CHK_TESTBIT_BASE
				&& LOWORD( wParam ) < ID_CHK_TESTBIT_BASE + TESTBIT_COUNT
				&& HIWORD( wParam ) == BN_CLICKED )
			{
				SyncTestsFromBits();
			}
			else if ( LOWORD( wParam ) >= ID_CHK_CLOCKBIT_BASE
				&& LOWORD( wParam ) < ID_CHK_CLOCKBIT_BASE + CLOCKBIT_COUNT
				&& HIWORD( wParam ) == BN_CLICKED )
			{
				SyncClocksFromBits();
			}
			else if ( LOWORD( wParam ) >= ID_CHK_FUNCBIT_BASE
				&& LOWORD( wParam ) < ID_CHK_FUNCBIT_BASE + FUNCBIT_COUNT
				&& HIWORD( wParam ) == BN_CLICKED )
			{
				SyncFuncsFromBits();
			}
			else if ( LOWORD( wParam ) >= ID_CHK_LOGBIT_BASE
				&& LOWORD( wParam ) < ID_CHK_LOGBIT_BASE + LOGBIT_COUNT
				&& HIWORD( wParam ) == BN_CLICKED )
			{
				SyncLogFromBits();
			}
			return 0;

		case WM_NOTIFY:
		{
			LPNMHDR hdr = (LPNMHDR)lParam;
			if ( hdr->idFrom == ID_TAB && hdr->code == TCN_SELCHANGE )
			{
				int sel = TabCtrl_GetCurSel( hdr->hwndFrom );
				if ( sel == 1 )
				{
					SyncBitsFromTests();
				}
				else if ( sel == 2 )
				{
					SyncBitsFromClocks();
				}
				else if ( sel == 3 )
				{
					SyncBitsFromFuncs();
				}
				else if ( sel == 4 )
				{
					SyncBitsFromLog();
				}
				for ( size_t i = 0; i < g_page0Ctrls.size(); i++ )
				{
					ShowWindow( g_page0Ctrls[i], sel == 0 ? SW_SHOW : SW_HIDE );
				}
				for ( size_t i = 0; i < g_page1Ctrls.size(); i++ )
				{
					ShowWindow( g_page1Ctrls[i], sel == 1 ? SW_SHOW : SW_HIDE );
				}
				for ( size_t i = 0; i < g_page2Ctrls.size(); i++ )
				{
					ShowWindow( g_page2Ctrls[i], sel == 2 ? SW_SHOW : SW_HIDE );
				}
				for ( size_t i = 0; i < g_page3Ctrls.size(); i++ )
				{
					ShowWindow( g_page3Ctrls[i], sel == 3 ? SW_SHOW : SW_HIDE );
				}
				for ( size_t i = 0; i < g_page4Ctrls.size(); i++ )
				{
					ShowWindow( g_page4Ctrls[i], sel == 4 ? SW_SHOW : SW_HIDE );
				}

				/* Force a repaint so no stale label/checkbox text lingers
				 * from the previously visible page. */
				InvalidateRect( hwnd, NULL, TRUE );
			}
			return 0;
		}

		case WM_TIMER:
			if ( wParam == MCU_TEST_TIMER_ID && g_mcuTestProcess != NULL )
			{
				ReadMcuSmokeTestOutput();
				DWORD waitResult = WaitForSingleObject( g_mcuTestProcess, 0 );
				if ( waitResult == WAIT_OBJECT_0 )
				{
					DWORD exitCode = 1;
					bool passed = GetExitCodeProcess( g_mcuTestProcess, &exitCode )
						&& exitCode == 0;
					FinishMcuSmokeTest( hwnd, passed );
				}
				else if ( waitResult == WAIT_FAILED )
				{
					FinishMcuSmokeTest( hwnd, false );
				}
				return 0;
			}
			break;

		case WM_DESTROY:
			KillTimer( hwnd, MCU_TEST_TIMER_ID );
			if ( g_mcuTestProcess != NULL )
			{
				CloseHandle( g_mcuTestProcess );
				g_mcuTestProcess = NULL;
			}
			if ( g_mcuTestOutputRead != NULL )
			{
				CloseHandle( g_mcuTestOutputRead );
				g_mcuTestOutputRead = NULL;
			}
			PostQuitMessage( 0 );
			return 0;
	}

	return DefWindowProc( hwnd, msg, wParam, lParam );
}

/*---------------------------------------------------------------------------
 * Entry point.
 *-------------------------------------------------------------------------*/
int APIENTRY
WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow )
{
	INITCOMMONCONTROLSEX icc = { sizeof( icc ), ICC_STANDARD_CLASSES };
	InitCommonControlsEx( &icc );

	WNDCLASSEX wc = { 0 };
	wc.cbSize        = sizeof( wc );
	wc.lpfnWndProc   = WndProc;
	wc.hInstance     = hInstance;
	wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)( COLOR_BTNFACE + 1 );
	wc.lpszClassName = "SendGuiWndClass";
	wc.hIcon         = LoadIcon( NULL, IDI_APPLICATION );

	if ( !RegisterClassEx( &wc ) )
	{
		return 1;
	}

	char windowTitle[128];
	snprintf( windowTitle, sizeof( windowTitle ),
		"Send Config Editor (sendgui) %c.%u.%u.%u.%u",
		Ver_rel, Ver_maj, Ver_min, Ver_pat, Ver_bld );

	g_hMain = CreateWindowEx( 0, wc.lpszClassName, windowTitle,
		WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT, 1060, 680,
		NULL, NULL, hInstance, NULL );

	if ( g_hMain == NULL )
	{
		return 1;
	}

	/* If a cfg file path was passed on the command line, load it. */
	if ( lpCmdLine != NULL && lpCmdLine[0] != '\0' )
	{
		std::string arg( lpCmdLine );
		Trim( arg );
		if ( !arg.empty() && arg.front() == '"' && arg.back() == '"' && arg.size() >= 2 )
		{
			arg = arg.substr( 1, arg.size() - 2 );
		}
		if ( LoadCfgFile( arg.c_str() ) )
		{
			strncpy( g_cfgPath, arg.c_str(), sizeof( g_cfgPath ) - 1 );
			UpdatePathLabel();
		}
	}
	else
	{
		/* Otherwise try SEND.CFG next to the executable / in the cwd. */
		if ( LoadCfgFile( "SEND.CFG" ) )
		{
			GetFullPathName( "SEND.CFG", sizeof( g_cfgPath ), g_cfgPath, NULL );
			UpdatePathLabel();
		}
	}

	ShowWindow( g_hMain, nCmdShow );
	UpdateWindow( g_hMain );

	MSG msg;
	while ( GetMessage( &msg, NULL, 0, 0 ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	return (int)msg.wParam;
}
