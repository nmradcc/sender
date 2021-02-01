/*********************************************************************
*
* SOURCE FILENAME:  Shell.h
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
#ifndef SHELLCS_H_
#define SHELLCS_H_

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
// command station
CMD_RETURN ShCabStat(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShLocoStat(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShAssign(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShSystemStatus(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShSetLoco(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShFindLoco(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShCabDisplay(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShTrack(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShProgTrack(uint8_t bPort, int argc, char *argv[]);

CMD_RETURN ShProgTrackWriteCV(uint8_t bPort, int argc, char *argv[]);
CMD_RETURN ShProgTrackReadCV(uint8_t bPort, int argc, char *argv[]);


//CMD_RETURN ShCreateLoco(uint8_t bPort, int argc, char *argv[]);


#endif /* SHELLCS_H_ */
