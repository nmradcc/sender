/*--------------------------------------------------------------------------*/
/* FILE: zreceive.c (Opus zmodem receiver)                                  */
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
#include <string.h>
#include <stdio.h>
#include <ZModem.h>
//#include "file.h"

/*--------------------------------------------------------------------------*/
/* External material                                                        */
/*--------------------------------------------------------------------------*/
extern int  dexists(char *);



/*--------------------------------------------------------------------------*/
/* Local routines                                                           */
/*--------------------------------------------------------------------------*/
int get_Zmodem(byte *,FILE *);

static int  RZ_ReceiveData(char *,int );
static int  RZ_InitReceiver(void);
static int  RZ_ReceiveBatch(FILE *);
static int  RZ_ReceiveFile(FILE *);
static int  RZ_GetHeader(void);
static int  RZ_SaveToDisk(unsigned long *);
static void RZ_AckBibi(void);


/*--------------------------------------------------------------------------*/
/* Private declarations                                                     */
/*--------------------------------------------------------------------------*/
static unsigned long DiskAvail;

extern uint8_t zmodem_port;

/*--------------------------------------------------------------------------*/
/* Private data                                                             */
/*--------------------------------------------------------------------------*/

/* Parameters for ZSINIT frame */
#define ZATTNLEN 32              /* Max length of attention string          */

static char  Attn[ZATTNLEN+1];   /* String rx sends to tx on err            */
static FILE *Outfile;            /* Handle of file being received           */
static char *Secbuf;             /* Pointer to receive buffer               */
static int   Tryzhdrtype;        /* Hdr type to send for Last rx close      */
static char  isBinary;           /* Current file is binary mode             */
static char  EOFseen;            /* indicates cpm eof (^Z) was received     */
static char  Zconv;              /* ZMODEM file conversion request          */
static int   RxCount;            /* Count of data bytes received            */


void Zmodem_SetPort(unsigned char bPort)
{
	zmodem_port = bPort;
}


/*--------------------------------------------------------------------------*/
/* GET ZMODEM                                                               */
/* Receive a batch of files.                                                */
/* returns TRUE (1) for good xfer, FALSE (0) for bad                        */
/* can be called from f_upload or to get mail from a WaZOO Opus             */
/*--------------------------------------------------------------------------*/
//int get_Zmodem(char *rcvpath, FILE *xferinfo)
int ZmodemGet(void)
{
  char        namebuf[PATHLEN];
  int         i;

//k  errno = 0;

//k  _BRK_DISABLE();
//k  XON_ENABLE();
//k  n_disable();

  Secbuf      = NULL;
  Outfile     = NULL;
  z_size      = 0;

//k  DiskAvail   = freespace(toupper(sys.uppath[0])-'@');

  Rxtimeout   = 10;
  Tryzhdrtype = ZRINIT;
  Secbuf      = (char*)zalloc();

//  strcpy(namebuf, rcvpath);
//  Filename = namebuf;

  if (((i=RZ_InitReceiver())==ZCOMPL) ||
	  ((i==ZFILE) && ((RZ_ReceiveBatch(NULL))==ZOK)) )
	 {
		free(Secbuf);
		return 1;
	 }


//  CLEAR_OUTBOUND();
  send_can();          /* transmit at least 10 cans */
//k  wait_for_clear();

  if (Secbuf)    free(Secbuf);
  if (Outfile)   fclose(Outfile);
  return 0;

} /* get_Zmodem */




