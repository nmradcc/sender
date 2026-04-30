/*--------------------------------------------------------------------------*/
/* FILE: zmisc.c    (Opus zmodem routines used by send and receive)         */
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

#include "Shell.h"

static int Rxtype;               /* Type of header received                 */
static int Rxframeind;           /* ZBIN ZBIN32,ZHEX type of frame received */

static char hex[] = "0123456789abcdef";

/* Send a byte as two hex digits */
#define Z_PUTHEX(i,c) {i=(c);SENDBYTE(hex[((i)&0xF0)>>4]);SENDBYTE(hex[(i)&0xF]);}

uint8_t zmodem_port = 0;
void SENDBYTE(char c)
{
	ShCharOut(zmodem_port, c);
}


char CHAR_AVAIL(void)
{
	return ShKbHit(zmodem_port);
}

char MODEM_IN(void)
{
	return ShGetChar(zmodem_port);
}

/*--------------------------------------------------------------------------*/
/* Routines used by other ZModem modules...                                 */
/*--------------------------------------------------------------------------*/
int  Z_GetByte(int);
void Z_PutString(char *);
void Z_SendHexHeader(unsigned short, char *);
int  Z_GetHeader(char *);
int  Z_GetZDL(void);
void Z_PutLongIntoHeader(long);
unsigned short Z_UpdateCRC(unsigned short,unsigned short);


/*--------------------------------------------------------------------------*/
/* Private routines                                                         */
/*--------------------------------------------------------------------------*/
static int  _Z_qk_read(void);
static int  _Z_GetBinaryHeader(char *);
static int  _Z_GetHexHeader(char *);
static int  _Z_GetHex(void);
static int  _Z_TimedRead(void);
static long _Z_PullLongFromHeader(char *);



extern byte *local_CEOL;
extern char *KBD_msg;

#ifdef NOT_USED
void z_message(byte *s)
   {
   	gotoxy( locate_x+20, locate_y );
      if (s) cputs(s);
      cputs( local_CEOL );
   }


void z_log(byte *s)
   {
      word x, y;

      z_message(s);

      x  = locate_x;
      y  = locate_y;
      status_line(s); /* also does disk file logging */
      locate_x = x;
      locate_y = y;

   }


void show_loc(unsigned long  l, unsigned int   w)
   {
      gotoxy( locate_x+35, locate_y );
      cprintf("Ofs=%ld Retries=%d%s",l,w,local_CEOL);
   }
#endif


void send_can(void)
{
	for(int i = 0; i < 10; i++)
	{
		SENDBYTE(CAN);
	}
}

byte * zalloc(void)
   {
      byte *sptr;

      sptr  = (byte*)malloc(KSIZE);
      if (!sptr)
         {
//         	status_line("!Z-MEMOVFL");
//            adios(2);
    	    return NULL;
         }
      return sptr;
  }







/*--------------------------------------------------------------------------*/
/* Z GET BYTE                                                               */
/* Get a byte from the modem;                                               */
/* return TIMEOUT if no read within timeout tenths,                         */
/* return RCDO if carrier lost                                              */
/*--------------------------------------------------------------------------*/
int Z_GetByte(int tenths)
{
	register int i;

	i^=i;
	do
	{
		if (CHAR_AVAIL())
		{
			return MODEM_IN();
		}
	}
    //while((i++)<2000);
    while((i++)<20000);

	return TIMEOUT;


    i   = tenths/10;
    if (i<1)   i =  1;
    if (i>58)  i = 58;

//k      i = set_timer(i);

    while(!CHAR_AVAIL())
    {
//k            if (!CARRIER)     return RCDO;
//k
//k            time_release();
//k            if (time_gone(i)) return TIMEOUT;
    }

    return (int)MODEM_IN();
}







