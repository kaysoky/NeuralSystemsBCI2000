#include "PCHIncludes.h"
#pragma hdrstop

#include <math.h>
#include <stdlib.h>

#define USE_LOGFILE

#ifdef USE_LOGFILE
# include <stdio.h>
#endif // USE_LOGFILE

#include "UState.h"
#include "BCIDirectry.h"
#include "ClassFilter.h"
#include "NormalFilter.h"
#include "StatFilter.h"

#ifdef USE_LOGFILE
FILE* estat;
#endif // USE_LOGFILE

RegisterFilter( StatFilter, 2.E );

// **************************************************************************
// Function:   StatFilter
// Purpose:    This is the constructor for the StatFilter Class
//             It is the Statistics module
//                      it does dynamic parameter setting
//             it requests parameters by adding parameters to the parameter list
//             it also requests states by adding them to the state list
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
// **************************************************************************
StatFilter::StatFilter()
: vis( NULL ),
  stat( NULL ),
  Statfile( NULL ),
  StatSignal( NULL ),
  nf( NULL ),
  clsf( NULL )
{
  cur_stat.NumT= 0;
  cur_lr_stat.NumT= 0;

  BEGIN_PARAMETER_DEFINITIONS
    "Statistics int InterceptControl= 1 "
      "0 0 2 // Online adaption of Intercept 1 = Up/Dn  2 = Up/Dn + Le/Ri",
    "Statistics int InterceptLength= 3 "
      "0 0 1000 // Length of time for running average",
    "Statistics float InterceptProportion= 1.0 "
      "0.0 0.0 2.0 // Proportion of signal mean for intercept",
    "Statistics float HorizInterceptProp= 1.0 "
        "0.0 0.0 2.0 // Proportion of horizontal signal intercept",
    "Statistics float DesiredPixelsPerSec= 70 "
      "70 0 400 // Desired pixels per second",
    "Statistics float LRPixelsPerSec= 0 "
      "70 0 400 // Horizontal Pixel Rate",
    "Visualize int VisualizeStatFiltering= 1 "
      "0 0 1  // visualize Stat filtered signals (0=no 1=yes)",
    "Statistics matrix BaselineCfg= 2 2 "
      "TargetCode 1 "
      "TargetCode 2 "
      "0 0 0 // states to watch for baseline",
    "Statistics matrix BaselineHits= 2 2 "
      "1 0.50 "
      "2 0.50 "
      "0 0 0 // proportion correct for each target",
    "Statistics int TrendControl= 1 "
      "0 0 2  // Online adaption of % Correct Trend 1= Lin 2 = Quad",
    "Statistics int HorizTrendControl= 1 "
      "0 0 2  // Adaption of Horizontal % Correct Trend 1= Lin 2 = Quad",
    "Statistics int TrendWinLth= 20 "
      "0 0 100 // Length of % Correct Window",
    "Statistics float LinTrendLrnRt= 0.001 "
      "0 0.000 0.010 // Learning Rate for Linear Trend Control",
    "Statistics float QuadTrendLrnRt= 0.001 "
      "0 0 0.010 // Learning Rate for Linear Trend Control",

    "Statistics matrix WeightControl= 3 1 "
    "Xadapt "
    "Yadapt "
    "AdaptCode "
      "0 0 0 // State Names controlling Classifier Adaptation",

    "Statistics int WeightUse= 0 "
      " 0 0 2 // Use of weights 0 = not 1= compute 2= use ",
    "Statistics float WtLrnRt= 0.001 "
      "0 0.000 0.010 // Rate of Learning for Classifier ",

     /*
    "Storage string FileInitials= Data "
      "Data a z // Initials of file name (max. 8 characters)",
    "Storage string SubjectName= Name "
      "Name a z // subject alias (max. 8 characters)",
    "Storage string SubjectSession= 001 "
      "001 0 999 // session number (max. 3 characters)",
     */
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
    "IntCompute 2 0 0 0",
  END_STATE_DEFINITIONS

#ifdef USE_LOGFILE
  estat= fopen("EStat.asc","w+");
#endif // USE_LOGFILE
}