/*--------------------------------------------------------------------------*/
/* RZ RECEIVE DATA                                                          */
/* Receive array buf of max length with ending ZDLE sequence                */
/* and CRC.  Returns the ending character or error code.                    */
/*--------------------------------------------------------------------------*/
static int RZ_ReceiveData(char *buf, int length)
   {
      register int   c;
      register word  crc;
      int            d;

      crc   = RxCount   = 0;
      while(1)
         {
            if ((c = Z_GetZDL()) & ~0xFF)
               {
CRCfoo:
                  switch (c)
                     {
                        case GOTCRCE:
                        case GOTCRCG:
                        case GOTCRCQ:
                        case GOTCRCW:  /*-----------------------------------*/
                                       /* C R C s                           */
                                       /*-----------------------------------*/
                                       crc = Z_UpdateCRC(((d=c)&0xFF), crc);
                                       if ((c=Z_GetZDL()) & ~0xFF) goto CRCfoo;

                                       crc = Z_UpdateCRC(c, crc);
                                       if ((c=Z_GetZDL()) & ~0xFF) goto CRCfoo;

                                       crc = Z_UpdateCRC(c, crc);
                                       if (crc & 0xFFFF)
                                          {
//                                             z_message( CRC_msg );
                                             return ERROR;
                                          }
                                       return d;

                        case GOTCAN:   /*-----------------------------------*/
                                       /* Cancel                            */
                                       /*-----------------------------------*/
//                                       z_log(Cancelled_msg);
                                       return ZCAN;

                        case TIMEOUT:  /*-----------------------------------*/
                                       /* Timeout                           */
                                       /*-----------------------------------*/
//                                       z_message( TIME_msg );
                                       return c;

                        case RCDO:     /*-----------------------------------*/
                                       /* No carrier                        */
                                       /*-----------------------------------*/
//                                       z_log( CARRIER_msg );
                                       return c;

                        default:       /*-----------------------------------*/
                                       /* Something bizarre                 */
                                       /*-----------------------------------*/
//                                       z_message("Debris");
                                       return c;
                     } /* switch */
               } /* if */

            if (--length < 0)
               {
//                  z_message("Long pkt");
                  return ERROR;
               }

            ++RxCount;
            *buf++ = c;
            crc = Z_UpdateCRC(c, crc);
            continue;
         } /* while(1) */

   } /* RZ_ReceiveData */







/*--------------------------------------------------------------------------*/
/* RZ INIT RECEIVER                                                         */
/* Initialize for Zmodem receive attempt, try to activate Zmodem sender     */
/* Handles ZSINIT, ZFREECNT, and ZCOMMAND frames                            */
/*                                                                          */
/* Return codes:                                                            */
/*    ZFILE .... Zmodem filename received                                   */
/*    ZCOMPL ... transaction finished                                       */
/*    ERROR .... any other condition                                        */
/*--------------------------------------------------------------------------*/
static int RZ_InitReceiver(void)
   {
      register int   n;
      int            errors = 0;


      for (n=10; --n>=0; )
         {

            /*--------------------------------------------------------------*/
            /* Set buffer length (0=unlimited, don't wait).                 */
            /* Also set capability flags                                    */
            /*--------------------------------------------------------------*/
            Z_PutLongIntoHeader(0L);
            Txhdr[ZF0] = CANFDX|CANOVIO;
            Z_SendHexHeader(Tryzhdrtype, Txhdr);

AGAIN:

            switch (Z_GetHeader(Rxhdr))
               {
                  case ZFILE:    /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
                                 Zconv = Rxhdr[ZF0];
                                 Tryzhdrtype = ZRINIT;
                                 if (RZ_ReceiveData(Secbuf,KSIZE)==GOTCRCW)
                                    return ZFILE;
                                 Z_SendHexHeader(ZNAK, Txhdr);
                                 goto AGAIN;

                  case ZSINIT:   /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
                                 if (RZ_ReceiveData(Attn, ZATTNLEN) == GOTCRCW)
                                    Z_SendHexHeader(ZACK, Txhdr);
                                 else Z_SendHexHeader(ZNAK, Txhdr);
                                 goto AGAIN;

                  case ZFREECNT: /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
                                 Z_PutLongIntoHeader(DiskAvail);
                                 Z_SendHexHeader(ZACK, Txhdr);
                                 goto AGAIN;

                  case ZCOMMAND: /*-----------------------------------------*/
                                 /* Paranoia is good for you...             */
                                 /* Ignore command from remote, but lie and */
                                 /* say we did the command ok.              */
                                 /*-----------------------------------------*/
                                 if (RZ_ReceiveData(Secbuf,KSIZE)==GOTCRCW)
                                    {
//k                                       status_line("!Ignoring `%s'", Secbuf);
                                       Z_PutLongIntoHeader(0L); 
                                       do
                                          {
                                             Z_SendHexHeader(ZCOMPL, Txhdr);
                                          }
                                       while (++errors<10 && Z_GetHeader(Rxhdr) != ZFIN);
                                       RZ_AckBibi();
                                       return ZCOMPL;
                                    }
                                 else Z_SendHexHeader(ZNAK, Txhdr);
                                 goto AGAIN;

                  case ZCOMPL:   /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
                                 goto AGAIN;

                  case ZFIN:     /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
                                 RZ_AckBibi();
                                 return ZCOMPL;

                  case ZCAN:
                  case RCDO:     /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
                                 return ERROR;
               } /* switch */

         } /* for */

      return ERROR;

   } /* RZ_InitReceiver */



