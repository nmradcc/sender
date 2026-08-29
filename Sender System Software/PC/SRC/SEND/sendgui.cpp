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
#include <string>
#include <vector>
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

	ID_CHK_DEBUG_ON,
	ID_CHK_MANUAL,
	ID_CHK_LAMP,
	ID_CHK_REPEAT,
	ID_CHK_TRIG_REV,
	ID_CHK_LOCO_FIRST,
	ID_CHK_LOG,
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

	ID_STATIC_PATH,

	ID_TAB,

	ID_CHK_TESTBIT_BASE,			// 32 sequential ids follow (bits 0..31).
	ID_BTN_TESTBITS_ALL = ID_CHK_TESTBIT_BASE + 32,
	ID_BTN_TESTBITS_NONE,
	ID_BTN_TESTBITS_DEFAULT,

	ID_CHK_CLOCKBIT_BASE,			// 32 sequential ids follow (bits 0..31).
	ID_BTN_CLOCKBITS_ALL = ID_CHK_CLOCKBIT_BASE + 32,
	ID_BTN_CLOCKBITS_NONE,
	ID_BTN_CLOCKBITS_DEFAULT
};

/* Number of individual TESTS mask bit checkboxes. */
static const int TESTBIT_COUNT = 32;

/* Short description of what each TESTS mask bit selects (see DEC_TST.CPP
 * print_user_docs() for the authoritative bit ordering). */
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
 * Dec_tst::dclk_tbl[] for the authoritative bit ordering). */
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

/*---------------------------------------------------------------------------
 * Global state.
 *-------------------------------------------------------------------------*/
static HWND	g_hMain;
static char	g_cfgPath[MAX_PATH] = "";
static std::vector<HWND>	g_page0Ctrls;	// Controls shown on the "General" tab.
static std::vector<HWND>	g_page1Ctrls;	// Controls shown on the "Test Mask Bits" tab.
static std::vector<HWND>	g_page2Ctrls;	// Controls shown on the "Clock Mask Bits" tab.

struct FlagCtrl
{
	int			ctrlId;		// Checkbox control id.
	const char	*key;		// SEND.CFG key name.
};

static const FlagCtrl g_flagCtrls[] =
{
	{ ID_CHK_DEBUG_ON,        "DEBUG_ON" },
	{ ID_CHK_MANUAL,          "MANUAL" },
	{ ID_CHK_LAMP,            "LAMP" },
	{ ID_CHK_REPEAT,          "REPEAT" },
	{ ID_CHK_TRIG_REV,        "TRIG_REV" },
	{ ID_CHK_LOCO_FIRST,      "LOCO_FIRST" },
	{ ID_CHK_LOG,             "LOG" },
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
	SetEditUInt( ID_EDIT_TESTS, mask, true );
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
	SetEditUInt( ID_EDIT_CLOCKS, mask, true );
}

/* Default mask values (mirrors ARGS.CPP Args_obj defaults). */
static const unsigned long DEF_TESTS_MASK = 0xFFFFFFFFUL;
static const unsigned long DEF_CLOCKS_MASK = 0xFFFFFFFFUL & ~0x8 & ~0x10 & ~0x100 & ~0x200;

static void
ResetTestsMaskDefault( void )
{
	SetEditUInt( ID_EDIT_TESTS, DEF_TESTS_MASK, true );
	SyncBitsFromTests();
}

