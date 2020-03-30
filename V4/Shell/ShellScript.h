/*********************************************************************
*
* SOURCE FILENAME:  ShellScript.h
*
* DATE CREATED:     Sep 13, 2011
*
* PROGRAMMER:       Karl
*
* DESCRIPTION:      <add description>
*
* COPYRIGHT (c)   All Rights Reserved.
*
*********************************************************************/
#ifndef SHELLSCRIPT_H_
#define SHELLSCRIPT_H_

#include "Shell.h"

/*********************************************************************
*
*                            DEFINITIONS
*
*********************************************************************/

/*********************************************************************
*
*                            FUNCTION PROTOTYPES
*
*********************************************************************/
// scripting
//CMD_RETURN ShRun(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShIf(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShElse(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShEndif(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShLoop(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShEndLoop(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShBreak(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShPrompt(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShScripts(uint8_t bPort, int argc, char *argv[]);

#endif /* SHELLSCRIPT_H_ */
