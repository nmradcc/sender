/*--------------------------------------------------------------------------*/
/* FILE: zsend.c    (Opus zmodem transmitter)                               */
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

#include "zmodem.h"

long cur_baud = 115200;
FIL Infile;

/*--------------------------------------------------------------------------*/
/* Global routines                                                          */
/*--------------------------------------------------------------------------*/
int send_Zmodem(byte *,byte *,int,int,int);


/*--------------------------------------------------------------------------*/
/* Private routines                                                         */
/*--------------------------------------------------------------------------*/
static void ZS_SendBinaryHeader(unsigned short ,byte *);
static void ZS_SendData(byte *,int,unsigned short);
static void ZS_SendByte(byte);
static int  ZS_GetReceiverInfo(void);
static int  ZS_SendFile(int);
static int  ZS_SendFileData(void);
static int  ZS_SyncWithReceiver(void);
static void ZS_EndSend(void);



/*--------------------------------------------------------------------------*/
/* Private data                                                             */
/*--------------------------------------------------------------------------*/
//static FILE *Infile;             /* Handle of file being sent               */
static byte *Txbuf;              /* Pointer to transmit buffer              */
static long  Strtpos;            /* Starting byte position of download      */
static long  Txpos;              /* Transmitted file position               */
static int Rxbuflen;             /* Receiver's max buffer length            */


/*--------------------------------------------------------------------------*/
/* External data not otherwise declared                                     */
/*--------------------------------------------------------------------------*/
extern char *FLAGGING_msg;
extern char *NOTSENT_msg;
extern char *TRUNC_msg;
extern char *KBD_msg;



