#include <stdio.h>

#ifndef BCInputH
#define BCInputH

#include "UBCI2000DATA.h"
#include "UParameter.h"
#include "StateForm1.h"
#include "BCIOutput.h"

#define MAXLTH  256             // max fir filter length

class BCIInput
{
	private:
		FILE *parmfile;

                char Fstatelist[NSTATES][STATELENGTH];     // list of unique states
                int Fgroup[NGROUPS];                    // group for each target
                int Fstate[NGROUPS][NSTATES];          // states for each target
                int Fvalue[NGROUPS][NSTATES];          // values of states for each target
                int reg;
                int reg_flag;
                int reg_value;
                int overlap;
                int subgroups;
                char ComputeWhen[64];

                float baseline[MAXCHANS];
                float basen[MAXCHANS];

                BCIOutput *bcioutput;

                int delayflag;

                float w1[MAXCHANS];
                float w2[MAXCHANS];
                float old[MAXCHANS];

                float offset[MAXCHANS];
                float gain[MAXCHANS];

                float __fastcall GetState(BCI2000DATA *, int, int);
                void __fastcall GetBaselines( BCI2000DATA *, int );
                __int16 GetCompuFlag(BCI2000DATA *, int );
                __int16 ConvertState(BCI2000DATA *, int );
                float __fastcall GetValue( BCI2000DATA *bci2000data,  int channel, int sample  );
	public:

                int start;
                int end;

                int Channels;                   // number of channels in data file
                int nchans;                     // number of channels to process
                int chan_list[MAXCHANS];

                int nstates;
                int state_flag;
                char state_list[16][32];
                short state_val[16];

                bool tfilterflag;
                bool sfilterflag;
                bool alignflag;
                bool memflag;
              
                int tcount;                     // number of FIR coefficients
                float tcoff[MAXLTH];            // FIR filter coefficients

                int ntimes;
                int outorder;

                int signal_count;
                int lap[MAXCHANS][MAXCHANS];
                float d_lap[MAXCHANS][MAXCHANS];
                int lapn[MAXCHANS];

                int BaselineUse;
                int BaseStart;
                int BaseEnd;

                void __fastcall Config( BCIOutput *);
                void __fastcall CheckCalibration( PARAMLIST * , char * , bool);
                void __fastcall ReadFile( BCI2000DATA *, int );
                BCIInput();
                ~BCIInput();
}  ;


#endif