/*
 * Variable.h
 *
 *  Created on: May 1, 2018
 *      Author: Karl Kobel
 */

// How to add a new variable/setting
// Variables.h			add the variable extern
// ObjectNames.c		add a string and initialize it (the convention is lower case)
// ObjectNames.h		add an extern to the string
// Variables.c			add the variable
//						add the string, variable, and action to the Variables table
//						if special, add the conversion to ASCII in VarToString,
//						and the conversion to a saved value in SetVariable


#ifndef VARIABLE_H_
#define VARIABLE_H_

#define VAR_TYPE_INT		1
#define VAR_TYPE_STRING		2
#define VAR_TYPE_VER		3
#define VAR_TYPE_TIME		4
#define VAR_TYPE_DATE		5
#define VAR_TYPE_TIME_FMT	6
#define VAR_TYPE_DATE_FMT	7
#define VAR_TYPE_ON_OFF		8
#define VAR_TYPE_PORT		9
#define VAR_TYPE_IP			10

#define VAR_TYPE_TRACK		20

#define VAR_TYPE_SPEED		40
#define VAR_TYPE_DIR		41
#define VAR_TYPE_FUNCTION	42

#define VAR_TYPE_MAX		63
#define VAR_TYPE_MASK		0x3f

#define VAR_TYPE_PERSIST	0x100
#define VAR_TYPE_READ_ONLY 	0x200
#define VAR_TYPE_BAT_BACKED	0x400



extern uint32_t lVersion;
extern uint32_t lSerialNumber;

//extern uint32_t nClockRatio;
extern uint32_t bClock12_24;

extern uint32_t CabPort;

extern uint32_t IpAddress;
extern uint32_t IpMask;
extern uint32_t GwAddress;
extern uint32_t DHCP;

extern char szPathVar[];

typedef struct var_table_t
{
	const char* szCmdString;
	void* var;
	uint32_t type;
	char* pszDefault;
	char* pszHelp;
} VAR_TABLE;

extern const VAR_TABLE VarCmdTable[];

//#define NUM_VARIABLES	(sizeof(VarCmdTable) / sizeof(VAR_TABLE))
#define NUM_VARIABLES 13

extern int IsVariable(char* pBuffer);

extern void ShowVariables(uint8_t bPort);

extern int FindVariable(char* szObject);

extern int GetNumVariables(void);
extern char* GetVariableName(uint8_t idx);
extern char* GetVariableHelp(uint8_t idx);

extern void GetVariable(const char* szObject, char* pStrValue, int max_len);
extern int SetVariable(const char* szObject, char* pStrValue);

extern char* VarToString(uint32_t idx);

#endif /* VARIABLE_H_ */