/*--------------------------------------------------------------------------*/
/* SEND ZMODEM (send a file)                                                */
/*   returns TRUE (1) for good xfer, FALSE (0) for bad                      */
/*   sends one file per call; 'fsent' flags start and end of batch          */
/*--------------------------------------------------------------------------*/
//int send_Zmodem(byte *fname, byte *alias, int doafter, int fsent, int wazoo)
int ZmodemSend(byte *fname)
{
	register byte *p;
//	register byte *q;
	struct stat    f;

	int   i;
	int   rc;


//k      _BRK_DISABLE();
//k      XON_ENABLE();

//k      errno    = 0;
	z_size   = 0;
//k      Infile   = NULL;
	Txbuf    = NULL;


//	if (fname) set_xy("");


//	n_disable();
	Z_PutString("rz\r");
	Z_PutLongIntoHeader(0L);
	Z_SendHexHeader(ZRQINIT, Txhdr);
	Rxtimeout   = 20;
	if (ZS_GetReceiverInfo() == ERROR) return FALSE;


	Rxtimeout   = (int )(614400L/(long )cur_baud);
	Rxtimeout  /= 10;

	if (Rxtimeout < 10) Rxtimeout = 10;
	if (Rxtimeout > 58) Rxtimeout = 58;


	if (fname == NULL) goto Done;

	/*--------------------------------------------------------------------*/
	/* Prepare the file for transmission.  Just ignore file open errors   */
	/* because there may be other files that can be sent.                 */
	/*--------------------------------------------------------------------*/
	Filename = fname;
//k      CLEAR_IOERR();
//k      Infile   = fopen(Filename,read_binary);
//k      if (had_error(OPEN_msg,Filename)) return OK;
      
	Txbuf    = zalloc();

	/*--------------------------------------------------------------------*/
	/* Send the file                                                      */
	/*--------------------------------------------------------------------*/
	rc       = TRUE;


	/*--------------------------------------------------------------------*/
	/* Display outbound filename, size, and ETA for sysop                 */
	/*--------------------------------------------------------------------*/
//k      fstat(fileno(Infile), &f);

//k      message(NULL);
//k      cprintf( "Z-Send %s, %ldb",Filename,f.st_size);

//	i  = (int)(f.st_size*10/cur_baud+27)/54;
//	if (i) cprintf(", %d min.",i);

//k      set_xy(NULL);

	/*--------------------------------------------------------------------*/
	/* Get outgoing file name; no directory path, lower case              */
	/*--------------------------------------------------------------------*/
//	for (p=(alias!=NULL)?alias:Filename, q=Txbuf ; *p; )
//	{
//		if ((*q++ = tolower(*p)) == '\\') q = Txbuf;
//		p++;
//	}
//
//	*q++  = '\0';
//	p     = q;

	/*--------------------------------------------------------------------*/
	/* Zero out remainder of file header packet                           */
	/*--------------------------------------------------------------------*/
//	while (q < (Txbuf + KSIZE)) *q++ = '\0';

	/*--------------------------------------------------------------------*/
	/* Store filesize, time last modified, and file mode in header packet */
	/*--------------------------------------------------------------------*/
//	sprintf(p, "%lu %lo %o", f.st_size, f.st_mtime, f.st_mode);

	/*--------------------------------------------------------------------*/
	/* Transmit the filename block and begin the download                 */
	/*--------------------------------------------------------------------*/
//      throughput(0,0L);


	/*--------------------------------------------------------------------*/
	/* Check the results                                                  */
	/*--------------------------------------------------------------------*/
	ZS_SendFile(1+strlen(p)+(p-Txbuf));


#ifdef NOT_USED
	 {

		case ERROR:
			/*--------------------------------------------------*/
			/* Something tragic happened                        */
			/*--------------------------------------------------*/
//			if (wazoo)
//			{
//k             status_line( NOTSENT_msg );
//k             mdm_hangup();
//			}
			goto Err_Out;
			break;

        case ZOK:
            	/*--------------------------------------------------*/
                        /* File was sent                                    */
                        /*--------------------------------------------------*/
//                        CLEAR_IOERR();
//k                        fclose(Infile);
//                        had_error(CLOSE_msg,Filename);
//k                        Infile   = NULL;

//                        status_line( "=DL %s", Filename );

                        /*--------------------------------------------------*/
                        /* Special WaZOO file handling following a          */
                        /* successful transmission.                         */
                        /*--------------------------------------------------*/
                        switch(doafter)
                        {
							case DELETE_AFTER:
							/*-----------------------*/
							/* Delete File           */
							/*-----------------------*/
//k                                                   CLEAR_IOERR();
				   unlink(Filename);
//                                                   had_error(UNLINK_msg,Filename);
				   break;
   
                              case TRUNC_AFTER:    /*-----------------------*/
                                                   /* Truncate File         */
                                                   /*-----------------------*/
//k                                                   CLEAR_IOERR();
//k                                                   i = open(Filename,O_TRUNC,S_IWRITE);
//k                                                   had_error( TRUNC_msg, Filename );
//k                                                   status_line(FLAGGING_msg,Filename);
//k                                                   close(i);
                                                   break;
                           } /* switch */


            default:    /*--------------------------------------------------*/
                        /* Probably a ZSKIP                                 */
                        /*--------------------------------------------------*/
                        goto Done;

         } /* switch */
#endif

Err_Out:
      rc = FALSE;

Done:
//k      if (Infile) fclose(&Infile);
      if (Txbuf)  free(Txbuf);

//      if (fsent < 0)
    	  ZS_EndSend();

      return rc;

   } /* send_Zmodem */






/*--------------------------------------------------------------------------*/
/* ZS SEND BINARY HEADER                                                    */
/* Send ZMODEM binary header hdr of type type                               */
/*--------------------------------------------------------------------------*/
static void ZS_SendBinaryHeader(unsigned short type, byte *hdr)
   {
      register byte          *hptr;
      register unsigned short crc;
      int                     n;

      SENDBYTE(ZPAD);
      SENDBYTE(ZDLE);

      SENDBYTE(ZBIN);
      ZS_SendByte(type);

      crc = Z_UpdateCRC(type, 0);

      hptr  = hdr;
      for (n=4; --n >= 0;)
         {
            ZS_SendByte(*hptr);
            crc = Z_UpdateCRC(((unsigned short)(*hptr++)), crc);
         }

      crc = Z_UpdateCRC(0,crc);
      crc = Z_UpdateCRC(0,crc);

      ZS_SendByte(crc>>8);
      ZS_SendByte(crc);

//k      if (type != ZDATA) wait_for_clear();

         } /* ZS_SendBinaryHeader */





