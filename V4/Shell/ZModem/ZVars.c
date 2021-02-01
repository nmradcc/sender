/*--------------------------------------------------------------------------*/
/* FILE: zvars.c (global data used by zmodem)                               */
/*                                                                          */
/*                                                                          */
/*               The Opus Computer-Based Conversation System                */
/*       (c) Copyright 1986, Wynn Wagner III, All Rights Reserved           */
/*                                                                          */
/*      This implementation of Chuck Forsberg's ZMODEM protocol was         */
/*              for Opus by Rick Huebner and Wynn Wagner III                */
/*                                                                          */
/* (MSC/4 with /Zp /Ox)                                                     */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  This module is similar to a routine used by Opus-Cbcs (1.00).  It is    */
/*  provided for your information only.  You will find routines that need   */
/*  to be coded and identifiers to be resolved.                             */
/*                                                                          */
/*  There is absolutely no guarantee that anything here will work.  If you  */
/*  break this routine, you own both pieces.                                */
/*                                                                          */
/*  USAGE:  You may use this material in any program with no obligation     */
/*          as long as there is no charge for your program.  For more       */
/*          information about commercial use, contact the "OPUSinfo HERE"   */
/*          BBS (124/111).                                                  */
/*                                                                          */
/*  NOTE:   There are a couple of things the Opus implementation does that  */
/*          aren't part of the original ZModem protocol.  They all deal     */
/*          with WaZOO type ("ZedZap") netmail and should only show up when */
/*          used under that condition.                                      */
/*                                                                          */
/*             * The maximum packet size can grow larger than 1k.  It is    */
/*               sensitive to the baud rate.  (2400b=2048k; 9600b=8192k)    */
/*             * The sender must be able to send nothing.  In other words,  */
/*               the sending system must be able to initiate and terminate  */
/*               a zmodem send session without having to actually send a    */
/*               file.  Normally this kind of thing would never happen in   */
/*               zmodem.                                                    */
/*                                                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
//#include "legible.h"
#include <stdio.h>
#include "zmodem.h"

                        /*--------------------------------------------------*/
                        /* B S S    S E G M E N T    S T U F F              */
                        /*--------------------------------------------------*/
char  Rxhdr[4];         /* Received header                                  */
char  Txhdr[4];         /* Transmitted header                               */
unsigned long  Rxpos;            /* Received file position                           */
int   Txfcs32;          /* TURE means send binary frames with 32 bit FCS    */
int   Znulls;           /* # of nulls to send at beginning of ZDATA hdr     */

int   Rxtimeout;        /* Tenths of seconds to wait for something          */

char *Filename;         /* Name of the file being up/downloaded             */



                        /*--------------------------------------------------*/
                        /* D A T A    S E G M E N T                         */
                        /*--------------------------------------------------*/
char *Skip_msg          = "SKIP command received";
char *Cancelled_msg     = "Transfer cancelled";
word  z_size            = 0;


/* END OF FILE: z_def.c */