/*--------------------------------------------------------------------------*/
/* QK_READ  (like Z_GetByte, but assumes the time to be Rxtimeout)          */
/* Get a byte from the modem;                                               */
/* return TIMEOUT if no read within timeout tenths,                         */
/* return RCDO if carrier lost                                              */
/*--------------------------------------------------------------------------*/
static int _Z_qk_read(void)
{

#ifdef USE_Z_GetByte
      register int i;
      register int time_val;

      time_val^=time_val;
//      do
//      	{
//            if ((i=MODEM_STATUS())&DATA_READY)  return MODEM_IN();
//            if (!(i&ctl.carrier_mask))          return RCDO;
//      	}
//      while((time_val++)<2000);

//k      time_val = set_timer(Rxtimeout);
//      do
//         {
//            if ((i=MODEM_STATUS())&DATA_READY)  return MODEM_IN();
      	  return MODEM_IN();


//            if (!(i&ctl.carrier_mask))          return RCDO;
//            time_release();
//         }
//      while(time_left(time_val));

      return TIMEOUT;
#else
      return Z_GetByte(Rxtimeout);
#endif
}







/*--------------------------------------------------------------------------*/
/* Z PUT STRING                                                             */
/* Send a string to the modem, processing for \336 (sleep 1 sec)            */
/* and \335 (break signal, ignored)                                         */
/*--------------------------------------------------------------------------*/
void Z_PutString(char *s)
   {
      register int c;

      while (*s)
         {
            switch (c = *s++)
               {
//k                  case '\336':   big_pause(2);
                  case '\335':   break;
                  default:       SENDBYTE(c);
               } /* switch */

         } /* while */

   } /* Z_PutString */









/*--------------------------------------------------------------------------*/
/* Z UPDATE CRC                                                             */
/* update CRC                                                               */
/*--------------------------------------------------------------------------*/
unsigned short Z_UpdateCRC(unsigned short c, unsigned short crc)
   {
      register int    count;
      register word   temp;

      temp   = crc;

      for (count=8; --count>=0; )
         {
            if (temp & 0x8000)
               {
                  temp <<= 1;
                  temp += (((c<<=1) & 0400)  !=  0);
                  temp ^= 0x1021;
               }
            else
               {
                  temp <<= 1;
                  temp += (((c<<=1) & 0400)  !=  0);
               }
         } /* for */

      return temp;

   } /* z_crc update */




/*--------------------------------------------------------------------------*/
/* Z SEND HEX HEADER                                                        */
/* Send ZMODEM HEX header hdr of type type                                  */
/*--------------------------------------------------------------------------*/
void Z_SendHexHeader(unsigned short type, char *hdr)
   {
      register int   n;
      register int   i;
      unsigned short crc;

      SENDBYTE(ZPAD);
      SENDBYTE(ZPAD);
      SENDBYTE(ZDLE);
      SENDBYTE(ZHEX);
      Z_PUTHEX(i,type);

      crc = Z_UpdateCRC(type, 0);
      for (n=4; --n >= 0;)
         {
            Z_PUTHEX(i,(*hdr));
            crc = Z_UpdateCRC(((unsigned short)(*hdr++)), crc);
         }
      crc = Z_UpdateCRC(0,crc);
      crc = Z_UpdateCRC(0,crc);
      Z_PUTHEX(i,(crc>>8));
      Z_PUTHEX(i,crc);

      /* Make it printable on remote machine */
      SENDBYTE('\r');
      SENDBYTE('\n');

      /* Uncork the remote in case a fake XOFF has stopped data flow */
      if (type != ZFIN) SENDBYTE(021);

//      wait_for_clear();

   } /* Z_SendHexHeader */





