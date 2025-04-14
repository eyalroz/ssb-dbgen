/* @(#)speed_seed.c	2.1.8.2 */
#include <stdio.h>
#include "dss.h"
#include "life_noise.h"

/*  _tal long RandSeed = "Random^SeedFromTimestamp" (void); */

#define FAKE_V_STR(avg, sd, cnt) \
	ADVANCE_STREAM(sd, \
		(long)(Seed[sd].boundary*cnt))
#define ADVANCE_STREAM(stream_id, num_calls) \
        NthElement(num_calls, &Seed[stream_id].value)

#define MAX_COLOR 92
long name_bits[MAX_COLOR / BITS_PER_LONG];
extern seed_t Seed[];

/* WARNING!  This routine assumes the existence of 64-bit                 */
/* integers.  The notation used here- "HUGE" is *not* ANSI standard. */
/* Hopefully, you have this extension as well.  If not, use whatever      */
/* nonstandard trick you need to in order to get 64 bit integers.         */
/* The book says that this will work if MAXINT for the type you choose    */
/* is at least 2**46  - 1, so 64 bits is more than you *really* need      */

static int Multiplier = 16807;        /* or whatever nonstandard */
static long Modulus =  2147483647L;   /* trick you use to get 64 bit int */

/* Advances value of Seed after N applications of the random number generator
   with multiplier Mult and given Modulus.
   NthElement(Seed[],count);

   Theory:  We are using a generator of the form
        X_n = [Mult * X_(n-1)]  mod Modulus.    It turns out that
        X_n = [(Mult ** n) X_0] mod Modulus.
   This can be computed using a divide-and-conquer technique, see
   the code below.

   In words, this means that if you want the value of the Seed after n
   applications of the generator,  you multiply the initial value of the
   Seed by the "super multiplier" which is the basic multiplier raised
   to the nth power, and then take mod Modulus.
*/

/* Nth Element of sequence starting with StartSeed */
/* Warning, needs 64-bit integers */
void NthElement (long N, long *StartSeed)
   {
   DSS_HUGE Z;
   DSS_HUGE Mult;
   static int ln=-1;
   int i;

   if ((verbose > 0) && ++ln % 1000 == 0)
       {
       i = ln % LN_CNT;
       fprintf(stderr, "%c\b", lnoise[i]);
       }
   Mult = Multiplier;
   Z = (DSS_HUGE) *StartSeed;
   while (N > 0 )
      {
      if (N % 2 != 0)    /* testing for oddness, this seems portable */
         Z = (Mult * Z) % Modulus;
      N = N / 2;         /* integer division, truncates */
      Mult = (Mult * Mult) % Modulus;
      }
   *StartSeed = (long)Z;

   return;
   }

/* updates Seed[column] using the a_rnd algorithm */
void
fake_a_rnd(int min, int max, int column)
{
   long len, itcount;
   RANDOM(len, (long)min, (long)max, (long)column);
   if (len % 5L == 0)
      itcount = len/5;
   else itcount = len/5 + 1L;
   NthElement(itcount, &Seed[column].usage);
   return;
}


long 
sd_part(int child, long skip_count)
{
   int i;
 
   UNUSED(child);
   for (i=P_MFG_SD; i<= P_CNTR_SD; i++)
       ADVANCE_STREAM(i, skip_count);
 
   FAKE_V_STR(P_CMNT_LEN, P_CMNT_SD, skip_count);
   ADVANCE_STREAM(P_NAME_SD, skip_count * 92);

   return(0L);
}

long 
sd_line(int child, long skip_count)
	{
	int i,j;
	
	for (j=0; j < O_LCNT_MAX; j++)
		{
		for (i=L_QTY_SD; i<= L_RFLG_SD; i++)
			ADVANCE_STREAM(i, skip_count);
		}
	
	FAKE_V_STR(L_CMNT_LEN, L_CMNT_SD, skip_count);
	/* need to special case this as the link between master and detail */
	if (child == 1)
		{
		ADVANCE_STREAM(O_ODATE_SD, skip_count);
		ADVANCE_STREAM(O_LCNT_SD, skip_count);
		}
		
	return(0L);
	}

long 
sd_order(int child, long skip_count)        
{
   UNUSED(child);
   ADVANCE_STREAM(O_LCNT_SD, skip_count);
   ADVANCE_STREAM(O_CKEY_SD, skip_count);
   FAKE_V_STR(O_CMNT_LEN, O_CMNT_SD, skip_count);
   ADVANCE_STREAM(O_SUPP_SD, skip_count);
   ADVANCE_STREAM(O_CLRK_SD, skip_count);
   ADVANCE_STREAM(O_PRIO_SD, skip_count);
   ADVANCE_STREAM(O_ODATE_SD, skip_count);

   return (0L);
}

long
sd_psupp(int child, long skip_count)
	{
	int j;
	
	UNUSED(child);
	for (j=0; j < SUPP_PER_PART; j++)
		{
		ADVANCE_STREAM(PS_QTY_SD, skip_count);
		ADVANCE_STREAM(PS_SCST_SD, skip_count);
		}
	FAKE_V_STR(PS_CMNT_LEN, PS_CMNT_SD, skip_count);

	return(0L);
	}

long 
sd_cust(int child, long skip_count)
{
   UNUSED(child);
   FAKE_V_STR(C_ADDR_LEN, C_ADDR_SD, skip_count);
   FAKE_V_STR(C_CMNT_LEN, C_CMNT_SD, skip_count);
   ADVANCE_STREAM(C_NTRG_SD, skip_count);
   ADVANCE_STREAM(C_PHNE_SD, 3L * skip_count);
   ADVANCE_STREAM(C_ABAL_SD, skip_count);
   ADVANCE_STREAM(C_MSEG_SD, skip_count);
   return(0L);
}

long
sd_supp(int child, long skip_count)
{
   UNUSED(child);
   ADVANCE_STREAM(S_NTRG_SD, skip_count);
   ADVANCE_STREAM(S_PHNE_SD, 3L * skip_count);
   ADVANCE_STREAM(S_ABAL_SD, skip_count);
   FAKE_V_STR(S_ADDR_LEN, S_ADDR_SD, skip_count);
   FAKE_V_STR(S_CMNT_LEN, S_CMNT_SD, skip_count);
   ADVANCE_STREAM(BBB_CMNT_SD, skip_count);
   ADVANCE_STREAM(BBB_JNK_SD, skip_count);
   ADVANCE_STREAM(BBB_OFFSET_SD, skip_count);
   ADVANCE_STREAM(BBB_TYPE_SD, skip_count);      /* avoid one trudge */
   
   return(0L);
}