/*--------------------------------------------------------------------------*/
/* ZS SEND DATA                                                             */
/* Send binary array buf with ending ZDLE sequence frameend                 */
/*--------------------------------------------------------------------------*/
static void ZS_SendData(byte *buf, int length, unsigned short frameend)
{
	register unsigned short crc;
	int t;

	crc = 0;
	for (;--length >= 0;)
	{
		ZS_SendByte(*buf);
		crc = Z_UpdateCRC(((unsigned short)(*buf++)), crc);
	}

	SENDBYTE(ZDLE);
	SENDBYTE(frameend);
	crc = Z_UpdateCRC(frameend, crc);

	crc = Z_UpdateCRC(0,crc);
	crc = Z_UpdateCRC(0,crc);
	ZS_SendByte(crc>>8);
	ZS_SendByte(crc);

	if (frameend == ZCRCW)
	{
		SENDBYTE(XON);
//		wait_for_clear();
	}
} /* ZS_SendData */






/*--------------------------------------------------------------------------*/
/* ZS SEND BYTE                                                             */
/* Send character c with ZMODEM escape sequence encoding.                   */
/* Escape XON, XOFF. Escape CR following @ (Telenet net escape)             */
/*--------------------------------------------------------------------------*/
static void ZS_SendByte(byte c)
   {
      static byte lastsent;

      switch (c)
         {
            case 015:
            case 0215:  /*--------------------------------------------------*/
                        /*                                                  */
                        /*--------------------------------------------------*/
                        if ((lastsent & 0x7F) != '@') goto SendIt;
            case 020:
            case 021:
            case 023:
            case 0220:
            case 0221:
            case 0223:
            case ZDLE:  /*--------------------------------------------------*/
                        /* Quoted characters                                */
                        /*--------------------------------------------------*/
                        SENDBYTE(ZDLE);
                        c ^= 0x40;

            default:    /*--------------------------------------------------*/
                        /* Normal character output                          */
SendIt:                 /*--------------------------------------------------*/
                        SENDBYTE(lastsent = c);

         } /* switch */

   } /* ZS_SendByte */






/*--------------------------------------------------------------------------*/
/* ZS GET RECEIVER INFO                                                     */
/* Get the receiver's init parameters                                       */
/*--------------------------------------------------------------------------*/
static int ZS_GetReceiverInfo(void)
   {
      int   n;
      int   rxflags;

      for (n=10; --n>=0; )
         {

            switch ( Z_GetHeader(Rxhdr) )
               {
                  case ZCHALLENGE:  /*--------------------------------------*/
                                    /* Echo receiver's challenge number     */
                                    /*--------------------------------------*/
                                    Z_PutLongIntoHeader(Rxpos);
                                    Z_SendHexHeader(ZACK, Txhdr);
                                    continue;

                  case ZCOMMAND:    /*--------------------------------------*/
                                    /* They didn't see our ZRQINIT          */
                                    /*--------------------------------------*/
                                    Z_PutLongIntoHeader(0L);
                                    Z_SendHexHeader(ZRQINIT, Txhdr);
                                    continue;

                  case ZRINIT:      /*--------------------------------------*/
                                    /*                                      */
                                    /*--------------------------------------*/
                                    Rxbuflen = ((word )Rxhdr[ZP1]<<8)|Rxhdr[ZP0];
                                    return 0; //ZOK;

                  case ZCAN:
                  case RCDO:
                  case TIMEOUT:     /*--------------------------------------*/
                                    /*                                      */
                                    /*--------------------------------------*/
                                    return ERROR;

                  case ZRQINIT:     /*--------------------------------------*/
                                    /*                                      */
                                    /*--------------------------------------*/
                                    if (Rxhdr[ZF0] == ZCOMMAND) continue;

                  default:          /*--------------------------------------*/
                                    /*                                      */
                                    /*--------------------------------------*/
                                    Z_SendHexHeader(ZNAK, Txhdr);
                                    continue;
               } /* switch */

         } /* for */

      return ERROR;

   } /* ZS_GetReceiverInfo */





