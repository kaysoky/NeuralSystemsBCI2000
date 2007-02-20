/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------


#pragma hdrstop

#include <sys\types.h>
#include <time.h>
#include <vector>

#include "UTaskUtil.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

static std::vector<int>  rannumbers;
static time_t            randseed;
static int               rannumbers_count=-1;

//--------------------------------------------------------------------
// Random # generator ran1() from Press et. al.
//   - Builder Random # function not good!

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

float ran1( long *idum )
{
        int j;
        long k;
        static long iy= 0;
        static long iv[NTAB];
        float temp;

        if(*idum <= 0 || !iy )
        {
                if(-(*idum) <1) *idum=1;
                else *idum= -(*idum);
                for(j=NTAB+7;j>=0;j--)
                {
                        k= (*idum)/IQ;
                        *idum= IA*(*idum-k*IQ)-IR*k;
                        if(*idum<0) *idum += IM;
                        if( j < NTAB ) iv[j]= *idum;
                 }
                 iy= iv[0];
         }
         k= (*idum)/IQ;
         *idum= IA*(*idum-k*IQ)-IR*k;
         if( *idum < 0 ) *idum += IM;
         j= iy/NDIV;
         iy= iv[j];
         iv[j]= *idum;
         if((temp=AM*iy) > RNMX) return RNMX;
         else return temp;
}


void ShuffleBlocks( int blocksize )
{
int     i,j;
float   rval;

 rannumbers.resize( blocksize );

 for (i=0; i<blocksize; i++)
  {
  rpt:    rval= ran1( &randseed );
  rannumbers[i]= 1 + (int)( rval * blocksize );

  if ((rannumbers[i] < 1) || (rannumbers[i] > blocksize))
     goto rpt;

  for(j=0;j<i; j++)
   if( rannumbers[j] == rannumbers[i] ) goto rpt;
  }
}


int GetBlockRandomizedNumber( int blocksize )
{
int     retval;

 rannumbers.resize( blocksize );
 // the first time, get a new seed
 if (rannumbers_count == -1)
    {
    time( &randseed );
    randseed= -randseed;
    rannumbers_count=0;
    }

 if (rannumbers_count == 0)
    ShuffleBlocks( blocksize );

 retval= rannumbers[rannumbers_count];

 rannumbers_count++;
 if (rannumbers_count > (blocksize-1))
    rannumbers_count= 0;

 return( retval );
}


void InitializeBlockRandomizedNumber()
{
 rannumbers_count=-1;
}

void ResetBlockCounter()
{
  rannumbers_count=0;
}