/*--------------------------------------------------------------------------*/
/* RZFILES                                                                  */
/* Receive a batch of files using ZMODEM protocol                           */
/*--------------------------------------------------------------------------*/
static int RZ_ReceiveBatch(FILE *xferinfo)
   {
      register int c;

//      set_xy("");
      while(1)
         {
            switch (c = RZ_ReceiveFile(xferinfo))
               {
                  case ZEOF:
                  case ZSKIP:
                              switch (RZ_InitReceiver())
                                 {
                                    case ZCOMPL:   return ZOK;
                                    default:       return ERROR;
                                    case ZFILE:    break;
                                 } /* switch */
                              break;

                  default:
                              fclose(Outfile);
                              Outfile  = NULL;
//k                              unlink(Filename);
//k                              errno    = 0;
                              return c;
               } /* switch */

         } /* while */

   } /* RZ_ReceiveBatch */




/*--------------------------------------------------------------------------*/
/* RZ RECEIVE FILE                                                          */
/* Receive one file; assumes file name frame is preloaded in Secbuf         */
/*--------------------------------------------------------------------------*/
static int RZ_ReceiveFile(FILE *xferinfo)
   {
      register int   c;
      int            n;
      unsigned long  rxbytes;

      EOFseen=FALSE;
      if (RZ_GetHeader() == ERROR)  return (Tryzhdrtype = ZSKIP);

      n        = 10;
      rxbytes  = 0L;

      while(1)
         {
            Z_PutLongIntoHeader(rxbytes);
            Z_SendHexHeader(ZRPOS, Txhdr);
NxtHdr:
            switch (c = Z_GetHeader(Rxhdr))
               {
                  case ZDATA:    /*-----------------------------------------*/
                                 /* Data Packet                             */
                                 /*-----------------------------------------*/
                                 if (Rxpos != rxbytes)
                                    {
                                       if ( --n < 0) return ERROR;
//                                       z_message(NULL);
//                                       cprintf("Bad pos; %ld/%ld",rxbytes,Rxpos);
                                       Z_PutString((unsigned char*)Attn);
                                       continue;
                                    }
MoreData:
                                 switch (c = RZ_ReceiveData(Secbuf,KSIZE))
                                    {
                                       case ZCAN:
                                       case RCDO:  /*-----------------------*/
                                                   /* CAN or CARRIER        */
                                                   /*-----------------------*/
//                                                   z_log(Cancelled_msg);
                                                   return ERROR;

                                       case ERROR: /*-----------------------*/
                                                   /* CRC error             */
                                                   /*-----------------------*/
                                                   if (--n<0)
                                                      {
//                                                         z_log( FUBAR_msg );
                                                         return ERROR;
                                                      }
//                                                   show_loc(rxbytes,n);
                                                   Z_PutString((unsigned char*)Attn);
                                                   continue;

                                       case TIMEOUT: /*---------------------*/
                                                   /*                       */
                                                   /*-----------------------*/
                                                   if (--n<0)
                                                      {
//                                                         z_log( TIME_msg );
                                                         return ERROR;
                                                      }
//                                                   show_loc(rxbytes,n);
                                                   continue;

                                       case GOTCRCW: /*---------------------*/
                                                   /* End of frame          */
                                                   /*-----------------------*/
                                                   n = 10;
                                                   if (RZ_SaveToDisk(&rxbytes)==ERROR)
                                                      return ERROR;
                                                   Z_PutLongIntoHeader(rxbytes);
                                                   Z_SendHexHeader(ZACK, Txhdr);
                                                   goto NxtHdr;

                                       case GOTCRCQ: /*---------------------*/
                                                   /* Zack expected         */
                                                   /*-----------------------*/
                                                   n = 10;
                                                   if (RZ_SaveToDisk(&rxbytes)==ERROR)
                                                      return ERROR;
                                                   Z_PutLongIntoHeader(rxbytes);
                                                   Z_SendHexHeader(ZACK, Txhdr);
                                                   goto MoreData;

                                       case GOTCRCG: /*---------------------*/
                                                   /* Non-stop              */
                                                   /*-----------------------*/
                                                   n = 10;
                                                   if (RZ_SaveToDisk(&rxbytes)==ERROR)
                                                      return ERROR;
                                                   goto MoreData;

                                       case GOTCRCE: /*---------------------*/
                                                   /* Header to follow      */
                                                   /*-----------------------*/
                                                   n = 10;
                                                   if (RZ_SaveToDisk(&rxbytes)==ERROR)
                                                      return ERROR;
                                                   goto NxtHdr;
                                                      } /* switch */

                  case ZNAK:
                  case TIMEOUT:  /*-----------------------------------------*/
                                 /* Packed was probably garbled             */
                                 /*-----------------------------------------*/
                                 if ( --n < 0)
                                    {
//                                       z_log( "Garbled packet" );
                                       return ERROR;
                                    }
//                                 show_loc(rxbytes,n);
                                 continue;

                  case ZFILE:    /*-----------------------------------------*/
                                 /* Sender didn't see our ZRPOS yet         */
                                 /*-----------------------------------------*/
                                 RZ_ReceiveData(Secbuf, KSIZE);
                                 continue;

                  case ZEOF:     /*-----------------------------------------*/
                                 /* End of the file                         */
                                 /*-----------------------------------------*/
                                 if (Rxpos != rxbytes)
                                    {
//                                       status_line("-Bad EOF; %ld/%ld", rxbytes, (unsigned char*)Rxpos);
                                       continue;
                                    }
//                                 throughput(2,rxbytes);
//                                 errno = 0;

                                 fclose(Outfile);
//k                                 had_error(CLOSE_msg,Filename);

//                                 status_line("=UL %s",Filename);

                                 Outfile  = NULL;
                                 if (xferinfo != NULL)
                                    {
                                       fprintf(xferinfo, "%s\n", Filename);
//k                                       had_error(WRITE_msg,"XferInfo");
                                    }
                                 return c;

                  case ERROR:    /*-----------------------------------------*/
                                 /* Too much garbage in header search error */
                                 /*-----------------------------------------*/
                                 if ( --n < 0) return ERROR;
//                                 show_loc(rxbytes,n);
                                 Z_PutString((unsigned char*)Attn);
                                 continue;

                  default:       /*-----------------------------------------*/
                                 /*                                         */
                                 /*-----------------------------------------*/
//                                 z_log( IDUNNO_msg );
                                 return ERROR;

                 } /* switch */
          } /* while */

   } /* RZ_ReceiveFile */


