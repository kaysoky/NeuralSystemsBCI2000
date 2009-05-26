/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
/**************************************************************
        BCIInput contains the input routines for the
        BCITime application
***************************************************************/

#include <string.h>
#include "OutputForm1.h"
#include "BCIInput.h"
#include "VCLDefines.h"

// FILE *bciin;

BCIInput::BCIInput()
{
     delayflag= 0;

//    bciin= fopen("c:/shared/misc/BCIIn.asc","w+");

}
BCIInput::~BCIInput()
{
//     fclose(bciin);
}

void __fastcall BCIInput::Config( BCIOutput *BCIOut )
{
        int i,j;
        float delta;

    // zero everything

        for(i=0;i<NSTATES;i++)
        {
                strcpy(Fstatelist[i],"");

                for(j=0;j<NGROUPS;j++)
                {
                        Fstate[i][j]= 0;
                        Fvalue[i][j]= 0;
                }
        }
        for(j=0;j<NGROUPS;j++)
                Fgroup[j]= 0;

        bcioutput= BCIOut;

        if( UseStateForm->Use->Checked == true ) reg= 1;
        else                                     reg= 0;

        if( OutputForm->OverlapMode->Checked == true )  overlap= 1;
        else                                            overlap= 0;

        if( OutputForm->SubGroups->Checked == true )  subgroups= 1;
        else                                          subgroups= 0;

        strcpy( ComputeWhen, AnsiString(OutputForm->vCompuMeans->Text).c_str() );

        for(i=0;i<UseStateForm->NUstates;i++)
                strcpy(Fstatelist[i],UseStateForm->StateList[i]);

        for(i=0;i<UseStateForm->ntargs;i++)
        {
                Fgroup[i]= UseStateForm->Group[i];

                for(j=0;j<UseStateForm->nstates;j++)
                {
                        Fstate[i][j]= UseStateForm->State[i][j];
                        Fvalue[i][j]= UseStateForm->Value[i][j];
                }
        }

        if( Channels > 0 )
        {
                delta= (float)(Channels);
                delta= 1/delta;

                for(i=0;i<Channels;i++)
                {
                        w2[i]= i * delta;
                        w1[i]= 1-w2[i];
                        old[i]= 0;
                }
         }
         else
         {
                Application->MessageBox(VCLSTR("Error"),VCLSTR("Number of Channels < 1 "),MB_OK);
         }

}

void __fastcall BCIInput::CheckCalibration( BCI2000FileReader* reader, char *calib , bool type )
{
        int i;
        int n_chans;
        int ncalib;

        n_chans= reader->Parameter("SourceCh");
        ncalib= reader->Parameter("SourceChOffset")->NumValues();

        for(i=0;i<n_chans;i++)
        {
                offset[i]= 0.0;
                gain[i]= 0.008;
        }

        if( (type == false ) && (n_chans == ncalib ) )
        {
                for(i=0;i<n_chans;i++)
                {
                        offset[i]=reader->Parameter("SourceChOffset")(i);
                        gain[i]=reader->Parameter("SourceChGain")(i);
                }
        }
        else
        {
          ParamList parmlist;
          if( parmlist.Load( calib ) == true )
          {
                  n_chans= parmlist["SourceChOffset"].NumValues();

                  for(i=0;i<n_chans;i++)
                  {
                          offset[i]=atoi(parmlist["SourceChOffset"].Value(i).c_str());
                          gain[i]=atof(parmlist["SourceChGain"].Value(i).c_str());
                  }
          }
        }
}

// convert one state of BCI2000 into Group coding

__int16 BCIInput::ConvertState(BCI2000FileReader *bci2000data, int sample)
{
        const StateVector *statevector;
        short statecode[NSTATES];
        int i,j;
        int match;
        short current_group;

        static short OldGroup;
        short cgp;

        statevector=bci2000data->StateVector();

        for(i=0;i<UseStateForm->NUstates;i++)
        {
                statecode[i]= statevector->StateValue(Fstatelist[i]);

                if( (reg == 1 ) && ( i== 1 ) )
                {
                        if( ( statecode[0] > 0  ) && (statecode[1] == 0 ) )
                        {
                                if( reg_flag == 0 )
                                {
                                        while( reg_value == 0 )
                                        {
                                                sample++;
                                                bci2000data->ReadStateVector(sample);
                                                reg_value= statevector->StateValue(Fstatelist[1]);
                                        }
                                        reg_flag = 1;
                                }
                                statecode[1]= reg_value;
                        }
                }

        }

        current_group= 0;

        for(i=0;i<UseStateForm->ntargs;i++)
        {
                match= 0;

                for(j=0;j<UseStateForm->nstates;j++)
                {
                        if( statecode[Fstate[i][j]] == Fvalue[i][j] )
                                match++;
                }
                if( match == UseStateForm->nstates )
                        current_group= Fgroup[i];
        }
        if( current_group > 0 )
        {
                OldGroup= current_group;
                return( current_group );
        }

        if( ( UseStateForm->IncludeNext->Checked == true ) && ( OldGroup > 0 ) )
        {
                cgp= OldGroup;
                OldGroup= 0;
                delayflag= 1;
                return( cgp );
        }


        if( reg == 1 ) reg_flag= 0;

        return(32);           // null value
}

