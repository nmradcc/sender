#include <windows.h>
#include "args.h"
#include "send_reg.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>

Send_reg Dcc_reg;

int main()
{
	char tempDir[MAX_PATH];
	char configPath[MAX_PATH];
	DWORD tempLength = GetTempPathA( MAX_PATH, tempDir );
	if ( tempLength == 0 || tempLength >= MAX_PATH ||
		GetTempFileNameA( tempDir, "dcc", 0, configPath ) == 0 )
	{
		fprintf( stderr, "Could not create temporary configuration file\n" );
		return 1;
	}

	FILE *config = fopen( configPath, "w" );
	if ( config == nullptr )
	{
		fprintf( stderr, "Could not open temporary configuration file\n" );
		DeleteFileA( configPath );
		return 1;
	}
	int writeResult = fprintf( config, "LOG 0x10\n" );
	int closeResult = fclose( config );
	if ( writeResult < 0 || closeResult != 0 )
	{
		fprintf( stderr, "Could not write temporary configuration file\n" );
		DeleteFileA( configPath );
		return 1;
	}
	if ( _putenv_s( "SEND_CFG", configPath ) != 0 )
	{
		fprintf( stderr, "Could not set SEND_CFG\n" );
		DeleteFileA( configPath );
		return 1;
	}

	char name[] = "args_test";
	char simulate[] = "--simulate";
	char logOption[] = "-P";
	char logZero[] = "0";
	char logNoAbort[] = "0x10";
	char *normalArgs[] = { name };
	char *simArgs[] = { name, simulate };
	char *simThenLog[] = { name, simulate, logOption, logZero };
	char *logThenSim[] = { name, logOption, logNoAbort, simulate };
	Args_obj normal;
	Args_obj sim;
	Args_obj simFirst;
	Args_obj simLast;
	bool passed =
		normal.get_args( 1, normalArgs ) == OK &&
		!normal.get_simulation_mode() && normal.get_log_mask() == 0x10 &&
		sim.get_args( 2, simArgs ) == OK &&
		sim.get_simulation_mode() && sim.get_log_mask() == 0x1f &&
		simFirst.get_args( 4, simThenLog ) == OK &&
		simFirst.get_simulation_mode() && simFirst.get_log_mask() == 0x0f &&
		simLast.get_args( 4, logThenSim ) == OK &&
		simLast.get_simulation_mode() && simLast.get_log_mask() == 0x1f;

	if ( Args.get_args( 1, normalArgs ) != OK )
	{
		passed = false;
	}
	auto start = std::chrono::steady_clock::now();
	Dcc_reg.delay( std::chrono::milliseconds( 80 ) );
	passed = passed && std::chrono::steady_clock::now() - start >=
		std::chrono::milliseconds( 60 );

	if ( Args.get_args( 2, simArgs ) != OK )
	{
		passed = false;
	}
	Dcc_reg.set_no_hw_sim( true );
	Dcc_reg.set_pkt_gap_msec( 3000 );
	if ( Dcc_reg.set_clk( 200, 100, 116 ) != OK )
	{
		passed = false;
	}
	start = std::chrono::steady_clock::now();
	Dcc_reg.delay( std::chrono::seconds( 3 ) );
	Dcc_reg.pkt_gap();
	passed = passed && std::chrono::steady_clock::now() - start <
		std::chrono::milliseconds( 1500 );

	if ( !DeleteFileA( configPath ) )
	{
		fprintf( stderr, "Could not remove temporary configuration file\n" );
		return 1;
	}
	if ( !passed )
	{
		fprintf( stderr, "Simulation argument, log mask, or delay test failed\n" );
		return 1;
	}
	return 0;
}