static void
ResetClocksMaskDefault( void )
{
	SetEditUInt( ID_EDIT_CLOCKS, DEF_CLOCKS_MASK, true );
	SyncBitsFromClocks();
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
	SetEditUInt( ID_EDIT_TESTS, DEF_TESTS_MASK, true );
	SetEditUInt( ID_EDIT_CLOCKS, DEF_CLOCKS_MASK, true );
	SetEditUInt( ID_EDIT_FUNCS, 0x1F, true );

	for ( size_t i = 0; i < _countof( g_flagCtrls ); i++ )
	{
		SetChecked( g_flagCtrls[i].ctrlId, false );
	}

	SyncBitsFromTests();
	SyncBitsFromClocks();
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

	LoadDefaults();

	char line[256];
	while ( fgets( line, sizeof( line ), fp ) )
	{
		std::string s( line );
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
			SetEditText( ID_EDIT_TESTS, val.c_str() );
		}
		else if ( _stricmp( key.c_str(), "CLOCKS" ) == 0 )
		{
			SetEditText( ID_EDIT_CLOCKS, val.c_str() );
		}
		else if ( _stricmp( key.c_str(), "FUNCS" ) == 0 )
		{
			SetEditText( ID_EDIT_FUNCS, val.c_str() );
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

	fclose( fp );
	SyncBitsFromTests();
	SyncBitsFromClocks();
	return true;
}

/*---------------------------------------------------------------------------
 * Write the controls out in SEND.CFG format.
 *-------------------------------------------------------------------------*/
static bool
SaveCfgFile( const char *path )
{
	FILE *fp = fopen( path, "w" );
	if ( fp == NULL )
	{
		return false;
	}

	fprintf( fp, "; SEND.CFG - generated by sendgui\n" );
	fprintf( fp, ";\n" );
	fprintf( fp, "; Flags below are active (uncommented) only when checked.\n" );
	fprintf( fp, ";\n\n" );

	auto writeFlag = [&]( int ctrlId, const char *key )
	{
		if ( IsChecked( ctrlId ) )
		{
			fprintf( fp, "%-16s\n", key );
		}
		else
		{
			fprintf( fp, ";%-16s\n", key );
		}
	};

	writeFlag( ID_CHK_MANUAL, "MANUAL" );

	int typeSel = (int)SendDlgItemMessage( g_hMain, ID_COMBO_TYPE, CB_GETCURSEL, 0, 0 );
	const char *typeChar = "l";
	switch ( typeSel )
	{
		case 0: typeChar = "l"; break;
		case 1: typeChar = "f"; break;
		case 2: typeChar = "a"; break;
		case 3: typeChar = "s"; break;
	}

	fprintf( fp, "%-16s %s\n", "ADDRESS", GetEditText( ID_EDIT_ADDRESS ).c_str() );
	fprintf( fp, "%-16s %s\n", "TYPE", typeChar );
	writeFlag( ID_CHK_LAMP, "LAMP" );
	fprintf( fp, "%-16s %s\n", "PRESET", GetEditText( ID_EDIT_PRESET ).c_str() );
	fprintf( fp, "%-16s %s\n", "TRIGGER", GetEditText( ID_EDIT_TRIGGER ).c_str() );
	writeFlag( ID_CHK_REPEAT, "REPEAT" );
	fprintf( fp, "%-16s %s\n", "TESTS", GetEditText( ID_EDIT_TESTS ).c_str() );
	fprintf( fp, "%-16s %s\n", "CLOCKS", GetEditText( ID_EDIT_CLOCKS ).c_str() );
	fprintf( fp, "%-16s %s\n", "FUNCS", GetEditText( ID_EDIT_FUNCS ).c_str() );
	fprintf( fp, "%-16s %s\n", "EXTRA_PRE", GetEditText( ID_EDIT_EXTRA_PRE ).c_str() );
	writeFlag( ID_CHK_TRIG_REV, "TRIG_REV" );
	writeFlag( ID_CHK_LOCO_FIRST, "LOCO_FIRST" );
	fprintf( fp, "%-16s %s\n", "FILL_MSEC", GetEditText( ID_EDIT_FILL_MSEC ).c_str() );
	fprintf( fp, "%-16s %s\n", "TEST_REPS", GetEditText( ID_EDIT_TEST_REPS ).c_str() );
	writeFlag( ID_CHK_LOG, "LOG" );
	writeFlag( ID_CHK_MCU_BYPASS_TST, "MCU_BYPASS_TST" );
	writeFlag( ID_CHK_NO_ABORT, "NO_ABORT" );
	writeFlag( ID_CHK_LATE_SCOPE, "LATE_SCOPE" );
	writeFlag( ID_CHK_FRAGMENT, "FRAGMENT" );
	writeFlag( ID_CHK_SAME_AMBIG_ADDR, "SAME_AMBIG_ADDR" );
	fprintf( fp, "%-16s %s\n", "ACC_PAIR", GetEditText( ID_EDIT_ACC_PAIR ).c_str() );
	writeFlag( ID_CHK_KICK_START, "KICK_START" );
	writeFlag( ID_CHK_DEBUG_ON, "DEBUG_ON" );
	fprintf( fp, "%-16s %s\n", "EXTRA_TRG", GetEditText( ID_EDIT_EXTRA_TRG ).c_str() );
	fprintf( fp, "%-16s %s\n", "MCU_PORT", GetEditText( ID_EDIT_MCU_PORT ).c_str() );
	fprintf( fp, "%-16s %s\n", "MCU_BAUD", GetEditText( ID_EDIT_MCU_BAUD ).c_str() );

	fclose( fp );
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
	char file[MAX_PATH] = "SEND.CFG";

	ZeroMemory( &ofn, sizeof( ofn ) );
	ofn.lStructSize     = sizeof( ofn );
	ofn.hwndOwner       = g_hMain;
	ofn.lpstrFilter     = "Config Files (*.cfg;*.ini)\0*.cfg;*.ini\0All Files\0*.*\0";
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof( file );
	ofn.lpstrDefExt     = "cfg";
	ofn.Flags           = OFN_OVERWRITEPROMPT;

	if ( GetSaveFileName( &ofn ) )
	{
		strncpy( outPath, file, outSize - 1 );
		outPath[outSize - 1] = '\0';
		return true;
	}
	return false;
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

	if ( CreateProcess( sendExe.c_str(), cmdLine, NULL, NULL, FALSE, 0,
		NULL, exeDir, &si, &pi ) )
	{
		CloseHandle( pi.hThread );
		CloseHandle( pi.hProcess );
	}
	else
	{
		MessageBox( hwnd, "Failed to launch send.exe.", "sendgui", MB_ICONERROR );
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
	SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)"LOCO (l)" );
	SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)"FUNC (f)" );
	SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)"ACC (a)" );
	SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)"SIG (s)" );
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

	g_page0Ctrls.push_back( MakeStatic( hwnd, "Tests Mask (hex):", xLabel, y, labelW, 18 ) );
	g_page0Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_TESTS, xEdit, y - 2, editW, 22 ) );
	y += rowH;

	g_page0Ctrls.push_back( MakeStatic( hwnd, "Clocks Mask (hex):", xLabel, y, labelW, 18 ) );
	g_page0Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_CLOCKS, xEdit, y - 2, editW, 22 ) );
	y += rowH;

	g_page0Ctrls.push_back( MakeStatic( hwnd, "Funcs Mask (hex):", xLabel, y, labelW, 18 ) );
	g_page0Ctrls.push_back( MakeEdit( hwnd, ID_EDIT_FUNCS, xEdit, y - 2, editW, 22 ) );
	y += rowH;

	/* Right column: boolean flag checkboxes. */
	int xChk = xEdit + editW + 40;
	int yChk = pageTop;
	const int chkW = 340;

	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_DEBUG_ON, "DEBUG_ON - Log debug messages", xChk, yChk, chkW, 20 ) ); yChk += rowH;
	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_MANUAL, "MANUAL - Don't run tests automatically", xChk, yChk, chkW, 20 ) ); yChk += rowH;
	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_LAMP, "LAMP - Use rear lamp for func tests", xChk, yChk, chkW, 20 ) ); yChk += rowH;
	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_REPEAT, "REPEAT - Continuously repeat tests", xChk, yChk, chkW, 20 ) ); yChk += rowH;
	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_TRIG_REV, "TRIG_REV - Use reverse as trigger cmd", xChk, yChk, chkW, 20 ) ); yChk += rowH;
	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_LOCO_FIRST, "LOCO_FIRST - Loco pkt before func pkt", xChk, yChk, chkW, 20 ) ); yChk += rowH;
	g_page0Ctrls.push_back( MakeCheck( hwnd, ID_CHK_LOG, "LOG - Send packet data to log", xChk, yChk, chkW, 20 ) ); yChk += rowH;
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

		for ( int i = 0; i < TESTBIT_COUNT; i++ )
		{
			char text[64];
			sprintf( text, "Bit %2d (0x%08lX) - %s", i, 1UL << i, g_testBitLabels[i] );

			int col = i / bitsPerCol;
			int row = i % bitsPerCol;
			HWND chk = MakeCheck( hwnd, ID_CHK_TESTBIT_BASE + i, text,
				xLabel + col * bitColW, pageTop + row * bitRowH, bitColW - 10, 20 );
			g_page1Ctrls.push_back( chk );
		}

		int by = pageTop + bitsPerCol * bitRowH + 10;
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

	/* Clock Mask Bits page: one checkbox per CLOCKS mask bit, 2 columns. */
	{
		const int bitColW = 500;
		const int bitRowH = 24;
		const int bitsPerCol = ( CLOCKBIT_COUNT + 1 ) / 2;

		for ( int i = 0; i < CLOCKBIT_COUNT; i++ )
		{
			char text[64];
			sprintf( text, "Bit %2d (0x%08lX) - %s", i, 1UL << i, g_clockBitLabels[i] );

			int col = i / bitsPerCol;
			int row = i % bitsPerCol;
			HWND chk = MakeCheck( hwnd, ID_CHK_CLOCKBIT_BASE + i, text,
				xLabel + col * bitColW, pageTop + row * bitRowH, bitColW - 10, 20 );
			g_page2Ctrls.push_back( chk );
		}

		int by = pageTop + bitsPerCol * bitRowH + 10;
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
	int yLaunch = contentBottom + 20;
	g_page0Ctrls.push_back(
		MakeButton( hwnd, ID_BTN_LAUNCH, "Launch send.exe", 20, yLaunch, btnW + 40, 26 ) );
	g_page0Ctrls.push_back(
		MakeButton( hwnd, ID_BTN_DEFAULTS, "Reset All Defaults", 20 + btnW + 50, yLaunch, btnW + 40, 26 ) );

	int yBtn = yLaunch + 36;
	int xBtn = 20;

	MakeButton( hwnd, ID_BTN_LOAD, "Load...", xBtn, yBtn, btnW, 26 ); xBtn += btnW + 10;
	MakeButton( hwnd, ID_BTN_SAVE, "Save", xBtn, yBtn, btnW, 26 ); xBtn += btnW + 10;
	MakeButton( hwnd, ID_BTN_SAVEAS, "Save As...", xBtn, yBtn, btnW, 26 );

	CreateWindowEx( 0, "STATIC", "", WS_CHILD | WS_VISIBLE,
		20, yBtn + 36, 1000, 20, hwnd, (HMENU)(INT_PTR)ID_STATIC_PATH, NULL, NULL );

	/* Start on the General page. */
	for ( size_t i = 0; i < g_page1Ctrls.size(); i++ )
	{
		ShowWindow( g_page1Ctrls[i], SW_HIDE );
	}
	for ( size_t i = 0; i < g_page2Ctrls.size(); i++ )
	{
		ShowWindow( g_page2Ctrls[i], SW_HIDE );
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

				/* Force a repaint so no stale label/checkbox text lingers
				 * from the previously visible page. */
				InvalidateRect( hwnd, NULL, TRUE );
			}
			return 0;
		}

		case WM_DESTROY:
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

	g_hMain = CreateWindowEx( 0, wc.lpszClassName, "Send Config Editor (sendgui)",
		WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT, 1060, 640,
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
