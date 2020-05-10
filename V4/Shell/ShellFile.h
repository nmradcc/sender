/*********************************************************************
*
* SOURCE FILENAME:  ShellFile.h
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
#ifndef SHELLFILE_H_
#define SHELLFile_H_

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
// file system
CMD_RETURN ShDir(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShType(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShDelete(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShFormat(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShMkdir(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShRmdir(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShChdir(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShCWD(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShAtrib(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShCopy(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShRename(uint8_t bPort, int argc, char *argv[]);

#endif /* SHELLfile_H_ */