float  __fastcall BCIInput::GetState(BCI2000FileReader *bci2000data, int state, int sample )
{
        int i;
        float val;
        const StateVector *statevector;

        statevector= bci2000data->StateVector();
        bci2000data->ReadStateVector( sample );

        val= (float)statevector->StateValue(state_list[state]);
        return( val );
}

__int16 BCIInput::GetCompuFlag(BCI2000FileReader *bci2000data, int sample )
{
        const StateVector *statevector;
        short compuflag;

        static short OldCompuflag= 0;
        short ocf;

        statevector= bci2000data->StateVector();
        bci2000data->ReadStateVector( sample );

        compuflag= (short)statevector->StateValue(ComputeWhen);

        if( delayflag > 0 )
        {
                ocf= OldCompuflag;
                OldCompuflag= compuflag;                   // get last cursor pos
                delayflag= 0;
                return( ocf );
         }

        OldCompuflag= compuflag;
        return( compuflag );
}


float __fastcall BCIInput::GetValue( BCI2000FileReader *bci2000data,  int channel, int sample  )
{
        float val;
        float neigh;
        float temp;
        float lastemp;
        float sum;
        int i,j;
        int index;
        float tval;


        if( tfilterflag == false )
        {
                if(sfilterflag == false )            // no temporal or spatial filtering
                {
                        val= bci2000data->RawValue( channel, sample );
                        val= ( val - offset[chan_list[channel]] ) * gain[chan_list[channel]];
                        if( alignflag )
                        {
                                val= (w1[chan_list[channel]]*val) + (w2[chan_list[channel]]*old[channel]);
                                old[chan_list[channel]]= val;
                        }
                }
                else                                // spatial but not temporal
                {
                        val= bci2000data->RawValue( channel, sample );
                        val= ( val - offset[chan_list[channel]] ) * gain[chan_list[channel]];
                        if( alignflag )
                        {
                                val= (w1[chan_list[channel]]*val) + (w2[chan_list[channel]]*old[channel]);
                                old[chan_list[channel]]= val;
                        }

                        neigh= 0;
                        sum= 0;

                        for(i=1;i<lapn[chan_list[channel]]+1;i++)
                        {
                                temp= bci2000data->RawValue( lap[chan_list[channel]][i], sample );
                                temp= ( temp - offset[lap[chan_list[channel]][i]] ) * gain[lap[chan_list[channel]][i]];
                                if( alignflag )
                                {
                                        lastemp= bci2000data->RawValue( lap[chan_list[channel]][i], sample-1 );
                                        lastemp= ( lastemp - offset[lap[chan_list[channel]][i]] ) * gain[lap[chan_list[channel]][i]];
                                        temp= (w1[lap[chan_list[channel]][i]]*temp) + (w2[lap[chan_list[channel]][i]]*lastemp);
                                }

                                neigh+= temp * d_lap[lap[chan_list[channel]][i]][i];
                                sum+= d_lap[lap[chan_list[channel]][i]][i];

                        }
                        val= val - (neigh/sum);
                }
        }
        else
        {
                if(sfilterflag == false )       // temporal but not spatial
                {
                        val= 0;

                        index= 1 + sample - tcount;

                        if( index >= 0 )
                        {

                                for(i=0;i<tcount;i++)
                                {
                                        index++;
                                        val+= tcoff[i]*  bci2000data->RawValue( channel, index );
                                }

                                val= ( val - offset[chan_list[channel]] ) * gain[chan_list[channel]];
                        }
                        else    val= 0.0;       // assign 0 if  not enough data to filter
                }
                else
                {
                        val= 0.0;

                        index= 1 + sample - tcount;

                        if( index >= 0 )
                        {

                                for(i=0;i<tcount;i++)
                                {
                                        index++;

                                        tval= bci2000data->RawValue( channel, index );
                                        tval= ( tval - offset[chan_list[channel]] ) * gain[chan_list[channel]];

                                        neigh= 0;
                                        sum= 0;

                                        for(j=1;j<lapn[chan_list[channel]]+1;j++)
                                        {
                                                temp= bci2000data->RawValue( lap[chan_list[channel]][j], index );
                                                temp= ( temp - offset[lap[chan_list[channel]][j]] ) * gain[lap[chan_list[channel]][j]];
                                                neigh+= temp * d_lap[lap[chan_list[channel]][j]][j];
                                                sum+= d_lap[lap[chan_list[channel]][j]][j];
                                        }

                                        val+= tcoff[i] * ( tval - (neigh/sum) );
                                }

                        }
                        else    val= 0.0;       // assign 0 if  not enough data to filter

                }
        }

        return( val );
}

