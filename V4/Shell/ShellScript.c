/*******************************************************************************
* @file ShellScript.c
* @brief The script commands for the shell. The command table is in Shell.c
*
* @author K. Kobel
* @date 9/15/2019
* @Revision: 24 $
* @Modtime: 10/31/2019
*
* @copyright	(c) 2019  all Rights Reserved.
*******************************************************************************/
#include "main.h"
#include <string.h>
#include <stdlib.h>
#include "Shell.h"
#include "ff.h"
#include "GetLine.h"
#include "Variables.h"

//*******************************************************************************
// Global Variables
//*******************************************************************************

//*******************************************************************************
// Definitions
//*******************************************************************************

typedef enum
{
	RUN_NORMAL,
	RUN_IF,
	SKIP_IF,
	SKIP_ELSE,
	SKIP_LOOP,
    RUN_DELAY,
	SCRIPT_EXIT,
	SCRIPT_PAUSE,
	SCRIPT_RESUME,
} RUN_MODE;

#define LOOP_FOREVER	0xffff

//*******************************************************************************
// Static Variables
//*******************************************************************************


//*******************************************************************************
// Global Variables
//*******************************************************************************

//*******************************************************************************
// Function prototypes
//*******************************************************************************

extern char* strsep(char **stringp, const char *delim);

extern uint8_t PortIndex(uint8_t p);

//*******************************************************************************
// Source
//*******************************************************************************

const char ConditionTable[][3] =
{
	"==",
	"!=",
	">",
	"<",
	">=",
	"<=",
};
#define CONDITIONAL_TABLE_COUNT (sizeof(ConditionTable) / 3)

typedef enum
{
	COND_NONE,
	COND_EQUAL,
	COND_NOT_EQUAL,
	COND_GREATER_THAN,
	COND_LESS_THAN,
	COND_GREATER_THAN_EQUAL,
	COND_LESS_THAN_EQUAL,
} CONDITIONAL;


typedef struct
{
	FIL Scriptfp;
	//FIL* Scriptfp;
	char buffer[BUFFER_SIZE];
	char *args[ARR_SIZE];
	size_t nargs;
	char name[16];
    uint8_t bPort;
    uint32_t LoopCount;
    uint32_t LoopPosition;
    uint32_t DelayCount;
	RUN_MODE RunMode;
	RUN_MODE RunMessage;
	uint32_t Variable[4];
	uint32_t Return;
} ScriptContext;

// these are the arguments passed in to the script
// and are valid for all of the nested scripts, one for each port
typedef struct
{
	size_t nargs;
	char args[ARG_SIZE][ARR_SIZE];
} ScriptArgs;

ScriptArgs scpargs[MAX_PORTS];

#define NUMBER_OF_SCRIPT_NESTS  3
ScriptContext ScriptNest[NUMBER_OF_SCRIPT_NESTS];
int CurrentScript = -1;


static char* RunModeName[] =
{
	"Normal",
	"If",
	"If",
	"Else",
    "Loop",
	"End Loop",
	"Loop",
    "Delay",
};

/*********************************************************************
*
* DoRun
*
* @brief	FileSystem Directory shell command 
*
* @param	bPort - port that issued this command
*			fp - pointer to an open file struct
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN DoRun(uint8_t bPort, FIL* fp, size_t nargs, char** args)
{

	CurrentScript++;
	if(CurrentScript >= NUMBER_OF_SCRIPT_NESTS)
	{
		// nested too deep
	}
	else
	{
		// setup the context
		//ScriptNest[CurrentScript].Scriptfp = fp;
		memcpy(&ScriptNest[CurrentScript].Scriptfp, fp, sizeof(FIL));
		ScriptNest[CurrentScript].RunMode = RUN_NORMAL;
		ScriptNest[CurrentScript].RunMessage = RUN_NORMAL;
		ScriptNest[CurrentScript].LoopCount = 0;
		ScriptNest[CurrentScript].DelayCount = 0;
		ScriptNest[CurrentScript].bPort = bPort;

		strcpy(ScriptNest[CurrentScript].name, args[0]);

		int pidx = PortIndex(bPort);
		scpargs[pidx].nargs = nargs;
		for(int i = 0; i < nargs; i++)
		{
			strcpy(scpargs[pidx].args[i], args[i+1]);
		}
	}
	return CMD_OK;
}

/*********************************************************************
*
* ScriptDone
*
* @brief	Closes and de-nests a script 
*
* @param	None
*
* @return	None
*
*********************************************************************/
void ScriptDone(void)
{
    
	if(CurrentScript != -1)
	{
		f_close(&ScriptNest[CurrentScript].Scriptfp);
		CurrentScript--;
	}
}

