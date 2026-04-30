/*
 * GetLine.h
 *
 *  Created on: Feb 19, 2017
 *      Author: Karl Kobel
 */

#ifndef GETLINE_H_
#define GETLINE_H_

extern int getLine(FIL *stream, char *buffer, int iBufLen);

extern int getLine2(FIL *stream, char *buffer, int iBufLen);	// without counting the cr & lf

#endif /* GETLINE_H_ */