/*--------------------------------------------------------------------------*/
/* RZ GET HEADER                                                            */
/* Process incoming file information header                                 */
/*--------------------------------------------------------------------------*/
static int RZ_GetHeader(void)
   {

      static char    suffix[] = ".001";

      register char *p;
      register int   n;

      char          *ourname;
      char          *theirname;
      unsigned long  filesize;


      /*--------------------------------------------------------------------*/
      /* Setup the transfer mode                                            */
      /*--------------------------------------------------------------------*/
      isBinary  = (!RXBINARY && Zconv == ZCNL)? 0 : 1;


      /*--------------------------------------------------------------------*/
      /* Extract and verify filesize, if given.                             */
      /* Reject file if not at least 10K free                               */
      /*--------------------------------------------------------------------*/
      filesize = 0L;
      p = (char*)Secbuf + 1 + strlen((char*)Secbuf);
      if (*p) sscanf((char*)p, "%ld", &filesize);
      if (filesize+10240 > DiskAvail)
         {
//            status_line("!Disk space");
            return ERROR;
         }

      /*--------------------------------------------------------------------*/
      /* Get and/or fix filename for uploaded file                          */
      /*--------------------------------------------------------------------*/
      p  = Filename+strlen(Filename)-1;     /* Find end of Opus upload path */
      while (p>=Filename && *p!='\\') p--;
      ourname = ++p;

      p = (char*)Secbuf+strlen((char*)Secbuf)-1;      /* Find transmitted simple filename */
      while (p >= Secbuf && *p!='\\' && *p!='/' && *p!=':') p--;
      theirname = ++p;


      strcpy(ourname,theirname);          /* Start w/ our path & their name */

#ifdef TAKE_OUT_FOR_NOW
      FRESULT f_stat (const TCHAR* path, FILINFO* fno);					/* Get file status */

      if (dexists(Filename))
         {                            /* If file already exists...      */
            p = ourname;
            while (*p && *p!='.') p++;    /* ...find the extension, if any  */
            for (n=0; n<4; n++)           /* ...fill it out if neccessary   */
               if (!*p)
                  {
                     *p       = suffix[n];
                     *(++p)   = '\0';
                  }
               else p++;

            while (dexists(Filename))    /* ...If 'file.ext' exists suffix++ */
               {
                  p = ourname+strlen(ourname)-1;
                  for (n=3; n--;)
                     {
//k                        if (!isdigit(*p)) *p = '0';
                        if (++(*p) <= '9') break;
                        else *p-- = '0';
                     } /* for */
               } /* while */
         } /* if exist */

#endif

//      errno = 0;
//      if (strcmp(ourname,theirname))
//         status_line("+Dupe renamed: %s",ourname);
   
//      errno = 0;
      Outfile = fopen( Filename, "w+" );
//      if (had_error(OPEN_msg,Filename))
      return ERROR;

//   	set_xy(NULL);
//      cprintf("Z-Recv %s, %s%ldb, %d min.",
//                  Filename,
//                  (isBinary)? "": "ASCII ",
//                  filesize,
//                  (int)(filesize*10/cur_baud+27)/54);

//      set_xy(NULL);

//      throughput(0,0L);

      return ZOK;

   } /* RZ_GetHeader */