/*********************************************************************
*
* DoScriptRun
*
* @brief	Runs a script 
*
* @param	None
*
* @return	None
*
*********************************************************************/
void DoScriptRun(void)
{
	int ret;
	int iCmdIndex;
	ScriptContext* cs;
	char* p;
	char cmdbuffer[80];

	if(CurrentScript != -1)
	{
		cs = &ScriptNest[CurrentScript];
		switch(cs->RunMode)
        {
            case RUN_NORMAL:
            case RUN_IF:

            	if(cs->RunMessage == SCRIPT_EXIT)
            	{
            		ScriptDone();
            	}
            	else if(cs->RunMessage == SCRIPT_PAUSE)
            	{
            		return;
            	}
            	else if(cs->RunMessage == SCRIPT_EXIT)
            	{
            		cs->RunMessage = RUN_NORMAL;
            	}

                if(getLine(&cs->Scriptfp, cs->buffer, BUFFER_SIZE) != 0)
                {
					// substitute any variable arguments, "%X" where X is 1-6
					// and replace it with the corresponding argument string
					// ex: DIR -c %1 %2
					p = cs->buffer;
					int argidx;
					int cmdidx = 0;
					int len;
					for(int i = 0; i < strlen(p); i++)
					{
						if(p[i] == '%')
						{
							// found the "arg" in the buffer
							i++;
							argidx = p[i] - '0' - 1;
							int pidx = PortIndex(cs->bPort);

							if(argidx >= 0 && argidx < scpargs[pidx].nargs)
							{
								len = strlen(scpargs[pidx].args[argidx]);
								if(len != 0)
								{
									// copy the argument into the command buffer
									for(int j = 0; j < len; j++)
									{
										cmdbuffer[cmdidx++] = scpargs[pidx].args[argidx][j];
									}
								}
							}
						}
						else
						{
							cmdbuffer[cmdidx++] = p[i];
						}
					}
					cmdbuffer[cmdidx] = 0;
					
        			ret = ShellMain(ScriptNest[CurrentScript].bPort, (char*)cmdbuffer);
                    if(ret & CMD_IF_TRUE)
                    {
                    	cs->RunMode = RUN_IF;
                    }
                    else if(ret & CMD_IF_FALSE)
                    {
                    	cs->RunMode = SKIP_IF;
                    }
                }
                else
                {
                    ScriptDone();
                }
            break;

            case SKIP_IF:
                if(getLine(&cs->Scriptfp, cs->buffer, sizeof(BUFFER_SIZE)) != (uint8_t)EOF)
                {
                    iCmdIndex = FindCommand(cs->buffer, &cs->nargs, cs->args);
                    if(strcasecmp(ShellTable[iCmdIndex].szCommand, "else") == 0)
                    {
                        if(cs->RunMode == RUN_IF)
                        {
                        	cs->RunMode = SKIP_ELSE;
                        }
                        else
                        {
                        	cs->RunMode = RUN_NORMAL;
                        }
                    }
                }
                else
                {
                    ScriptDone();
                }
            break;

            case SKIP_ELSE:
                if(getLine(&cs->Scriptfp, cs->buffer, sizeof(BUFFER_SIZE)) != (uint8_t)EOF)
                {
                    iCmdIndex = FindCommand(cs->buffer, &cs->nargs, cs->args);
                    if(strcasecmp(ShellTable[iCmdIndex].szCommand, "endif") == 0)
                    {
                    	cs->RunMode = RUN_NORMAL;
                    }
                }
                else
                {
                    ScriptDone();
                }
            break;

            case SKIP_LOOP:
                if(getLine(&cs->Scriptfp, cs->buffer, sizeof(BUFFER_SIZE)) != (uint8_t)EOF)
                {
                    iCmdIndex = FindCommand(cs->buffer, &cs->nargs, cs->args);
                    if(strcasecmp(ShellTable[iCmdIndex].szCommand, "endloop") == 0)
                    {
                    	cs->RunMode = RUN_NORMAL;
                    }
                }
                else
                {
                    ScriptDone();
                }
            break;

            case RUN_DELAY:
                if(cs->DelayCount == 0)
                {
                	cs->RunMode = RUN_NORMAL;
                }
                else
                {
                	cs->DelayCount--;
                    if(cs->DelayCount == 0)
                    {
                    	cs->RunMode = RUN_NORMAL;
                    }
                }
            break;

            default:
            break;
        }
        
        // ToDo - see if there is anything in the port input buffer
        // if ^C, break out
    }
}


