This Folder contains configuration information for the TS-5300 Single Board Computer.

Here’s a summary of each .zip file:

autoexec.zip:    	This .zip file contains the modified AUTOEXEC.BAT file:
    AUTOEXEC.ORG	Original AUTOEXEC.BAT file.
    AUTOEXEC.BAT	Modified AUTOEXEC.BAT file.
       			Modifications to AUTOEXEC.BAT file:
        		1.	Added C:\SEND and C:\BIN to PATH.
        		2.	Changed directory to C:\RESULTS at end of AUTOEXEC.BAT.

CF_B551.zip:		This .zip file contains an image of the CF card C: drive:
    BIN			C:\BIN directory. Contains additional utilities not supplied
			on the TS-5300 A: drive.
			C:\BIN\READ_BIN.TXT lists the programs in this directory.

RESULTS		C:\RESULTS directory. Contains the test results and SEND.INI file.
		SND_ORG.INI	Original SEND.INI file.
		SEND.INI	Current SEND.INI file. This one is modified to delay
				700 msec between test steps.
		S_USER.TXT	“SEND –u” user output for SEND.EXE test program

SEND		C:\SEND directory. Contains the SEND.EXE program.
		SEND.EXE        Version B.5.5.1 test program.
		SEND.INI        Original SEND.INI file. This is a backup copy only since the program
				normally runs from the C:\RESULTS directory and uses the
				C:\RESULTS\SEND.INI file.
		S_USER.TXT	“SEND –u” user output for SEND.EXE test program