/*--------------------------------------------------------------------------*/
/* Z GET HEADER                                                             */
/* Read a ZMODEM header to hdr, either binary or hex.                       */
/*   On success, set Zmodem to 1 and return type of header.                 */
/*   Otherwise return negative on error                                     */
/*--------------------------------------------------------------------------*/
int Z_GetHeader(char *hdr)
   {

      register int   c;
      register int   n;
      int            cancount;


//      n = cur_baud;   /* Max characters before start of frame */
      n = 115200;
      cancount = 5;

Again:

//      if (((KEYPRESS()) and (READKB()==27)))
//         {
//            send_can();
//            z_log( KBD_msg );
//            return ZCAN;
//         }

      Rxframeind = Rxtype = 0;

      switch (c = _Z_TimedRead())
         {

            case ZPAD:     /*-----------------------------------------------*/
                           /* This is what we want.                         */
                           /*-----------------------------------------------*/
                           break;

            case RCDO:
            case TIMEOUT:  /*-----------------------------------------------*/
                           /*                                               */
                           /*-----------------------------------------------*/
                           goto Done;

            case CAN:      /*-----------------------------------------------*/
                           /*                                               */
                           /*-----------------------------------------------*/
                           if (--cancount <= 0)
                              {
                                 c = ZCAN;
                                 goto Done;
                              }

                           /* fallthrough... */

            default:       /*-----------------------------------------------*/
                           /*                                               */
                           /*-----------------------------------------------*/
Agn2:

                           if (--n <= 0)
                              {
//                                 z_log( FUBAR_msg );
//                                 send_can();
                                 return ZCAN;
                              }

                           if (c != CAN) cancount = 5;
                           goto Again;

         } /* switch */

      cancount = 5;

Splat:

      switch (c = _Z_TimedRead())
         {
            case ZDLE:     /*-----------------------------------------------*/
                           /* This is what we want.                         */
                           /*-----------------------------------------------*/
                           break;

            case ZPAD:     /*-----------------------------------------------*/
                           /*                                               */
                           /*-----------------------------------------------*/
                           goto Splat;

            case RCDO:
            case TIMEOUT:  /*-----------------------------------------------*/
                           /*                                               */
                           /*-----------------------------------------------*/
                           goto Done;

            default:       /*-----------------------------------------------*/
                           /*                                               */
                           /*-----------------------------------------------*/
                           goto Agn2;

         } /* switch */


      switch (c = _Z_TimedRead())
         {

            case ZBIN:     /*-----------------------------------------------*/
                           /*                                               */
                           /*-----------------------------------------------*/
                           Rxframeind = ZBIN;
                           c =  _Z_GetBinaryHeader(hdr);
                           break;

            case ZHEX:     /*-----------------------------------------------*/
                           /*                                               */
                           /*-----------------------------------------------*/
                           Rxframeind = ZHEX;
                           c =  _Z_GetHexHeader(hdr);
                           break;

            case CAN:      /*-----------------------------------------------*/
                           /*                                               */
                           /*-----------------------------------------------*/
                           if (--cancount <= 0)
                              {
                                 c = ZCAN;
                                 goto Done;
                              }
                           goto Agn2;

            case RCDO:
            case TIMEOUT:  /*-----------------------------------------------*/
                           /*                                               */
                           /*-----------------------------------------------*/
                           goto Done;

            default:       /*-----------------------------------------------*/
                           /*                                               */
                           /*-----------------------------------------------*/
                           goto Agn2;

         } /* switch */

      Rxpos = _Z_PullLongFromHeader(hdr);

Done:

      return c;

   } /* Z_GetHeader */





/*--------------------------------------------------------------------------*/
/* Z GET BINARY HEADER                                                      */
/* Receive a binary style header (type and position)                        */
/*--------------------------------------------------------------------------*/
static int _Z_GetBinaryHeader(char *hdr)
   {
      register int    c;
      register word   crc;
      int             n;

      if ((c   = Z_GetZDL()) & ~0xFF)   return c;
      Rxtype   = c;
      crc      = Z_UpdateCRC(c, 0);

      for (n=4; --n >= 0;)
         {
            if ((c = Z_GetZDL()) & ~0xFF) return c;
            crc = Z_UpdateCRC(c, crc);
            *hdr++ = c;
         }
      if ((c   = Z_GetZDL()) & ~0xFF) return c;

      crc      = Z_UpdateCRC(c, crc);
      if ((c   = Z_GetZDL()) & ~0xFF) return c;

      crc = Z_UpdateCRC(c, crc);
      if (crc & 0xFFFF)
         {
//            z_message( CRC_msg );
            return ERROR;
         }

      return Rxtype;

   } /* _Z_GetBinaryHeader */




