//---------------------------------------------------------------------------

#include "PCHIncludes.h"
#pragma hdrstop

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "UCoreComm.h"
#include "UState.h"
#include "BCIDirectry.h"
#include "UFilterHandling.h"
#include "FIRStatFilter.h"

FILE *Statfile = NULL;

// FILE *estat;


//---------------------------------------------------------------------------

#pragma package(smart_init)

RegisterFilter( FIRStatFilter, 2.E );

// **************************************************************************
// Function:   FIRStatFilter
// Purpose:    This is the constructor for the FIRStatFilter Class
//             It is the Statistics module
//                      it does dynamic parameter setting
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
FIRStatFilter::FIRStatFilter()
: nf( NULL ),
  vis( NULL ),
  stat( NULL ),
  StatSignal( NULL )
{
 cur_stat.NumT= 0;

 BEGIN_PARAMETER_DEFINITIONS
  "Statistics int InterceptControl= 1 0 0 1  // Online adaption of Intercept 1 = Up/Dn  2 = Up/Dn + Le/Ri",
  "Statistics int InterceptLength= 3 0 0 1 // Length of time for running average",
  "Statistics float InterceptProportion= 1.0 0.0 0.0 2.0 // Proportion of signal mean for intercept",
  "Statistics float DesiredPixelsPerSec= 70 70 0 400 // Desired pixels per second",
  "Visualize int VisualizeStatFiltering= 1 0 0 1  // visualize Stat filtered signals (0=no 1=yes)",
  "Statistics matrix BaselineCfg= 2 2 TargetCode 1 TargetCode 2 0 0 1 // states to watch for baseline",
  "Statistics int TrendControl= 1 0 0 1  // Online adaption of % Correct Trend 1= Lin 2 = Quad",
  "Statistics int TrendWinLth= 20 0 0 100 // Length of % Correct Window",
  "Statistics float LinTrendLrnRt= 0.001 0 0.000 0.010 // Rate of Learning for Linear Trend Control",
  "Statistics float QuadTrendLrnRt= 0.001 0 0 0.010 // Rate of Learning for Linear Trend Control",
  "Storage string FileInitials= TD TD a z // Initials of file name (max. 8 characters)",
  "Storage string SubjectName= Name Name a z // subject alias (max. 8 characters)",
  "Storage string SubjectSession= 001 001 0 999 // session number (max. 3 characters)",
 END_PARAMETER_DEFINITIONS

 BEGIN_STATE_DEFINITIONS
  "IntCompute 2 0 0 0",
 END_STATE_DEFINITIONS

 // estat= fopen("EStat.asc","w+");

}


// **************************************************************************
// Function:   ~FIRStatFilter
// Purpose:    This is the destructor for the FIRStatFilter Stat
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
FIRStatFilter::~FIRStatFilter()
{
  delete vis;
  delete stat;
  delete StatSignal;

  if (Statfile) fclose( Statfile );
  // fclose( estat );
}

// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistence with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
void FIRStatFilter::Preflight( const SignalProperties& inSignalProperties,
                                  SignalProperties& outSignalProperties ) const
{
  // By just enumerating parameters this way, we have them checked for
  // existence and range.
  // For now, we don't perform any more checks, which amounts to declaring
  // that this filter works provided all parameters are in the range
  // given in their definition string.
  Parameter("UD_A");
  Parameter("UD_B");
  Parameter("LR_A");
  Parameter("LR_B");
  Parameter("FileInitials");
  Parameter("SubjectSession");
  Parameter("SubjectName");

  // This one is not required to exist.
  // If we didn't ask for it here, the framework would not let us access
  // it later on.
  OptionalParameter( 2, "NumberTargets" );

  // Required states.
  State("TargetCode");
  State("ResultCode");
  State("StimulusTime");
  State("Feedback");
  State("IntertrialInterval");

  // Optional states.
#ifdef TODO
# error Enter optional states used in Process().
#endif // TODO

  // We don't check the input signal, thus implicitly declaring that this 
  // filter works for input signals of any size.
  /* no checking done */

  // This filter does not use its output signal argument, so we specify
  // minimal requirements.
  outSignalProperties = SignalProperties( 0, 0, 0 );
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the FIRStatFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void FIRStatFilter::Initialize()
{
        AnsiString AName,SName,SSes,FInit;
        int visualizeyn;
        char slash[2];
        char numbuf[16];
        BCIDtry *bcidtry;

        static int init_flag= 0;
        trend_flag= 0;
        intercept_flag= 0;

  nf = GenericFilter::GetFilter< NormalFilter >();

  InterceptEstMode= Parameter("InterceptControl");
  InterceptLength= Parameter("InterceptLength");
  InterceptProportion= Parameter("InterceptProportion");
  ud_intercept= Parameter("UD_A");
  ud_gain     = Parameter("UD_B");
  lr_intercept= Parameter("LR_A");
  lr_gain     = Parameter("LR_B");
  Trend_Control= Parameter("TrendControl");
  Trend_Win_Lth= Parameter("TrendWinLth");
  LinTrend_Lrn_Rt= Parameter("LinTrendLrnRt");
  QuadTrend_Lrn_Rt= Parameter("QuadTrendLrnRt");
  desiredpix= Parameter("DesiredPixelsPerSec");
  visualizeyn= Parameter("VisualizeStatFiltering");
  Ntargets= Parameter("NumberTargets");

  FInit= ( const char* )Parameter("FileInitials");
  SSes = ( const char* )Parameter("SubjectSession");
  SName= ( const char* )Parameter("SubjectName");

  bcidtry= new BCIDtry();

  bcidtry->SetDir( FInit.c_str() );
  bcidtry->ProcPath();
  bcidtry->SetName( SName.c_str() );
  bcidtry->SetSession( SSes.c_str() );

  strcpy(FName, bcidtry->ProcSubDir() );

  slash[0]= 0x5c;
  slash[1]= 0;
  strcat(FName,slash);

  AName= SName + "S" + SSes + ".sta";
  strcat(FName, AName.c_str() );         // CAT vs CPY
  if (Statfile) fclose(Statfile);
  Statfile= fopen(FName,"a+");

 if( InterceptEstMode > 0 && init_flag < 1 )               // need to update if different targets
 {
        delete stat;
        stat= new STATISTICS;
        stat->SetNumMaxTrials(InterceptLength);
        stat->SetIntercept( 0, ud_intercept );
        stat->SetIntercept( 1, lr_intercept );
        if (ud_gain != 0)
           stat->SetGain( 0, desiredpix/ud_gain );
        else
           stat->SetGain( 0, 1000);
        if (lr_gain != 0)
           stat->SetGain( 1, desiredpix/lr_gain );
        else
           stat->SetGain( 1, 1000 );

        if( Trend_Control > 0 )
        {
                stat->SetDTWinMaxTrials( Trend_Win_Lth );
        }
        init_flag++;
 }

 //       cur_stat.bper= 1;
        cur_stat.pix= desiredpix;
        cur_stat.aper= InterceptProportion;

 if( visualizeyn == 1 )
 {
        visualize=true;
        delete vis;
        vis= new GenericVisualization;
        vis->SetSourceID(SOURCEID_STATISTICS);
        vis->SendCfg2Operator(SOURCEID_STATISTICS, CFGID_WINDOWTITLE, "Target Trends");
        vis->SendCfg2Operator(SOURCEID_STATISTICS, CFGID_MINVALUE, "0");
        vis->SendCfg2Operator(SOURCEID_STATISTICS, CFGID_MAXVALUE, "1");

        itoa( Ntargets, numbuf, 10 );

        vis->SendCfg2Operator(SOURCEID_STATISTICS, CFGID_NUMSAMPLES, numbuf );

        if (StatSignal) delete StatSignal;
        StatSignal= new GenericSignal( 1, Ntargets );
 }
 else
 {
        visualize=false;
 }

  delete bcidtry;

  return;
}


void FIRStatFilter::GetStates( void )
{
PARAM   *paramptr;
int     i, numbl;
int     j,numbl_states;
const char    *statename;
int     stateval;
int     match;

 CurrentTarget=       State("TargetCode");
 CurrentOutcome=      State("ResultCode");
 CurrentStimulusTime= State("StimulusTime");
 CurrentFeedback=     State("Feedback");
 CurrentIti=          State("IntertrialInterval");



 paramptr=Parameters->GetParamPtr("BaselineCfg");
 if (!paramptr) return;

 numbl=paramptr->GetNumValuesDimension1();
 numbl_states= paramptr->GetNumValuesDimension2();
 CurrentBaseline=-1;
 for (i=0; i<numbl; i++)
  {
         match= 1;

         for(j=0;j<numbl_states/2;j++)                  // test each
         {
                statename=paramptr->GetValue(i, (j*2) );
                stateval=atoi(paramptr->GetValue(i, (j*2)+1) );

                if ( !(State(statename) == stateval) )
                            match= 0;                  //    all must match
                            
// fprintf(Statfile,"i= %d j= %d match= %d  name %s val %d \n",i,j,match,statename,stateval);
         }
         if( match == 1 ) CurrentBaseline= i;
  }

}

// **************************************************************************
// Function:   Resting
// Purpose:    This function operates when the state running = 0
// **************************************************************************

void FIRStatFilter::Resting( void )
{
        char memotext[256];

        if( intercept_flag > 0 )               // return  operator
        {
                intercept_flag= 0;

                // change the value of the parameter

                // jm: removed probably unwanted trailing \r characters from parameter values
                // (they would show up in the operator if the "Config" button was pressed
                // after "Suspend").
                // Instead the two lines, one might consider writing
                // Parameter( "UD_A" ) = ud_intercept;
                // and do the rounding before the output.
                sprintf(memotext, "%.2f", ud_intercept);
                Parameter("UD_A") = memotext;

                sprintf(memotext, "%.2f", ud_gain);
                Parameter("UD_B") = memotext;


                Corecomm->StartSendingParameters();

                Corecomm->PublishParameter( Parameters->GetParamPtr("UD_A") );
                Corecomm->PublishParameter( Parameters->GetParamPtr("UD_B") );

                Corecomm->StopSendingParameters();
        }
        if( trend_flag > 0 )         // return trends to operator
        {
                trend_flag= 0;

                sprintf(memotext, "%.4f", cur_stat.aper);
                Parameter("InterceptProportion") = memotext;


    //     sprintf(memotext, "%.2f", cur_stat.bper * desiredpix);
                sprintf(memotext, "%.2f", cur_stat.pix);
                Parameter("DesiredPixelsPerSec") = memotext;


                Corecomm->StartSendingParameters();

                Corecomm->PublishParameter( Parameters->GetParamPtr("InterceptProportion") );
                Corecomm->PublishParameter( Parameters->GetParamPtr("DesiredPixelsPerSec") );

                Corecomm->StopSendingParameters();
        }
}

// **************************************************************************
// Function:   Process
// Purpose:    This function applies the Stat routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
void FIRStatFilter::Process(      //  CalibrationFilter       *calf,
                              //  SpatialFilter           *sf
                              //  TemporalFilter          *tf,
                              //  ClassFilter             *clsf,
                              const GenericSignal *SignalE,
                              //  NormalFilter  *nf,
                                GenericSignal *SignalF
                        )
{
static int recno= 1;
int i;
float   value;
char    memotext[512];
static  float old_ud_intercept=0, old_ud_gain=0, old_lr_intercept=0, old_lr_gain=0;
static int oldtarget, oldoutcome;

 // actually perform the Stat Filtering on the NormalFilter output signal

     GetStates();
     
   //  fprintf(Statfile,"CurrentTarget= %2d  CurrentBaseline= %2d \n", CurrentTarget, CurrentBaseline );


     if( InterceptEstMode >0 )
     {
        size_t sample= 0;

        for(size_t in_channel=0; in_channel<SignalE->Channels(); in_channel++)
         {
         value= SignalE->GetValue(in_channel, sample);

         if (in_channel == 0)
            {
            stat->ProcRunningAvg(CurrentBaseline, in_channel, value, &cur_stat);
            ud_intercept=cur_stat.Intercept;
            if (cur_stat.StdDev != 0)
               ud_gain=desiredpix/cur_stat.StdDev;
       /*
            if (( visualize ) && (cur_stat.update_flag > 0 ) ) //((ud_intercept != old_ud_intercept) || (ud_gain != old_ud_gain)))
               {
               // sprintf(memotext, "Adjusted CH0 intercept to %.2f and slope to %.2f", ud_intercept, ud_gain);
               // vis->SendMemo2Operator(memotext);

               for(i=0;i<Ntargets;i++)
                      output->SetValue( 1,i pwr[j] );

               vis->SetSourceID(59);
               vis->Send2Operator(StatSignal);

               cur_stat.update_flag= 0;
               }
              */
               old_ud_intercept= ud_intercept;
               old_ud_gain= ud_gain;
      //      fprintf(Statfile, "CurrentUDIntercept= %.2f  CurrentUDGain= %.2f\n", ud_intercept, ud_gain);
            }

         intercept_flag= 1;   

         if (InterceptEstMode > 1)
         {
            if (in_channel == 1)
            {
               stat->ProcRunningAvg(CurrentBaseline, in_channel, value, &cur_stat);
               stat->ProcRunningAvg(CurrentBaseline, in_channel, value, &cur_stat);
               lr_intercept=cur_stat.Intercept;
               if (cur_stat.StdDev != 0)
                  lr_gain=desiredpix/cur_stat.StdDev;
       //  else    lr_gain=1;
               if (( visualize ) && ((lr_intercept != old_lr_intercept) || (lr_gain != old_lr_gain)))
               {
                  //rintf(memotext, "Adjusted CH1 intercept to %.2f and slope to %.2f", lr_intercept, lr_gain);
                  //s->SendMemo2Operator(memotext);
               }
             }
         }

       }

         if( Trend_Control > 0 )
         {
              stat->ProcTrendControl(Ntargets, CurrentBaseline, CurrentTarget, CurrentOutcome, &cur_stat, LinTrend_Lrn_Rt, QuadTrend_Lrn_Rt);

            //  fprintf(estat,"CB= %2d CT= %2d CO= %2d cstf= %2d \n",CurrentBaseline,CurrentTarget,CurrentOutcome,cur_stat.trial_flag);

              ud_intercept *= cur_stat.aper;
              if (cur_stat.StdDev != 0)
                       ud_gain=cur_stat.pix/cur_stat.StdDev;

            //  ud_gain      *= cur_stat.bper;

              trend_flag= 1;

              if( cur_stat.trial_flag > 0 )
              {
                fprintf(Statfile,"%4d ",recno++);
                for(i=0;i<Ntargets;i++)       //  was   cur_stat.NumT;i++)
                        fprintf(Statfile,"%4.2f ",cur_stat.TargetPC[i]);
                fprintf(Statfile,"%1d %1d %7.4f %7.3f %7.3f %7.3f\n",CurrentTarget,CurrentOutcome,cur_stat.aper,cur_stat.pix,ud_intercept,ud_gain);
                fflush( Statfile );
             }
         }
         oldtarget= CurrentTarget;
         oldoutcome= CurrentOutcome;



        // now, update the parameters (i.e., intercept, gain) for the normalizer
        nf->UpdateParameters( ud_intercept, ud_gain, lr_intercept, lr_gain );

        if (( visualize ) && (cur_stat.update_flag > 0 ) ) //((ud_intercept != old_ud_intercept) || (ud_gain != old_ud_gain)))
               {
               // sprintf(memotext, "Adjusted CH0 intercept to %.2f and slope to %.2f", ud_intercept, ud_gain);
               // vis->SendMemo2Operator(memotext);

               for(i=0;i<Ntargets;i++)
                      StatSignal->SetValue( 0 ,i,  cur_stat.TargetPC[i] );

               vis->SetSourceID(SOURCEID_STATISTICS);
               vis->Send2Operator(StatSignal);

               cur_stat.update_flag= 0;
               }

      }



      // store the old values
      old_ud_intercept=ud_intercept;
      old_ud_gain=ud_gain;
      old_lr_intercept=lr_intercept;
      old_lr_gain=lr_gain;
      return;
}





