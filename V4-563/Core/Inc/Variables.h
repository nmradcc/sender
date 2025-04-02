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
//						add the variable to the case in GetVariableValue and add a case if special

enum
{
	MV_ADD,
	MV_SUBTRACT,
	MV_MULTIPLY,
	MV_DIVIDE,
} ;


#ifndef VARIABLE_H_
#define VARIABLE_H_

#define VAR_TYPE_INT		1
#define VAR_TYPE_STRING		2
#define VAR_TYPE_FLOAT		3
#define VAR_TYPE_VER		4
#define VAR_TYPE_TIME		5
#define VAR_TYPE_DATE		6
#define VAR_TYPE_TIME_FMT	7
#define VAR_TYPE_DATE_FMT	8
#define VAR_TYPE_ON_OFF		9
#define VAR_TYPE_PORT		10
#define VAR_TYPE_IP			11
#define VAR_TYPE_RLED		12
#define VAR_TYPE_YLED		13
#define VAR_TYPE_GLED		14
#define VAR_TYPE_INPUTS		15
#define VAR_TYPE_LOOP_CNT	16
#define VAR_TYPE_THEME		17

#define VAR_TYPE_TRACK		20
#define VAR_TYPE_TRACK_IDLE	21
#define VAR_TYPE_TRACK_LOCK	22

#define VAR_TYPE_SPEED		40
#define VAR_TYPE_DIR		41
#define VAR_TYPE_FUNCTION	42

#define VAR_TYPE_MAX		63
#define VAR_TYPE_MASK		0x3f

#define VAR_TYPE_PERSIST	0x100
#define VAR_TYPE_READ_ONLY 	0x200
#define VAR_TYPE_BAT_BACKED	0x400

#define VAR_TYPE_HIDDEN		0x8000



extern uint32_t lVersion;
extern uint32_t lSerialNumber;
extern char szBuildDateVar[];

//extern uint32_t nClockRatio;
//extern uint32_t bClock12_24;
extern uint32_t bTimeFmt;
extern uint32_t bDateFmt;

extern uint32_t CabPort;

extern uint32_t IpAddress;
extern uint32_t IpMask;
extern uint32_t GwAddress;
extern uint32_t DHCP;
extern char szDnsName[];

extern char szPathVar[];

extern uint32_t Theme;

extern float TrackCurrent;
extern float TrackVoltage;

extern uint32_t TrackIdle;
//extern uint32_t TrackLck;

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
#define NUM_VARIABLES 20

extern int IsVariable(char* pBuffer);
//extern int IsVariableHidden(char* pBuffer);
extern int IsVariableVisible(uint8_t idx);

extern void ShowVariables(uint8_t bPort);

extern int FindVariable(char* szObject);

extern int GetNumVariables(void);
extern char* GetVariableName(uint8_t idx);
extern char* GetVariableHelp(uint8_t idx);

extern void GetVariable(const char* szObject, char* pStrValue, int max_len);
extern int SetVariable(const char* szObject, char* pStrValue);
extern int MathVariable(const char* szObject, char* pStrValue, const int op);

extern uint32_t GetVariableValue(int idx);

extern char* VarToString(uint32_t idx);

extern int GetNextPath(char* pStr, char* path);

#endif /* VARIABLE_H_ */