/*--------------------------------------------------------------------------*/
/* ZS SEND FILE                                                             */
/* Send ZFILE frame and begin sending ZDATA frame                           */
/*--------------------------------------------------------------------------*/
static int ZS_SendFile(int blen)
   {
      register int c;

      while(1)

         {

//            if (((KEYPRESS()) and (READKB()==27)))
//               {
//                  send_can();
//                  z_log( KBD_msg );
//                  return ERROR;
//               }
//            else if (!CARRIER) return ERROR;

   
            Txhdr[ZF0] = LZCONV;    /* Default file conversion mode */
            Txhdr[ZF1] = LZMANAG;   /* Default file management mode */
            Txhdr[ZF2] = LZTRANS;   /* Default file transport mode */
            Txhdr[ZF3] = 0;
            ZS_SendBinaryHeader(ZFILE, Txhdr);
            ZS_SendData(Txbuf, blen, ZCRCW);
   
Again:
            switch (c = Z_GetHeader(Rxhdr))
               {

                  case ZRINIT:   /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
                                 goto Again;

                  case ZCAN:     
                  case ZCRC:     
                  case RCDO:     
                  case TIMEOUT:  
                  case ZFIN:
                  case ZABORT:   
                                 /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
                                 return ERROR;
   
                  case ZSKIP:    /*-----------------------------------------*/
                                 /* Other system wants to skip this file    */
                                 /*-----------------------------------------*/
                                 return c;
   
                  case ZRPOS:    /*-----------------------------------------*/
                                 /* Resend from this position...            */
                                 /*-----------------------------------------*/
//k                                 fseek(Infile, Rxpos, SEEK_SET);
                                 Strtpos = Txpos = Rxpos;
//                                 CLEAR_INBOUND();
                                 return ZS_SendFileData();
               } /* switch */

         } /* while */

   } /* ZS_SendFile */




/*--------------------------------------------------------------------------*/
/* ZS SEND FILE DATA                                                        */
/* Send the data in the file                                                */
/*--------------------------------------------------------------------------*/
static int ZS_SendFileData(void)
   {

      register int c, e;
      word         newcnt, blklen, maxblklen, goodblks, goodneeded = 1;
      byte        *p;


      maxblklen = (cur_baud<300) ? 128 : cur_baud/300*256;

      if (maxblklen>KSIZE)              maxblklen = KSIZE;
      //if (!wazoo && maxblklen>KSIZE)       maxblklen = KSIZE;
      if (Rxbuflen && maxblklen>Rxbuflen)  maxblklen = Rxbuflen;
      blklen = maxblklen;


SomeMore:

      if (CHAR_AVAIL())
         {
WaitAck:

            switch (c = ZS_SyncWithReceiver())
               {
                  default:       /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
//                                 z_log(Cancelled_msg);
//                                 errno = 0;
//k                                 fclose(Infile);
//                                 had_error(CLOSE_msg,Filename);
                                 return ERROR;

                  case ZSKIP:    /*-----------------------------------------*/
                                 /* Skip this file                          */
                                 /*-----------------------------------------*/
                                 return c;

                  case ZACK:     /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
                                 break;

                  case ZRPOS:    /*-----------------------------------------*/
                                 /* Resume at this position                 */
                                 /*-----------------------------------------*/
                                 blklen = (blklen>>2 > 64) ? blklen>>2 : 64;
                                 goodblks = 0;
                                 goodneeded = (goodneeded<<1) | 1;
                                 break;

                  case ZRINIT:   /*-----------------------------------------*/
                                 /* Receive init                            */
                                 /*-----------------------------------------*/
//                                 throughput(1,Txpos-Strtpos);
                                 return ZOK;
               } /* switch */

            while (CHAR_AVAIL())
               {
                  switch (MODEM_IN())
                     {
                        case CAN:
                        case RCDO:
                        case ZPAD:  goto WaitAck;
                     } /* switch */
               } /* while */

         } /* while */

      newcnt = Rxbuflen;
      Z_PutLongIntoHeader(Txpos);
      ZS_SendBinaryHeader(ZDATA, Txhdr);

      do
         {

//            if (((KEYPRESS()) and (READKB()==27)))
//               {
//                  send_can();
//                  z_log( KBD_msg );
//                  goto oops;
//               }

//            if (!CARRIER) goto oops;

//k            if ((c=fread(Txbuf,1,blklen,Infile))!=z_size)
               {
//                  gotoxy( locate_x+10, locate_y );
//                  cputs( ultoa(((unsigned long )(z_size=c)),e_input,10) );
//                  putch(' ');
               }

            if (c < blklen) e = ZCRCE;
            else if (Rxbuflen && (newcnt -= c) <= 0) e = ZCRCW;
            else e = ZCRCG;

            ZS_SendData(Txbuf, c, e);

//            gotoxy( locate_x, locate_y );
//            cputs( ultoa(((unsigned long )Txpos),e_input,10) );
//            putch(' ');

            Txpos += c;
            if (blklen<maxblklen && ++goodblks>goodneeded)
               {
                  blklen = (blklen<<1 < maxblklen) ? blklen<<1 : maxblklen;
                  goodblks = 0;
               }

            if (e == ZCRCW) goto WaitAck;

            while (CHAR_AVAIL())
               {
                  switch (MODEM_IN())
                     {
                        case CAN:
                        case RCDO:
                        case ZPAD:  /*--------------------------------------*/
                                    /* Interruption detected;               */
                                    /* stop sending and process complaint   */
                                    /*--------------------------------------*/
//                                    z_message("Trouble?");
//                                    CLEAR_OUTBOUND();
                                    ZS_SendData(Txbuf, 0, ZCRCE);
                                    goto WaitAck;
                     } /* switch */
               } /* while */
      
         } /* do */
      while (e == ZCRCG);

      while(1)
         {
            Z_PutLongIntoHeader(Txpos);
            ZS_SendBinaryHeader(ZEOF, Txhdr);

            switch (ZS_SyncWithReceiver())
               {
                  case ZACK:     /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
                                 continue;

                  case ZRPOS:    /*-----------------------------------------*/
                                 /* Resume at this position...              */
                                 /*-----------------------------------------*/
                                 goto SomeMore;

                  case ZRINIT:   /*-----------------------------------------*/
                                 /* Receive init                            */
                                 /*-----------------------------------------*/
//                                 throughput(1,Txpos-Strtpos);
                                 return ZOK;

                  case ZSKIP:    /*-----------------------------------------*/
                                 /* Request to skip the current file        */
                                 /*-----------------------------------------*/
//                                 z_log(Skip_msg);
//                                 CLEAR_IOERR();
//k                                 fclose(Infile);
//                                 had_error(CLOSE_msg,Filename);
                                 return c;

                  default:       /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
oops:
//								z_log(Cancelled_msg);
//                                 errno = 0;
//k                                 fclose(Infile);
//                                 had_error(CLOSE_msg,Filename);
                                 return ERROR;
               } /* switch */
         } /* while */

   } /* ZS_SendFileData */






