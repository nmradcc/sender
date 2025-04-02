/*****************************************************************************
 *
 * File:                 $Workfile: DEC_TST.CPP $
 * Path:                 $Logfile: /NMRA/SRC/SEND/DEC_TST.CPP $
 * Project:              NMRA DCC Conformance Tests
 *
 * Creation date:        #
 * Designed by:          K. West
 * Coded by:             K. West
 *
 * Current version:      $Revision: 61 $
 * Last Modified on:     $Modtime: 9/12/23 8:00p $
 *               by:     $Author: Kenw $
 * Last checkin on:      $Date: 9/12/23 8:09p $
 *
 *****************************************************************************
 *
 * DESCRIPTION:
 *
 *	dec_tst.cpp	-	Methods for Dec_tst decoder test class.
 *
 *****************************************************************************/

#include "dec_tst.h"
#include "args.h"
#include "send_reg.h"

#if SEND_VERSION < 4
 #include <conio.h>
#endif
#include <string.h>

#if SEND_VERSION >= 4
extern "C"
{
	//extern void OUT_PC(uint8_t mask, uint8_t value);
	extern unsigned char kbhit(void);
	extern unsigned char getch(void);
};
#endif

static const char sccsid[]      = "@(#) $Workfile: DEC_TST.CPP $$ $Revision: 61 $$";
static const char sccsid_h[]    = DEC_TST_H_DECLARED;

// Func to eliminate bogus sccsid and sccsid_h declared but never used warning.
inline void dummy(const char *, const char *) {}

/*
 * Uncomment PKT_REP_DEBUG to show all 3 fill packet counts when
 * LOG_PKTS is uncommented to log packets rather than using hardware.
 */
//#define PKT_REP_DEBUG

const int		SP_TEST_MIN		= 14;				// Min. test speed.
													// (Step 7 + light 14 step
                                                    //  mode)
// Note: PKT_REP_MIN moved to SEND_REG.H to share with rep calulator methods.
const int		BEST_PRE		= PRE_BITS;			// Best preamble bit cnt.
const int		BEST_IDLE		= 1;				// Best idle packet cnt.
const int		BAD_BIT_BITS	= BASE_BITS+1; 		// Packet plus stop bit.
const int		PRE_SKIP_BITS	= 2;				// Indetermintate preamble
													// bits from S9.1.
const u_int		FSOC_DELAY		= 2;				// Seconds to elay after
													// initial fsocc sequences.
/*
 *	AMBIG1_BITS is the number of 0s to add before before a packet
 *	so that the ambigous bits start at the beginning of the next BYTE.
 *	This includes the baseline packet plus 1 packet end bit.
 *
 */
const int		AMBIG1_BITS	= (BASE_BITS + 1) % BITS_IN_BYTE;

/*
 *	AMBIG 2 bit times for first and second ambiguous bit.
 */
const int		AMBIG1_0H1		=	END_0H_MIN + 1;
const int		AMBIG1_0T1		=	AMBIG1_0H1 + 50;
const int		AMBIG1_0H2		=	DECODER_0H_NOM;
const int		AMBIG1_0T2		=	AMBIG1_0H2 + 50;


/*
 *	Range of 1T clock values tested.
 */
const u_int		TL1_MIN			= 50;
const u_int		TU1_MIN			= DECODER_1T_MAX - 5;
const u_int		TL1_MAX			= DECODER_1T_MIN + 10;
const u_int		TU1_MAX			= DECODER_0T_MIN - 10;

/*
 *	Range of 1H values tested.
 */
const u_int		DUTY_T1H_MIN	= DECODER_1H_NOM - 30;
const u_int		DUTY_T1H_MAX	= DECODER_1H_NOM + 30;

/*
 *	Primary truncated command address.
 */
const BYTE		PRI_TRUNC_ADDR	= 0x3f;

/*
 *	Possible ramp test states.
 */
enum Ramp_state
{
	RAMP_INIT,									// Initialize the test.
	RAMP_UP,									// Ramp up.
	RAMP_DOWN_FW,								// Ramp forward down.
	RAMP_DOWN_REV,								// Ramp reverse down.
	RAMP_E_STOP,								// Emergency stop.
	RAMP_END									// End of test.
};


/*
 *	Array of clock value to use for decoder tests.
 */
const Dec_clk	Dec_tst::dclk_tbl[] = {
	/*         CLK0T	           CLK0H  CLK1T   	Message */
	{	         200,                100,   116,  	"All nominal"		   },
	{	         196,                 98,   113,  	"All 1/4 fast"		   },
	{	         190,                 95,   110,  	"Command station min"  },
	{	         184,                 92,   106,  	"Minimum + 2"		   },
	{	         182,                 91,   105,  	"Minimum + 1"		   },
	{	         180,                 90,   104,  	"Decoder minimum"  	   },
	{	         204,                102,   119,	"All 1/4 slow"		   },
	{	         210,	             105,   122,	"Command station max"  },
	{	         216,	             108,   126,	"Maximum - 2"		   },
	{	         218,	             109,   127,	"Maximum - 1"		   },
	{	         220,                110,   128,	"Decoder maximum"	   },
	{	         300,                100,   116,	"Negative stretched 0" },
	{	         300,                200,   116,	"Positive stretched 0" },
	{           2560,                100,   116,  	"Very negative 0"	   },
	{           2560,               2460,   116,  	"Very positive 0"      },
	{ DECODER_0T_MAX, DECODER_0T_MAX-DECODER_0H_MAX, 116, "Max Decoder Neg 0"},
	{ DECODER_0T_MAX,     DECODER_0H_MAX,   116,  	"Max Decoder Pos 0"	   }
};

const u_int	Dec_tst::dclk_size		=	sizeof(Dec_tst::dclk_tbl) /
										sizeof(Dec_clk);
/*
 *	Array of clock value to use for decoder tests.
 */
const Str0_clk	Dec_tst::str0_tbl[] = {
	/*	CLK0T_delta			Stretch_type		Message */
	{				 0,		S_NONE,				"No stretch"				},
	{			 11800,		S_NEG,				"Long negative stretch"		},
	{			 11800,		S_POS,				"Long positive stretch"		},
	{	DECODER_0T_MAX,		S_NEG,				"Maximum negative stretch"	},
	{	DECODER_0T_MAX,		S_POS,				"Maximum positive stretch"	}
};

const u_int	Dec_tst::str0_size		=	sizeof(Dec_tst::str0_tbl) /
										sizeof(Str0_clk);

/*
 *	Names and filler types of various Ames tests.
 */
const Ames_type Dec_tst::ames_tbl[] =
{
//		pre_cnt		idle_cnt
	{	12,			1	},
	{	12,			2	},
	{	13,			1	},
	{	15,			1	},
	{	15,			2	}
};
const u_int Dec_tst::ames_tbl_size	=	sizeof(Dec_tst::ames_tbl) /
										sizeof(Ames_type);
/*
 *	Ambiguous 1 bit 0T and 0H times for single ambiguous bit test.
 */
const Ambig1_type Dec_tst::ambig1_tbl[] =
{
//		            0T		        0H
	{	    AMBIG1_0T1,	    AMBIG1_0H1	},
	{	    AMBIG1_0T2,	    AMBIG1_0H2	},
    {	           448,		    	 8	},
    {	           450,		   		10  },
    {	           455,		   		15  },
    {	      	   462,		   		22  },
    {	DECODER_0T_NOM,	DECODER_0H_NOM	}
};
const u_int Dec_tst::ambig1_tbl_size	=	sizeof(Dec_tst::ambig1_tbl) /
										sizeof(Ambig1_type);
/*
 *	Array of clock value to use for non-ambiguous bit times.
 */
const Dec_clk	Dec_tst::ambig2_dclk_tbl[] = {
	/*         CLK0T	           CLK0H  CLK1T   	Message */
	{	         200,                100,   116,  	"All nominal"		   },
	{	         180,                 90,   104,  	"Decoder minimum"  	   },
	{	         220,                110,   128,	"Decoder maximum"	   }
};

const u_int	Dec_tst::ambig2_dclk_size	=	sizeof(Dec_tst::ambig2_dclk_tbl) /
											sizeof(Dec_clk);

/*
 *	Ambiguous 2 bit 0T and 0H times for double ambiguous bit test.
 */
const Ambig2_type Dec_tst::ambig2_tbl[] =
{
//		           0T1		       0H1             0T2             0H2
#ifdef BLOMP
	{	    AMBIG1_0T1,	    AMBIG1_0H1,	    AMBIG1_0T2,	    AMBIG1_0H2	},
    {	           425,		        25,	            66,              8	},
    {	           425,		        25,	            73,             15	},
    {	           425,		        25,	            80,             22	},
    {	           425,		        25,	            96,             38	},
    {	           448,		       440,	           116,             58	},
    {	           448,		       435,	           116,             58	},
    {	           448,		       425,	           116,             58	},
	{			   448,            422,            116,             58  },
    {	           448,		       415,	           116,             58	},
	{			   448,			   414,			   116,				58	},
	{			   456,			   414,			   116,				58	},
	{			   464,			   414,			   116,				58	},
	{			   472,			   414,			   116,				58	},
	{			   480,			   414,			   116,				58	},
	{			   488,			   414,			   116,				58	},
	{			   496,			   414,			   116,				58	},
	{			   504,			   414,			   116,				58	},
    {	           448,		        25,	            94,             38	},
    {	           448,		        25,	            90,             34	},
    {	           448,		        25,	            86,             30	},
    {	           448,		        25,	            82,             26	},
	{			   472,			    34,			   118,			    59	},
	{			   472, 			22, 		   118,			    59	},
	{			   460,			    22, 		   115,			    57	},
	{			   460,			    22, 		   115,			    58	},
	{			   460,			    10,			   115,			    57	},
	{			   460,			    10,			   115,			    58	},
    {	DECODER_0T_NOM,	DECODER_0H_NOM,	DECODER_0T_NOM,	DECODER_0H_NOM	}
#endif
	{			   448,			   444,			   112,				56	},
	{			   448,			   443,			   112,				56	},
	{			   448,			   442,			   112,				56	},
	{			   448,			   441,			   112,				56	},
	{			   448,			   440,			   112,				56	},
	{			   448,			   439,			   112,				56	},
	{			   448,			   438,			   112,				56	},
	{			   448,			   437,			   112,				56	},

	{			   456,			   452,			   114,				57	},
	{			   456,			   451,			   114,				57	},
	{			   456,			   450,			   114,				57	},
	{			   456,			   449,			   114,				57	},
	{			   456,			   448,			   114,				57	},
	{			   456,			   447,			   114,				57	},
	{			   456,			   446,			   114,				57	},
	{			   456,			   445,			   114,				57	},
	{			   456,			   444,			   114,				57	},
	{			   456,			   443,			   114,				57	},
	{			   456,			   442,			   114,				57	},
	{			   456,			   441,			   114,				57	},
	{			   456,			   440,			   114,				57	},
	{			   456,			   439,			   114,				57	},
	{			   456,			   438,			   114,				57	},
	{			   456,			   437,			   114,				57	},

	{			   464,			   459,			   116,				58	},
	{			   464,			   458,			   116,				58	},
	{			   464,			   457,			   116,				58	},
	{			   464,			   456,			   116,				58	},
	{			   464,			   455,			   116,				58	},
	{			   464,			   454,			   116,				58	},
	{			   464,			   453,			   116,				58	},
	{			   464,			   452,			   116,				58	},
	{			   464,			   451,			   116,				58	},
	{			   464,			   450,			   116,				58	},
	{			   464,			   449,			   116,				58	},
	{			   464,			   448,			   116,				58	},
	{			   464,			   447,			   116,				58	},
	{			   464,			   446,			   116,				58	},
	{			   464,			   445,			   116,				58	},
	{			   464,			   444,			   116,				58	},
	{			   464,			   443,			   116,				58	},
	{			   464,			   442,			   116,				58	},
	{			   464,			   441,			   116,				58	},
	{			   464,			   440,			   116,				58	},
	{			   464,			   439,			   116,				58	},
	{			   464,			   438,			   116,				58	},
	{			   464,			   437,			   116,				58	},

	{			   472,			   467,			   118,				59	},
	{			   472,			   466,			   118,				59	},
	{			   472,			   465,			   118,				59	},
	{			   472,			   464,			   118,				59	},
	{			   472,			   463,			   118,				59	},
	{			   472,			   462,			   118,				59	},
	{			   472,			   461,			   118,				59	},
	{			   472,			   460,			   118,				59	},
	{			   472,			   459,			   118,				59	},
	{			   472,			   458,			   118,				59	},
	{			   472,			   457,			   118,				59	},
	{			   472,			   456,			   118,				59	},
	{			   472,			   455,			   118,				59	},
	{			   472,			   454,			   118,				59	},
	{			   472,			   453,			   118,				59	},
	{			   472,			   452,			   118,				59	},
	{			   472,			   451,			   118,				59	},
	{			   472,			   450,			   118,				59	},
	{			   472,			   449,			   118,				59	},
	{			   472,			   448,			   118,				59	},
	{			   472,			   447,			   118,				59	},
	{			   472,			   446,			   118,				59	},
	{			   472,			   445,			   118,				59	},
	{			   472,			   444,			   118,				59	},
	{			   472,			   443,			   118,				59	},
	{			   472,			   442,			   118,				59	},
	{			   472,			   441,			   118,				59	},
	{			   472,			   440,			   118,				59	},
	{			   472,			   439,			   118,				59	},
	{			   472,			   438,			   118,				59	},
	{			   472,			   437,			   118,				59	},

	{			   480,			   475,			   120,				60	},
	{			   480,			   474,			   120,				60	},
	{			   480,			   473,			   120,				60	},
	{			   480,			   472,			   120,				60	},
	{			   480,			   471,			   120,				60	},
	{			   480,			   470,			   120,				60	},
	{			   480,			   469,			   120,				60	},
	{			   480,			   468,			   120,				60	},
	{			   480,			   467,			   120,				60	},
	{			   480,			   466,			   120,				60	},
	{			   480,			   465,			   120,				60	},
	{			   480,			   464,			   120,				60	},
	{			   480,			   463,			   120,				60	},
	{			   480,			   462,			   120,				60	},
	{			   480,			   461,			   120,				60	},
	{			   480,			   460,			   120,				60	},
	{			   480,			   459,			   120,				60	},
	{			   480,			   458,			   120,				60	},
	{			   480,			   457,			   120,				60	},
	{			   480,			   456,			   120,				60	},
	{			   480,			   455,			   120,				60	},
	{			   480,			   454,			   120,				60	},
	{			   480,			   453,			   120,				60	},
	{			   480,			   452,			   120,				60	},
	{			   480,			   451,			   120,				60	},
	{			   480,			   450,			   120,				60	},
	{			   480,			   449,			   120,				60	},
	{			   480,			   448,			   120,				60	},
	{			   480,			   447,			   120,				60	},
	{			   480,			   446,			   120,				60	},
	{			   480,			   445,			   120,				60	},
	{			   480,			   444,			   120,				60	},
	{			   480,			   443,			   120,				60	},
	{			   480,			   442,			   120,				60	},
	{			   480,			   441,			   120,				60	},
	{			   480,			   440,			   120,				60	},
	{			   480,			   439,			   120,				60	},
	{			   480,			   438,			   120,				60	},
	{			   480,			   437,			   120,				60	},

	{			   488,			   483,			   122,				61	},
	{			   488,			   482,			   122,				61	},
	{			   488,			   481,			   122,				61	},
	{			   488,			   480,			   122,				61	},
	{			   488,			   479,			   122,				61	},
	{			   488,			   478,			   122,				61	},
	{			   488,			   477,			   122,				61	},
	{			   488,			   476,			   122,				61	},
	{			   488,			   475,			   122,				61	},
	{			   488,			   474,			   122,				61	},
	{			   488,			   473,			   122,				61	},
	{			   488,			   472,			   122,				61	},
	{			   488,			   471,			   122,				61	},
	{			   488,			   470,			   122,				61	},
	{			   488,			   469,			   122,				61	},
	{			   488,			   468,			   122,				61	},
	{			   488,			   467,			   122,				61	},
	{			   488,			   466,			   122,				61	},
	{			   488,			   465,			   122,				61	},
	{			   488,			   464,			   122,				61	},
	{			   488,			   463,			   122,				61	},
	{			   488,			   462,			   122,				61	},
	{			   488,			   461,			   122,				61	},
	{			   488,			   460,			   122,				61	},
	{			   488,			   459,			   122,				61	},
	{			   488,			   458,			   122,				61	},
	{			   488,			   457,			   122,				61	},
	{			   488,			   456,			   122,				61	},
	{			   488,			   455,			   122,				61	},
	{			   488,			   454,			   122,				61	},
	{			   488,			   453,			   122,				61	},
	{			   488,			   452,			   122,				61	},
	{			   488,			   451,			   122,				61	},
	{			   488,			   450,			   122,				61	},
	{			   488,			   449,			   122,				61	},
	{			   488,			   448,			   122,				61	},
	{			   488,			   447,			   122,				61	},
	{			   488,			   446,			   122,				61	},
	{			   488,			   445,			   122,				61	},
	{			   488,			   444,			   122,				61	},
	{			   488,			   443,			   122,				61	},
	{			   488,			   442,			   122,				61	},
	{			   488,			   441,			   122,				61	},
	{			   488,			   440,			   122,				61	},
	{			   488,			   439,			   122,				61	},
	{			   488,			   438,			   122,				61	},
	{			   488,			   437,			   122,				61	},
};
const u_int Dec_tst::ambig2_tbl_size	=	sizeof(Dec_tst::ambig2_tbl) /
											sizeof(Ambig2_type);

static const char *err_msg[] =
{
	"No",							// (000) No errors.
	"Preset",						// (001) Preset error.
	"Trigger",						// (010) Trigger error.
	"Pre & Trig",					// (011) Preset & Trigger error.
	"Reset",						// (100) Reset error.
	"Rst & Pre",					// (101) Reset & Preset error.
	"Rst & Trig",					// (110) Reset & Trigger error.
	"All"							// (111) All have errors.
};


// Define short macros for accessory preset and trigger get methods.
#define ACC_PRE  (Args.get_acc_pre())
#define ACC_TRG  (Args.get_acc_trg())

/* The FSOC_TST directive evals as true if the fscoc sequence
 * should be skipped.
 */
#define FSOC_TST ((decoder_type) == DEC_FUNC && (trig_rev))


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		debug_gen()						-	 Print GEN input states.
 *
 *	RETURN VALUE
 *
 *		None.
 *
 *	DESCRIPTION
 *
 *		debug_gen() prints the 'msg' variable followed by the GEN input.states.
 *		Note: This is a temporary DEBUG method that can be removed at some
 *      point.
 */
/*--------------------------------------------------------------------------*/