//CMD_RETURN ShRun(uint8_t bPort, int argc, char *argv[])
//{
//
//    if(argc >= 2)
//    {
//    	return DoRun(bPort, argv[1]);
//    }
//	return CMD_BAD_PARAMS;
//}


/*********************************************************************
*
* TestCondition
*
* @brief	Evaluate the conditions of an If statement 
*
* @param	szArg1 - Value or variable
*			szCondition - Condition - See conditions above
*			szArg2 - Value or variable
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN TestCondition(char* szArg1, char* szCondition, char* szArg2)
{
	int i;
	char szValue1[16];
	char szValue2[16];
	int iValue1;
	int iValue2;
    #ifdef NO_FLOATS
    	float fValue1;
    	float fValue2;
    #endif
        
//k	GetVariable(szArg1, szValue1, 16);
//k	GetVariable(szArg2, szValue2, 16);

	// find the conditional
	for(i = 0; i < CONDITIONAL_TABLE_COUNT; i++)
	{
		if(strcasecmp(ConditionTable[i], szCondition) == 0)
		{
			break;
		}
	}
	if(i >= CONDITIONAL_TABLE_COUNT)
	{
		return CMD_IF_FALSE;
	}

#ifdef NO_FLOATS
	if(isfloat(pszValue1) || isfloat(pszValue2))
	{
		fValue1 = atof(pszValue1);
		fValue2 = atof(pszValue2);

		switch(i)
		{
			case COND_NONE:
			break;
			case COND_EQUAL:
				if(fValue1 == fValue2)
				{
					return CMD_IF_TRUE;
				}
			break;
			case COND_NOT_EQUAL:
				if(fValue1 != fValue2)
				{
					return CMD_IF_TRUE;
				}
			break;
			case COND_GREATER_THAN:
				if(fValue1 > fValue2)
				{
					return CMD_IF_TRUE;
				}
			break;
			case COND_LESS_THAN:
				if(fValue1 < fValue2)
				{
					return CMD_IF_TRUE;
				}
			break;
			case COND_GREATER_THAN_EQUAL:
				if(fValue1 >= fValue2)
				{
					return CMD_IF_TRUE;
				}
			break;
			case COND_LESS_THAN_EQUAL:
				if(fValue1 <= fValue2)
				{
					return CMD_IF_TRUE;
				}
			break;
		}
	}
	else
#endif
	{
		i = FindVariable(szValue1);
		if(i != -1)
		{
			iValue1 = GetVariableValue(i);
		}
		else
		{
			iValue1 = atoi(szValue1);
		}

		i = FindVariable(szValue2);
		if(i != -1)
		{
			iValue2 = GetVariableValue(i);
		}
		else
		{
			iValue2 = atoi(szValue2);
		}

		switch(i)
		{
			case COND_NONE:
			break;
			case COND_EQUAL:
				if(iValue1 == iValue2)
				{
					return CMD_IF_TRUE;
				}
			break;
			case COND_NOT_EQUAL:
				if(iValue1 != iValue2)
				{
					return CMD_IF_TRUE;
				}
			break;
			case COND_GREATER_THAN:
				if(iValue1 > iValue2)
				{
					return CMD_IF_TRUE;
				}
			break;
			case COND_LESS_THAN:
				if(iValue1 < iValue2)
				{
					return CMD_IF_TRUE;
				}
			break;
			case COND_GREATER_THAN_EQUAL:
				if(iValue1 >= iValue2)
				{
					return CMD_IF_TRUE;
				}
			break;
			case COND_LESS_THAN_EQUAL:
				if(iValue1 <= iValue2)
				{
					return CMD_IF_TRUE;
				}
			break;
		}
	}

	return CMD_IF_FALSE;
}


/*********************************************************************
*
* ShScripts
* @catagory	Shell Command
*
* @brief	List active scripts
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShScripts(uint8_t bPort, int argc, char *argv[])
{
	char szMode[16];

#ifdef REF
	char *args[ARR_SIZE];
	size_t nargs;

	uint32_t Variable[4];
	uint32_t Return;
#endif

	if(argc == 3)
	{
		if(CurrentScript != -1)
		{
			// search for the names script add do something with it
			for(int i = CurrentScript; i >= 0; i--)
			{
				if(stricmp(ScriptNest[i].name, argv[1]) == 0)
				{
					if(stricmp("kill", argv[2]) == 0)
					{
						ScriptNest[CurrentScript].RunMessage = SCRIPT_EXIT;
					}
					else if(stricmp("pause", argv[2]) == 0)
					{
						ScriptNest[CurrentScript].RunMessage = SCRIPT_PAUSE;
					}
					else if(stricmp("resume", argv[2]) == 0)
					{
						ScriptNest[CurrentScript].RunMessage = SCRIPT_RESUME;
					}
					else
					{
						return CMD_BAD_PARAMS;
					}
					return CMD_OK;
				}
			}
			return CMD_BAD_PARAMS;
		}
	}
	else
	{
		ShNL(bPort);
		if(CurrentScript != -1)
		{
			ShFieldOut(bPort, "Name", 15);
			ShFieldOut(bPort, "Mode", 10);
			ShNL(bPort);

			for(int i = CurrentScript; i >= 0; i--)
			{
				ShFieldOut(bPort, ScriptNest[i].name, 15);

				strcpy(szMode, RunModeName[ScriptNest[i].RunMode]);
	//			if(ScriptNest[i].RunMode == LOOP)
	//			{
	//				strcat(szMode, "-");
	////				strcat(szMode, atoi(ScriptNest[i].LoopCount));
	//			}
	//			else
				if(ScriptNest[i].RunMode == RUN_DELAY)
				{
					strcat(szMode, "-");
	//				strcat(szMode, atoi(ScriptNest[i].DelayCount));
				}
				ShFieldOut(bPort, szMode, 10);
				ShNL(bPort);
			}
		}
		else
		{
			ShFieldOut(bPort, "No Running scripts", 0);
			ShNL(bPort);
		}
		return CMD_OK;
	}
	return CMD_BAD_PARAMS;
}


/*********************************************************************
*
* ShIf
* @catagory	Script Command
*
* @brief	If <condition> - See conditions above 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShIf(uint8_t bPort, int argc, char *argv[])
{
	if(argc == 4)
	{
		return TestCondition(argv[1], argv[2], argv[3]);
	}
	return CMD_IF_FALSE;
}

/*********************************************************************
*
* ShElse
* @catagory	Script Command
*
* @brief	Alternate execution of an If 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShElse(uint8_t bPort, int argc, char *argv[])
{
	return CMD_OK;
}

/*********************************************************************
*
* ShEndif
* @catagory	Script Command
*
* @brief	End of an If 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShEndif(uint8_t bPort, int argc, char *argv[])
{

	//RunMode = RUN_NORMAL;
	return CMD_OK;
}

/*********************************************************************
*
* ShLoop
* @catagory	Script Command
*
* @brief	Loop a group of shell commands in a script N times 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShLoop(uint8_t bPort, int argc, char *argv[])
{
	uint32_t cnt;

	if(argc == 2)
	{
		if(CurrentScript != -1)
		{
			cnt = atoi(argv[1]);
			if(cnt == 0)
			{
				ScriptNest[CurrentScript].LoopCount = LOOP_FOREVER;
			}
			else
			{
				ScriptNest[CurrentScript].LoopCount = cnt;
			}
			ScriptNest[CurrentScript].LoopPosition = ScriptNest[CurrentScript].Scriptfp.fptr;
		}
	}
	return CMD_OK;
}


/*********************************************************************
*
* ShEndLoop
* @catagory	Script Command
*
* @brief	The end of a Loop 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShEndLoop(uint8_t bPort, int argc, char *argv[])
{

	if(CurrentScript != -1)
	{
		if(ScriptNest[CurrentScript].LoopCount)
		{
			if(ScriptNest[CurrentScript].LoopCount != LOOP_FOREVER)
			{
				ScriptNest[CurrentScript].LoopCount--;
			}
			if(ScriptNest[CurrentScript].LoopCount != 0)
			{
                f_lseek(&ScriptNest[CurrentScript].Scriptfp, ScriptNest[CurrentScript].LoopPosition);	// Move file pointer to the captured position
			}
		}
	}
	return CMD_OK;
}

/*********************************************************************
*
* ShBreak
* @catagory	Script Command
*
* @brief	Break out of a Loop 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShBreak(uint8_t bPort, int argc, char *argv[])
{

	if(CurrentScript != -1)
	{
		ScriptNest[CurrentScript].LoopCount = 0;
	}
	return SKIP_LOOP;
}

/*********************************************************************
*
* ShPrompt
* @catagory	Script Command
*
* @brief	Issue a prompt at the end of a script 
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShPrompt(uint8_t bPort, int argc, char *argv[])
{

	Prompt(bPort);
	return CMD_OK;
}


/*********************************************************************
*
* ShReturn
* @catagory	Script Command
*
* @brief	Set the script return variable
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShReturn(uint8_t bPort, int argc, char *argv[])
{

	if(CurrentScript != -1)
	{
		ScriptNest[CurrentScript].Return = atoi(argv[1]);
	}
	return CMD_OK;
}


/*********************************************************************
*
* ShInput
* @catagory	Script Command
*
* @brief	Display the inputs
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShInputs(uint8_t bPort, int argc, char *argv[])
{

    ShNL(bPort);

	ShFieldOut(bPort, "Inputs: ", 0);

	if(GetInput1())
	{
		ShFieldOut(bPort, "1=ON ", 0);
	}
	else
	{
		ShFieldOut(bPort, "1=OFF ", 0);
	}

	if(GetInput2())
	{
		ShFieldOut(bPort, "2=ON ", 0);
	}
	else
	{
		ShFieldOut(bPort, "2=OFF ", 0);
	}

	if(GetInput3())
	{
		ShFieldOut(bPort, "3=ON ", 0);
	}
	else
	{
		ShFieldOut(bPort, "3=OFF ", 0);
	}

	if(GetInput4())
	{
		ShFieldOut(bPort, "4=ON ", 0);
	}
	else
	{
		ShFieldOut(bPort, "4=OFF ", 0);
	}

    ShNL(bPort);


	return CMD_OK;
}


#ifdef NOT_USED
/*********************************************************************
*
* ShVar
* @catagory	Script Command
*
* @brief	Get or set a script variable
*
* @param	bPort - port that issued this command
*			argc - argument count
*			argv - argc array of arguments
*
* @return	CMD_RETURN - shell result
*
*********************************************************************/
CMD_RETURN ShVar(uint8_t bPort, int argc, char *argv[])
{
	int idx;

	if(argc == 2)
	{
		idx = atoi(argv[1]);

		if(CurrentScript != -1)
		{
			ScriptNest[CurrentScript].Variable[idx];

			return CMD_OK;
		}
	}
	return CMD_BAD_PARAMS;
}
#endif