void __fastcall BCIInput::GetBaselines( BCI2000FileReader *bci2000data, int sample )
{
        int channel;
        int index;

        for(channel=0;channel<nchans;channel++)
        {
                baseline[channel]= 0;
                basen[channel]= 0;
        }

        for(index=BaseStart;index<BaseEnd;index++)
        {
                for(channel=0;channel<nchans;channel++)
                {
                        baseline[channel]+= GetValue(bci2000data, chan_list[channel],index+sample);
                        basen[channel]++;
                }
        }

        for(channel=0;channel<nchans;channel++)
                if( basen[channel] > 0 )
                        baseline[channel]/= basen[channel];
}

void __fastcall BCIInput::ReadFile( BCI2000FileReader *bci2000data, int numsamples )
{
        int sample;
        int state;
        int cur_state;
        int oldstate;
        int cur_compu_flag;
        static int old_compu_flag;
        int vmax;
        int i;
 //       int n[64];
        int channel;
        float val;
   //     int timecount;
        int index;


        oldstate= 0;
        old_compu_flag= -999;

 //       for(i=0;i<MAXCHANS;i++)
 //               n[i]= 0;

                                // go through all samples in each run
        for (sample=0; sample<numsamples; sample++)
        {

                if (sample % 1000 == 0) Application->ProcessMessages();
                                // go through each channel

                bci2000data->ReadStateVector(sample);
                cur_state=ConvertState(bci2000data,sample);

                cur_compu_flag= GetCompuFlag(bci2000data, sample );

                
                if( subgroups == 1 )
                {
                        if( cur_compu_flag != old_compu_flag )
                        {
                                if( sample > 0 ) bcioutput->PrintVals( old_compu_flag );
                                bcioutput->ClearVals();
                        }
                }        

                old_compu_flag= cur_compu_flag;


                if( overlap == 0 )
                {

                        if( cur_state == oldstate ) vmax++;
                        else
                        {
                                vmax= 0;
                                if( memflag ) bcioutput->DumpSpc();
                                if( ( BaselineUse == 1 )&& ( cur_state < 32 ) )
                                        GetBaselines(bci2000data, sample);
                        }

                        oldstate= cur_state;


                        if( ( vmax >= start ) && ( vmax <= end ) )           // in segmnet range
                        {
                                if( cur_state < 32 )
                                {
                                        for (channel=0; channel<nchans; channel++)
                                        {

                                                val= GetValue(bci2000data, chan_list[channel], sample);
                                                if( BaselineUse == 1 )  val-= baseline[channel];
                                                if( memflag ) bcioutput->AddSpcPoint( cur_state, channel, vmax-start, val );
                                                else bcioutput->AddPoint( cur_state, channel, vmax-start, val );
                                        }
                                        for(state=0;state<nstates;state++)
                                        {
                                                val= GetState(bci2000data, state,sample);
                                                bcioutput->AddStateVal( cur_state, state, vmax-start, val );
                                        }
                                }
                        }
                }
                else if( overlap == 1 )
                {
                        if( ( cur_state < 32 ) && ( cur_state != oldstate ) )
                        {
                                if( BaselineUse == 1 )
                                        GetBaselines(bci2000data, sample);

                        //        timecount= 0;
                                for(i=start; i<end+1; i++)
                                {
                                        index= i-start;


                                        for( channel=0; channel<nchans; channel++)
                                        {
                                                val= GetValue(bci2000data, chan_list[channel], sample+i);
                                                if( BaselineUse == 1 ) val-= baseline[channel];
                                                if( memflag ) bcioutput->AddSpcPoint( cur_state, channel, index, val );
                                                else bcioutput->AddPoint( cur_state, channel, index, val );
                                        }
                                        for(state=0;state<nstates;state++)
                                        {
                                                val= GetState(bci2000data, state,sample+i);
                                                bcioutput->AddStateVal( cur_state, state, index, val );
                                        }

                                }
                                jmp2: ;       // end of topo list
                                if( memflag ) bcioutput->DumpSpc();
                        }
                        oldstate= cur_state;
                }
        }
}