void
Dec_tst::debug_gen( const char *msg ) // DEBUG
{
	TO_LOG ( "!>DEBUG %s: Gen %s\n", msg,
    		 (Dcc_reg.get_gen() & 0x1) ? "HiGH" : " LOW" );
	Dcc_reg.clr_under();
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		fsoc_send()					-	 Conditionally send the fsoc sequence..
 *
 *	RETURN VALUE
 *
 *		None.
 *
 *	DESCRIPTION
 *
 *		fsoc_send() sends the fsoc sequence for all cases except the
 *		case when FSOC_TST is true. The 'msg' variable
 *		is added as part of the message generated by this call.
 */
/*--------------------------------------------------------------------------*/

void
Dec_tst::fsoc_send( const char *msg )
{
	if ( FSOC_TST == true )
	{
		DBGPRINT( "Skipping fsoc for %s.", msg );
		Dcc_reg.clr_under();
	}
	else
	{
		DBGPRINT( "Sending fsoc for %s.", msg );
		Dcc_reg.clr_under();
		Dcc_reg.send_pkt( fsoc, "Send fail safe start up sequence." );
	}
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		Dec_tst()						-	 Dec_tst constructor.
 *
 *	RETURN VALUE
 *
 *		None.
 *
 *	DESCRIPTION
 *
 *		Dec_tst() constructs a Dec_tst object.
 */
/*--------------------------------------------------------------------------*/

Dec_tst::Dec_tst( void )
:	dcc_bits( 256 ), dcc_bits2( 256 ), dcc_bits3( 256 )
{
	t_stat.reset();						// Reset test statistics.
	tst_cnt			= 0L;				// Reset test count.
	run_mask		= ~0L;				// Run all tests.
	clk_mask		= ~0L;				// Use all clock values.
	trig_cmd_speed	= SP_E_STOP;		// Use emeergency stop as trigger.
	fill_usec		= USEC_PER_SEC;		// Default to 1 second of filler.
	filler_idles	= PKT_REP_MIN;		// Default to minimum.
	pkt_rep_cnt		= PKT_REP_MIN;		// Default to minimum.
    m_tst_name[0]	= '\0';				// Clear test name buffer.
	trig_rev        = false;			// Handle reverse differently.
	decoder_type    = DEC_LOCO;			// Decoder type.

    // Eliminate bogus sccsid and sccsid_h declared but never used warning.
    dummy( sccsid, sccsid_h );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		~Dec_tst()						-	 Dec_tst destructor.
 *
 *	RETURN VALUE
 *
 *		None.
 *
 *	DESCRIPTION
 *
 *		~Dec_tst() destroys a Dec_tst object.
 */
/*--------------------------------------------------------------------------*/

Dec_tst::~Dec_tst()
{
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		print_user_docs()			   	-	 Print user docs.
 *
 *	RETURN VALUE
 *
 *		msg		-	Min phase message.
 *
 *	DESCRIPTION
 *
 *		print_user_docs() prints out the list of tests that will be run
 *		along with their associated '-t'  bit and then prints out the
 *		associated clock values along with their associated '-c' bit.
 */
/*--------------------------------------------------------------------------*/

void
Dec_tst::print_user_docs(
	FILE		*ofp )						// Output file stream.
{
	int			i;							// Index variable.
	u_long		bit_msk = 1UL;				// Bit mask.
    u_int		n       = 0;                // Test or clock number.

	fputs(
		"List of tests to run and their corresponding '-t'  parameter:\n\n"
		"PARAMETER   VARIANT                          TEST\n"
	"----------------------------------------------------------------------"
	"------\n",
		ofp );
	m_tst_name[0]	=	'\0';
	fprintf( ofp,
			"0x%08lx  %-28s %2u: 1T margin test.\n",
            bit_msk, m_tst_name, ++n );
	bit_msk <<= 1;
	fprintf( ofp,
			"0x%08lx  %-28s %2u: 1H duty cycle test.\n",
            bit_msk, m_tst_name, ++n );
	bit_msk <<= 1;
	fprintf( ofp,
			"0x%08lx  %-28s %2u: Ramp test.\n",
            bit_msk, m_tst_name, ++n );
	bit_msk <<= 1;

	// Packet acceptance test.
	for ( i = 0; i < ames_tbl_size; i++, bit_msk <<= 1 )
	{
		sprintf( m_tst_name, "Pre %d Idle %d",
			ames_tbl[i].pre_cnt, ames_tbl[i].idle_cnt );
		fprintf( ofp,
			"0x%08lx  %-28s %2u: Packet acceptance test.\n",
			bit_msk, m_tst_name, ++n );
	}

	m_tst_name[0]	=	'\0';
	fprintf( ofp,
			"0x%08lx  %-28s %2u: Bad address test.\n",
            bit_msk, m_tst_name, ++n );
	bit_msk <<= 1;
	fprintf( ofp,
			"0x%08lx  %-28s %2u: Bad bit test.\n", bit_msk, m_tst_name, ++n );
	bit_msk <<= 1;

	// Single stretched 0 test.
	for ( i = 0; i < str0_size; i++, bit_msk <<= 1 )
	{
		fprintf( ofp,
			"0x%08lx  %-28s %2u: Single stretched 0 test.\n",
			bit_msk, str0_tbl[i].msg, ++n );
	}

	m_tst_name[0]	=	'\0';
	fprintf( ofp,
			"0x%08lx  %-28s %2u: Truncated packet test.\n",
            bit_msk, m_tst_name, ++n );
	bit_msk <<= 1;

	m_tst_name[0]	=	'\0';
	fprintf( ofp,
			"0x%08lx  %-28s %2u: Prior packet test.\n",
            bit_msk, m_tst_name, ++n );
	bit_msk <<= 1;

	m_tst_name[0]	=	'\0';
	fprintf( ofp,
			"0x%08lx  %-28s %2u: 6 prior byte test.\n",
            bit_msk, m_tst_name, ++n );

	bit_msk <<= 1;

	m_tst_name[0]	=	'\0';
	fprintf( ofp,
			"0x%08lx  %-28s %2u: 1 ambiguous bit test.\n",
            bit_msk, m_tst_name, ++n );

	bit_msk <<= 1;

	m_tst_name[0]	=	'\0';
	fprintf( ofp,
			"0x%08lx  %-28s %2u: 2 ambiguous bits test.\n",
            bit_msk, m_tst_name, ++n );

	bit_msk <<= 1;

	fputs(
		"\nList of clocks to use and their corresponding '-c'  parameters:\n\n"
		"PARAMETER   CLK0T  CLK0H  CLK1T        CLOCK NAME\n"
	"----------------------------------------------------------------------"
	"------\n",
		ofp );


	// Clock settings to use.
	for ( i = 0, bit_msk =  1UL, n = 0; i < dclk_size; i++, bit_msk <<= 1 )
	{
		fprintf( ofp, "0x%08lx  %5hu  %5hu  %5hu    %2u: %s.\n",
			bit_msk, dclk_tbl[i].clk0t, dclk_tbl[i].clk0h, dclk_tbl[i].clk1t,
			++n, dclk_tbl[i].msg );
	}
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		decoder_test()						-	 Run decoder test sequence.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		decoder_test() runs the decoder tests.  It returns FAIL
 *		if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::decoder_test(
	Rslt_t			&tst_rslt )				// Decoder test result.
{
	const char		*my_name = "decoder_test";
	int				clk_idx;				// Clock test index value.
	int				i;						// Index variable.
	int				j;						// Index variable.
	u_short			tclk0t;					// Temp tclk0t.
	u_short			tclk0h;				 	// Temp tclk0h.
	u_short			tclk1t;				 	// Temp tclk1t.
	u_short			sclk0t;					// Stretched tclk0t.
	u_short			sclk0h;				 	// Stretched tclk0h.
	Bits_t			l_run_msk = 1L;			// Local test mask.
	Bits_t			t_run_msk;				// Temp run test mask.
	Bits_t			l_clk_msk = 1L;			// Local clock mask.
	int				l_clk_run;				// True if tests run for this clk.
    bool			fwd =					// True for forward loco direction.
    						!Args.get_lamp_rear();
	const char		*func_msg;				// Function init message.
    int 			ver_rel_tmp = Ver_rel;	// Tmp to hold Ver_rel to get
    										//   around compiler warning.

	if ( fsoc.get_obj_errs() )
	{
		fprintf( stderr,
			"fsoc object has errors 0x%08lx\n", fsoc.get_obj_errs() );
		ERRPRINT( my_name, LOG_ERR,
			"fsoc object has errors 0x%08lx", fsoc.get_obj_errs() );

		return ( FAIL );
	}

	if ( get_test_break() )
	{
		return ( FAIL );
	}

	CLR_LINE;
	STATPRINT(	"Starting Decoder test cycle %4lu", ++tst_cnt );
	printf(		"Starting Decoder test cycle %4lu\n", tst_cnt );

    if ( ver_rel_tmp == VER_DEB )
    {
    	STATPRINT(	">> WARNING: Debug only, not for release."   );
        printf(     ">> WARNING: Debug only, not for release.\n" );

        // Add debug specific information here.
    	STATPRINT(	"   Added packet acceptance test with 0 idles."   );
        printf(     "   Added packet acceptance test with 0 idles.\n" );
    }

	if ( Args.get_crit_flag() )
	{
		Dcc_reg.set_do_crit( true );
	}

	STATPRINT(  "Deginning booster warm up." );
	printf(		"Beginning booster warm up.\n" );
	if ( !Dcc_reg.get_running() )
	{
		Dcc_reg.start_clk();
	}
	Dcc_reg.set_clk( DECODER_0T_NOM, DECODER_0H_NOM, DECODER_1T_NOM );
	calc_filler( DECODER_0T_NOM, DECODER_1T_NOM );

 
	if ( FSOC_TST == true )
	{
		STATPRINT( "Skipping initial fsoc sequences." );
		printf(    "Skipping initial fsoc sequences.\n" );
	}
	else
	{
		STATPRINT( "sending 2 fail safe operation sequences." );
		printf(    "sending 2 fail safe operation sequences.\n" );
		Dcc_reg.clr_under();
		Dcc_reg.send_pkt( fsoc, "Send fail safe start up sequence." );
		j = Send_reg::fill_cmds_1( FSOC_DELAY * USEC_PER_SEC, DECODER_0T_NOM, DECODER_1T_NOM );
		Dcc_reg.clr_under();
		for ( i = 0; i < j; i++ )
		{
			Dcc_reg.send_idle();
		}
		
		Dcc_reg.send_pkt( fsoc, "Send fail safe start up sequence again." );
	
		j = Send_reg::fill_cmds_1( FSOC_DELAY * USEC_PER_SEC, DECODER_0T_NOM, DECODER_1T_NOM );
		Dcc_reg.clr_under();
		for ( i = 0; i < j; i++ )
		{
			Dcc_reg.send_idle();
		}
	}

	/*
     *	Do special setup for certain decoder types.
     */
    if ( Args.get_decoder_type() == DEC_FUNC )
    {
		/*	Setup the function decoder so the function is on
		 *	and the motor is set  for the correct direction.
		 */
		if ( Args.get_kick_start() == true )
		{
			/*	Send the kick start sequence.
			 * This starts the loco, turns on the function,
			 * and set the correct motor dirction.
			 */
			STATPRINT( "Starting Send_reg::kick_start() sequence." );
			printf(    "Starting Send_reg::kick_start() sequence.\n" );
			Dcc_reg.kick_start( dcc_bits, Args );
		}
		else
		{
			/*
			 *	Send forward or reverse speed 0 packet followd by
			 *	a function on packet.
			 */
			STATPRINT( "Starting loco %s setup for a function decoder.",
						fwd ? "forward" : "reverse" );
			printf(    "Starting loco %s setup for a function decoder.\n",
						fwd ? "forward" : "reverse" );
			dcc_bits.clr_in();
			dcc_bits.put_cmd_pkt_28(	Args.get_decoder_address(),
										fwd, 0, BEST_PRE );
			dcc_bits.put_idle_pkt( 1 );
			dcc_bits.put_func_grp_pkt(	Args.get_decoder_address(),
									GRP_1, Args.get_func_mask(), BEST_PRE );
			dcc_bits.put_idle_pkt( 1 );
			dcc_bits.put_1s(1).done();

			func_msg	=	fwd ?	"Function decoder, Send Fwd Spd 0."
								:	"Function decoder, Send Rev Spd 0.";
								
			j = Send_reg::fill_cmds_2( 5 * USEC_PER_SEC, DECODER_0T_NOM, DECODER_1T_NOM );
			DBGPRINT( "Sending %d loco %s speed 0 + function on packets.",
			           j, fwd ? "Fwd" : "Rev" );
			Dcc_reg.clr_under();
			for ( i = 0; i < j; i++ )
			{
				Dcc_reg.send_pkt( dcc_bits, func_msg );
			}
		}

		func_msg	=	fwd ?	"Initialized Function Decoder Forward Speed 0."
							:	"Initialized Function Decoder Reverse Speed 0.";
		STATPRINT(	func_msg );
		printf(		"%s\n", func_msg );
    }
    else if ( Args.get_decoder_type() == DEC_ACC )
    {
    	// Print message detailing accesory out preset and trigger.
        STATPRINT(	"Accessory output preset %u, trigger %u.",
        			ACC_PRE, ACC_TRG );
        printf(		"Accessory output preset %u, trigger %u.\n",
        			ACC_PRE, ACC_TRG );
    }
	
	STATPRINT( "Starting decoder tests." );
	printf(    "Starting decoder tests.\n" );

	Dcc_reg.clr_under();
	
	/*
	 *	Do 1T margin test.
	 */
	if ( l_run_msk & run_mask )
	{
		if ( decoder_margin_1() != OK )
		{
			Dcc_reg.set_do_crit( false );
			return ( FAIL );
		}
	}
	l_run_msk <<=  1;

	/*
	 *	Do 1H duty cycle test.
	 */
	if ( l_run_msk & run_mask )
	{
		if ( decoder_duty_1() != OK )
		{
			Dcc_reg.set_do_crit( false );
			return ( FAIL );
		}
	}
	l_run_msk <<=  1;

	/*
	 *	Run test series for each dclk_tbl[] entry.
	 */
	for (	clk_idx = 0, t_run_msk = l_run_msk;
			clk_idx < dclk_size;
			clk_idx++, l_clk_msk <<= 1 )
	{
		/*	Skip this dclk_tbl[] entry if Clock mask bit is not set.
		 *	Do it by setting l_clk_run so that l_run_mask is updated.
		 */
		if ( l_clk_msk & clk_mask )
		{
			l_clk_run	=	true;

			tclk0t		=	dclk_tbl[clk_idx].clk0t;
			tclk0h		=	dclk_tbl[clk_idx].clk0h;
			tclk1t		=	dclk_tbl[clk_idx].clk1t;

			STATPRINT(	"Starting clock <%s>\n- Clock 0T %4u, 0H %4u, 1T %4u",
			dclk_tbl[clk_idx].msg, tclk0t, tclk0h, tclk1t );
			printf(		"Starting clock <%s>\n- Clock 0T %4u, 0H %4u, 1T %4u\n",
			dclk_tbl[clk_idx].msg, tclk0t, tclk0h, tclk1t );

			Dcc_reg.set_clk( tclk0t, tclk0h, tclk1t );
			calc_filler( tclk0t, tclk1t );

			Dcc_reg.clr_under();
			send_filler();						// Reset the decoder.
			fsoc_send( "each clock value" );
		}
		else
		{
			l_clk_run	=	false;
		}

		l_run_msk	=	t_run_msk;			// Reset test mask on each pass.

		if ( l_clk_run && (l_run_msk & run_mask) )
		{
			if ( Args.get_decoder_type() == DEC_LOCO )
			{
				if ( decoder_ramp( tst_rslt ) != OK )
				{
					Dcc_reg.set_do_crit( false );
					return ( FAIL );
				}
			}
			else if ( Args.get_decoder_type() == DEC_FUNC )
			{
				if ( func_ramp( tst_rslt ) != OK )
				{
					Dcc_reg.set_do_crit( false );
					return ( FAIL );
				}
			}
			else if ( Args.get_decoder_type() == DEC_SIG )
			{
				if ( sig_ramp( tst_rslt ) != OK )
				{
					Dcc_reg.set_do_crit( false );
					return ( FAIL );
				}
			}
			else // Accessory decoder.
			{
				if ( acc_ramp( tst_rslt ) != OK )
				{
					Dcc_reg.set_do_crit( false );
					return ( FAIL );
				}
			}
		}
		l_run_msk <<= 1;

		for ( i = 0; i < ames_tbl_size; i++ )
		{
			if ( l_clk_run && (l_run_msk & run_mask) )
			{
				if ( decoder_ames(	tst_rslt,
									ames_tbl[i].pre_cnt,
									ames_tbl[i].idle_cnt ) != OK )
				{
					Dcc_reg.set_do_crit( false );
					return ( FAIL );
				}
			}
			l_run_msk <<=  1;
		}

		if ( l_clk_run && (l_run_msk & run_mask) )
		{
			if ( decoder_bad_addr( tst_rslt ) != OK )
			{
				Dcc_reg.set_do_crit( false );
				return ( FAIL );
			}
		}
		l_run_msk <<= 1;

		if ( l_clk_run && (l_run_msk & run_mask) )
		{
			if ( decoder_bad_bit( tst_rslt ) != OK )
			{
				Dcc_reg.set_do_crit( false );
				return ( FAIL );
			}
		}
		l_run_msk <<= 1;

		for ( i = 0; i < str0_size; i++ )
		{
			if ( l_clk_run && (l_run_msk & run_mask) )
			{
				switch ( str0_tbl[i].s_type )
				{
				case S_NONE:
					sclk0t	=	tclk0t;
					sclk0h	=	tclk0h;
					break;

				case S_POS:
					sclk0t	=	tclk0t + str0_tbl[i].clk0t_delta;
					sclk0t	=	sclk0t > DECODER_0T_MAX	? DECODER_0T_MAX
														: sclk0t;
					sclk0h	=   sclk0t - (tclk0t - tclk0h);
                    sclk0h  =   sclk0h > DECODER_0H_MAX ? DECODER_0H_MAX
                                                        : sclk0h;
					break;

				case S_NEG:
					sclk0t	=	tclk0t + str0_tbl[i].clk0t_delta;
					sclk0t	=	sclk0t > DECODER_0T_MAX	? DECODER_0T_MAX
														: sclk0t;
					sclk0h	=	tclk0h;
                    sclk0h  =   sclk0t - sclk0h > DECODER_0H_MAX ?
                                         sclk0t - DECODER_0H_MAX : sclk0h;
					break;
				};

				if ( decoder_str0_ames(	tst_rslt,
										sclk0t,
										sclk0h ) != OK )
				{
					Dcc_reg.set_do_crit( false );
					return ( FAIL );
				}
			}
			l_run_msk <<=  1;
		}

		if ( l_clk_run  == true )
      {
         print_test_rslt( tst_rslt );
      }
	}

	/*
	 *	Do truncated packet test.
	 */
	if ( l_run_msk & run_mask )
	{
		if ( decoder_truncate( tst_rslt ) != OK )
		{
			Dcc_reg.set_do_crit( false );
			return ( FAIL );
		}
	}
	l_run_msk <<=  1;

	/*
	 *	Do prior packet test.
	 */
	if ( l_run_msk & run_mask )
	{
		if ( decoder_prior( tst_rslt ) != OK )
		{
			Dcc_reg.set_do_crit( false );
			return ( FAIL );
		}
	}

	l_run_msk <<=  1;
	/*
	 *	Do 6 prior byte test.
	 */
	if ( l_run_msk & run_mask )
	{
		if ( decoder_6_byte( tst_rslt ) != OK )
		{
			Dcc_reg.set_do_crit( false );
			return ( FAIL );
		}
	}
 
	l_run_msk <<=  1;
	/*
	 *	Do 1 ambiguous bit test.
	 */
	if ( l_run_msk & run_mask )
	{
		if ( decoder_ambig1( tst_rslt ) != OK )
		{
			Dcc_reg.set_do_crit( false );
			return ( FAIL );
		}
	}

	l_run_msk <<=  1;
	/*
	 *	Do 2 ambiguous bits test.
	 */
	if ( l_run_msk & run_mask )
	{
		if ( decoder_ambig2( tst_rslt ) != OK )
		{
			Dcc_reg.set_do_crit( false );
			return ( FAIL );
		}
	}

	l_run_msk <<=  1;

	Dcc_reg.set_do_crit( false );
	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		get_test_break()			   		-	 test for manual test break.
 *
 *	RETURN VALUE
 *
 *		false	-	Continue tests.
 *		true	-	Stop tests.
 *
 *	DESCRIPTION
 *
 *		get_test_break() tests for the manual test break sequence '<ESC> q'
 *		and returns true if it is seen.  It returns false otherwise.
 */
/*--------------------------------------------------------------------------*/

bool
Dec_tst::get_test_break( void )
{
	static const char	*my_name = "get_test_break";
	static char			lchar = '\0';	   	// Last character received.
	char				tchar;			 	// Present character.
	bool				retval = false;	 	// Return value.

	while ( kbhit() )
	{
		tchar	=	lchar;
		lchar	=	getch();
		if ( (tchar == '\033') && (lchar == 'q') )
		{
			retval	=	true;
		}
	}

	if ( retval == true )
	{
		printf( "\n<BREAK>\n" );
		ERRPRINT( my_name, LOG_WARNING, "Test stopped by BREAK" );
	}

	return( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		decoder_ramp()						-	 Run loco decoder ramp.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		decoder_ramp() runs the loco decoder ramp test.  It returns FAIL
 *		if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::decoder_ramp(
	Rslt_t			&tst_rslt )				// Decoder test result.
{
	const char		*my_name = "Ramp";
	const char		*tst_name = "Ramp:";   	// Test name.
	int				i;						// Index variable.
	Rslt_t			retval = OK;	   		// Return value.
	bool			fw;				   		// Go forward if true.
	int				speed;			  		// Speed.
	Ramp_state		state;			  		// Present ramp test state.
	BYTE			tgen;			  		// Generic input BYTE.
	u_int			pre_cnt;				// Preamble count for test.

	pre_cnt	=	BEST_PRE + Args.get_extra_preamble();

	Dcc_reg.clr_err_cnt();					// Restart error counter.
	t_stat.reset();
	
	fw		=	true;
	speed	=	SP_MIN;

	CLR_LINE;

	for ( state = RAMP_INIT ; state != RAMP_END; )
	{
		switch ( state )
		{
		case RAMP_INIT:
			fw		=	true;
			speed	=	SP_MIN;
			state	=	RAMP_UP;
			break;

		case RAMP_UP:
			if ( speed == SP_28_MAX )
			{
				speed--;
				state	=	RAMP_DOWN_FW;
			}
			else
			{
				speed++;
			}
			break;

		case RAMP_DOWN_FW:
			if ( speed == SP_MIN )
			{
				speed++;
				fw	=	false;
				state	=	RAMP_DOWN_REV;
			}
			else
			{
				speed--;
			}
			break;

		case RAMP_DOWN_REV:
			if ( speed == SP_28_MAX )
			{
				speed	=	SP_E_STOP;
				fw		=	false;
				state	=	RAMP_E_STOP;
			}
			else
			{
				speed++;
			}
			break;

		case RAMP_E_STOP:
			state	=	RAMP_END;
			break;
		}

		if ( state != RAMP_END )
		{
        	printf(
            	"  %-18s Pre %2d, Addr %5u, Speed %2d, "
				"Dir. %c, Fails %4lu\r",
				tst_name, pre_cnt, Args.get_decoder_address(), speed,
				fw ? 'F' :  'R',
				t_stat.get_f_cnt() );

			dcc_bits.clr_in();
			dcc_bits.put_0s(1);	// Make sure a 0 is ahead of the preamble.
			for ( i = 0; i < PKT_REP_MIN; i++ )
			{
		 		dcc_bits.put_cmd_pkt_28(	Args.get_decoder_address(),
		 									fw, speed, pre_cnt );
		 		dcc_bits.put_idle_pkt( BEST_IDLE );
		 	}
		 	dcc_bits.put_1s(1).done();

		 	Dcc_reg.clr_under();
            Dcc_reg.set_scope( true );

		 	Dcc_reg.send_pkt( dcc_bits, "Send loco ramp test packet." );

		 	send_filler();
		 	/*
		 	 *	Test the input only when the decoder is going reverse
		 	 *	at relatively high speed with an even speed step.
             *	Even speed steps are equivalent to 14 speed mode with
             *	the lamp on.
             *	This will work correctly for motor decoders, FL only
             *	decoders, and will show 100% failures if no decoder is
             *	connected.
			 */
			if ( speed >= SP_TEST_MIN && fw ==  false && !(speed & 0x01) )
			{
				t_stat.incr_t_cnt();
				tgen	=	Dcc_reg.get_gen();
				if ( (tgen & 0x01) != fw )
				{
					t_stat.incr_f_cnt();
					tst_rslt	=	FAIL;
					ERRPRINT( my_name, LOG_WARNING,
						"Test FAILED: Speed %2d, Dir. %c",
						speed, fw ? 'F' :  'R' );
				}
			}
		}

		if ( get_test_break() )
		{
			retval	=	FAIL;
			break;
		}
	}

	CLR_LINE;
	TO_STAT(	"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
		tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
		t_stat.get_percent() );
	printf(		"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
		tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
		t_stat.get_percent() );

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		func_ramp()						-	 Run function decoder ramp.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		func_ramp() runs the function decoder ramp test.  It returns FAIL
 *		if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::func_ramp(
	Rslt_t			&tst_rslt )				// Decoder test result.
{
	const char		*my_name = "Func";
	const char		*tst_name = "Func:";   	// Test name.
	int				i;						// Function variable.
    int				func;					// Function state.
	Rslt_t			retval = OK;	   		// Return value.
	BYTE			tgen;			  		// Generic input BYTE.
	u_int			pre_cnt;				// Preamble count for test.

	pre_cnt	=	BEST_PRE + Args.get_extra_preamble();

	Dcc_reg.clr_err_cnt();					// Restart error counter.
	t_stat.reset();
	
	CLR_LINE;

	for ( func = 1; func >= 0; func-- )
    {
		TO_LOG( "Dce_tst::func_ramp() Starting func test loop func %s\n",
                    func == 0 ? "OFF" : " ON" );
       	printf(
           	"  %-18s Pre %2d, Addr %5u, Function <%s>, "
	 		"Fails %4lu\r",
	 		tst_name, pre_cnt, Args.get_decoder_address(),
	 		func == 0 ? "OFF" : " ON",
	 		t_stat.get_f_cnt() );

		dcc_bits.clr_in();
		// See .\NMRA\put_........_pktx Info.txt
		dcc_bits.put_func_grp_pktx(	Args.get_decoder_address(),
	   			 GRP_1, func == 0 ? 0 : Args.get_func_mask(), pre_cnt );
		dcc_bits.put_1s( 1 ).put_byte( 0x00 );	// Align next packet on Byte boundary.
		dcc_bits.put_idle_pkt( 1 );
		dcc_bits.put_1s( 1 ).done();	// Align next packet on Byte boundary.
	  	Dcc_reg.clr_under();

        Dcc_reg.set_scope( true );
		TO_LOG( "Dce_tst::func_ramp\(\) Sending %d function ramp packets.\n", pkt_rep_cnt );
		debug_gen( "Dce_tst::func_ramp() Just before send_pkt() to send function sequence" );
	  	for ( i = 0; i < pkt_rep_cnt; i++ )
	  	{
			Dcc_reg.send_pkt( dcc_bits, "Send function ramp test packet." );
	   	}
		debug_gen( "Dce_tst::func_ramp() Just after send_pkt() to send function sequence" );

	  	/*
	  	 *	Test the function input to verify it matches the
         *	expected value.
	 	 */
	 	t_stat.incr_t_cnt();
	 	tgen	=	Dcc_reg.get_gen();
	 	if ( (tgen & 0x01) != (func == 0 ? 0x01 : 0) )
	 	{
	 		t_stat.incr_f_cnt();
	 		tst_rslt	=	FAIL;
	 		ERRPRINT( my_name, LOG_WARNING,
	 			"Test FAILED: Function <%s>",
	 			func == 0 ? "OFF" : "ON" );
	 	}
		debug_gen( "Dce_tst::func_ramp() Just after input test" );

		if ( get_test_break() )
		{
			retval	=	FAIL;
			break;
		}
	}

	CLR_LINE;
	TO_STAT(	"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
		tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
		t_stat.get_percent() );
	printf(		"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
		tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
		t_stat.get_percent() );

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		acc_ramp()							-	 Run accessory ramp.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		acc_ramp() runs the accessory ramp test.  It returns FAIL
 *		if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::acc_ramp(
	Rslt_t			&tst_rslt )				// Decoder test result.
{
	const char		*my_name = "Acc";
	const char		*tst_name = "Acc:";   	// Test name.
	int				i;						// Index variable.
	int				j;						// Index variable.
	Rslt_t			retval = OK;	   		// Return value.
	bool			active;					// Output active if true.
	BYTE		  	out_id;			  		// Output ID.
	BYTE			tgen;			  		// Generic input BYTE.
	bool			test_expect;			// Expected test result.
	u_int			pre_cnt;				// Preamble count for test.

	pre_cnt	=	BEST_PRE + Args.get_extra_preamble();

	Dcc_reg.clr_err_cnt();					// Restart error counter.
	t_stat.reset();
	
	CLR_LINE;

	for ( out_id =  0; out_id <= ACC_MAX; out_id++ )
	{
		for ( i = 0, active = false; i < 2; i++, active =  true )
		{
			printf(
				"  %-18s Pre %2d, Addr %5u, Out_id %1d, Output %3s, Fails %4lu\r",
				tst_name, pre_cnt, Args.get_decoder_address(), out_id,
				active ? "ON" : "OFF",
				t_stat.get_f_cnt() );

			dcc_bits.clr_in();
			dcc_bits.put_0s(1);	// Make sure a 0 is ahead of the preamble.
			for ( j = 0; j < PKT_REP_MIN; j++ )
			{
				dcc_bits.put_acc_pkt(	Args.get_decoder_address(), active,
										out_id, pre_cnt );
				dcc_bits.put_idle_pkt( BEST_IDLE );
			}
			dcc_bits.put_1s(1).done();
				
			Dcc_reg.clr_under();
            Dcc_reg.set_scope( true );

			Dcc_reg.send_pkt( dcc_bits, "Send accessory ramp test packet." );

			send_filler();

			// Test output for accessory preset and trigger values.
			if ( (out_id==ACC_PRE || out_id==ACC_TRG) && active == true )
			{
				t_stat.incr_t_cnt();
				tgen	=	Dcc_reg.get_gen();
				if ( out_id == ACC_PRE && active == true )
				{
					test_expect	=	false;
				}
				else
				{
					test_expect	=	true;
				}

				if ( (tgen & 0x01) != test_expect )
				{
					t_stat.incr_f_cnt();
					tst_rslt	=	FAIL;
					ERRPRINT( my_name, LOG_WARNING,
						"Test FAILED: Out_id %1d, Active %3s, Input %3s",
						out_id, active ? "ON" : "OFF",
                        (tgen & 0x01) ? "ON" : "OFF" );
				}
			}
		}

		if ( get_test_break() )
		{
			retval	=	FAIL;
			break;
		}
	}

	CLR_LINE;
	TO_STAT(	"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
		tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
		t_stat.get_percent() );
	printf(		"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
		tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
		t_stat.get_percent() );

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		sig_ramp()						-	 Run signal decoder ramp.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		sig_ramp() runs the signal decoder ramp test.  It returns FAIL
 *		if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::sig_ramp(
	Rslt_t			&tst_rslt )				// Decoder test result.
{
	const char		*my_name = "Sig";
	const char		*tst_name = "Sig:";   	// Test name.
	BYTE			i;						// Function variable.
    int				aspect;					// Function state.
	Rslt_t			retval = OK;	   		// Return value.
	BYTE			tgen;			  		// Generic input BYTE.
	u_int			pre_cnt;				// Preamble count for test.

	pre_cnt	=	BEST_PRE + Args.get_extra_preamble();

	Dcc_reg.clr_err_cnt();					// Restart error counter.
	t_stat.reset();
	
	CLR_LINE;

	for ( aspect = 0; aspect <= ASPECT_MAX; aspect++ )
    {
       	printf(
           	"  %-18s Pre %2d, Addr %5u, Aspect %2d, "
	 		"Fails %4lu\r",
	 		tst_name, pre_cnt, Args.get_decoder_address(), aspect,
	 		t_stat.get_f_cnt() );

	  	dcc_bits.clr_in();
	  	dcc_bits.put_0s(1);	// Make sure a 0 is ahead of the preamble.
	  	for ( i = 0; i < PKT_REP_MIN; i++ )
	  	{
	   		dcc_bits.put_sig_pkt(	Args.get_decoder_address(),
            						aspect, pre_cnt );
	   		dcc_bits.put_idle_pkt( BEST_IDLE );
	   	}
	   	dcc_bits.put_1s(1).done();

	  	Dcc_reg.clr_under();
        Dcc_reg.set_scope( true );

	  	Dcc_reg.send_pkt( dcc_bits, "Send signal ramp test packet." );

	  	send_filler();

	  	/*
	  	 *	Test the function input to verify it matches the
         *	expected value.
	 	 */
        if ( 		( aspect == Args.get_aspect_preset()  )
        		||	( aspect == Args.get_aspect_trigger() ) )
        {
	 		t_stat.incr_t_cnt();
	 		tgen	=	Dcc_reg.get_gen();

	 		if ( 	(tgen & 0x01)
            	!= (aspect == Args.get_aspect_preset() ? 0 : 0x01) )
	 		{
	 			t_stat.incr_f_cnt();
	 			tst_rslt	=	FAIL;
	 			ERRPRINT( my_name, LOG_WARNING,
	 				"Test FAILED: aspect %d", aspect );
	 		}
        }

		if ( get_test_break() )
		{
			retval	=	FAIL;
			break;
		}
	}

	CLR_LINE;
	TO_STAT(	"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
		tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
		t_stat.get_percent() );
	printf(		"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
		tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
		t_stat.get_percent() );

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		decoder_ames()						-	 Run decoder ames test.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		decoder_ames() runs the decoder ames test.  It returns FAIL
 *		if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::decoder_ames(
	Rslt_t			&tst_rslt,				// Decoder test result.
	u_int			pre_cnt,				// Preamble bit count.
	u_int			idle_cnt )				// Count of idles to use.
{
	const char		*my_name = "Ames";
	int				i;			  			// Index variable.
	int				j;						// Index variable.
	Rslt_t			retval = OK;			// Return value.
	BYTE			tgen;					// Generic input BYTE.
	bool			pre_fail;	  			// Preset failed.
	bool			trig_fail;	  			// Trigger failed.
	int				t_pkt_cnt;				// Temp repeat count.

	sprintf( m_tst_name, "pre %d idle %d:", pre_cnt, idle_cnt );
	Dcc_reg.clr_err_cnt();					// Restart error counter.
	t_stat.reset();

	/*
	 *	Double the packet repeats if no idles are being sent.
	 */
	t_pkt_cnt	=	(idle_cnt == 0) ? 2 * pkt_rep_cnt : pkt_rep_cnt;

	CLR_LINE;

	/*
	 *	Preset loco decoder to reverse with lamp on or
	 *  preset accessory decoder to the reverse position.
	 */
	dcc_bits.clr_in();
	dcc_bits.put_0s(1);	// Make sure a 0 is ahead of the preamble.
	if ( Args.get_decoder_type() == DEC_LOCO )
	{
		dcc_bits.put_cmd_pkt_28(	Args.get_decoder_address(),
									false, SP_TEST_MIN, pre_cnt );
	}
	else if ( Args.get_decoder_type() == DEC_FUNC )
	{
		// See .\NMRA\put_........_pktx Info.txt
		dcc_bits.put_func_grp_pktx(	Args.get_decoder_address(),
									GRP_1, Args.get_func_mask(), pre_cnt );
	}
    else if ( Args.get_decoder_type() == DEC_SIG )
    {
    	dcc_bits.put_sig_pkt(		Args.get_decoder_address(),
        							Args.get_aspect_preset() );
    }
	else // Accessory decoder.
	{
		dcc_bits.put_acc_pkt(		Args.get_decoder_address(),
									true, ACC_PRE, pre_cnt );
	}
	dcc_bits.put_idle_pkt( idle_cnt );
	dcc_bits.put_1s(1).done();

	/*
	 *	Set decoder to trigger speed, forward, lamp off or
	 *	set accessory decoder to the normal position.
	 */
	dcc_bits2.clr_in();
	dcc_bits2.put_0s(1);	// Make sure a 0 is ahead of the preamble.
	if ( Args.get_decoder_type() ==  DEC_LOCO )
	{
		dcc_bits2.put_cmd_pkt_28(	Args.get_decoder_address(),
									true, trig_cmd_speed, pre_cnt );
	}
	else if ( Args.get_decoder_type() == DEC_FUNC )
	{
		// See .\NMRA\put_........_pktx Info.txt
		dcc_bits2.put_func_grp_pktx(	Args.get_decoder_address(),
										GRP_1, 0, pre_cnt );
	}
    else if ( Args.get_decoder_type() == DEC_SIG )
    {
    	dcc_bits2.put_sig_pkt(		Args.get_decoder_address(),
        							Args.get_aspect_trigger() );
    }
	else // Accessory decoder.
	{
		dcc_bits2.put_acc_pkt(		Args.get_decoder_address(),
									true, ACC_TRG, pre_cnt );
	}
	dcc_bits2.put_idle_pkt( idle_cnt );
	dcc_bits2.put_1s(1).done();

	/*
	 *	Note: This test is run 100 times to make sure there is less
	 *	Than a 1% chance of not seeing at least 1 failure assuming the
	 *	average pass rate is 95%.  100 tests gives a miss rate of .59%.
	 */
	for ( i = 0; i < 100; i++ )
	{
		pre_fail	=	false;
		trig_fail	=	false;

		printf(
			"  %-18s Addr %5u, Speed %2d cycle %4d, Fails %4lu\r",
			m_tst_name, Args.get_decoder_address(), SP_TEST_MIN,
			i+1, t_stat.get_f_cnt() );

		fsoc_send( "decoder_ames\(\)" );
		
		for ( j = 0; j < t_pkt_cnt;  j++ )   	// Preset decoder reverse.
		{
			Dcc_reg.send_pkt(	dcc_bits,
								"Send packet acceptance preset packet." );
		}

		tgen	=	Dcc_reg.get_gen();
		if ( (tgen & 0x01) != false )
		{
			pre_fail	=	true;
		}

		/*
		 * Send a scope trigger at start of trigger packet
		 * if the normal (early) scope trigger is desired.
		 */
		if ( Args.get_late_scope() != true )
		{
			Dcc_reg.set_scope( true );
		}

		Dcc_reg.send_pkt(	dcc_bits2,			// Attempt emergency stop.
							"Send packet acceptance trigger packet." );

		/*
		 * The following code sends another idle filler, turns
		 * on the scope trigger, and then sends one second of idle filler.
		 * This results in scope trigger starting 2 idles after the trigger
		 * command.
		 */
		Dcc_reg.send_idle();					// Send one idle space.

		if ( Args.get_late_scope() == true )
		{
			Dcc_reg.set_scope( true );
		}

		send_filler();							// Send filler.

		tgen	=	Dcc_reg.get_gen();
		if ( (tgen & 0x01) != true )
		{
			trig_fail	=	true;
		}

		t_stat.incr_t_cnt();

		if ( pre_fail || trig_fail )
		{
			t_stat.incr_f_cnt();
			tst_rslt	=	FAIL;
			ERRPRINT( my_name, LOG_WARNING,
				"%-10s FAILED: cycle %3d",
				err_phrase( pre_fail, trig_fail ), i+1 );
		}

		if ( get_test_break() )
		{
			retval	=	FAIL;
			break;
		}
	}

	CLR_LINE;
	TO_STAT(	"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
		m_tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
		t_stat.get_percent() );
	printf(		"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
		m_tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
		t_stat.get_percent() );

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		decoder_str0_ames()		  			-	 Run stretched ames test.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		decoder_str0_ames() runs the stretched ames test.  It returns FAIL
 *		if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::decoder_str0_ames(
	Rslt_t			&tst_rslt,				// Decoder test result.
	u_short			iclk0t,					// Stretched clk0t.
	u_short			iclk0h )				// Stretched clk0h.
{
	const char		*my_name  = "Stretch 0";
	int				i;			  			// Index variable.
	int				j;						// Index variable.
	Rslt_t			retval = OK;			// Return value.
	BYTE			tgen;					// Generic input BYTE.
	bool			pre_fail;	  			// Preset failed.
	bool			trig_fail;	  			// Trigger failed.
	BYTE			pbyte;					// Present byte to send.

	sprintf( m_tst_name, "0T %5u 0H %5u:", iclk0t, iclk0h );
	Dcc_reg.clr_err_cnt();					// Restart error counter.
	t_stat.reset();

	CLR_LINE;

	/*
	 *	Preset loco decoder to reverse with lamp on or
	 *  preset accessory decoder to the reverse position.
	 */
	dcc_bits.clr_in();
	dcc_bits.put_0s(1);	// Make sure a 0 is ahead of the preamble.
	if ( Args.get_decoder_type() == DEC_LOCO )
	{
		dcc_bits.put_cmd_pkt_28(	Args.get_decoder_address(),
									false, SP_TEST_MIN, BEST_PRE );
	}
	else if ( Args.get_decoder_type() == DEC_FUNC )
	{
		dcc_bits.put_func_grp_pktx(	Args.get_decoder_address(),
									GRP_1, Args.get_func_mask(), BEST_PRE );
	}
    else if ( Args.get_decoder_type() == DEC_SIG )
    {
    	dcc_bits.put_sig_pkt(		Args.get_decoder_address(),
        							Args.get_aspect_preset() );
    }
	else // Accessory decoder.
	{
		dcc_bits.put_acc_pkt(		Args.get_decoder_address(),
									true, ACC_PRE, BEST_PRE );
	}
	dcc_bits.put_idle_pkt( 1 );
	dcc_bits.put_1s(1).done();

	/*
	 *	Set decoder to trigger speed, forward, lamp off or
	 *	set accessory decoder to the normal position.
	 */
	dcc_bits2.clr_in();
	dcc_bits2.put_0s( STRETCH_BITS );	// Align first 0 on bit boundary.
	if ( Args.get_decoder_type() ==  DEC_LOCO )
	{
		dcc_bits2.put_cmd_pkt_28(	Args.get_decoder_address(),
									true, trig_cmd_speed, BEST_PRE );
	}
	else if ( Args.get_decoder_type() == DEC_FUNC )
	{
		dcc_bits2.put_func_grp_pktx(	Args.get_decoder_address(),
									GRP_1, 0, BEST_PRE );
	}
    else if ( Args.get_decoder_type() == DEC_SIG )
    {
    	dcc_bits2.put_sig_pkt(		Args.get_decoder_address(),
        							Args.get_aspect_trigger() );
    }
	else // Accessory decoder.
	{
		dcc_bits2.put_acc_pkt(		Args.get_decoder_address(),
									true, ACC_TRG, BEST_PRE );
	}
	dcc_bits2.put_idle_pkt( 1 );
	dcc_bits2.put_1s(1).done();

	/*
	 *	Note: This test is run 100 times to make sure there is less
	 *	Than a 1% chance of not seeing at least 1 failure assuming the
	 *	average pass rate is 95%.  100 tests gives a miss rate of .59%.
	 */
	for ( i = 0; i < 100; i++ )
	{
		pre_fail	=	false;
		trig_fail	=	false;

		printf(
			"  %-18s Addr %5u, Speed %2d cycle %4d, Fails %4lu\r",
			m_tst_name, Args.get_decoder_address(), SP_TEST_MIN,
			i+1, t_stat.get_f_cnt() );

		fsoc_send( "decoder_str0_ames\(\) loop" );
		for ( j = 0; j < pkt_rep_cnt;  j++ )   	// Preset decoder reverse.
		{
			Dcc_reg.send_pkt( dcc_bits, "Send stretched test preset packet." );
		}

		tgen	=	Dcc_reg.get_gen();
		if ( (tgen & 0x01) != false )
		{
			pre_fail	=	true;
		}
        /*
         * Send BYTES immediately before single stretched 0.
         */
		dcc_bits2.rst_out();
		dcc_bits2.get_byte( pbyte );			// Send first 2 preamble bits.
		Dcc_reg.send_bytes( 1, pbyte, "Send 1st stretched trigger byte." );
		dcc_bits2.get_byte( pbyte );			// Send remaining 8 preambles.
		Dcc_reg.send_bytes( 1, pbyte, "Send 2nd stretched trigger byte." );

		/*
		 * Send a scope trigger at start of trigger packet
		 * if the normal (early) scope trigger is desired.
		 */
		if ( Args.get_late_scope() != true )
		{
			Dcc_reg.set_scope( true );
		}

		dcc_bits2.get_byte( pbyte );			// Get first part of packet.
		Dcc_reg.send_stretched_byte(	iclk0t,	// Send single stretched 0.
										iclk0h,
										pbyte,
										"Send byte with single stretched 0." );
		// Send remaining part of packet.
		while ( dcc_bits2.get_byte( pbyte ) == OK )
		{
			Dcc_reg.send_bytes(	1, pbyte,
								"Send remaining stretched trigger packet." );
		}

		/*
		 * The following code sends another idle filler, turns
		 * on the scope trigger, and then sends one second of idle filler.
		 * This results in scope trigger starting 2 idles after the trigger
		 * command.
		 */
		Dcc_reg.send_idle();					// Send one idle space.

		if ( Args.get_late_scope() == true )
		{
			Dcc_reg.set_scope( true );
		}

		send_filler();							// Send filler.

		tgen	=	Dcc_reg.get_gen();
		if ( (tgen & 0x01) != true )
		{
			trig_fail	=	true;
		}

		t_stat.incr_t_cnt();

		if ( pre_fail || trig_fail )
		{
			t_stat.incr_f_cnt();
			tst_rslt	=	FAIL;
			ERRPRINT( my_name, LOG_WARNING,
				"%-10s FAILED: cycle %3d",
				err_phrase( pre_fail, trig_fail ), i+1 );
		}

		if ( get_test_break() )
		{
			retval	=	FAIL;
			break;
		}
	}

	CLR_LINE;
	TO_STAT(	"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
		m_tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
		t_stat.get_percent() );
	printf(		"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
		m_tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
		t_stat.get_percent() );

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		decoder_bad_addr()						-	 Run bad address test.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		decoder_bad_addr() runs the decoder bad address test.  It returns FAIL
 *		if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::decoder_bad_addr(
	Rslt_t			&tst_rslt )					// Decoder test result.
{
	const char		*my_name = "Addr";
	const char		*tst_name = "Addr:"; 		// Test name.
	int				j;							// Index variable.
	Rslt_t			retval = OK;				// Return value.
	int				addr;						// Test address.
	int				addr_min;					// Minimum test address.
	int				addr_max;					// Maximum test address.
	int				send_addr;					// Address to send to decoder.
	BYTE			tgen;						// Generic input BYTE.
	bool			pre_fail;	  				// Preset failed.
	bool			trig_fail;	  				// Trigger failed.
	bool			rst_fail;					// Reset failed.
	u_int			pre_cnt;					// Preamble count for test.
	const char      *rst_msg;						// Reset message.

	pre_cnt	=	BEST_PRE + Args.get_extra_preamble();

	Dcc_reg.clr_err_cnt();						// Restart error counter.
    t_stat.reset();

	CLR_LINE;

	/*
	 *	Set up the minimum and maximum addresses.
	 */
	if ( Args.get_decoder_type() != DEC_ACC )	// Set Loco/Func addresses.
	{
		addr_min	=	LOC_ADDR_MIN;
		addr_max	=	LOC_ADDR_MAX;
	}
	else										// Set accessory addresses.
	{
		addr_min	=	ACC_ADDR_MIN;
		addr_max	=	ACC_ADDR_MAX;
	}
	
	/*  Set up special reset decoder bit array if fsoc is not used.
	 *  dcc_bits3 will have 0 Bytes if fsoc should be used.
	 *  A special reseet sequence using dcc_bits3 is sed for:
	 *      Accessory decoders: Use ACC_TRG.
	 *		Function decoders:  Turn functions off.
	 */
	dcc_bits3.clr_in();
	if ( Args.get_decoder_type() == DEC_FUNC )
	{
		dcc_bits3.put_0s(1);	// Make sure a 0 is ahead of the preamble.
		dcc_bits3.put_func_grp_pktx(	Args.get_decoder_address(),
									GRP_1, 0, pre_cnt );
		dcc_bits3.put_idle_pkt( BEST_IDLE );
		dcc_bits3.put_1s(1).done();
		
		rst_msg = "Send bad address fuction reset packet.";
	}
	else if ( Args.get_decoder_type() == DEC_ACC )
	{
		dcc_bits3.put_0s(1);	// Make sure a 0 is ahead of the preamble.
		dcc_bits3.put_acc_pkt( 	Args.get_decoder_address(),
								true, ACC_TRG, pre_cnt );
		dcc_bits3.put_idle_pkt( BEST_IDLE );
		dcc_bits3.put_1s(1).done();
		
		rst_msg = "Send bad address accessory reset packet.";
	}
	
	/*
	 *	Preset decoder to reverse with lamp on for LOCO,
	 *  presete function on for FUNC decoder, or
	 *  preset accessory decoder to the reverse position.
	 */
	dcc_bits.clr_in();
	dcc_bits.put_0s(1);	// Make sure a 0 is ahead of the preamble.
	if ( Args.get_decoder_type() == DEC_LOCO )
	{
		dcc_bits.put_cmd_pkt_28(	Args.get_decoder_address(),
									false, SP_TEST_MIN, pre_cnt );
	}
	else if ( Args.get_decoder_type() == DEC_FUNC )
	{
		dcc_bits.put_func_grp_pktx(	Args.get_decoder_address(),
									GRP_1, Args.get_func_mask(), pre_cnt );
	}
	else // Accessory decoder.
	{
		dcc_bits.put_acc_pkt( 		Args.get_decoder_address(),
									true, ACC_PRE, pre_cnt );
	}
	dcc_bits.put_idle_pkt( BEST_IDLE );
	dcc_bits.put_1s(1).done();

	for ( addr = addr_min; addr <= addr_max + 1; addr++ )
	{
		pre_fail	=	false;
		trig_fail	=	false;
		rst_fail	=	false;

		if ( addr == addr_max + 1 )
		{
			send_addr	=	Args.get_decoder_address();
		}
		else if ( addr == Args.get_decoder_address() )
		{
			continue;
		}
		else
		{
			send_addr	=	addr;
		}

		printf(
			"  %-18s Preamble %2d, Address %5u, Fails %4lu\r",
			tst_name, pre_cnt, send_addr, t_stat.get_f_cnt() );

		dcc_bits2.clr_in();
		dcc_bits2.put_0s(1);	// Make sure a 0 is ahead of the preamble.
		if ( Args.get_decoder_type() == DEC_LOCO )
		{
			dcc_bits2.put_cmd_pkt_28(	send_addr, true,
										trig_cmd_speed, pre_cnt );
		}
		else if ( Args.get_decoder_type() == DEC_FUNC )
		{
			dcc_bits2.put_func_grp_pktx(	send_addr,
									GRP_1, 0, pre_cnt );
		}
		else // Accessory decoder.
		{
			dcc_bits2.put_acc_pkt(		send_addr, true, ACC_TRG, pre_cnt );
		}
		dcc_bits2.put_idle_pkt( BEST_IDLE );
		dcc_bits2.put_1s(1).done();

		/*	Reset the decoder using the appropriate sequence by decoder type.
		 *	fsoc is used to reset the decoder unless dcc_bits3 has s special reset packet.
		 */
		Dcc_reg.clr_under();
		if ( dcc_bits3.get_byte_size() == 0 )	// Send Fail Safe Rst cmd.
		{
			Dcc_reg.send_pkt( fsoc, "Send fail safe start up sequence." );
			send_filler();
		}
		else									// Send special reset packets.
		{
			for ( j = 0; j < pkt_rep_cnt;  j++ )
			{
				Dcc_reg.send_pkt( dcc_bits3, rst_msg );
			}
		}

		tgen	=	Dcc_reg.get_gen();
		if ( (tgen & 0x01) != true )
		{
			rst_fail	=	true;
		}

		for ( j = 0; j < pkt_rep_cnt;  j++ )		// Send preset command.
		{
			Dcc_reg.send_pkt( dcc_bits, "Send bad address preset packet." );
		}

		tgen	=	Dcc_reg.get_gen();
		if ( (tgen & 0x01) != false )
		{
			pre_fail	=	true;
		}

		for ( j = 0; j < pkt_rep_cnt; j++ )			// Send test command.
		{
        	Dcc_reg.set_scope( true );
			Dcc_reg.send_pkt( dcc_bits2, "Send bad address trigger packet." );
		}

		tgen	=	Dcc_reg.get_gen();
		if ( (tgen & 0x01) != ( send_addr ==
								Args.get_decoder_address() ? true : false) )
		{
			trig_fail	=	true;
		}

		t_stat.incr_t_cnt();

		if ( pre_fail || trig_fail || rst_fail )
		{
			t_stat.incr_f_cnt();
			tst_rslt	=	FAIL;
			ERRPRINT( my_name, LOG_WARNING,
			"%-10s FAILED: %s addr %5u",
			err_phrase( pre_fail, trig_fail, rst_fail ),
			send_addr ==  Args.get_decoder_address() ? "Good" : " Bad",
			send_addr );
		}

		if ( get_test_break() )
		{
			retval	=	FAIL;
			break;
		}
	}

	CLR_LINE;
	TO_STAT(	"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
		tst_name, t_stat.get_t_cnt(), t_stat.get_p_cnt(),
		t_stat.get_percent() );
	printf(		"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
		tst_name, t_stat.get_t_cnt(), t_stat.get_p_cnt(),
		t_stat.get_percent() );

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		decoder_bad_bit()						-	 Run bad bit test.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		decoder_bad_bit() runs the decoder bad bit test.  It returns FAIL
 *		if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::decoder_bad_bit(
	Rslt_t			&tst_rslt )				// Decoder test result.
{
	const char		*my_name = "Bad bit";
	const char		*tst_name = "Bad bit:"; // Test name.
	int				i;			  			// Index variable.
	int				j;						// Index variable.
	Rslt_t			retval = OK;			// Return value.
	BYTE			tgen;					// Generic input BYTE.
	bool			pre_fail;	  			// Preset failed.
	bool			trig_fail;	  			// Trigger failed.
	bool			rst_fail;				// Reset failed.
	u_int			pre_cnt;				// Preamble count for test.
    int				first_bit;				// First bit position to test.
    int				last_bit;				// Last bit position to test.
	const char      *rst_msg;				// Reset message.

	pre_cnt		=	BEST_PRE + Args.get_extra_preamble();

    // Begin testing after extra preamble + indeterminate PRE_SKIP_BITS.
    // First bit is bit 0.
    first_bit	=   Args.get_extra_preamble() + PRE_SKIP_BITS;

    // End testing pointing at stop bit after base packet + extra preambles.
    last_bit	=   Args.get_extra_preamble() + BAD_BIT_BITS;

	Dcc_reg.clr_err_cnt();					// Restart error counter.
    t_stat.reset();

	CLR_LINE;
	
	/*  Set up special reset decoder bit array if fsoc is not used.
	 *  dcc_bits3 will have 0 Bytes if fsoc should be used.
	 *  A special reseet sequence using dcc_bits3 is sed for:
	 *      Accessory decoders: Use ACC_TRG.
	 *		Function decoders:  Turn functions off.
	 */
	dcc_bits3.clr_in();
	if ( decoder_type == DEC_FUNC )
	{
		dcc_bits3.put_0s(1);	// Make sure a 0 is ahead of the preamble.
		dcc_bits3.put_func_grp_pktx(	Args.get_decoder_address(),
									GRP_1, 0, pre_cnt );
		dcc_bits3.put_idle_pkt( BEST_IDLE );
		dcc_bits3.put_1s(1).done();
		
		rst_msg = "Send bad bit function reset packet.";
	}
	else if ( decoder_type == DEC_ACC )
	{
		dcc_bits3.put_0s(1);	// Make sure a 0 is ahead of the preamble.
		dcc_bits3.put_acc_pkt( 	Args.get_decoder_address(),
								true, ACC_TRG, pre_cnt );
		dcc_bits3.put_idle_pkt( BEST_IDLE );
		dcc_bits3.put_1s(1).done();
		
		rst_msg = "Send bad bit accessory reset packet.";
	}

	/*
	 *	Preset decoder to reverse with lamp on for LOCO,
	 *  presete function on for FUNC decoder, or
	 *  preset accessory decoder to the reverse position.
	 */
	dcc_bits.clr_in();
	dcc_bits.put_0s(1);	// Make sure a 0 is ahead of the preamble.
	if ( decoder_type == DEC_LOCO )
	{
		dcc_bits.put_cmd_pkt_28(	Args.get_decoder_address(),
									false, SP_TEST_MIN, pre_cnt );
	}
	else if ( decoder_type == DEC_FUNC )
	{
		dcc_bits.put_func_grp_pktx(	Args.get_decoder_address(),
									GRP_1, Args.get_func_mask(), pre_cnt );
	}
	else // Accessory decoder.
	{
		dcc_bits.put_acc_pkt(		Args.get_decoder_address(),
									true, ACC_PRE, pre_cnt );
	}
	dcc_bits.put_idle_pkt( BEST_IDLE );
	dcc_bits.put_1s(1).done();

	/*
	 *	Set up test packet.
	 */
	dcc_bits2.clr_in();
	dcc_bits2.put_0s(1);	// Make sure a 0 is ahead of the preamble.
	if ( decoder_type == DEC_LOCO )
	{
		dcc_bits2.put_cmd_pkt_28(	Args.get_decoder_address(),
									true, trig_cmd_speed, pre_cnt );
	}
	else if ( decoder_type == DEC_FUNC )
	{
		dcc_bits2.put_func_grp_pktx(	Args.get_decoder_address(),
									GRP_1, 0, pre_cnt );
	}
	else // Accessory decoder.
	{
		dcc_bits2.put_acc_pkt(		Args.get_decoder_address(),
									true, ACC_TRG, pre_cnt );
	}
	dcc_bits2.put_idle_pkt( BEST_IDLE );
	dcc_bits2.put_1s(1).done();

    /*	Loop through and flip each bit.  Note: skip the first PRE_SKIP_BITS
     *	Because the decoder may or may not accept packets with this length
     *	of preamble.
     */
	for ( i = first_bit; i <= last_bit; i++ )
	{
		pre_fail	=	false;
		trig_fail	=	false;
		rst_fail	=	false;

		if ( i == last_bit )
		{
			printf(
				"  %-18s Preamble %2d,    good pkt, Fails %4lu\r",
				tst_name, pre_cnt,
				t_stat.get_f_cnt() );
			dcc_bits2.clr_flip();
		}
		else
		{
			printf(
				"  %-18s Preamble %2d, test bit %2d, Fails %4lu\r",
				tst_name, pre_cnt,
				i + 1, t_stat.get_f_cnt() );

			// Flip the test bit, making sure to skip the initial 0.
			dcc_bits2.set_flip( i + 1 );
		}

		/*	Reset the decoder using the appropriate sequence by decoder type.
		 *	fsoc is used to reset the decoder unless dcc_bits3 has s special reset packet.
		 */
		Dcc_reg.clr_under();
		if ( dcc_bits3.get_byte_size() == 0 )	// Send fsoc sequence.
		{
			Dcc_reg.send_pkt( fsoc, "Reset decoder using fsoc sequence." );
			send_filler();
		}
		else									// Send special reset packets.
		{
			for ( j = 0; j < pkt_rep_cnt;  j++ )
			{
				Dcc_reg.send_pkt( dcc_bits3, rst_msg );
			}
		}

		tgen	=	Dcc_reg.get_gen();
		if ( (tgen & 0x01) != true )
		{
			rst_fail	=	true;
		}

		for ( j = 0; j < pkt_rep_cnt;  j++ )		// Send preset packet.
		{
			Dcc_reg.send_pkt( dcc_bits, "Send bad bit preset packet." );
		}

		tgen	=	Dcc_reg.get_gen();
		if ( (tgen & 0x01) != false )
		{
			pre_fail	=	true;
		}

		for ( j = 0; j < pkt_rep_cnt; j++ )		 	// Send test packet.
		{
            Dcc_reg.set_scope( true );
			Dcc_reg.send_pkt( dcc_bits2, "Send bad bit trigger packet." );
		}

		tgen	=	Dcc_reg.get_gen();
		if ( (tgen & 0x01) != (i == last_bit ? true : false) )
		{
			trig_fail	=	true;
		}

		t_stat.incr_t_cnt();

		if ( pre_fail || trig_fail || rst_fail )
		{
			t_stat.incr_f_cnt();
			tst_rslt	=	FAIL;
			if ( i == last_bit )
			{
				ERRPRINT( my_name, LOG_WARNING,
					"%-10s FAILED: good pkt",
					err_phrase( pre_fail, trig_fail, rst_fail ) );
			}
			else
			{
				ERRPRINT( my_name, LOG_WARNING,
					"%-10s FAILED: bit %2d",
					err_phrase( pre_fail, trig_fail, rst_fail ), i + 1 );
			}
		}

		if ( get_test_break() )
		{
			retval	=	FAIL;
			break;
		}
	}

	CLR_LINE;
	TO_STAT(	"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
		tst_name, t_stat.get_t_cnt(), t_stat.get_p_cnt(),
		t_stat.get_percent() );
	printf(		"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
		tst_name, t_stat.get_t_cnt(), t_stat.get_p_cnt(),
		t_stat.get_percent() );

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		decoder_margin_1()					  -	 Run margin 1T test.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		decoder_margin_1() runs the quick ames test for a variety of clock 1
 *		times in order to find the decoder clock one margins.  It uses a
 *		binary search algorithm to find the minimum and maximum margin in
 *		order to speed up the test.
 *		It returns FAIL if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::decoder_margin_1( void )
{
	const char		*tst_name = "1T Margin:"; // Test name.
	u_short			tclk0t;					// Clk 0T.
	u_short			tclk0h;					// Clk 0H.
	u_short			tclk1t;					// Clk 1T.
	Rslt_t			retval = OK;			// Return value.
	u_short			tl;						// Lower 1T bound.
	u_short			tu;						// Upper 1T bound.
	u_short			tclk1t_min = T_INV;		// Minimum 1T clock.
	u_short			tclk1t_max = T_INV;		// Maximum 1T clock.
	u_int			f_cnt;					// Fail count from quick_ames.
	u_int			margin_pre;				// Preamble count for margin test.

	tclk0t		=	DECODER_0T_NOM;
	tclk0h		=	DECODER_0H_NOM;
	margin_pre	=	BEST_PRE + Args.get_extra_preamble();

	CLR_LINE;
	STATPRINT(	"Margin test for 1T clock, %3u preambles",   margin_pre );
	printf(		"Margin test for 1T clock, %3u preambles\n", margin_pre );

	/*
	 *	Test for minimum 1T value.
	 */
	tl	=	TL1_MIN;
	tu	=	TU1_MIN;
	while (	tu - tl > 1	)
	{
		tclk1t	=	( tl + tu )/2;
		f_cnt	=	0;
		if ( quick_ames(	f_cnt, "Min 1T:",
							tclk0t, tclk0h, tclk1t, margin_pre, false )
							!= OK )
		{
			retval = FAIL;
			break;
		}
		if ( f_cnt == 0 )	// We're above the minimum.
		{
			tu			=	tclk1t;
			tclk1t_min	=	tclk1t;
		}
		else				// We're below the minimum.
		{
			tl	=	tclk1t;
		}
	}

	/*
	 *	Test for maximum 1T value.
	 */
	if ( retval == OK )
	{
		CLR_LINE;
		printf(		"  %-18s Minimum 1T %8s\n",
			tst_name,
			min_phrase( tclk1t_min, TL1_MIN ) );

		tl	=	TL1_MAX;
		tu	=	TU1_MAX;
		while (	tu - tl > 1	)
		{
			tclk1t	=	( tl + tu )/2;
			f_cnt	=	0;
			if ( quick_ames(	f_cnt, "Max 1T:",
								tclk0t, tclk0h, tclk1t, margin_pre, false )
								!= OK )
			{
				retval = FAIL;
				break;
			}
			if ( f_cnt == 0 )	// We're below the maximum.
			{
				tl			=	tclk1t;
				tclk1t_max	=	tclk1t;
			}
			else				// We're above the maximum.
			{
				tu	=	tclk1t;
			}
		}
	}

	CLR_LINE;
	TO_STAT(	"  %-18s Minimum 1T %8s, Maximum 1T %8s\n",
		tst_name,
		min_phrase( tclk1t_min, TL1_MIN ),
		max_phrase( tclk1t_max, TU1_MAX ) );
	printf(		"  %-18s Minimum 1T %8s, Maximum 1T %8s\n",
		tst_name,
		min_phrase( tclk1t_min, TL1_MIN ),
		max_phrase( tclk1t_max, TU1_MAX ) );

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		decoder_duty_1()					  -	 Run duty cycle 1 test.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		decoder_duty_1() runs the quick ames test for a variety of clock 1
 *		duty cycles in order to find the decoder clock margins.  It uses a
 *		binary search algorithm to find the minimum and maximum margin in
 *		order to speed up the test.
 *		It returns FAIL if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::decoder_duty_1( void )
{
	const char		*tst_name = "1H Duty:"; // Test name.
	u_short			tclk0t;					// Clk 0T.
	u_short			tclk1h;					// Clk 1H.
	u_short			tclk1t;					// Clk 1T.
	Rslt_t			retval = OK;			// Return value.
	u_short			tl;						// Lower 1T bound.
	u_short			tu;						// Upper 1T bound.
	u_short			tclk1h_min = T_INV;		// Minimum 1H clock.
	u_short			tclk1h_max = T_INV;		// Maximum 1H clock.
	u_int			f_cnt;					// Fail count from quick_ames.
	u_int			margin_pre;				// Preamble count for margin test.

	tclk0t		=	DECODER_0T_NOM;
	margin_pre	=	BEST_PRE + Args.get_extra_preamble();

	CLR_LINE;
	STATPRINT(	"Duty cycle test for 1 clock, %3u preambles",   margin_pre );
	printf(		"Duty cycle test for 1 clock, %3u preambles\n", margin_pre );

	/*
	 *	Test for minimum 1H value.
	 */
	tclk1t	=	DECODER_1T_NOM;
	tl		=	DUTY_T1H_MIN;
	tu		=	DECODER_1H_NOM;
	while (	tu - tl > 1	)
	{
		tclk1h	=	( tl + tu )/2;
		f_cnt	=	0;
		if ( quick_ames(	f_cnt, "Min 1 Duty:",
							tclk1t, tclk1h, tclk0t, margin_pre, true )
							!= OK )
		{
			retval = FAIL;
			break;
		}
		if ( f_cnt == 0 )	// We're above the minimum.
		{
			tu			=	tclk1h;
			tclk1h_min	=	tclk1h;
		}
		else				// We're below the minimum.
		{
			tl	=	tclk1h;
		}
	}

	/*
	 *	Test for maximum 1T value.
	 */
	if ( retval == OK )
	{
		CLR_LINE;
		printf(		"  %-18s Min 1H %8s from %3u nominal\n",
			tst_name,
			min_duty_phrase( DECODER_1H_NOM, tclk1h_min, DUTY_T1H_MIN ),
			DECODER_1H_NOM );

		tclk1t	=	DECODER_1T_NOM;
		tl		=	DECODER_1H_NOM;
		tu		=	DUTY_T1H_MAX;
		while (	tu - tl > 1	)
		{
			tclk1h	=	( tl + tu )/2;
			f_cnt	=	0;
			if ( quick_ames(	f_cnt, "Max 1 Duty:",
								tclk1t, tclk1h, tclk0t, margin_pre, true )
								!= OK )
			{
				retval = FAIL;
				break;
			}
			if ( f_cnt == 0 )	// We're below the maximum.
			{
				tl			=	tclk1h;
				tclk1h_max	=	tclk1h;
			}
			else				// We're above the maximum.
			{
				tu	=	tclk1h;
			}
		}
	}

	CLR_LINE;
	TO_STAT(	"  %-18s Min 1H %8s, Max 1H %8s from %3u nominal\n",
		tst_name,
		min_duty_phrase( DECODER_1H_NOM, tclk1h_min, DUTY_T1H_MIN ),
		max_duty_phrase( DECODER_1H_NOM, tclk1h_max, DUTY_T1H_MAX ),
		DECODER_1H_NOM );
	printf(		"  %-18s Min 1H %8s, Max 1H %8s from %3u nominal\n",
		tst_name,
		min_duty_phrase( DECODER_1H_NOM, tclk1h_min, DUTY_T1H_MIN ),
		max_duty_phrase( DECODER_1H_NOM, tclk1h_max, DUTY_T1H_MAX ),
		DECODER_1H_NOM );

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		quick_ames()						-	 Run quick ames test.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		quick_ames() runs the quick ames test as part of a clock margin test.
 *		It returns FAIL if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::quick_ames(
	u_int			&f_cnt,					// Decoder fail count.
	const char		*tst_name,				// Test name.
	u_short			tclk0t,					// Clk 0T.
	u_short			tclk0h,					// Clk 0H.
	u_short			tclk1t,				  	// Clk 1T.
	u_int			margin_pre,				// Preamble bits to use.
	bool			swap_0_1 )				// If true, swap meaning of 1 & 0.
{
	int				i;			  			// Index variable.
	int				j;						// Index variable.
	Rslt_t			retval = OK;			// Return value.
	BYTE			tgen;					// Generic input BYTE.
	bool			pre_fail;	  			// Preset failed.
	bool			trig_fail;	  			// Trigger failed.

	Dcc_reg.clr_err_cnt();					// Restart error counter.

	f_cnt		=	0;

	Dcc_reg.set_clk( tclk0t, tclk0h, tclk1t );

	/*
	 *	Check to see if the meaning of the 0 & 1 bit is swapped,
	 *	If so, treat 0 as 1 and 1 as 0.
	 */
	if ( swap_0_1 )
	{
		Dcc_reg.set_swap_0_1( true );
		calc_filler( tclk1t, tclk0t );
	}
	else
	{
		Dcc_reg.set_swap_0_1( false );
		calc_filler( tclk0t, tclk1t );
	}

	fsoc_send( "quick_ames\(\) initialization" );
	Dcc_reg.clr_under();
	send_filler();							// Wake up the booster.

	CLR_LINE;

	/*
	 *	Preset decoder to reverse with lamp on or
	 *  preset accessory decoder to the reverse position.
	 */
	dcc_bits.clr_in();
	dcc_bits.put_0s(1);	// Make sure a 0 is ahead of the preamble.
	if ( Args.get_decoder_type() == DEC_LOCO )
	{
		dcc_bits.put_cmd_pkt_28(	Args.get_decoder_address(),
									false, SP_TEST_MIN, margin_pre );
	}
	else if ( Args.get_decoder_type() == DEC_FUNC )
	{
		dcc_bits.put_func_grp_pktx(	Args.get_decoder_address(),
									GRP_1, Args.get_func_mask(), margin_pre );
	}
    else if ( Args.get_decoder_type() == DEC_SIG )
    {
    	dcc_bits.put_sig_pkt(		Args.get_decoder_address(),
        							Args.get_aspect_preset() );
    }
	else // Accessory decoder.
	{
		dcc_bits.put_acc_pkt(		Args.get_decoder_address(),
									true, ACC_PRE, margin_pre );
	}
	dcc_bits.put_idle_pkt( 1 );
	dcc_bits.put_1s(1).done();

	/*
	 *	Set decoder to trigger speed, forward, lamp off or
	 *  preset accessory decoder to the normal position.
	 */
	dcc_bits2.clr_in();
	dcc_bits2.put_0s(1);	// Make sure a 0 is ahead of the preamble.
	if ( Args.get_decoder_type() == DEC_LOCO )
	{
		dcc_bits2.put_cmd_pkt_28(	Args.get_decoder_address(),
									true, trig_cmd_speed, margin_pre );
	}
	else if ( Args.get_decoder_type() == DEC_FUNC )
	{
		dcc_bits2.put_func_grp_pktx(	Args.get_decoder_address(),
									GRP_1, 0, margin_pre );
	}
    else if ( Args.get_decoder_type() == DEC_SIG )
    {
    	dcc_bits2.put_sig_pkt(		Args.get_decoder_address(),
        							Args.get_aspect_trigger() );
    }
	else // Accessory decoder.
	{
		dcc_bits2.put_acc_pkt(		Args.get_decoder_address(),
									true, ACC_TRG, margin_pre );
	}
	dcc_bits2.put_idle_pkt( 1 );
	dcc_bits2.put_1s(1).done();

	/*
	 *	Note: This test is run 100 times to make sure there is less
	 *	Than a 1% chance of not seeing at least 1 failure assuming the
	 *	average pass rate is 95%.  100 tests gives a miss rate of .59%.
	 *	This test fails if any of the 100 tests fails making it more
	 *	stringent than the normal Ames test.
	 */
	for ( i = 0; i < 100; i++ )
	{
		pre_fail	=	false;
		trig_fail	=	false;

		if ( swap_0_1 )
		{
			printf(
				"  %-18s 0T %4u, 1H %4u, 1T %4u cycle %4d, Fails %4u\r",
				tst_name, tclk1t, tclk0h, tclk0t, i+1, f_cnt );
		}
		else
		{
			printf(
				"  %-18s 0T %4u, 0H %4u, 1T %4u cycle %4d, Fails %4u\r",
				tst_name, tclk0t, tclk0h, tclk1t, i+1, f_cnt );
		}

		fsoc_send( "quick_ames\(\) loop" );
		for ( j = 0; j < pkt_rep_cnt;  j++ )   	// Preset decoder reverse.
		{
			Dcc_reg.send_pkt( dcc_bits, "Send preset for quick test." );
		}

		tgen	=	Dcc_reg.get_gen();
		if ( (tgen & 0x01) != false )
		{
			pre_fail	=	true;
		}

		/*
		 * Send a scope trigger at start of trigger packet
		 * if the normal (early) scope trigger is desired.
		 */
		if ( Args.get_late_scope() != true )
		{
			Dcc_reg.set_scope( true );
		}

		Dcc_reg.send_pkt( dcc_bits2, "Send trigger for quick test." );

		/*
		 * The following code sends another idle filler, turns
		 * on the scope trigger, and then sends one second of idle filler.
		 * This results in scope trigger starting 2 idles after the trigger
		 * command.
		 */
		Dcc_reg.send_idle();					// Send one idle space.

		if ( Args.get_late_scope() == true )
		{
			Dcc_reg.set_scope( true );
		}

		send_filler();							// Send filler.

		tgen	=	Dcc_reg.get_gen();
		if ( (tgen & 0x01) != true )
		{
			trig_fail	=	true;
		}

		if ( pre_fail || trig_fail )
		{
			++f_cnt;
		}

		if ( get_test_break() )
		{
			retval	=	FAIL;
			break;
		}

		/*
		 *	Stop testing on first fail.
		 */
		if ( f_cnt > 0 )
		{
			break;
		}
	}

	Dcc_reg.set_swap_0_1( false );
	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		decoder_truncate()					  -	 Run truncated packet test.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		decoder_truncate() runs the truncated packet test.  It returns FAIL
 *		if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::decoder_truncate(
	Rslt_t			&tst_rslt )				// Decoder test result.
{
	const char		*my_name = "Trunc";
	int				i;			  			// Index variable.
	int				j;						// Index variable.
	int				bit_size;				// Present truncated bit size.
	BYTE			t_addr;					// Truncated address.
	BYTE			t_cmd;					// Truncated command.
	Rslt_t			retval = OK;			// Return value.
	BYTE			tgen;					// Generic input BYTE.
	bool			pre_fail;	  			// Preset failed.
	bool			trig_fail;	  			// Trigger failed.
	u_int			test_repeats;			// Number of times to repeat test.
	int				test_pre;				// Test preamble count.
	int				bit_num;				// Number of bits in packet.
    bool			mandatory_test;			// Test is mandatory.

	Dcc_reg.clr_err_cnt();					// Restart error counter.
	STATPRINT(	"Truncate clock\n- Clock 0T %4u, 0H %4u, 1T %4u",
		DECODER_0T_NOM, DECODER_0H_NOM, DECODER_1T_NOM );
	printf(		"Truncate clock\n- Clock 0T %4u, 0H %4u, 1T %4u\n",
		DECODER_0T_NOM, DECODER_0H_NOM, DECODER_1T_NOM );

	Dcc_reg.set_clk( DECODER_0T_NOM, DECODER_0H_NOM, DECODER_1T_NOM );
	calc_filler( DECODER_0T_NOM, DECODER_1T_NOM );

	fsoc_send( "decoder_truncate\(\) initialization" );
	Dcc_reg.clr_under();
	send_filler();							// Reset the decoder.

	test_repeats	=	Args.get_test_repeats();

	/*
	 *	Initialize the truncated command.  This will be a baseline
	 *	command to an address other than the test address.  It is setup
	 *	so that the checksum has as many 1's as possible.
	 */
	if (	( Args.get_decoder_type() != DEC_ACC )
		&&	( Args.get_decoder_address() == PRI_TRUNC_ADDR ) )
	{
		t_addr	=	PRI_TRUNC_ADDR ^ 0x1;	// Loco/Func address.
	}
	else
	{
		t_addr	=	PRI_TRUNC_ADDR;			// Accessory address.
	}

	t_cmd	=	~t_addr & 0x7f;

	CLR_LINE;

	/*
	 *	Preset loco decoder to reverse with lamp on or
	 *  preset accessory decoder to the reverse position.
	 */
	dcc_bits.clr_in();
	dcc_bits.put_0s(1);	// Make sure a 0 is ahead of the preamble.
	if ( Args.get_decoder_type() == DEC_LOCO )
	{
		dcc_bits.put_cmd_pkt_28(	Args.get_decoder_address(),
									false, SP_TEST_MIN, BEST_PRE );
	}
	else if ( Args.get_decoder_type() == DEC_FUNC )
	{
		dcc_bits.put_func_grp_pktx(	Args.get_decoder_address(),
									GRP_1, Args.get_func_mask(), BEST_PRE );
	}
	else // Accessory decoder.
	{
		dcc_bits.put_acc_pkt(		Args.get_decoder_address(),
									true, ACC_PRE, BEST_PRE );
	}
	dcc_bits.put_idle_pkt( BEST_IDLE );
	dcc_bits.put_1s(1).done();

	for ( test_pre = BEST_PRE; test_pre <= BEST_PRE + 4; test_pre++ )
	{
		bit_num	=	BASE_BITS + test_pre - BEST_PRE;

		for ( bit_size = bit_num; bit_size >= 0; bit_size-- )
		{
			if (	( ( bit_num - bit_size) % 9	== 0 )
				||	( bit_size <= test_pre ) )
			{
            	mandatory_test	=	true;
            }
            else
            {
            	mandatory_test	=	false;
            }

            if ( !mandatory_test && !Args.get_fragment_flag() )
            {
            	continue;
            }

			t_stat.reset();

			sprintf( m_tst_name, "Pre %2d Frag %2u:", test_pre, bit_size );

			/*
			*	Precede the trigger command with a truncated packet fragment.
			*/
			dcc_bits2.clr_in();
			dcc_bits2.put_0s(1);	// Make sure a 0 is ahead of the preamble.
			dcc_bits2.put_1s( test_pre ).put_0s( 1 ).put_byte( t_addr );
			dcc_bits2.put_0s( 1 ).put_byte( t_cmd ).put_0s( 1 ).put_check();
			dcc_bits2.truncate( bit_size + 1 );	// Add one to leave leading 0.

			/*
			*	Next, set decoder to trigger speed, forward, lamp off or
			*	set accessory decoder to the normal position.
			*/
			if ( Args.get_decoder_type() ==  DEC_LOCO )
			{
				dcc_bits2.put_cmd_pkt_28(	Args.get_decoder_address(),
											true, trig_cmd_speed, test_pre );
			}
			else if ( Args.get_decoder_type() == DEC_FUNC )
			{
				dcc_bits2.put_func_grp_pktx(	Args.get_decoder_address(),
											GRP_1, 0, test_pre );
			}
			else // Accessory decoder.
			{
				dcc_bits2.put_acc_pkt(		Args.get_decoder_address(),
											true, ACC_TRG, test_pre );
			}
			dcc_bits2.put_idle_pkt( BEST_IDLE );
			dcc_bits2.put_1s(1).done();

			for ( i = 0; i < test_repeats; i++ )
			{
				pre_fail	=	false;
				trig_fail	=	false;

				printf(
					"  %-18s Addr %5u, Speed %2d cycle %4d, Fails %4lu\r",
					m_tst_name, Args.get_decoder_address(), SP_TEST_MIN,
					i+1, t_stat.get_f_cnt() );

				fsoc_send( "decoder_truncate\(\) loop" );
				for ( j = 0; j < pkt_rep_cnt;  j++ )	// Preset reverse.
				{
					Dcc_reg.send_pkt( dcc_bits, "Send truncate test preset." );
				}

				tgen	=	Dcc_reg.get_gen();
				if ( (tgen & 0x01) != false )
				{
					pre_fail	=	true;
				}

				/*
				* Send a scope trigger at start of trigger packet
				* if the normal (early) scope trigger is desired.
				*/
				if ( Args.get_late_scope() != true )
				{
					Dcc_reg.set_scope( true );
				}

				Dcc_reg.send_pkt( dcc_bits2,		// Attempt emergency stop.
					"Send truncate trigger packet." );

				/*
				* The following code sends another idle filler, turns
				* on the scope trigger, and then sends one second of
				* idle filler.  This results in scope trigger starting 2
				* idles after the trigger command.
				*/
				Dcc_reg.send_idle();	 			// Send one idle space.

				if ( Args.get_late_scope() == true )
				{
					Dcc_reg.set_scope( true );
				}
			
				send_filler();			 			// Send filler.

				tgen	=	Dcc_reg.get_gen();
				if ( (tgen & 0x01) != true )
				{
					trig_fail	=	true;
				}

				t_stat.incr_t_cnt();

				if ( pre_fail || trig_fail )
				{
					t_stat.incr_f_cnt();
					tst_rslt	=	FAIL;
					ERRPRINT( my_name, LOG_WARNING,
					"%-10s FAILED: cycle %3d",
						err_phrase( pre_fail, trig_fail ), i+1 );
				}

				if ( get_test_break() )
				{
					retval	=	FAIL;
					break;
				}
			}

			CLR_LINE;
			if ( mandatory_test )
			{
				// Pass is mandatory, show *
				TO_STAT(	"  %-18s Tests %4lu; Passes %4lu, %4lu%% *\n",
					m_tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
					t_stat.get_percent() );
				printf(		"  %-18s Tests %4lu; Passes %4lu, %4lu%% *\n",
					m_tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
					t_stat.get_percent() );
			}
			else
			{
				// Pass is optional.
				TO_STAT(	"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
					m_tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
					t_stat.get_percent() );
				printf(		"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
					m_tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
					t_stat.get_percent() );
			}

			if ( retval == FAIL )
			{
				break;
			}
		}

		if ( retval == FAIL )
		{
			break;
		}
	}

    print_test_rslt( tst_rslt );

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		decoder_prior()					  	-	 Run prior packet test.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		decoder_prior() runs the prior packet test.  It returns FAIL
 *		if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::decoder_prior(
	Rslt_t			&tst_rslt )				// Decoder test result.
{
	const char		*my_name = "Prior";
	int				i;			  			// Index variable.
	int				j;						// Index variable.
	u_int			chk_msk;				// Checksum mask.
	BYTE			t_chk_byte;				// Check byte.
	int				one_cnt;				// Count of 1s to add after pkt.
	int				zero_cnt;				// Count of 0s to add after pkt.
	int				pre_bits;				// Count of preambles.
	BYTE			t_addr;					// Prior address.
	BYTE			t_cmd;					// Prior command.
	Rslt_t			retval = OK;			// Return value.
	BYTE			tgen;					// Generic input BYTE.
	bool			pre_fail;	  			// Preset failed.
	bool			trig_fail;	  			// Trigger failed.
	u_int			test_repeats;			// Number of times to repeat test.

	Dcc_reg.clr_err_cnt();					// Restart error counter.
	STATPRINT(	"Prior test clock\n- Clock 0T %4u, 0H %4u, 1T %4u",
		DECODER_0T_NOM, DECODER_0H_NOM, DECODER_1T_NOM );
	printf(		"Prior test clock\n- Clock 0T %4u, 0H %4u, 1T %4u\n",
		DECODER_0T_NOM, DECODER_0H_NOM, DECODER_1T_NOM );

	Dcc_reg.set_clk( DECODER_0T_NOM, DECODER_0H_NOM, DECODER_1T_NOM );
	calc_filler( DECODER_0T_NOM, DECODER_1T_NOM );

	fsoc_send( "decoder_prior\(\) initialization" );
	Dcc_reg.clr_under();
	send_filler();							// Reset the decoder.

	test_repeats	=	Args.get_test_repeats();

	/*
	 *	Initialize the prior address, making sure to stay away from
	 *	the decoder address and address 0.
	 */
	if (	( Args.get_decoder_type() != DEC_ACC )
		&&	( Args.get_decoder_address() == 0x1 ) )
	{
    	// Loco/Func address.
		t_addr	=	(Args.get_decoder_address() ^ 0x2) | 0x40;
	}
	else
	{
    	// Accessory address.
		t_addr	=	(Args.get_decoder_address() ^ 0x1) | 0x40;
	}

	CLR_LINE;

	/*
	 *	Preset loco decoder to reverse with lamp on or
	 *  preset accessory decoder to the reverse position.
	 */
	dcc_bits.clr_in();
	dcc_bits.put_0s(1);	// Make sure a 0 is ahead of the preamble.
	if ( Args.get_decoder_type() == DEC_LOCO )
	{
		dcc_bits.put_cmd_pkt_28(	Args.get_decoder_address(),
									false, SP_TEST_MIN, BEST_PRE );
	}
	else if ( Args.get_decoder_type() == DEC_FUNC )
	{
		dcc_bits.put_func_grp_pktx(	Args.get_decoder_address(),
									GRP_1, Args.get_func_mask(), BEST_PRE );
	}
	else // Accessory decoder.
	{
		dcc_bits.put_acc_pkt(		Args.get_decoder_address(),
									true, ACC_PRE, BEST_PRE );
	}
	dcc_bits.put_idle_pkt( BEST_IDLE );
	dcc_bits.put_1s(1).done();

	for ( chk_msk = 0; chk_msk <= 0xff; chk_msk = (chk_msk << 1) | 0x01 )
	{
	  for ( one_cnt = 0; one_cnt <= 1; one_cnt++ )
	  {
		for ( zero_cnt = 0; zero_cnt <= 1; zero_cnt++ )
		{
		  for ( pre_bits = PRE_BITS; pre_bits <= PRE_BITS + 10; pre_bits++ )
		  {
			t_stat.reset();

			/*
			 *	Skip the ambiguous case of a packet followed by a single
			 *	0 (no packet end 1 character) and less than PRE_BITS preamble
			 *	bits.  This can be interpreted as either an invalid 3 Byte
			 *	packet (no end bit) or an invalid 4 or more byte packet.
			 */
			if ( one_cnt == 0 && zero_cnt == 1 && pre_bits < (PRE_BITS + 8) )
			{
				continue;
			}

			/*
			 *	Clear bit 7 of address on next to last test
			 *	in order to put maximum 1s in checksum.
			 */
			if ( chk_msk == 0x7f )
			{
				t_addr &= ~0x40;
			}

			t_cmd = (t_addr ^ chk_msk) | 0x40;

			/*
			 *	Switch to idle command for last loop.
			 */
			if ( chk_msk == 0xff )
			{
				t_addr	=	0xff;
				t_cmd	=	0x00;
			}

			/*
			 *	Set up the prior and trigger packet.
			 */
			dcc_bits2.clr_in();
			dcc_bits2.put_0s(1);	// 0 is ahead of the preamble.
			dcc_bits2.put_1s( BEST_PRE ).put_0s( 1 ).put_byte( t_addr );
			dcc_bits2.put_0s( 1 ).put_byte( t_cmd ).put_0s( 1 );
			t_chk_byte = dcc_bits2.get_check();
			dcc_bits2.put_check();
			dcc_bits2.put_1s( one_cnt );
			dcc_bits2.put_0s( zero_cnt );
			/*
			 *	Next, set decoder to trigger speed, forward, lamp off or
			 *	set accessory decoder to the normal position.
			 */
			 if ( Args.get_decoder_type() ==  DEC_LOCO )
			 {
				dcc_bits2.put_cmd_pkt_28(	Args.get_decoder_address(),
											true, trig_cmd_speed, pre_bits );
			 }
             else if ( Args.get_decoder_type() == DEC_FUNC )
			 {
				dcc_bits2.put_func_grp_pktx(	Args.get_decoder_address(),
											GRP_1, 0, pre_bits );
			 }
			 else // Accessory decoder.
			 {
				dcc_bits2.put_acc_pkt(		Args.get_decoder_address(),
											true, ACC_TRG, pre_bits );
			 }
			 dcc_bits2.put_idle_pkt( BEST_IDLE );
			 dcc_bits2.put_1s(1).done();

			 sprintf( m_tst_name, "<%02x %02x %02x>%1s %1s<pre %2d>",
				t_addr, t_cmd, t_chk_byte,
				one_cnt ? "1" : " ",zero_cnt ? "0" : " ",
				pre_bits );

			 for ( i = 0; i < test_repeats; i++ )
			 {
				pre_fail	=	false;
				trig_fail	=	false;

				printf(
					"  %-18s Addr %5u, Cycle %4d, Fails %4lu\r",
					m_tst_name, Args.get_decoder_address(),
					i+1, t_stat.get_f_cnt() );

				fsoc_send( "decoder_prior\(\) loop" );
				for ( j = 0; j < pkt_rep_cnt;  j++ )	// Preset.
				{
					Dcc_reg.send_pkt( dcc_bits,
						"Send prior packet preset." );
				}

				tgen	=	Dcc_reg.get_gen();
				if ( (tgen & 0x01) != false )
				{
					pre_fail	=	true;
				}

				/*
				 * Send a scope trigger at start of trigger packet
				 * if the normal (early) scope trigger is desired.
				 */
				if ( Args.get_late_scope() != true )
				{
						Dcc_reg.set_scope( true );
				}

				Dcc_reg.send_pkt( dcc_bits2,			// Trigger.
					"Send prior packet trigger." );

				/*
				 * The following code sends another idle filler, turns
				 * on the scope trigger, and then sends one second of
				 * idle filler.  This results in scope trigger starting
				 * 2 idles after the trigger command.
				 */
				Dcc_reg.send_idle();		  	// Send one idle space.

				if ( Args.get_late_scope() == true )
				{
					Dcc_reg.set_scope( true );
				}

				send_filler();				  	// Send filler.

				tgen	=	Dcc_reg.get_gen();
				if ( (tgen & 0x01) != true )
				{
					trig_fail	=	true;
				}

				t_stat.incr_t_cnt();

				if ( pre_fail || trig_fail )
				{
					t_stat.incr_f_cnt();
					tst_rslt	=	FAIL;
					ERRPRINT( my_name, LOG_WARNING,
						"%-10s FAILED: cycle %3d",
					err_phrase( pre_fail, trig_fail ), i+1 );
				}

				if ( get_test_break() )
				{
					retval	=	FAIL;
					break;
				}
			}

			CLR_LINE;
			TO_STAT(	"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
				m_tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
				t_stat.get_percent() );
			printf(		"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
				m_tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
				t_stat.get_percent() );

			if ( retval == FAIL )
			{
				break;
			}
		  }
		  if ( retval == FAIL )
		  {
			  break;
		  }
		}
		if ( retval == FAIL )
		{
			break;
		}
	  }
	  if ( retval == FAIL )
	  {
		  break;
	  }
	}

	print_test_rslt( tst_rslt );

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		decoder_6_byte()  			  	-	 6 prior byte test.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		decoder_6_byte() sends 6 Bytes without a packet end bit
 *		immediately followed by a trigger packet.  A decoder should
 *		accept the trigger packet.
 *		It returns FAIL if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::decoder_6_byte(
	Rslt_t			&tst_rslt )				// Decoder test result.
{
	const char		*my_name = "6 Byte";
	int				i;			  			// Index variable.
	int				j;						// Index variable.
	BYTE			t_chk_byte;				// Check byte.
	int				cmd_id;					// Command id.
	int				one_cnt;				// Count of 1s to add after pkt.
	int				zero_cnt;				// Count of 0s to add after pkt.
	int				pre_bits;				// Count of preambles.
	BYTE			t_addr;					// Prior address.
	BYTE			t_cmd1;					// Prior command byte 1.
	BYTE			t_cmd2;					// Prior command byte 2.
	BYTE			t_cmd3;					// Prior command byte 3.
	BYTE			t_cmd4;					// Prior command byte 4.
	Rslt_t			retval = OK;			// Return value.
	BYTE			tgen;					// Generic input BYTE.
	bool			pre_fail;	  			// Preset failed.
	bool			trig_fail;	  			// Trigger failed.
	u_int			test_repeats;			// Number of times to repeat test.

	Dcc_reg.clr_err_cnt();					// Restart error counter.
	STATPRINT(	"6 byte prior test clock\n- Clock 0T %4u, 0H %4u, 1T %4u",
		DECODER_0T_NOM, DECODER_0H_NOM, DECODER_1T_NOM );
	printf(		"6 byte prior test clock\n- Clock 0T %4u, 0H %4u, 1T %4u\n",
		DECODER_0T_NOM, DECODER_0H_NOM, DECODER_1T_NOM );

	Dcc_reg.set_clk( DECODER_0T_NOM, DECODER_0H_NOM, DECODER_1T_NOM );
	calc_filler( DECODER_0T_NOM, DECODER_1T_NOM );

	fsoc_send( "decoder_6_byte\(\) initialization" );
	Dcc_reg.clr_under();
	send_filler();							// Reset the decoder.

	test_repeats	=	Args.get_test_repeats();

	/*
	 *	Initialize the prior address, making sure to stay away from
	 *	the decoder address and address 0.
	 */
	if (	( Args.get_decoder_type() != DEC_ACC )
		&&	( Args.get_decoder_address() == 0x1 ) )
	{
    	// Loco/Func address.
		t_addr	=	(Args.get_decoder_address() ^ 0x2) | 0x40;
	}
	else
	{
    	// Accessory address.
		t_addr	=	(Args.get_decoder_address() ^ 0x1) | 0x40;
	}

	CLR_LINE;

	/*
	 *	Preset loco decoder to reverse with lamp on or
	 *  preset accessory decoder to the reverse position.
	 */
	dcc_bits.clr_in();
	dcc_bits.put_0s(1);	// Make sure a 0 is ahead of the preamble.
	if ( Args.get_decoder_type() == DEC_LOCO )
	{
		dcc_bits.put_cmd_pkt_28(	Args.get_decoder_address(),
									false, SP_TEST_MIN, BEST_PRE );
	}
	else if ( Args.get_decoder_type() == DEC_FUNC )
	{
		dcc_bits.put_func_grp_pktx(	Args.get_decoder_address(),
									GRP_1, Args.get_func_mask(), BEST_PRE );
	}
	else // Accessory decoder.
	{
		dcc_bits.put_acc_pkt(		Args.get_decoder_address(),
									true, ACC_PRE, BEST_PRE );
	}
	dcc_bits.put_idle_pkt( BEST_IDLE );
	dcc_bits.put_1s(1).done();

	for ( cmd_id = 0; cmd_id <= 2; cmd_id++ )
	{

	  switch ( cmd_id )
	  {
	  case 0:
		t_cmd1 = t_cmd2 = t_cmd3 = t_cmd4 =	0x61;	// Emergency stop.
		break;

	  case 1:
		t_cmd1 = ~t_addr;
		t_cmd2 = t_cmd3 = t_cmd4 = 0xFF;			// All 1s.
		break;

	  case 2:
		t_cmd1 = ~t_addr;
		t_cmd2 = t_cmd3 = t_cmd4 = 0x55;			// Alternating 1s & 0s.
		break;
	  }

	  for ( one_cnt = 0; one_cnt <= 1; one_cnt++ )
	  {
		for ( zero_cnt = 0; zero_cnt <= 1; zero_cnt++ )
		{
		  for ( pre_bits = PRE_BITS; pre_bits <= PRE_BITS + 9; pre_bits++ )
		  {
			t_stat.reset();

			/*
			 *	Set up the prior and trigger packet.
			 */
			dcc_bits2.clr_in();
			dcc_bits2.put_0s(1);	// 0 is ahead of the preamble.
			dcc_bits2.put_1s( pre_bits ).put_0s( 1 ).put_byte( t_addr );
			dcc_bits2.put_0s( 1 ).put_byte( t_cmd1 );
			dcc_bits2.put_0s( 1 ).put_byte( t_cmd2 );
			dcc_bits2.put_0s( 1 ).put_byte( t_cmd3 );
			dcc_bits2.put_0s( 1 ).put_byte( t_cmd4 );
			dcc_bits2.put_0s( 1 );
			t_chk_byte = dcc_bits2.get_check();
			dcc_bits2.put_check();
			dcc_bits2.put_1s( one_cnt );
			dcc_bits2.put_0s( zero_cnt );

			/*
			 *	Next, set decoder to trigger speed, forward, lamp off or
			 *	set accessory decoder to the normal position.
			 */
			 if ( Args.get_decoder_type() ==  DEC_LOCO )
			 {
				dcc_bits2.put_cmd_pkt_28(	Args.get_decoder_address(),
											true, trig_cmd_speed, pre_bits );
			 }
             else if ( Args.get_decoder_type() == DEC_FUNC )
			 {
				dcc_bits2.put_func_grp_pktx(	Args.get_decoder_address(),
											GRP_1, 0, pre_bits );
			 }
			 else // Accessory decoder.
			 {
				dcc_bits2.put_acc_pkt(		Args.get_decoder_address(),
											true, ACC_TRG, pre_bits );
			 }
			 dcc_bits2.put_idle_pkt( BEST_IDLE );
			 dcc_bits2.put_1s(1).done();

			 sprintf( m_tst_name,
				"<%02x %02x %02x %02x %02x %02x>%1s %1s<pre %2d>",
				t_addr, t_cmd1, t_cmd2, t_cmd3, t_cmd4, t_chk_byte,
				one_cnt ? "1" : " ",zero_cnt ? "0" : " ",
				pre_bits );

			 for ( i = 0; i < test_repeats; i++ )
			 {
				pre_fail	=	false;
				trig_fail	=	false;

				printf(
					"  %-18s Addr %5u, Cycle %4d, Fails %4lu\r",
					m_tst_name, Args.get_decoder_address(),
					i+1, t_stat.get_f_cnt() );

				fsoc_send( "decoder_6_byte\(\) loop" );
				for ( j = 0; j < pkt_rep_cnt;  j++ )	// Preset.
				{
					Dcc_reg.send_pkt( dcc_bits,
						"Send 6 byte prior packet preset." );
				}

				tgen	=	Dcc_reg.get_gen();
				if ( (tgen & 0x01) != false )
				{
					pre_fail	=	true;
				}

				/*
				 * Send a scope trigger at start of trigger packet
				 * if the normal (early) scope trigger is desired.
				 */
				if ( Args.get_late_scope() != true )
				{
						Dcc_reg.set_scope( true );
				}

				Dcc_reg.send_pkt( dcc_bits2,			// Trigger.
					"Send 6 byte prior packet trigger." );

				/*
				 * The following code sends another idle filler, turns
				 * on the scope trigger, and then sends one second of
				 * idle filler.  This results in scope trigger starting
				 * 2 idles after the trigger command.
				 */
				Dcc_reg.send_idle();		  	// Send one idle space.

				if ( Args.get_late_scope() == true )
				{
					Dcc_reg.set_scope( true );
				}

				send_filler();				  	// Send filler.

				tgen	=	Dcc_reg.get_gen();
				if ( (tgen & 0x01) != true )
				{
					trig_fail	=	true;
				}

				t_stat.incr_t_cnt();

				if ( pre_fail || trig_fail )
				{
					t_stat.incr_f_cnt();
					tst_rslt	=	FAIL;
					ERRPRINT( my_name, LOG_WARNING,
						"%-10s FAILED: cycle %3d",
					err_phrase( pre_fail, trig_fail ), i+1 );
				}

				if ( get_test_break() )
				{
					retval	=	FAIL;
					break;
				}
			}

			CLR_LINE;
			TO_STAT(	"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
				m_tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
				t_stat.get_percent() );
			printf(		"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
				m_tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
				t_stat.get_percent() );

			if ( retval == FAIL )
			{
				break;
			}
		  }
		  if ( retval == FAIL )
		  {
			  break;
		  }
		}
		if ( retval == FAIL )
		{
			break;
		}
	  }
	  if ( retval == FAIL )
	  {
		  break;
	  }
	}

    print_test_rslt( tst_rslt );

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		decoder_ambig1()		  			-	 Run 1 ambiguous bit test.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		decoder_ambig1() runs the single ambiguous bit test.  It returns
 *		FAIL if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::decoder_ambig1(
	Rslt_t			&tst_rslt )				// Decoder test result.
{
	const char		*my_name = "Ambig 1";
	int				i;			  			// Index variable.
	int				j;						// Index variable.
	Rslt_t			retval = OK;			// Return value.
	BYTE			tgen;					// Generic input BYTE.
	bool			pre_fail;	  			// Preset failed.
	bool			trig_fail;	  			// Trigger failed.
    int				ambig1_bit;				// Ambiguous bit number.
    int				fill_bits;				// Number of fill bits.
    int				pre_bytes;				// Bytes prior to ambiguous byte.
	BYTE			pbyte;					// Present byte to send.
	u_int			test_repeats;			// Number of times to repeat test.
    u_short	 		ambig1_0t;				// Ambiguous bit 0T time.
    u_short			ambig1_0h;				// Ambiguous bit 0H time.
    int				test_cycle;				// Ambiguous test cycle.
	u_int			pre_cnt;				// Preamble count for test.
	u_short			t_addr;					// Prior address.

	pre_cnt		=	BEST_PRE + Args.get_extra_preamble();

	STATPRINT(	"Ambig 1 test clock\n- Clock 0T %4u, 0H %4u, 1T %4u",
		DECODER_0T_NOM, DECODER_0H_NOM, DECODER_1T_NOM );
	printf(		"Ambig 1 test clock\n- Clock 0T %4u, 0H %4u, 1T %4u\n",
		DECODER_0T_NOM, DECODER_0H_NOM, DECODER_1T_NOM );
    TO_STAT(	"         0 Times < 0T   0H>\n" );
    printf(		"         0 Times < 0T   0H>\n" );

	Dcc_reg.set_clk( DECODER_0T_NOM, DECODER_0H_NOM, DECODER_1T_NOM );
	calc_filler( DECODER_0T_NOM, DECODER_1T_NOM );

	fsoc_send( "decoder_ambig1\(\) initialization" );
	Dcc_reg.clr_under();
	send_filler();							// Reset the decoder.

	test_repeats	=	Args.get_test_repeats();

	Dcc_reg.clr_err_cnt();					// Restart error counter.

	CLR_LINE;

    /*	Initially set the prior address
     *	to be the same as the trigger address.
     */
    t_addr	=	Args.get_decoder_address();

    /*
     *	Modify the prior address so it doesn't match the trigger address
     *	if the get_ambig_addr_same() flag is not true.
     *	Make sure the modified address is not 0.
     */
	if ( Args.get_ambig_addr_same() != true )
    {
		if ( t_addr == 0x0001 )
		{
			t_addr	=	0x0002;
		}
		else
		{
			t_addr	=	t_addr ^ 0x0001;
		}
    }

	/*
	 *	Preset loco decoder to reverse with lamp on or
	 *  preset accessory decoder to the reverse position.
	 */
	dcc_bits.clr_in();
	dcc_bits.put_0s(1);	// Make sure a 0 is ahead of the preamble.
	if ( Args.get_decoder_type() == DEC_LOCO )
	{
		dcc_bits.put_cmd_pkt_28(	Args.get_decoder_address(),
									false, SP_TEST_MIN, pre_cnt );
	}
	else if ( Args.get_decoder_type() == DEC_FUNC )
	{
		dcc_bits.put_func_grp_pktx(	Args.get_decoder_address(),
									GRP_1, Args.get_func_mask(), pre_cnt );
	}
	else // Accessory decoder.
	{
		dcc_bits.put_acc_pkt(		Args.get_decoder_address(),
									true, ACC_PRE, pre_cnt );
	}
	dcc_bits.put_idle_pkt( BEST_IDLE );
	dcc_bits.put_1s(1).done();

    for ( test_cycle = 0; test_cycle < ambig1_tbl_size; test_cycle++ )
    {
		for (	ambig1_bit	=	BEST_PRE + 1;
    			ambig1_bit	<=	BASE_BITS + 2;
            	ambig1_bit	=	ambig1_bit < (BASE_BITS + 1 - BITS_IN_BYTE -1)
                				? ambig1_bit + BITS_IN_BYTE + 1
                            	: ambig1_bit + 1 )
    	{

    		t_stat.reset();	// Reset statistics

        	/*
         	 *	Set up ambiguous bit times.
         	*/
        	ambig1_0t	=	ambig1_tbl[ test_cycle ].ambig1_0t;
        	ambig1_0h	=	ambig1_tbl[ test_cycle]. ambig1_0h;

    		/*
     		 *	Set up a preset packet aligned with test ambiguous 0 bit
        	 *	just after BYTE boundary.
     		 */
        	fill_bits	=	BITS_IN_BYTE - ((ambig1_bit - 1) % BITS_IN_BYTE);
        	if ( fill_bits == BITS_IN_BYTE )
        	{
        		fill_bits = 0;
        	}

        	pre_bytes	=	(ambig1_bit - 1 + fill_bits) / BITS_IN_BYTE;

        	sprintf( m_tst_name,
        		"Ambig 1 bit %2d <%3d, %3d> PAddr %5u:",
        	    ambig1_bit, ambig1_0t, ambig1_0h, t_addr );

			dcc_bits2.clr_in();

        	if ( fill_bits > 0 )
        	{
        		// Align pre_bytes to end on BYTE boundary.
				dcc_bits2.put_0s( fill_bits );
        	}

			if ( Args.get_decoder_type() == DEC_LOCO )
			{
				dcc_bits2.put_cmd_pkt_28(	t_addr,
											false, SP_TEST_MIN, BEST_PRE );
			}
        	else if ( Args.get_decoder_type() == DEC_FUNC )
            {
            	dcc_bits2.put_func_grp_pktx(	Args.get_decoder_address(),
                                    GRP_1, Args.get_func_mask(), BEST_PRE );
        	}
			else // Accessory decoder.
			{
				dcc_bits2.put_acc_pkt(		t_addr,
											true, ACC_PRE, BEST_PRE );
			}

			if ( ambig1_bit == (BASE_BITS + 1) )		// Packet end bit.
        	{
        		/*
				 *	Add a zero immediately after the checksum byte if we
           		 *	are pointed at the stop bit.  This zero will be replaced
				 *	with an ambiguous bit.  Follow this bit with 9 1 bits
                 *	so the decoder will not think this packet is a 4 or
                 *	longer Byte packet.
         		 */
        		dcc_bits2.put_0s( 1 );
                dcc_bits2.put_1s( 9 );
        	}
        	else if ( ambig1_bit == (BASE_BITS + 2) )
        	{
        		/*
         		 *	Add a normal stop bit followed by a 0 bit if we
            	 *	are pointed at the bit after the stop bit.  This
            	 *	zero will be replaced with an ambiguous bit.
         		 */
        		dcc_bits2.put_1s( 1 );
            	dcc_bits2.put_0s( 1 );
        	}

    		/*
     	 	 *	Set up a normal trigger packet.
     	 	 */
    		if ( Args.get_decoder_type() ==  DEC_LOCO )
    		{
    			dcc_bits2.put_cmd_pkt_28(	Args.get_decoder_address(),
        									true, trig_cmd_speed, pre_cnt );
    		}
            else if ( Args.get_decoder_type() == DEC_FUNC )
            {
            	dcc_bits2.put_func_grp_pktx(	Args.get_decoder_address(),
											GRP_1, 0, pre_cnt );
            }
    		else // Accessory decoder.
    		{
    			dcc_bits2.put_acc_pkt(		Args.get_decoder_address(),
        									true, ACC_TRG, pre_cnt );
    		}
    		dcc_bits2.put_idle_pkt( 1 );
    		dcc_bits2.put_1s(1).done();

            // Force the selected ambiguous bit to 0
            // Subtract 1 from ambig1_bit to adjust to 0 based array index.
            dcc_bits2.clr_bit( (fill_bits + ambig1_bit) - 1 );

			for ( i = 0; i < test_repeats; i++ )
			{
    			pre_fail	=	false;
        		trig_fail	=	false;

        		printf(
        			"  %-18s Pre %2d, Cycle %3d, Fails %4lu\r",
            		m_tst_name, pre_cnt,
            		i+1, t_stat.get_f_cnt() );

				fsoc_send( "decoder_ambig1\(\) loop" );

        		for ( j = 0; j < pkt_rep_cnt;  j++ )	// Preset.
        		{
        			Dcc_reg.send_pkt( dcc_bits,
            			"Send ambig 1 packet preset." );
        		}

        		tgen	=	Dcc_reg.get_gen();
        		if ( (tgen & 0x01) != false )
        		{
        			pre_fail	=	true;
        		}

            	//	Reset dcc_bits2 to the beginning of the packet.
            	dcc_bits2.rst_out();

        		//	Send pre_bytes just before ambiguous bit.
            	for ( j = 0; j < pre_bytes; j++ )
            	{
            		dcc_bits2.get_byte( pbyte );
                	Dcc_reg.send_bytes( 1, pbyte,
        				"Send preset BYTE just before ambiguous bit." );
            	}

				/*
		 	 	 * Send a scope trigger at start of trigger packet
                 * This test requires an early scope trigger.
		 	 	 */
                 Dcc_reg.set_scope( true );

				dcc_bits2.get_byte( pbyte );	// Get first BYTE of packet.
				Dcc_reg.send_1_ambig_bit(	ambig1_0t,
                							ambig1_0h,
                                            pbyte,
                                            "Send byte with 1 ambiguous bit." );

				// Send remaining part of preset packet & the trigger packet.
				while ( dcc_bits2.get_byte( pbyte ) == OK )
				{
					Dcc_reg.send_bytes(	1, pbyte,
										"Send remaining ambig 1 packet." );
				}

				send_filler();		  			// Send filler.

        		tgen	=	Dcc_reg.get_gen();
        		if ( (tgen & 0x01) != true )
        		{
        			trig_fail	=	true;
        		}

        		t_stat.incr_t_cnt();

        		if ( pre_fail || trig_fail )
        		{
        			t_stat.incr_f_cnt();
            		tst_rslt	=	FAIL;
            		ERRPRINT( my_name, LOG_WARNING,
            			"%-10s FAILED: cycle %3d",
             		   	err_phrase( pre_fail, trig_fail ), i+1 );
            	}

        		if ( get_test_break() )
    			{
         			retval	=	FAIL;
            		break;
        		}
			}

    		CLR_LINE;
    		TO_STAT(	"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
						m_tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
						t_stat.get_percent() );
    		printf(		"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
						m_tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
						t_stat.get_percent() );

    		if ( retval == FAIL )
    		{
    			break;
    		}
    	}
        
    	if ( retval == FAIL )
    	{
    		break;
    	}
    }

    print_test_rslt( tst_rslt );

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		decoder_ambig2()		  			-	 Run ambiguous 2 test.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Test interrupted.
 *
 *	DESCRIPTION
 *
 *		decoder_ambiguous2() runs the ambiguous 2 test.  It returns FAIL
 *		if the user types the test break key sequence.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::decoder_ambig2(
	Rslt_t			&tst_rslt )				// Decoder test result.
{
	const char		*my_name = "Ambig 2";
	int				clk_idx;				// Clock test index value.
	u_short			tclk0t;					// Temp tclk0t.
	u_short			tclk0h;				 	// Temp tclk0h.
	u_short			tclk1t;				 	// Temp tclk1t.
	int				i;			  			// Index variable.
	int				j;						// Index variable.
	Rslt_t			retval = OK;			// Return value.
	BYTE			tgen;					// Generic input BYTE.
	bool			pre_fail;	  			// Preset failed.
	bool			trig_fail;	  			// Trigger failed.
	BYTE			pbyte;					// Present byte to send.
	u_int			pre_cnt;				// Preamble count for test.
	u_short			t_addr;					// Prior address.
    int				test_cycle;				// Ambiguous 2 test cycle.
	u_int			test_repeats;			// Number of times to repeat test.
    u_short	 		ambig2_0t1;				// Ambiguous bit 0T1 time.
    u_short			ambig2_0h1;				// Ambiguous bit 0H1 time.
    u_short	 		ambig2_0t2;				// Ambiguous bit 0T2 time.
    u_short			ambig2_0h2;				// Ambiguous bit 0H2 time.

	pre_cnt		=	BEST_PRE + Args.get_extra_preamble();

	test_repeats	=	Args.get_test_repeats();

	Dcc_reg.clr_err_cnt();					// Restart error counter.

	CLR_LINE;

    /*	Initially set the prior address
     *	to be the same as the trigger address.
     */
    t_addr	=	Args.get_decoder_address();

    /*
     *	Modify the prior address so it doesn't match the trigger address
     *	if the get_ambig_addr_same() flag is not true.
     *	Make sure the modified address is not 0.
     */
	if ( Args.get_ambig_addr_same() != true )
    {
		if ( t_addr == 0x0001 )
		{
			t_addr	=	0x0002;
		}
		else
		{
			t_addr	=	t_addr ^ 0x0001;
		}
    }

	/*
	 *	Preset loco decoder to reverse with lamp on or
	 *  preset accessory decoder to the reverse position.
	 */
	dcc_bits.clr_in();
	dcc_bits.put_0s(1);	// Make sure a 0 is ahead of the preamble.
	if ( Args.get_decoder_type() == DEC_LOCO )
	{
		dcc_bits.put_cmd_pkt_28(	Args.get_decoder_address(),
									false, SP_TEST_MIN, pre_cnt );
	}
	else if ( Args.get_decoder_type() == DEC_FUNC )
	{
		dcc_bits.put_func_grp_pktx(	Args.get_decoder_address(),
									GRP_1, Args.get_func_mask(), pre_cnt );
	}
	else // Accessory decoder.
	{
		dcc_bits.put_acc_pkt(		Args.get_decoder_address(),
									true, ACC_PRE, pre_cnt );
	}
	dcc_bits.put_idle_pkt( BEST_IDLE );
	dcc_bits.put_1s(1).done();

    /*
     *	Set up a normal preset packet aligned so it ends at a BYTE boundary.
     */
	dcc_bits2.clr_in();
	dcc_bits2.put_0s( AMBIG1_BITS );	// Align preset to end on BYTE boundary.
	if ( Args.get_decoder_type() == DEC_LOCO )
	{
		dcc_bits2.put_cmd_pkt_28(	t_addr,
									false, SP_TEST_MIN, BEST_PRE );
	}
	else if ( Args.get_decoder_type() == DEC_FUNC )
	{
		dcc_bits2.put_func_grp_pktx(	Args.get_decoder_address(),
									GRP_1, Args.get_func_mask(), BEST_PRE );
	}
	else // Accessory decoder.
	{
		dcc_bits2.put_acc_pkt(		t_addr,
									true, ACC_PRE, BEST_PRE );
	}
    dcc_bits2.put_1s( 1 );				// Put the packet end bit.
    dcc_bits2.done();

    /*
     *	Set up a trigger packet with 2 ambiguous 0s at the front of it.
     */
	dcc_bits3.clr_in();
    dcc_bits3.put_0s( 2 );				// Put 2 zeroes used for feedback bits.
	if ( Args.get_decoder_type() ==  DEC_LOCO )
	{
		dcc_bits3.put_cmd_pkt_28(	Args.get_decoder_address(),
									true, trig_cmd_speed, pre_cnt );
	}
    else if ( Args.get_decoder_type() == DEC_FUNC )
    {
    	dcc_bits3.put_func_grp_pktx(	Args.get_decoder_address(),
        							GRP_1, 0, pre_cnt );
    }
	else // Accessory decoder.
	{
		dcc_bits3.put_acc_pkt(		Args.get_decoder_address(),
									true, ACC_TRG, pre_cnt );
	}
	dcc_bits3.put_idle_pkt( 1 );
	dcc_bits3.put_1s(1).done();

	/*
	 *	Run test series for each ambig2_dclk_tbl[] entry.
	 */
	for (	clk_idx = 0; clk_idx < ambig2_dclk_size; clk_idx++ )
	{
    	tclk0t		=	ambig2_dclk_tbl[clk_idx].clk0t;
        tclk0h		=	ambig2_dclk_tbl[clk_idx].clk0h;
        tclk1t		=	ambig2_dclk_tbl[clk_idx].clk1t;

        STATPRINT(	"Ambig2 clock <%s>\n- Clock 0T %4u, 0H %4u, 1T %4u",
        	ambig2_dclk_tbl[clk_idx].msg, tclk0t, tclk0h, tclk1t );
        printf(		"Ambig2 clock <%s>\n- Clock 0T %4u, 0H %4u, 1T %4u\n",
			ambig2_dclk_tbl[clk_idx].msg, tclk0t, tclk0h, tclk1t );

    	TO_STAT(	"  0 Times <0T1  0H1  0T2  0H2>\n" );
    	printf(		"  0 Times <0T1  0H1  0T2  0H2>\n" );

		Dcc_reg.set_clk( tclk0t, tclk0h, tclk1t );
		calc_filler( tclk0t, tclk1t );

		fsoc_send( "decoder_ambig2\(\) outer clock loop" );
		Dcc_reg.clr_under();
		send_filler();							// Reset the decoder.

    	for ( test_cycle = 0; test_cycle < ambig2_tbl_size; test_cycle++ )
    	{
    		t_stat.reset();	// Reset statistics

        	/*
         	 *	Set up ambiguous bit times.
         	 */
         	ambig2_0t1	=	ambig2_tbl[ test_cycle ].ambig2_0t1;
         	ambig2_0h1	=	ambig2_tbl[ test_cycle]. ambig2_0h1;
         	ambig2_0t2	=	ambig2_tbl[ test_cycle ].ambig2_0t2;
         	ambig2_0h2	=	ambig2_tbl[ test_cycle]. ambig2_0h2;

         	sprintf( m_tst_name,
         		"Ambig 2 <%3d, %3d, %3d, %3d> PAddr %5u:",
         		ambig2_0t1, ambig2_0h1, ambig2_0t2, ambig2_0h2, t_addr );

			for ( i = 0; i < test_repeats; i++ )
			{
    			pre_fail	=	false;
        		trig_fail	=	false;

        		printf(
        			"  %-18s Pre %2d, Cycle %3d, Fails %4lu\r",
            		m_tst_name, pre_cnt,
            		i+1, t_stat.get_f_cnt() );

				fsoc_send( "decoder_ambig2\(\) inner test loop" );
            
        		for ( j = 0; j < pkt_rep_cnt;  j++ )	// Preset.
        		{
        			Dcc_reg.send_pkt( dcc_bits,
            			"Send ambig 2 packet preset." );
        		}

        		tgen	=	Dcc_reg.get_gen();
        		if ( (tgen & 0x01) != false )
        		{
        			pre_fail	=	true;
        		}

        		//	Send preset just before feedback bits.
        		Dcc_reg.send_pkt(	dcc_bits2,
        			"Send preset just before ambig 2 bits." );

				/*
             	 * Send a scope trigger at start of trigger packet
             	 * This test requires an early scope trigger.
             	 */
            	Dcc_reg.set_scope( true );

        		dcc_bits3.rst_out();
				dcc_bits3.get_byte( pbyte );	// Get first BYTE of packet.
				Dcc_reg.send_2_ambig_bits(	ambig2_0t1,
        								ambig2_0h1,
                                    	ambig2_0t2,
                                    	ambig2_0h2,
                                    	pbyte,
                                    	"Send byte with 2 ambiguous bits." );

				// Send remaining part of packet.
				while ( dcc_bits3.get_byte( pbyte ) == OK )
				{
					Dcc_reg.send_bytes(	1, pbyte,
									"Send remaining ambig 2 trigger packet." );
				}

				send_filler();					// Send filler.
        
        		tgen	=	Dcc_reg.get_gen();
        		if ( (tgen & 0x01) != true )
        		{
        			trig_fail	=	true;
        		}

        		t_stat.incr_t_cnt();

        		if ( pre_fail || trig_fail )
        		{
        			t_stat.incr_f_cnt();
            		tst_rslt	=	FAIL;
            		ERRPRINT( my_name, LOG_WARNING,
            			"%-10s FAILED: cycle %3d",
                		err_phrase( pre_fail, trig_fail ), i+1 );
        		}

        		if ( get_test_break() )
    			{
         			retval	=	FAIL;
            		break;
        		}
			}

    		CLR_LINE;
    		TO_STAT(	"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
						m_tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
						t_stat.get_percent() );
    		printf(		"  %-18s Tests %4lu; Passes %4lu, %4lu%%\n",
						m_tst_name,t_stat.get_t_cnt(), t_stat.get_p_cnt(),
						t_stat.get_percent() );

    		if ( retval == FAIL )
    		{
    			break;
    		}
    	}

    	if ( retval == FAIL )
    	{
    		break;
    	}

    	print_test_rslt( tst_rslt );
    }

	return ( retval );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		calc_filler()				   	-	 Calculate number of filler idles.
 *
 *	RETURN VALUE
 *
 *		None.
 *
 *	DESCRIPTION
 *
 *		calc_filler() calculates the number of idle packets to send between
 *		tests and stores it in 'filler_idles'.  It next calculates the number
 *		of times to repeat the setup commands which are assumed to be an idle
 *		followed by a command.  It stores the results in 'pkt_rep_cnt'.  Note
 *		that 'pkt_rep_cnt' is approximate since the actual 1 and 0 bits in the
 *		command changes for each test.
 */
/*--------------------------------------------------------------------------*/

void
Dec_tst::calc_filler(
	u_short			clk0t,					// Clock 0T value.
	u_short			clk1t )					// Clock 1T value.
{
	/*
	 *	Calculate number of idles to fill 'fill_usec' amount of time.
	 */
	filler_idles	=	Send_reg::fill_idles(fill_usec,clk0t,clk1t);

	/*
	 *	Calculate approximate number of command/idle pairs to
	 *	fill 'fill_usec' amount of time.
	 */
	pkt_rep_cnt		=	Send_reg::fill_cmds_1(fill_usec,clk0t,clk1t);

	if ( Dcc_reg.get_log_pkts() )
	{
#ifndef PKT_REP_DEBUG
		TO_LOG(
        "!Dec_tst::calc_filler() Idle cnt %3u, Preset cnt %3u.\n",
        	filler_idles, pkt_rep_cnt );
#else
		TO_LOG(
		"!Dec_tst::calc_filler() fill_usec %10lu, clk0t %5u, clk1t %5u.\n",
			fill_usec, clk0t, clk1t );
		TO_LOG(
        "!Dec_tst::calc_filler() Fill_Idle %3u, Cmd_one %3u, Cmd_two %3u.\n",
			filler_idles, pkt_rep_cnt,
        	Send_reg::fill_cmds_2(fill_usec,clk0t,clk1t) );
#endif
	}
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		send_filler()				   	-	 Send filler idles.
 *
 *	RETURN VALUE
 *
 *		OK		-	Normal return.
 *		FAIL	-	Send failed.
 *
 *	DESCRIPTION
 *
 *		send_filler() sends 'filler_idles' number of idle packets using the
 *		send_idle() method.
 */
/*--------------------------------------------------------------------------*/

Rslt_t
Dec_tst::send_filler( void )
{
	u_short			i;						// Index variable.

	if ( Dcc_reg.get_log_pkts() )
	{
		TO_LOG( "!Dec_tst::send_filler() Sending %d idles.\n",
		filler_idles );
	}

	for ( i = 0; i < filler_idles; i++ )
	{
		if ( Dcc_reg.send_idle() != OK )
		{
			return ( FAIL );
		}
	}

	return ( OK );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		err_phrase()		   	   	-	 Return preset, trigger error msg.
 *
 *	RETURN VALUE
 *
 *		msg		-	Error phase message.
 *
 *	DESCRIPTION
 *
 *		err_phrase() uses 'pre_fail', 'trig_fail', and the optional 'rst_fail'
 *		to return the appropriate error message phrase.
 */
/*--------------------------------------------------------------------------*/

const char *
Dec_tst::err_phrase(
	bool			pre_fail,			   	// Preset test failed.
	bool			trig_fail,	 			// Trigger test failed.
	bool			rst_fail )				// Reset test failed (optional).
{
	u_short			index = 0;				// Fail message index.

	if ( pre_fail )
	{
		index |= 0x1;
	}

	if ( trig_fail )
	{
		index |= 0x2;
	}

	if ( rst_fail )
	{
		index |= 0x4;
	}

	return ( err_msg[ index ] );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		min_phrase()		   	   	-	 Return min search msg.
 *
 *	RETURN VALUE
 *
 *		msg		-	Min phrase message.
 *
 *	DESCRIPTION
 *
 *		min_phrase() uses 't_min', and 'out_of_range', to return the
 *		appropriate minimum range test message phrase.  Note: This methods
 *		uses a static buffer to hold the phrase which is reused for each call.
 */
/*--------------------------------------------------------------------------*/

const char *
Dec_tst::min_phrase(
	u_short			t_min,					// Minimum test value.
	u_short			out_of_range )			// Out of range value.
{
	static char	   	p_buf[ 16 ];			// Buffer to hold phrase.

	if ( t_min == T_INV )					// None of the tests passed.
	{
		strcpy( p_buf, "NONE" );
	}
	else if ( t_min - 1 <= out_of_range )
	{
		sprintf( p_buf, "<= %4u", t_min );
	}
	else
	{
		sprintf( p_buf, "%4u", t_min );
	}

	return ( p_buf );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		max_phrase()		   	   	-	 Return max search msg.
 *
 *	RETURN VALUE
 *
 *		msg		-	Min phrase message.
 *
 *	DESCRIPTION
 *
 *		max_phrase() uses 't_max', and 'out_of_range', to return the
 *		appropriate maximum range test message phrase.  Note: This methods
 *		uses a static buffer to hold the phrase which is reused for each call.
 */
/*--------------------------------------------------------------------------*/

const char *
Dec_tst::max_phrase(
	u_short			t_max,					// Maximum test value.
	u_short			out_of_range )			// Out of range value.
{
	static char	   	p_buf[ 16 ];			// Buffer to hold phrase.

	if ( t_max == T_INV )					// None of the tests passed.
	{
		strcpy( p_buf, "NONE" );
	}
	else if ( t_max + 1 >= out_of_range )
	{
		sprintf( p_buf, ">= %4u", t_max );
	}
	else
	{
		sprintf( p_buf, "%4u", t_max );
	}

	return ( p_buf );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		min_duty_phrase()		   	   -	 Return duty min search msg.
 *
 *	RETURN VALUE
 *
 *		msg		-	Min duty message.
 *
 *	DESCRIPTION
 *
 *		min_duty_phrase() uses 't_nom', 't_min', and 'out_of_range',
 *		to return the appropriate minimum duty cycle test message phrase.
 *		Note: This methods uses a static buffer to hold the phrase which
 *		is reused for each call.
 */
/*--------------------------------------------------------------------------*/

const char *
Dec_tst::min_duty_phrase(
	u_short			t_nom,					// Nominal value.
	u_short			t_min,					// Minimum test value.
	u_short			out_of_range )			// Out of range value.
{
	static char	   	p_buf[ 16 ];			// Buffer to hold phrase.

	if ( t_min == T_INV )					// None of the tests passed.
	{
		strcpy( p_buf, "NONE" );
	}
	else if ( t_min - 1 <= out_of_range )
	{
		sprintf( p_buf, "<= %4d", (int)t_min - (int)t_nom );
	}
	else
	{
		sprintf( p_buf, "%4d", (int)t_min - (int)t_nom );
	}

	return ( p_buf );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		max_duty_phrase()		   	   -	 Return duty max search msg.
 *
 *	RETURN VALUE
 *
 *		msg		-	Min duty message.
 *
 *	DESCRIPTION
 *
  *		max_duty_phrase() uses 't_nom', 't_max', and 'out_of_range',
 *		to return the appropriate maximum duty cycle test message phrase.
 *		Note: This methods uses a static buffer to hold the phrase which
 *		is reused for each call.
 */
/*--------------------------------------------------------------------------*/

const char *
Dec_tst::max_duty_phrase(
	u_short			t_nom,					// Nominal value.
	u_short			t_max,					// Maximum test value.
	u_short			out_of_range )			// Out of range value.
{
	static char	   	p_buf[ 16 ];			// Buffer to hold phrase.

	if ( t_max == T_INV )					// None of the tests passed.
	{
		strcpy( p_buf, "NONE" );
	}
	else if ( t_max + 1 >= out_of_range )
	{
		sprintf( p_buf, ">= %4u", t_max - t_nom );
	}
	else
	{
		sprintf( p_buf, "%4u", t_max - t_nom );
	}

	return ( p_buf );
}


/*--------------------------------------------------------------------------*/
/*
 *	NAME
 *
 *		set_trig_rev()		   	   		-	 Set trigger command.
 *
 *	RETURN VALUE
 *
 *		None.
 *
 *	DESCRIPTION
 *
 *		set_trig_rev() sets the 'trig_cmd_speed' member variable using
 *		the 'itrig_rev' boolean as input.
 */
/*--------------------------------------------------------------------------*/

void
Dec_tst::set_trig_rev(
	bool			itrig_rev )				// If true, set use reverse cmd.
{
	trig_rev = itrig_rev;
	
	if ( itrig_rev == true )
	{
		trig_cmd_speed	=	SP_TEST_MIN;
	}
	else
	{
		trig_cmd_speed	=	SP_E_STOP;
	}
}


/*****************************************************************************
 * $History: DEC_TST.CPP $
 * 
 * *****************  Version 61  *****************
 * User: Kenw         Date: 9/12/23    Time: 8:09p
 * Updated in $/NMRA/SRC/SEND
 * Changed TO_LOG() to DBGPRINT() in fsoc_send().
 * 
 * *****************  Version 60  *****************
 * User: Kenw         Date: 8/27/23    Time: 2:24p
 * Updated in $/NMRA/SRC/SEND
 * Corrected Dec_tst Function decoder initialization sequence.
 * Updated bad_bit() method to change Function and Accessory reset
 * sequences.
 * 
 * *****************  Version 57  *****************
 * User: Kenw         Date: 8/12/23    Time: 8:28p
 * Updated in $/NMRA/SRC/SEND
 * Added debug iitems to func_ramp() packet.
 * Removed unneeded debug_gen() calls.
 * 
 * *****************  Version 56  *****************
 * User: Kenw         Date: 7/04/23    Time: 7:51p
 * Updated in $/NMRA/SRC/SEND
 * Changed func_ramp() and decoder_ames() to use
 * Bits::put_func_grp_pktx() rather than
 * Bits::put_func_grp_pkt()
 * 
 * *****************  Version 55  *****************
 * User: Kenw         Date: 6/23/23    Time: 7:35p
 * Updated in $/NMRA/SRC/SEND
 * Added kick_start() to test init if '-k' flag is set.
 * 
 * *****************  Version 54  *****************
 * User: Kenw         Date: 6/22/23    Time: 9:08p
 * Updated in $/NMRA/SRC/SEND
 * Changed "OUT_PC( PC_POS_UNDERCLRL, 0 )" to
 * "Dcc_reg.clr_under()" when logging output.
 * 
 * *****************  Version 52  *****************
 * User: Kenw         Date: 9/19/20    Time: 3:42p
 * Updated in $/NMRA/SRC/SEND
 * Rolled to X.5.10.2
 * 
 * *****************  Version 51  *****************
 * User: Kenw         Date: 3/26/20    Time: 8:52p
 * Updated in $/NMRA/SRC/SEND
 * Intermediate build to 5.10.1
 * ACC_PAIR suppot added.
 * Unit tests comparing packet logs passed.
 * 
 * *****************  Version 50  *****************
 * User: Kenw         Date: 8/17/19    Time: 5:15p
 * Updated in $/NMRA/SRC/SEND
 * Added test item number and clock item number to each item in the user
 * print out.
 * 
 * *****************  Version 49  *****************
 * User: Kenw         Date: 7/24/19    Time: 7:18p
 * Updated in $/NMRA/SRC/SEND
 * Check in prior to B.5.9.3 release.
 * 
 * *****************  Version 48  *****************
 * User: Kenw         Date: 2/12/19    Time: 9:23p
 * Updated in $/NMRA/SRC/SEND
 * Added warning message to logs for VER_DEB debug only builds.
 * 
 * *****************  Version 47  *****************
 * User: Kenw         Date: 7/03/18    Time: 7:28p
 * Updated in $/NMRA/SRC/SEND
 * Intermin Check in X.5.8.3
 * 
 * *****************  Version 45  *****************
 * User: Kenw         Date: 9/17/14    Time: 4:43p
 * Updated in $/NMRA/SRC/SEND
 * Added function decoder support for manual mode.
 * Modified decoder quiestions.
 * Print <SEND_END n> when program exits.
 * 
 * *****************  Version 43  *****************
 * User: Kenw         Date: 5/20/14    Time: 7:17p
 * Updated in $/NMRA/SRC/SEND
 * Removed BETA designation from Ambig1 & Ambig2 tests.
 * 
 * *****************  Version 42  *****************
 * User: Kenw         Date: 11/09/13   Time: 7:49p
 * Updated in $/NMRA/SRC/SEND
 * Added Function tests.
 * Added test summary to screen after each test phase.
 * 
 * *****************  Version 41  *****************
 * User: Kenw         Date: 9/15/05    Time: 7:13p
 * Updated in $/NMRA/SRC/SEND
 * 
 * *****************  Version 40  *****************
 * User: Kenw         Date: 12/21/04   Time: 12:34p
 * Updated in $/NMRA/SRC/SEND
 * Fixed problem with parameter count for Ambig 2 screen test message
 * 
 * *****************  Version 39  *****************
 * User: Kenw         Date: 12/21/04   Time: 12:11p
 * Updated in $/NMRA/SRC/SEND
 * Updating to Build 5.2 X.
 * 
 * *****************  Version 38  *****************
 * User: Kenw         Date: 12/15/04   Time: 3:51p
 * Updated in $/NMRA/SRC/SEND
 * Updated to roll to Version 5.2 X.
 * Added back 11800 single stretch times
 * 
 * *****************  Version 37  *****************
 * User: Kenw         Date: 12/15/04   Time: 2:44p
 * Updated in $/NMRA/SRC/SEND
 * Forced early scope trigger for ambig 1 and ambig 2 tests.
 * Reduced stack size for messages to try and fix underflows
 * on slow machines.
 * Some cosmetic cleanup
 * 
 * *****************  Version 36  *****************
 * User: Kenw         Date: 12/15/04   Time: 12:30p
 * Updated in $/NMRA/SRC/SEND
 * Updated for Version 5.1 Experimental
 * 
 * *****************  Version 35  *****************
 * User: Kenw         Date: 12/15/04   Time: 12:09p
 * Updated in $/NMRA/SRC/SEND
 * Interim check-in prior to changing ambig_ to ambig1_
 * 
 * *****************  Version 33  *****************
 * User: Kenw         Date: 11/01/04   Time: 7:15p
 * Updated in $/NMRA/SRC/SEND
 * Added very long (13000 usec) stretched 0 test.
 * 
 * *****************  Version 32  *****************
 * User: Kenw         Date: 7/20/04    Time: 2:27p
 * Updated in $/NMRA/SRC/SEND
 * Updated for X.4.8.  Added extra preamble support to
 * Feedback 1 and Feedback 2 tests.
 * 
 * *****************  Version 31  *****************
 * User: Kenw         Date: 7/01/04    Time: 7:31p
 * Updated in $/NMRA/SRC/SEND
 * Updated to include testing feedback bits in all error Byte bits
 * 
 * *****************  Version 30  *****************
 * User: Kenw         Date: 6/06/04    Time: 2:26p
 * Updated in $/NMRA/SRC/SEND
 * Extended -E command to add preambles to Ramp, Address, and bits
 * test.  Changed manual -i and -d command to send 4 packet sequences
 * to help debug LV102
 * 
 * *****************  Version 29  *****************
 * User: Kenw         Date: 6/04/04    Time: 4:39p
 * Updated in $/NMRA/SRC/SEND
 * Added Feedback 1 Test
 * 
 * *****************  Version 28  *****************
 * User: Kenw         Date: 5/26/04    Time: 7:29p
 * Updated in $/NMRA/SRC/SEND
 * Added packet end bit to feedback test
 * 
 * *****************  Version 27  *****************
 * User: Kenw         Date: 5/15/04    Time: 8:32a
 * Updated in $/NMRA/SRC/SEND
 * Added 1 interior feedback bit test
 * 
 * *****************  Version 26  *****************
 * User: Kenw         Date: 1/11/04    Time: 12:30p
 * Updated in $/NMRA/SRC/SEND
 * Updated for Version 4.1 Beta.
 * Adds feedback test.
 * 
 * *****************  Version 25  *****************
 * User: Kenw         Date: 9/01/00    Time: 6:32p
 * Updated in $/NMRA/SRC/SEND
 * Increased tests for 6 BYTE packets to max of 21
 * 
 * *****************  Version 24  *****************
 * User: Kenw         Date: 8/31/00    Time: 7:57p
 * Updated in $/NMRA/SRC/SEND
 * Updated to Version 3.1.
 * Includes changes to support 28 speed steps as the default
 * and a minimum of 12 bit preambles.
 * 
 * *****************  Version 22  *****************
 * User: Kenw         Date: 8/10/98    Time: 7:06p
 * Updated in $/NMRA/SRC/SEND
 * Moving to B.2.14
 * 
 * *****************  Version 21  *****************
 * User: Kenw         Date: 8/08/98    Time: 7:40p
 * Updated in $/NMRA/SRC/SEND
 * Updated to X.2.14
 * 
 * *****************  Version 20  *****************
 * User: Kenw         Date: 7/11/98    Time: 12:43p
 * Updated in $/NMRA/SRC/SEND
 * Added new test 1 usec from minimum
 * 
 * *****************  Version 19  *****************
 * User: Kenw         Date: 7/11/98    Time: 8:36a
 * Updated in $/NMRA/SRC/SEND
 * Rolling to 2.13.
 * Added tests near maximum packet times.
 * Added ability to move scope trigger before or after trigger packet..
 * 
 * *****************  Version 18  *****************
 * User: Kenw         Date: 6/29/98    Time: 6:47p
 * Updated in $/NMRA/SRC/SEND
 * Add code to skip ambiguous prior packet.
 * 
 * *****************  Version 17  *****************
 * User: Kenw         Date: 11/01/97   Time: 8:24p
 * Updated in $/NMRA/SRC/SEND
 * X.2.10 - Cleared accessory decoder init packet in dcc_bits3.
 * 
 * *****************  Version 16  *****************
 * User: Kenw         Date: 10/30/97   Time: 5:59p
 * Updated in $/NMRA/SRC/SEND
 * X.2.8 plus - Removed commented out //kgw lines
 * 
 * *****************  Version 15  *****************
 * User: Kenw         Date: 10/30/97   Time: 5:53p
 * Updated in $/NMRA/SRC/SEND
 * X.2.9 - Format clean ups only.
 * 
 * *****************  Version 14  *****************
 * User: Kenw         Date: 10/29/97   Time: 4:26p
 * Updated in $/NMRA/SRC/SEND
 * X.2.8 - Added packet log command.
 * 
 * *****************  Version 13  *****************
 * User: Kenw         Date: 10/28/97   Time: 6:39p
 * Updated in $/NMRA/SRC/SEND
 * Added idle as prior command for last loop of prior packet test
 * 
 * *****************  Version 12  *****************
 * User: Kenw         Date: 10/28/97   Time: 2:38p
 * Updated in $/NMRA/SRC/SEND
 * Added Prior Packet Test & Idle manual command.
 * 
 * *****************  Version 11  *****************
 * User: Kenw         Date: 1/21/97    Time: 7:41p
 * Updated in $/NMRA/SRC/SEND
 * Increased Bits buffer size to 256.  Added error message if Bits objects
 * have
 * errors.
 * 
 * *****************  Version 10  *****************
 * User: Kenw         Date: 10/15/96   Time: 4:32p
 * Updated in $/NMRA/SRC/SEND
 * Added zero as last bit of packet fragment test.
 * Turned this test off by default
 * 
 * *****************  Version 9  *****************
 * User: Kenw         Date: 9/25/96    Time: 7:54p
 * Updated in $/NMRA/SRC/SEND
 * Added nested packet test
 * 
 * *****************  Version 8  *****************
 * User: Kenw         Date: 7/29/96    Time: 6:49p
 * Updated in $/NMRA/SRC/SEND
 * Added separate reset command for accesory decoders in
 * the bad_addr() and bad_bit() methods.
 * 
 * *****************  Version 7  *****************
 * User: Kenw         Date: 7/26/96    Time: 5:46p
 * Updated in $/NMRA/SRC/SEND
 * Rolled to revision 2.3
 * Added support for 1 pulse duty cycle test.
 * Added Reset test to Address and Bad bit tests.
 * Did some text cleanup.
 *
 * *****************  Version 6  *****************
 * User: Kenw         Date: 7/26/96    Time: 9:04a
 * Updated in $/NMRA/SRC/SEND
 * Changes for version 2.2
 * Added support for 'o' oscilloscope setup  command.
 * Renamed 1/2 fast and 1/2 slow to command station max/min
 * 
 * *****************  Version 5  *****************
 * User: Kenw         Date: 5/12/96    Time: 1:14p
 * Updated in $/NMRA/SRC/SEND
 * Made changes as part of review.
 * Made print_user_docs() buf[] smaller to save stack.
 * Used PKT_REP_MIN in decoder_ramp() and acc_ramp instead of hard coding
 * 10.
 * Shortened some things to fit on a single line.
 * 
 * *****************  Version 4  *****************
 * User: Kenw         Date: 5/10/96    Time: 10:08a
 * Updated in $/NMRA/SRC/SEND
 * Fixed problem with print_user_docs().  I wasn't printing clock
 * mask parameter in clock table.
 * 
 * *****************  Version 3  *****************
 * User: Kenw         Date: 5/10/96    Time: 9:30a
 * Updated in $/NMRA/SRC/SEND
 * Added support for 'trig_rev' and 'fill_msec'.
 * Added support for print_user_docs().
 * Removed X_BIT_SKIP support.
 * Cleaned up some formatting.
 *
 * *****************  Version 2  *****************
 * User: Kenw         Date: 4/30/96    Time: 8:24p
 * Updated in $/NMRA/SRC/SEND
 * Added Single Stretched 0 test.  Made changes to change from small
 * to large memory model.
 *
 * *****************  Version 1  *****************
 * User: Kenw         Date: 4/21/96    Time: 3:57p
 * Created in $/NMRA/SRC/SEND
 * Methods for 'class Dec_tst'.
 *
 *****************************************************************************/