// **************************************************************************
// Function:   ~StatFilter
// Purpose:    This is the destructor for the StatFilter Stat
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
StatFilter::~StatFilter()
{
  delete vis;
  delete stat;
  delete StatSignal;
  if (Statfile) fclose( Statfile );
#ifdef USE_LOGFILE
  fclose( estat );
#endif // USE_LOGFILE
}

// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistence with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
void StatFilter::Preflight( const SignalProperties& inSignalProperties,
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

  // States we need.
  OptionalState( Parameter( "WeightControl", 0, 0 ) );
  OptionalState( Parameter( "WeightControl", 1, 0 ) );
  OptionalState( Parameter( "WeightControl", 2, 0 ) );
  State( "TargetCode" );
  State( "ResultCode" );
  State( "StimulusTime" );
  State( "Feedback" );
  State( "IntertrialInterval" );

  // We don't check the input signal, thus implicitly declaring that this
  // filter works for input signals of any size.
  /* no checking done */

  // This filter does not use its output signal argument, so we specify
  // minimal requirements.
  outSignalProperties = SignalProperties( 0, 0, 0 );
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the StatFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    N/A
// **************************************************************************
void StatFilter::Initialize()
{
  nf = GenericFilter::GetFilter< NormalFilter >();
  clsf = GenericFilter::GetFilter< ClassFilter >();

  AnsiString AName,SName,SSes,FInit;
  int visualizeyn = 0;

  static int init_flag= 0;
  static int wt_init_flag= 0;
  trend_flag= 0;
  intercept_flag= 0;
  weight_flag= 0;

  InterceptEstMode= Parameter("InterceptControl");
  InterceptLength= Parameter("InterceptLength");
  InterceptProportion= Parameter("InterceptProportion");
  HorizInterceptProp= Parameter("HorizInterceptProp");
  ud_intercept= Parameter("UD_A");
  ud_gain     = Parameter("UD_B");
  lr_intercept= Parameter("LR_A");
  lr_gain     = Parameter("LR_B");
  Trend_Control= Parameter("TrendControl");
  HorizTrend_Control= Parameter("HorizTrendControl");
  Trend_Win_Lth= Parameter("TrendWinLth");
  LinTrend_Lrn_Rt= Parameter("LinTrendLrnRt");
  QuadTrend_Lrn_Rt= Parameter("QuadTrendLrnRt");
  desiredpix= Parameter("DesiredPixelsPerSec");
  horizpix= Parameter("LRPixelsPerSec");
  visualizeyn= Parameter("VisualizeStatFiltering");

//  WtControl= Parameter("WeightControl");

  WtRate= Parameter("WtLrnRt");

  WtControl= Parameter("WeightUse");

//  LRWtControl= Parameter("LRWeightControl");
//  LRWtRate= Parameter("LRWtLrnRt");


  FInit= ( const char* )Parameter("FileInitials");
  SSes = ( const char* )Parameter("SubjectSession");
  SName= ( const char* )Parameter("SubjectName");

  // in case NumberTargets defined (maybe we use it with a task that doesn't have different # targets)
  // in this case, of course, all functions of statistics need to be turned off
  Ntargets = OptionalParameter( 2, "NumberTargets" );

  {
    BCIDtry bcidtry;
    bcidtry.SetDir( FInit.c_str() );
    bcidtry.ProcPath();
    bcidtry.SetName( SName.c_str() );
    bcidtry.SetSession( SSes.c_str() );

    strcpy(FName, bcidtry.ProcSubDir() );
    strcat(FName,"\\");

    AName= SName + "S" + SSes + ".sta";
    strcpy(OName,FName);
    strcat(OName, AName.c_str() );         // CAT vs CPY
    if (Statfile) fclose(Statfile);
    Statfile= fopen(OName,"a+");
  }

  if( ( (InterceptEstMode>0)||(Trend_Control>0)||(WtControl > 0 ) ) && init_flag < 1 )               // need to update if different targets
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
      stat->SetGain( 1, horizpix/lr_gain );
    else
      stat->SetGain( 1, 1000 );

    if( Trend_Control > 0 )
    {
      stat->SetDTWinMaxTrials( Trend_Win_Lth );
      int num= GetBaselineHits();

      for(int i=0;i<num;i++)
      {
        stat->SetTrendControl( i, BaseHits[i], Trend_Win_Lth );
      }
    }



    init_flag++;
  }

  if( ( WtControl > 0 ) && ( wt_init_flag == 0 ) )
    {
      AName= SName + "S" + SSes + ".lms";
      strcpy(OName,FName);
      strcat(OName, AName.c_str() );         // CAT vs CPY
      if (Sfile) fclose(Sfile);
      Sfile= fopen(OName,"a+");
      stat->SetWeightControl( Sfile );
      wt_init_flag= 1;
    }

  //       cur_stat.bper= 1;
  cur_stat.pix= desiredpix;
  cur_stat.aper= InterceptProportion;

  cur_lr_stat.pix= horizpix;
  cur_lr_stat.aper= HorizInterceptProp;

  if( visualizeyn == 1 )
  {
    visualize=true;
    delete vis;
    vis= new GenericVisualization;
    vis->SetSourceID(SOURCEID_STATISTICS);
    vis->SendCfg2Operator(SOURCEID_STATISTICS, CFGID_WINDOWTITLE, "Target Trends");
    vis->SendCfg2Operator(SOURCEID_STATISTICS, CFGID_MINVALUE, "0");
    vis->SendCfg2Operator(SOURCEID_STATISTICS, CFGID_MAXVALUE, "1");

    static char numbuf[16] = "";
    itoa( Ntargets, numbuf, 10 );

    vis->SendCfg2Operator(SOURCEID_STATISTICS, CFGID_NUMSAMPLES, numbuf );

    delete StatSignal;
    StatSignal= new GenericSignal( 1, Ntargets );
  }
  else
  {
    visualize=false;
  }
}