/*--------------------------------------------------------------------------*/
/* Z GET HEX HEADER                                                         */
/* Receive a hex style header (type and position)                           */
/*--------------------------------------------------------------------------*/
static int _Z_GetHexHeader(char *hdr)
   {
      register int   c;
      register word  crc;
      int            n;

      if ((c   = _Z_GetHex()) < 0) return c;
      Rxtype   = c;
      crc      = Z_UpdateCRC(c, 0);

      for (n=4; --n >= 0;)
         {
            if ((c = _Z_GetHex()) < 0) return c;
            crc      = Z_UpdateCRC(c, crc);
            *hdr++   = c;
         }

      if ((c = _Z_GetHex()) < 0) return c;
      crc = Z_UpdateCRC(c, crc);
      if ((c = _Z_GetHex()) < 0) return c;
      crc = Z_UpdateCRC(c, crc);
      if (crc & 0xFFFF)
         {
//            z_message( CRC_msg );
            return ERROR;
         }
      if (Z_GetByte(1) == '\r') Z_GetByte(1);  /* Throw away possible cr/lf */

      return Rxtype;

   }





/*--------------------------------------------------------------------------*/
/* Z GET HEX                                                                */
/* Decode two lower case hex digits into an 8 bit byte value                */
/*--------------------------------------------------------------------------*/
static int _Z_GetHex(void)
   {
      register int c, n;

      if ((n = _Z_TimedRead()) < 0) return n;
      n -= '0';
      if (n > 9) n -= ('a' - ':');
      if (n & ~0xF) return ERROR;

      if ((c = _Z_TimedRead()) < 0) return c;
      c -= '0';
      if (c > 9) c -= ('a' - ':');
      if (c & ~0xF) return ERROR;

      return (n<<4 | c);
   }




/*--------------------------------------------------------------------------*/
/* Z GET ZDL                                                                */
/* Read a byte, checking for ZMODEM escape encoding                         */
/* including CAN*5 which represents a quick abort                           */
/*--------------------------------------------------------------------------*/
int Z_GetZDL(void)
   {
      register int c;

      if ((c = _Z_qk_read()) != ZDLE)         return c;

      switch (c=_Z_qk_read())
         {
            case CAN:   return ((c=_Z_qk_read())<0)?               c :
                               ((c==CAN) && ((c=_Z_qk_read())<0))? c :
                               ((c==CAN) && ((c=_Z_qk_read())<0))? c : (GOTCAN);

            case ZCRCE:
            case ZCRCG:
            case ZCRCQ:
            case ZCRCW: return (c | GOTOR);

            case ZRUB0: return 0x7F;

            case ZRUB1: return 0xFF;

            default:    return   (c<0)?            c :
                                 ((c&0x60)==0x40)? (c ^ 0x40)  : ERROR;

         } /* switch */

   } /* Z_GetZDL */





/*--------------------------------------------------------------------------*/
/* Z TIMED READ                                                             */
/* Read a character from the modem line with timeout.                       */
/*  Eat parity, XON and XOFF characters.                                    */
/*--------------------------------------------------------------------------*/
static int _Z_TimedRead(void)
   {
      register int c;

      for (;;)
         {
            if ((c = _Z_qk_read()) < 0) return c;

            switch (c &= 0x7F)
               {
                  case XON:
                  case XOFF:  continue;

                  default:    return c;
               } /* switch */

         } /* for */

   } /* _Z_TimedRead */




/*--------------------------------------------------------------------------*/
/* Z LONG TO HEADER                                                         */
/* Store long integer pos in Txhdr                                          */
/*--------------------------------------------------------------------------*/
void Z_PutLongIntoHeader(long pos)
   {
      Txhdr[ZP0] = pos;
      Txhdr[ZP1] = pos>>8;
      Txhdr[ZP2] = pos>>16;
      Txhdr[ZP3] = pos>>24;
   } /* Z_PutLongIntoHeader */




/*--------------------------------------------------------------------------*/
/* Z PULL LONG FROM HEADER                                                  */
/* Recover a long integer from a header                                     */
/*--------------------------------------------------------------------------*/
static long _Z_PullLongFromHeader(char *hdr)
   {
      long l;

      l = hdr[ZP3];
      l = (l << 8) | hdr[ZP2];
      l = (l << 8) | hdr[ZP1];
      l = (l << 8) | hdr[ZP0];
      return l;
   } /* _Z_PullLongFromHeader */


/* END OF FILE: n_zmodem.c */

