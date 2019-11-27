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
//#include "Variables.h"

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
    LOOP,
	END_LOOP,
	SKIP_LOOP,
    RUN_DELAY,
} RUN_MODE;

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
	FIL* Scriptfp;
	char buffer[BUFFER_SIZE];
	char *args[ARR_SIZE];
	size_t nargs;
    uint8_t bPort;
    uint8_t LoopCount;
    uint32_t LoopPosition;
    uint32_t DelayCount;
	RUN_MODE RunMode;
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
		ScriptNest[CurrentScript].Scriptfp = fp;
		ScriptNest[CurrentScript].RunMode = RUN_NORMAL;
		ScriptNest[CurrentScript].LoopCount = 0;
		ScriptNest[CurrentScript].DelayCount = 0;
		ScriptNest[CurrentScript].bPort = bPort;

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
    
    f_close(ScriptNest[CurrentScript].Scriptfp);
    CurrentScript--;
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
                if(getLine(cs->Scriptfp, cs->buffer, BUFFER_SIZE) != 0)
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
					
                    //iCmdIndex = FindCommand(cs->buffer, &cs->nargs, cs->args);
                    iCmdIndex = FindCommand(cmdbuffer, &cs->nargs, cs->args);
                    if(iCmdIndex != -1)
                    {
                        if(strcasecmp(ShellTable[iCmdIndex].szCommand, "pause") == 0)
                        {
                        	cs->DelayCount = atoi(cs->args[1]);
                        	cs->RunMode = RUN_DELAY;
                        }
                        else
                        {
                            ret = ExecuteCommand(iCmdIndex, ScriptNest[CurrentScript].bPort, ScriptNest[CurrentScript].nargs, ScriptNest[CurrentScript].args);
                            if(ret & CMD_IF_TRUE)
                            {
                            	cs->RunMode = RUN_IF;
                            }
                            else if(ret & CMD_IF_FALSE)
                            {
                            	cs->RunMode = SKIP_IF;
                            }
                        }
                    }
                }
                else
                {
                    ScriptDone();
                }
            break;

            case SKIP_IF:
                if(getLine(cs->Scriptfp, cs->buffer, sizeof(BUFFER_SIZE)) != (uint8_t)EOF)
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
                if(getLine(cs->Scriptfp, cs->buffer, sizeof(BUFFER_SIZE)) != (uint8_t)EOF)
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

            case LOOP:
            	cs->LoopPosition = cs->Scriptfp->fptr;
            	cs->RunMode = RUN_NORMAL;
            break;

            case END_LOOP:
                f_lseek (cs->Scriptfp, cs->LoopPosition);	// Move file pointer to the captured position
                cs->RunMode = RUN_NORMAL;
            break;

            case SKIP_LOOP:
                if(getLine(cs->Scriptfp, cs->buffer, sizeof(BUFFER_SIZE)) != (uint8_t)EOF)
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
		iValue1 = atoi(szValue1);
		iValue2 = atoi(szValue2);

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
* ShIf
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
	if(argc == 2)
	{
		if(CurrentScript != -1)
		{
			ScriptNest[CurrentScript].LoopCount = atoi(argv[1]);
		}
	}
	return LOOP;
}

/*********************************************************************
*
* ShEndLoop
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
			ScriptNest[CurrentScript].LoopCount--;
			if(ScriptNest[CurrentScript].LoopCount == 0)
			{
				return CMD_OK;
			}
			else
			{
				return END_LOOP;
			}
		}
	}
	return CMD_OK;
}

/*********************************************************************
*
* ShBreak
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