//**************************************************************************************

int StatFilter::GetBaselineHits( void )
{
  PARAM  *paramptr= Parameters->GetParamPtr("BaselineHits");
  if( !paramptr ) return(0);
  int num= paramptr->GetNumValuesDimension1();

  for(int i=0;i<num;i++)
  {
    BaseNum[i]= atoi( paramptr->GetValue(i,0) );
    BaseHits[i]= atof( paramptr->GetValue(i,1) );
  }
  return(num);
}


//**************************************************************************************

void StatFilter::GetStates( void )
{
  CurrentTarget=       State("TargetCode");
  CurrentOutcome=      State("ResultCode");
  CurrentStimulusTime= State("StimulusTime");
  CurrentFeedback=     State("Feedback");
  CurrentIti=          State("IntertrialInterval");

  PARAM* paramptr=Parameters->GetParamPtr("BaselineCfg");
  if (!paramptr) return;

  int numbl=paramptr->GetNumValuesDimension1();
  int numbl_states= paramptr->GetNumValuesDimension2();
  CurrentBaseline=-1;
  for (int i=0; i<numbl; i++)
  {
    int match= 1;

    for(int j=0;j<numbl_states/2;j++)                  // test each
    {
      const char* statename=paramptr->GetValue(i, (j*2) );
      int stateval=atoi(paramptr->GetValue(i, (j*2)+1) );

      if ( !(State(statename) == stateval) )
      match= 0;                  //    all must match


    }
    if( match == 1 ) CurrentBaseline= i;
  }

  Xadapt= OptionalState( Parameter( "WeightControl", 0, 0 ) );
  Yadapt= OptionalState( Parameter( "WeightControl", 1, 0 ) );
  AdaptCode= OptionalState( Parameter( "WeightControl", 2, 0 ) );

#ifdef USE_LOGFILE
     fprintf(estat,"Xadapt= %d Yadapt= %d AdaptCode= %d \n",Xadapt,Yadapt,AdaptCode);
#endif // USE_LOGFILE

}

// **************************************************************************
// Function:   Resting
// Purpose:    This function operates when the state running = 0
// **************************************************************************