/*--------------------------------------------------------------------------*/
/* RZ SAVE TO DISK                                                          */
/* Writes the received file data to the output file.                        */
/* If in ASCII mode, stops writing at first ^Z, and converts all            */
/*   solo CR's or LF's to CR/LF pairs.                                      */
/*--------------------------------------------------------------------------*/
static int RZ_SaveToDisk(unsigned long *rxbytes)
   {
      static byte    lastsent;

      register char *p;
      register int   count;

      count = RxCount;

//      if (((KEYPRESS()) and (READKB()==27)))
//         {
//            send_can();
//            z_log( KBD_msg );
//            return ERROR;
//         }

//      if (count!=z_size)
//         {
//            gotoxy( locate_x+10, locate_y );
//            cputs( ultoa(((unsigned long )(z_size=count)),e_input,10) );
//            putch(' ');
//         }

//      errno = 0;

      if (isBinary)
         {
            fwrite( Secbuf, count, 1, Outfile );
//k            if (errno) goto oops;
         }
      else
         {
            if (EOFseen) return ZOK;
            for (p=Secbuf; --count>=0; ++p )
               {
                  if ( *p == CPMEOF)
                     {
                        EOFseen = TRUE;
                        return ZOK;
                     }
                  if ( *p=='\n' ) {
                     if (lastsent!='\r' && putc('\r', Outfile) == EOF)
                        goto oops;
                  } else {
                     if (lastsent=='\r' && putc('\n', Outfile) == EOF)
                        goto oops;
                  }
                  if (putc((lastsent=*p), Outfile) == EOF)  goto oops;
               }
         }

      *rxbytes += RxCount;
      
//      gotoxy( locate_x, locate_y );
//      cputs( ultoa(((unsigned long )(*rxbytes)),e_input,10) );
      return ZOK;

oops:
//      had_error(WRITE_msg,Filename);
      return ERROR;

   } /* RZ_SaveToDisk */



/*--------------------------------------------------------------------------*/
/* RZ ACK BIBI                                                              */
/* Ack a ZFIN packet, let byegones be byegones                              */
/*--------------------------------------------------------------------------*/
static void RZ_AckBibi()
   {
      register int n;

      Z_PutLongIntoHeader(0L);
      for (n=4; --n;)
         {
            Z_SendHexHeader(ZFIN, Txhdr);
            switch (Z_GetByte(100))
               {
                  case 'O':      Z_GetByte(1);    /* Discard 2nd 'O' */

                  case TIMEOUT:
                  case RCDO:     return;
               } /* switch */
         } /* for */

   } /* RZ_AckBibi */

