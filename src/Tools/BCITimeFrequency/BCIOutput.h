#ifndef BCIOutputH
#define BCIOutputH

#include <stdio.h>

#include "StateForm1.h"
#include "GETMEM.h"
#include "WindowingFunction.h"
/********************************************************************
        BCIOutput contains the output routines for the
        BCITime application
*********************************************************************/

#define MAXCHANS 65
#define MAXPOINTS 3200
#define MAXSTATES 8
#define MAXOUT 512

class BCIOutput
{
    private:
                int Hz;
                float fstart;
                double point[NGROUPS][MAXCHANS][MAXPOINTS];
                float spoint[NGROUPS][MAXCHANS][MAXPOINTS];
                float n[NGROUPS][MAXCHANS][MAXPOINTS];             // int ? float ? 
                int sn[NGROUPS][MAXCHANS];                         // length of spectral data
                double sspoint[NGROUPS][MAXCHANS][MAXPOINTS];
                double xypoint[NGROUPS][MAXCHANS][MAXPOINTS];
                double stateval[NGROUPS][MAXSTATES][MAXPOINTS];
                int nsv[NGROUPS][MAXSTATES][MAXPOINTS];
                double sstate[NGROUPS][MAXSTATES][MAXPOINTS];
                double xystate[NGROUPS][MAXSTATES][MAXPOINTS];
                FILE * fileptr;
                int xyorder;
                int maxgroup;
                int maxchan;
                int maxpoint;
                int maxstate;
                int statistics;
                char inf[MAXOUT];
              //  int achans;
              //  int buf_lth;
                char outf_base[MAXOUT];

                int wintype;          // type of windowing  0 is all data in one window
                int wblocksz;         // window block size
                int winnum;           // number of blocks per window
                int winlength;        // total data length = wblocksz * winnum
                WindowingFunction mSidelobeSuppression;

                double __fastcall GetLr( double *t, double *ss, double *ssxy, int *n, int ntarg );
                void __fastcall print_hdr(FILE *otf, char *, int, int);
	public:
                int time_list[64];
                int value_list[64];
                int ntimes;
                int decimate;
                bool memflag;
                MEM *memptr;

                BCIOutput();
                ~BCIOutput();

                 void setWindow( int, int, int, int, const WindowingFunction& );
                __fastcall void CloseFiles( void );
                __fastcall void ClearVals( void );
                __fastcall void AddPoint( int group, int chan, int point, float val );
                __fastcall void AddSpcPoint( int group, int chan, int point, float val );
                __fastcall void DumpSpc( void );
                __fastcall void AddStateVal( int group, int chan, int point, float val );
                __fastcall void PrintVals( int );
                __fastcall void Config( char *, int sr, float start, int order, int stat);

} ;

#endif