void StatFilter::Resting()
{
  char memotext[256];
  int classmode;
  int weightbin;

  if( intercept_flag > 0 )               // return to operator
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
    Parameter( "UD_A" ) = memotext;

    sprintf(memotext, "%.2f", ud_gain);
    Parameter( "UD_B" ) = memotext;

    sprintf(memotext, "%.2f", lr_intercept);
    Parameter( "LR_A" ) = memotext;

    sprintf(memotext, "%.2f", lr_gain);
    Parameter( "LR_B" ) = memotext;

    Corecomm->StartSendingParameters();

    Corecomm->PublishParameter( Parameters->GetParamPtr("UD_A") );
    Corecomm->PublishParameter( Parameters->GetParamPtr("UD_B") );

    Corecomm->PublishParameter( Parameters->GetParamPtr("LR_A") );
    Corecomm->PublishParameter( Parameters->GetParamPtr("LR_B") );

    Corecomm->StopSendingParameters();
  }
  if( trend_flag > 0 )         // return trends to operator
  {
    trend_flag= 0;

    sprintf(memotext, "%.4f", cur_stat.aper);
    Parameter( "InterceptProportion" ) = memotext;

    sprintf(memotext, "%.4f",cur_lr_stat.aper);
    Parameter( "HorInterceptProp" ) = memotext;


    //     sprintf(memotext, "%.2f", cur_stat.bper * desiredpix);
    sprintf(memotext, "%.2f", cur_stat.pix);
    Parameter( "DesiredPixelsPerSec" ) = memotext;

    sprintf(memotext, "%.2f", cur_lr_stat.pix);
    Parameter( "LRPixelsPerSec" ) = memotext;

    Parameter( "BaselineHits" )->SetDimensions( cur_stat.NumT, 2 );

    for(int i=0; i< cur_stat.NumT; i++)
    {
      sprintf(memotext, "%d", BaseNum[i]);
      Parameter( "BaselineHits", i, 0) = memotext;

      sprintf(memotext, "%.2f", cur_stat.TargetPC[i]);
      Parameter( "BaselineHits", i, 1 ) = memotext;
    }

    Corecomm->StartSendingParameters();

    Corecomm->PublishParameter( Parameters->GetParamPtr("InterceptProportion") );
    Corecomm->PublishParameter( Parameters->GetParamPtr("DesiredPixelsPerSec") );

    Corecomm->StopSendingParameters();
  }

  classmode= Parameter("ClassMode");   // ytest classifier type
  weightbin= 2;
  if( classmode == 2 ) weightbin= 4;

  if( weight_flag > 0 )            // return weights to operator
  {
    weight_flag= 0;
    for(int i=0;i<clsf->n_vmat;i++)
    {
      sprintf(memotext, "%.5f",clsf->wtmat[0][i]);
      Parameter("MUD",i,weightbin) = memotext;   // 2nd or 4th value is the weight
    }
    Corecomm->StartSendingParameters();
    Corecomm->PublishParameter( Parameters->GetParamPtr("MUD") );
    Corecomm->StopSendingParameters();
fprintf(estat,"Horizontal weights \n");
    for(int i=0;i<clsf->n_hmat;i++)
    {

fprintf(estat,"%2d %8.5f \n",i,clsf->wtmat[1][i]);

      sprintf(memotext, "%.5f",clsf->wtmat[1][i]);
      Parameter("MLR",i,weightbin) = memotext;   // 2nd or 4th value is the weight
    }
    Corecomm->StartSendingParameters();
    Corecomm->PublishParameter( Parameters->GetParamPtr("MLR") );
    Corecomm->StopSendingParameters();
  }
}