/*--------------------------------------------------------------------------*/
/* ZS SYNC WITH RECEIVER                                                    */
/* Respond to receiver's complaint, get back in sync with receiver          */
/*--------------------------------------------------------------------------*/
static int ZS_SyncWithReceiver(void)
   {
      register int c;
      int          num_errs   = 7;

      while(1)
         {
            c = Z_GetHeader(Rxhdr);
//            CLEAR_INBOUND();
            switch (c)
               {
                  case TIMEOUT:
//                	  z_message( TIME_msg );
                      if ((num_errs--) >= 0) break;

                  case ZCAN:
                  case ZABORT:
                  case ZFIN:
                  case RCDO:
                                 /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
  //                               z_log("Err");
                                 return ERROR;

                  case ZRPOS:    /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
//k                                 rewind(Infile);   /* In case file EOF seen */
//k                                 fseek(Infile, Rxpos, SEEK_SET);
                                 Txpos = Rxpos;
//                                 z_message(NULL);
//                                 cputs("Resending from ");
//                                 cputs( ultoa(((unsigned long )(Txpos)),e_input,10) );
                                 return c;

                  case ZSKIP:    /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
//                                 z_log(Skip_msg);

                  case ZRINIT:   /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
//                                 CLEAR_IOERR();
//k                                 fclose(Infile);
//                                 had_error(CLOSE_msg,Filename);
                                 return c;

                  case ZACK:     /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
//                                 z_message( NULL );
                                 return c;

                  default:       /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
//                                 z_message( IDUNNO_msg );
                                 ZS_SendBinaryHeader(ZNAK, Txhdr);
                                 continue;
               } /* switch */
         } /* while */
   } /* ZS_SyncWithReceiver */




/*--------------------------------------------------------------------------*/
/* ZS END SEND                                                              */
/* Say BIBI to the receiver, try to do it cleanly                           */
/*--------------------------------------------------------------------------*/
static void ZS_EndSend(void)
   {

      while(1)
         {
            Z_PutLongIntoHeader(0L);
            ZS_SendBinaryHeader(ZFIN, Txhdr);

            switch (Z_GetHeader(Rxhdr))
               {
                  case ZFIN:     /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
                                 SENDBYTE('O');
                                 SENDBYTE('O');
//                                 wait_for_clear();
                                 /* fallthrough... */
                  case ZCAN:
                  case RCDO:
                  case TIMEOUT:  /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
                                 return;
               } /* switch */
         } /* while */
   } /* ZS_EndSend */


/* END OF FILE: z_send.c */

