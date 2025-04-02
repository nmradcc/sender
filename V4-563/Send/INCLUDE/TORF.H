/*****************************************************************************
 *
 * File:                 $Workfile: TORF.H $
 * Path:                 $Logfile: /NMRA/INCLUDE/TORF.H $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        8/21/2023
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 2 $
 * Last Modified on:     $Modtime: 8/21/23 4:57p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 8/21/23 4:59p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *  TORF.H    -   Declares to torf() method that return the const char *
 *                "true" or "falso" for a bool input.
 *
 *****************************************************************************/

#ifndef TORF_H_DECLARED
#define TORF_H_DECLARED   "@(#) $Workfile: TORF.H $, $Revision: 2 $, $Modtime: 8/21/23 4:57p $.$"

/*
 * return "true" or "false" for bool input.
 */
const char *
torf( bool ib )
{
	return ib ? "true" : "false";
}

#endif /* TORF_H_DECLARED */


/*****************************************************************************
 * $History: TORF.H $
 * 
 * *****************  Version 2  *****************
 * User: Kenw         Date: 8/21/23    Time: 4:59p
 * Updated in $/NMRA/INCLUDE
 * Fix sccsid string by changing $Data $ to !Modtime: $.
 * 
 * *****************  Version 1  *****************
 * User: Kenw         Date: 8/21/23    Time: 4:51p
 * Created in $/NMRA/INCLUDE
 * TORF,H declares the torf() method.
 * 
 *
 *****************************************************************************/
