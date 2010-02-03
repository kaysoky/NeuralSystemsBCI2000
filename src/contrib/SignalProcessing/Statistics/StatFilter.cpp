/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop

#include <math.h>
#include <stdlib.h>

# include <stdio.h>

#include "MessageHandler.h"
#include "State.h"
#include "BCIDirectory.h"
#include "ClassFilter.h"
#include "NormalFilter.h"
#include "StatFilter.h"

//  FILE* estat;

using namespace std;

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
  Sfile(NULL),
  StatSignal( NULL ),
  nf( NULL ),
  clsf( NULL )
{
  cur_ystat.NumT= 0;
  cur_xstat.NumT= 0;

  BEGIN_PARAMETER_DEFINITIONS
    "Statistics int YTrendControl= 1 "
      "0 0 2 // Y Intercept Adapt 0 none 1 mean 2 mean prop 3 slope",
    "Statistics int SignalWinLth= 3 "
      "0 0 1000 // Trials in Signal Running Average",
    "Statistics int OutcomeDirection= 1 "
      "0 1 2 // Direction of trial outcome statistic",
    "Statistics float YMeanProportion= 1.0 "
      "0.0 0.0 2.0 // Proportion of signal mean for intercept",
    "Statistics float XMeanProportion= 1.0 "
        "0.0 0.0 2.0 // Proportion of horizontal signal intercept",
    "Statistics float YPixelsPerSec= 70 "
      "70 0 400 // Desired pixels per second",
    "Statistics float XPixelsPerSec= 0 "
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
    "Statistics int XTrendControl= 1 "
      "0 0 2  // X Intercept Adapt 0 none 1 mean 2 mean prop 3 slope",
    "Statistics int TargetWinLth= 20 "
      "0 0 100 // Length of Target % Window",
    "Statistics float TrendControlRate= 0.001 "
      "0 0.000 0.010 // Learning Rate for Linear Trend Control",

    "Statistics matrix WeightControl= 4 1 "
    "Xadapt "
    "Yadapt "
    "AdaptCode "
    "ResultCode "
      "0 0 0 0 // State Names controlling Adaptation",

    "Statistics int WeightUse= 0 "
      " 0 0 2 // Use of weights 0 = not 1= compute 2= use ",
    "Statistics float WtLrnRt= 0.001 "
      "0 0.000 0.010 // Rate of Learning for Classifier ",

     /*
    "Storage string DataDirectory= Data "
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


// estat= fopen("c:/current/log/EStat.asc","w+");

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

 //   fclose( estat );

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
  Parameter("YMean");
  Parameter("YGain");
  Parameter("XMean");
  Parameter("XGain");
  Parameter("DataDirectory");
  Parameter("SubjectSession");
  Parameter("SubjectName");

  // Files accessible?
#ifdef USE_LOGFILE
  if( estat == NULL )
    bcierr << "Cannot write to log file" << std::endl;
#endif // USE_LOGFILE
  // We should check for "Statfile" and "Sfile" accessibility here.

  // This one is not required to exist.
  // If we didn't ask for it here, the framework would not let us access
  // it later on.
  OptionalParameter( "NumberTargets", 2 );

  // States we need.
  OptionalState( Parameter( "WeightControl" )( 0, 0 ) );
  OptionalState( Parameter( "WeightControl" )( 1, 0 ) );
  OptionalState( Parameter( "WeightControl" )( 2, 0 ) );
  OptionalState( Parameter( "WeightControl" )( 3, 0 ) );
  State( "TargetCode" );
  State( "ResultCode" );
  State( "StimulusTime" );
  State( "Feedback" );
  State( "IntertrialInterval" );

  // We don't check the input signal, thus implicitly declaring that this
  // filter works for input signals of any size.
  /* no checking done */

  // This filter connects its input through to its output.
  outSignalProperties = inSignalProperties;
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the StatFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    N/A
// **************************************************************************
void StatFilter::Initialize(const SignalProperties&, const SignalProperties&)
{
  nf = GenericFilter::GetFilter< NormalFilter >();
  clsf = GenericFilter::GetFilter< ClassFilter >();

  AnsiString AName,SName,SSes,FInit;
  int visualizeyn = 0;

  static int init_flag= 0;
  trend_flag= 0;
  intercept_flag= 0;
  weight_flag= 0;

  YInterceptEstMode= Parameter("YTrendControl");
  XInterceptEstMode= Parameter("XTrendControl");
  SignalWinLth= Parameter("SignalWinLth");
  OutcomeDirection= Parameter("OutcomeDirection");
  YMeanProportion= Parameter("YMeanProportion");
  XMeanProportion= Parameter("XMeanProportion");
  yintercept= Parameter("YMean");
  ud_gain     = Parameter("YGain");
  xintercept= Parameter("XMean");
  lr_gain     = Parameter("XGain");
  Trend_Win_Lth= Parameter("TargetWinLth");
  TrendControlRate= Parameter("TrendControlRate");
  ypix= Parameter("YPixelsPerSec");
  horizpix= Parameter("XPixelsPerSec");
  visualizeyn= Parameter("VisualizeStatFiltering");

  WtRate= Parameter("WtLrnRt");

  WtControl= Parameter("WeightUse");

  FInit= Parameter("DataDirectory").c_str();
  SSes = Parameter("SubjectSession").c_str();
  SName= Parameter("SubjectName").c_str();

  // in case NumberTargets defined (maybe we use it with a task that doesn't have different # targets)
  // in this case, of course, all functions of statistics need to be turned off
  Ntargets = OptionalParameter( "NumberTargets", 2 );

  {
    string logName = BCIDirectory()
                     .SetDataDirectory( Parameter( "DataDirectory" ) )
                     .SetSubjectName( Parameter( "SubjectName" ) )
                     .SetSessionNumber( Parameter( "SubjectSession" ) )
                     .SetRunNumber( 1 )
                     .FilePath() + ".sta";

    if (Statfile) fclose(Statfile);
    Statfile= fopen(logName.c_str(),"a+");
    if( Statfile == NULL )
      bcierr << "Could not open " << logName << " for writing" << std::endl;
  }

  if( ( (YInterceptEstMode>0)||(WtControl > 0 )||(XInterceptEstMode>0))
         && init_flag < 1 )               // need to update if different targets
  {

    delete stat;
    stat= new STATISTICS;
    stat->SetNumMaxTrials(SignalWinLth);
    stat->SetOutcomeDirection(OutcomeDirection);
    stat->SetIntercept( 0, yintercept );
    stat->SetIntercept( 1, xintercept );
    if (ud_gain != 0)
      stat->SetGain( 0, ypix/ud_gain );
    else
      stat->SetGain( 0, 1000);
    if (lr_gain != 0)
      stat->SetGain( 1, horizpix/lr_gain );
    else
      stat->SetGain( 1, 1000 );
    init_flag++;
  }

    if( (YInterceptEstMode > 1 ) || (XInterceptEstMode > 1 ) )
    {
      stat->SetDTWinMaxTrials( Trend_Win_Lth );
      int num= GetBaselineHits();

      for(int i=0;i<num;i++)
      {
        stat->SetTrendControl( BaseNum[i]-1, BaseHits[i], Trend_Win_Lth );
      }
    }

  if( ( WtControl > 0 ) && ( Sfile == NULL ) )
    {
      AName= SName + "S" + SSes + ".lms";
      strcpy(OName,FName);
      strcat(OName, AName.c_str() );         // CAT vs CPY
      Sfile= fopen(OName,"a+");
      if( Sfile == NULL )
        bcierr << "Could not open " << OName << " for writing" << std::endl;
      stat->SetWeightControl( Sfile );
    }

  //       cur_ystat.bper= 1;
  cur_ystat.pix= ypix;
  cur_ystat.aper= YMeanProportion;

  cur_xstat.pix= horizpix;
  cur_xstat.aper= XMeanProportion;

  if( visualizeyn == 1 )
  {
    visualize=true;
    delete vis;
    vis= new GenericVisualization(SourceID::Statistics);
    vis->Send(CfgID::WindowTitle, "Target Trends");
    vis->Send(CfgID::MinValue, "0");
    vis->Send(CfgID::MaxValue, "1");

    static char numbuf[16] = "";
    itoa( Ntargets, numbuf, 10 );

    vis->Send(CfgID::NumSamples, numbuf );

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
  int num = 0;
  if( Parameters->Exists( "BaselineHits" ) )
    num = Parameter( "BaselineHits" )->NumRows();

  for(int i=0;i<num;i++)
  {
    BaseNum[i]= Parameter( "BaselineHits" )(i,0);
    BaseHits[i]= Parameter( "BaselineHits" )(i,1);
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

  if( Parameters->Exists( "BaselineCfg" ) )
  {
    ParamRef baselineCfg = Parameter( "BaselineCfg" );
    int numbl=baselineCfg->NumRows();
    int numbl_states= baselineCfg->NumColumns();
    CurrentBaseline=-1;
    for (int i=0; i<numbl; i++)
    {
      int match= 1;

      for(int j=0;j<numbl_states/2;j++)                  // test each
      {
        string statename=baselineCfg(i, (j*2) );
        int stateval=baselineCfg(i, (j*2)+1);

        if ( !(State(statename) == stateval) )
        match= 0;                  //    all must match


      }
      if( match == 1 ) CurrentBaseline= i;
    }

    Xadapt= OptionalState( Parameter( "WeightControl" )( 0, 0 ) );
    Yadapt= OptionalState( Parameter( "WeightControl" )( 1, 0 ) );
    AdaptCode= OptionalState( Parameter( "WeightControl" )( 2, 0 ) );
    OutcomeCode= OptionalState( Parameter( "WeightControl" )( 3, 0 ) );
#ifdef USE_LOGFILE
    fprintf(estat,"Xadapt= %d Yadapt= %d AdaptCode= %d Outcome= %d \n",Xadapt,Yadapt,AdaptCode,OutcomeCode);
#endif // USE_LOGFILE
  }
}

// **************************************************************************
// Function:   StopRun
// Purpose:    This function operates when the state running switches to 0
// **************************************************************************

void StatFilter::StopRun()
{
  char memotext[256];
  int classmode;
  int weightbin;

  if( intercept_flag > 0 )               // return to operator
  {
    intercept_flag= 0;

    // change the value of the parameter

    sprintf(memotext, "%.2f", yintercept);
    Parameter( "YMean" ) = memotext;

    sprintf(memotext, "%.2f", ud_gain);
    Parameter( "YGain" ) = memotext;

    sprintf(memotext, "%.2f", xintercept);
    Parameter( "XMean" ) = memotext;

    sprintf(memotext, "%.2f", lr_gain);
    Parameter( "XGain" ) = memotext;

  }
  if( trend_flag > 0 )         // return trends to operator
  {
    trend_flag= 0;

    sprintf(memotext, "%.6f", cur_ystat.aper);
    Parameter( "YMeanProportion" ) = memotext;

    sprintf(memotext, "%.6f",cur_xstat.aper);
    Parameter( "XMeanProportion" ) = memotext;

    sprintf(memotext, "%.2f", cur_ystat.pix);
    Parameter( "YPixelsPerSec" ) = memotext;

    sprintf(memotext, "%.2f", cur_xstat.pix);
    Parameter( "XPixelsPerSec" ) = memotext;

    Parameter( "BaselineHits" )->SetDimensions( cur_ystat.NumT, 2 );

    for(int i=0; i< cur_ystat.NumT; i++)
    {
      sprintf(memotext, "%2d",BaseNum[i] );
      Parameter( "BaselineHits")(i, 0) = memotext;

      sprintf(memotext, "%.2f", cur_ystat.TargetPC[i]);
      Parameter( "BaselineHits")(i, 1 ) = memotext;
    }
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
      Parameter("MUD")(i,weightbin) = memotext;   // 2nd or 4th value is the weight
    }

    for(int i=0;i<clsf->n_hmat;i++)
    {
      sprintf(memotext, "%.5f",clsf->wtmat[1][i]);
      Parameter("MLR")(i,weightbin) = memotext;   // 2nd or 4th value is the weight
    }
  }
}

// **************************************************************************
// Function:   Process
// Purpose:    This function applies the Stat routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    N/A
// **************************************************************************

void StatFilter::Process( const GenericSignal& input,
                                GenericSignal& output )

{
  static int recno= 1;
  static  float old_yintercept=0, old_ud_gain=0, old_xintercept=0, old_lr_gain=0;
  static int oldtarget, oldoutcome;
  float value;
  int sample;

  GetStates();

  if( YInterceptEstMode > 0 )
  {
          // channel 0 1st element

          value= input( 0, 0);

          stat->ProcRunningAvg(CurrentBaseline, 0, value, &cur_ystat);
          yintercept=cur_ystat.Intercept;

          if ( cur_ystat.StdDev > 0.001 )
                  ud_gain= ypix/cur_ystat.StdDev;

          old_yintercept= yintercept;
          old_ud_gain= ud_gain;

          intercept_flag= 1;
  }

  if( YInterceptEstMode > 1 )            // this uses sign from above !!!
    {
      if( CurrentTarget > 0 )
        stat->ProcTrendControl(0, Ntargets, CurrentBaseline, CurrentTarget,
                         OutcomeCode, Yadapt, &cur_ystat, TrendControlRate, YInterceptEstMode );

      yintercept *= cur_ystat.aper;
      if (cur_ystat.StdDev != 0)
      ud_gain=cur_ystat.pix/cur_ystat.StdDev;

      trend_flag= 1;

      if( cur_ystat.trial_flag > 0 )
      {
        fprintf(Statfile,"%4d ",recno++);
        for(int i=0;i<Ntargets;i++)       //  was   cur_ystat.NumT;i++)
        fprintf(Statfile,"%4.2f ",cur_ystat.TargetPC[i]);

        fprintf(Statfile,"%1d %1d %7.4f %7.3f %7.3f %7.3f \n",CurrentTarget,CurrentOutcome,cur_ystat.aper,cur_ystat.pix,yintercept,ud_gain);
        fflush( Statfile );
      }
    }

  if (XInterceptEstMode > 0)
  {
          // channel 1 1st element

          value= input( 1, 0 );
          stat->ProcRunningAvg(CurrentBaseline, 1, value, &cur_xstat);

          xintercept=cur_xstat.Intercept;
          if ( cur_xstat.StdDev > 0.001 )
                  lr_gain= horizpix/cur_xstat.StdDev;

          intercept_flag= 1;

      //    fprintf(estat,"lr_gain= %f  cur_xstat.StdDev= %f \n",lr_gain,cur_xstat.StdDev);
  }




    if( XInterceptEstMode > 1 )
    {
      if( CurrentTarget > 0 )
        stat->ProcTrendControl(1, Ntargets, CurrentBaseline, CurrentTarget,
                         OutcomeCode, Xadapt, &cur_xstat, TrendControlRate, XInterceptEstMode );

      xintercept *= cur_xstat.aper;
      if (cur_xstat.StdDev != 0)
      lr_gain=cur_xstat.pix/cur_xstat.StdDev;

      trend_flag= 1;

      if( cur_xstat.trial_flag > 0 )
      {
        fprintf(Statfile,"%4d ",recno++);
        for(int i=0;i<Ntargets;i++)       //  was   cur_ystat.NumT;i++)
        fprintf(Statfile,"%4.2f ",cur_xstat.TargetPC[i]);
        fprintf(Statfile,"%1d %1d %7.4f %7.3f %7.3f %7.3f \n",CurrentTarget,CurrentOutcome,cur_xstat.aper,cur_xstat.pix,xintercept,lr_gain);
        fflush( Statfile );
      }
    }

    oldtarget= CurrentTarget;
    oldoutcome= CurrentOutcome;

    // now, update the parameters (i.e., intercept, gain) for the normalizer
    nf->UpdateParameters( yintercept, ud_gain, xintercept, lr_gain );

    if (( visualize ) && (cur_ystat.update_flag > 0 ) ) //((yintercept != old_yintercept) || (ud_gain != old_ud_gain)))
    {
      for(int i=0;i<Ntargets;i++)
      StatSignal->SetValue( 0 ,i,  cur_ystat.TargetPC[i] );
      vis->Send(*StatSignal);

      cur_ystat.update_flag= 0;
    }
//  }

  // store the old values
  old_yintercept=yintercept;
  old_ud_gain=ud_gain;
  old_xintercept=xintercept;
  old_lr_gain=lr_gain;

  if( WtControl > 1 )
  {
        if( (AdaptCode == 2)||(AdaptCode == 3) )
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
        if( (AdaptCode == 1)||(AdaptCode == 3) )
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

  output = input;
}