// **************************************************************************
// Function:   Process
// Purpose:    This function applies the Stat routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    N/A
// **************************************************************************
void StatFilter::Process( const GenericSignal *SignalE,
                                GenericSignal */*SignalF*/ )
{
  static int recno= 1;
  static  float old_ud_intercept=0, old_ud_gain=0, old_lr_intercept=0, old_lr_gain=0;
  static int oldtarget, oldoutcome;

  // actually perform the Stat Filtering on the NormalFilter output signal
/* What does the above line say?
   The actual input signal is the NormalFilter _input_ signal.
   The output signal argument is unused in this filter. */

  GetStates();

  if( InterceptEstMode >0 )
  {
    int sample= 0;

    for(size_t in_channel=0; in_channel<SignalE->Channels(); in_channel++)
    {
      float value= SignalE->GetValue(in_channel, sample);

      if (in_channel == 0)
      {
        stat->ProcRunningAvg(CurrentBaseline, in_channel, value, &cur_stat);
        ud_intercept=cur_stat.Intercept;
        if (cur_stat.StdDev != 0)
        ud_gain=desiredpix/cur_stat.StdDev;

        old_ud_intercept= ud_intercept;
        old_ud_gain= ud_gain;

      }

      intercept_flag= 1;

      if (InterceptEstMode > 1)
      {
        if (in_channel == 1)
        {
          stat->ProcRunningAvg(CurrentBaseline, in_channel, value, &cur_lr_stat);
 
          lr_intercept=cur_lr_stat.Intercept;
          if (cur_lr_stat.StdDev != 0)
            lr_gain=horizpix/cur_lr_stat.StdDev;

          if (( visualize ) && ((lr_intercept != old_lr_intercept) || (lr_gain != old_lr_gain)))
          {
            //char memotext[512];
            //sprintf(memotext, "Adjusted CH1 intercept to %.2f and slope to %.2f", lr_intercept, lr_gain);
            //s->SendMemo2Operator(memotext);
          }
        }
      }
    }

    if( Trend_Control > 0 )
    {
      stat->ProcTrendControl(Ntargets, CurrentBaseline, CurrentTarget, CurrentOutcome, &cur_stat, LinTrend_Lrn_Rt, QuadTrend_Lrn_Rt);

      ud_intercept *= cur_stat.aper;
      if (cur_stat.StdDev != 0)
      ud_gain=cur_stat.pix/cur_stat.StdDev;

      trend_flag= 1;

      if( cur_stat.trial_flag > 0 )
      {
        fprintf(Statfile,"%4d ",recno++);
        for(int i=0;i<Ntargets;i++)       //  was   cur_stat.NumT;i++)
        fprintf(Statfile,"%4.2f ",cur_stat.TargetPC[i]);
        fprintf(Statfile,"%1d %1d %7.4f %7.3f %7.3f %7.3f \n",CurrentTarget,CurrentOutcome,cur_stat.aper,cur_stat.pix,ud_intercept,ud_gain);
        fflush( Statfile );
      }
    }
    oldtarget= CurrentTarget;
    oldoutcome= CurrentOutcome;

    // now, update the parameters (i.e., intercept, gain) for the normalizer
    nf->UpdateParameters( ud_intercept, ud_gain, lr_intercept, lr_gain );

    if (( visualize ) && (cur_stat.update_flag > 0 ) ) //((ud_intercept != old_ud_intercept) || (ud_gain != old_ud_gain)))
    {
      for(int i=0;i<Ntargets;i++)
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

  if( WtControl > 1 )
  {
        if( (AdaptCode == 1)||(AdaptCode == 3) )
        {
        // control vertical (Y) Weights !!
        stat->ProcWeightControl(        Yadapt,            // Y value assigned target
                                        CurrentFeedback,   // trial interval
                                        clsf->n_vmat,      // # vertical control elements
                                        WtControl,         // control mode (1 vs 2 dim)
                                        clsf->feature[0], // pointer to linear equation matrx
                                        clsf->wtmat[0],   // matrix of weights
                                        WtRate,           // learning rate
                                        1 );              // chan code for Y
                weight_flag= 1;
        }
        if( (AdaptCode == 2)||(AdaptCode == 3) )
        {
        // control horizontal (X) Weight !!
        stat->ProcWeightControl(        Xadapt,            // X value assigned target
                                        CurrentFeedback,   // trial interval
                                        clsf->n_hmat,      // # horizontal control elements
                                        WtControl,         // control mode (1 vs 2 dim)
                                        clsf->feature[1], // pointer to linear equation matrx
                                        clsf->wtmat[1],   // matrix of weights
                                        WtRate,
                                        2 );              // chan code for x
                weight_flag= 1;

        }
  }
}





