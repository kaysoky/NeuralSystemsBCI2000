//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UFilterHandling.h"
#include "StatFilter.h"
#include "BCIDirectry.h"

#include <math.h>

// #include <stdio.h>
#include <stdlib.h>
// FILE *Statfile;

// FILE *estat;


//---------------------------------------------------------------------------

#pragma package(smart_init)

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
StatFilter::StatFilter(PARAMLIST *plist, STATELIST *slist)
{

char line[512];

statelist= slist;
cur_stat.NumT= 0;

// instance=my_instance;

 vis=NULL;
 stat=NULL;

 Statfile=NULL;
 StatSignal=NULL;

 strcpy(line,"Statistics int InterceptControl= 1 0 0 1  // Online adaption of Intercept 1 = Up/Dn  2 = Up/Dn + Le/Ri");
 plist->AddParameter2List(line,strlen(line) );

 strcpy(line,"Statistics int InterceptLength= 3 0 0 1 // Length of time for running average");
 plist->AddParameter2List(line,strlen(line) );

 strcpy(line, "Statistics float InterceptProportion= 1.0 0.0 0.0 2.0 // Proportion of signal mean for intercept");
 plist->AddParameter2List(line, strlen(line) );

 strcpy(line,"Statistics float DesiredPixelsPerSec= 70 70 0 400 // Desired pixels per second");
 plist->AddParameter2List(line,strlen(line) );

 strcpy(line, "Visualize int VisualizeStatFiltering= 1 0 0 1  // visualize Stat filtered signals (0=no 1=yes)");
 plist->AddParameter2List( line, strlen(line) );

 strcpy(line, "Statistics matrix BaselineCfg= 2 2 TargetCode 1 TargetCode 2 0 0 1 // states to watch for baseline");
 plist->AddParameter2List( line, strlen(line) );

 strcpy(line, "Statistics int TrendControl= 1 0 0 1  // Online adaption of % Correct Trend 1= Lin 2 = Quad");
 plist->AddParameter2List( line, strlen(line) );

 strcpy(line, "Statistics int TrendWinLth= 20 0 0 100 // Length of % Correct Window");
 plist->AddParameter2List( line, strlen(line) );

 strcpy(line, "Statistics float LinTrendLrnRt= 0.001 0 0.000 0.010 // Learning Rate for Linear Trend Control");
 plist->AddParameter2List( line, strlen(line) );

 strcpy(line, "Statistics float QuadTrendLrnRt= 0.001 0 0 0.010 // Learning Rate for Linear Trend Control");
 plist->AddParameter2List( line, strlen(line) );

 strcpy(line,"Statistics int WeightControl= 0 0 0 1 // Classifier Adaptation 0= no  1= Compute  2= use");
 plist->AddParameter2List( line, strlen(line) );

 strcpy(line,"Statistics float WtLrnRt= 0.001 0 0.000 0.010 // Rate of Learning for Classifier ");
 plist->AddParameter2List( line, strlen( line ) );


 /*
 strcpy(line, "Storage string FileInitials= Data Data a z // Initials of file name (max. 8 characters)");
 plist->AddParameter2List(line, strlen(line));
 strcpy(line, "Storage string SubjectName= Name Name a z // subject alias (max. 8 characters)");
 plist->AddParameter2List(line, strlen(line));
 strcpy(line, "Storage string SubjectSession= 001 001 0 999 // session number (max. 3 characters)");
 plist->AddParameter2List(line, strlen(line));
 */

 statelist->AddState2List("IntCompute 2 0 0 0 \n");

// estat= fopen("EStat.asc","w+");

}


// **************************************************************************
// Function:   ~StatFilter
// Purpose:    This is the destructor for the StatFilter Stat
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
StatFilter::~StatFilter()
{
  if( vis ) delete vis;
     vis= NULL;

  if (stat) delete stat;
  stat=NULL;

  if (StatSignal) delete StatSignal;
  StatSignal=NULL;

  if (Statfile) fclose( Statfile );
  Statfile=NULL;
  // fclose( estat );
}


// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the StatFilter
// Parameters: paramlist - list of the (fully configured) parameter list
//             new_statevector - pointer to the statevector (which also has a pointer to the list of states)
//             new_corecomm - pointer to the communication object to the operator
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
int StatFilter::Initialize(PARAMLIST *plist, STATEVECTOR *new_statevector, CORECOMM *new_corecomm)
{
        AnsiString AName,SName,SSes,FInit;
        int visualizeyn;
        char slash[2];
        char numbuf[16];
        BCIDtry *bcidtry;

        static int init_flag= 0;
        trend_flag= 0;
        intercept_flag= 0;
        weight_flag= 0;

 statevector=new_statevector;
 corecomm=new_corecomm;
 paramlist=plist;

 try // in case one of the parameters is not defined (should always be, since we requested them)
  {

       InterceptEstMode= atoi(paramlist->GetParamPtr("InterceptControl")->GetValue());
       InterceptLength= atoi(paramlist->GetParamPtr("InterceptLength")->GetValue());
       InterceptProportion= atof(paramlist->GetParamPtr("InterceptProportion")->GetValue() );
       ud_intercept= atof(paramlist->GetParamPtr("UD_A")->GetValue());
       ud_gain     = atof(paramlist->GetParamPtr("UD_B")->GetValue());
       lr_intercept= atof(paramlist->GetParamPtr("LR_A")->GetValue());
       lr_gain     = atof(paramlist->GetParamPtr("LR_B")->GetValue());
       Trend_Control= atoi(paramlist->GetParamPtr("TrendControl")->GetValue());
       Trend_Win_Lth= atoi(paramlist->GetParamPtr("TrendWinLth")->GetValue());
       LinTrend_Lrn_Rt= atof(paramlist->GetParamPtr("LinTrendLrnRt")->GetValue() );
       QuadTrend_Lrn_Rt= atof(paramlist->GetParamPtr("QuadTrendLrnRt")->GetValue() );
       desiredpix= atof(paramlist->GetParamPtr("DesiredPixelsPerSec")->GetValue());
       visualizeyn= atoi(paramlist->GetParamPtr("VisualizeStatFiltering")->GetValue() );
       Ntargets= atoi(plist->GetParamPtr("NumberTargets")->GetValue() );

       WtControl= atoi(plist->GetParamPtr("WeightControl")->GetValue() );
       WtRate= atof(plist->GetParamPtr("WtLrnRt")->GetValue() );

       FInit= AnsiString (paramlist->GetParamPtr("FileInitials")->GetValue());
       SSes = AnsiString (paramlist->GetParamPtr("SubjectSession")->GetValue());
       SName= AnsiString (paramlist->GetParamPtr("SubjectName")->GetValue());

  }
 catch(...)
  { return(0); }

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
  strcpy(OName,FName);
  strcat(OName, AName.c_str() );         // CAT vs CPY
  if (Statfile) fclose(Statfile);
  Statfile= fopen(OName,"a+");


 if( ( (InterceptEstMode>0)||(Trend_Control>0)||(WtControl>0) ) && init_flag < 1 )               // need to update if different targets
 {
        if (stat) delete stat;
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

        if( WtControl > 0 )
        {
                AName= SName + "S" + SSes + ".lms";
                strcpy(OName,FName);
                strcat(OName, AName.c_str() );         // CAT vs CPY
                if (Sfile) fclose(Sfile);
                Sfile= fopen(OName,"a+");
                stat->SetWeightControl( Sfile );
        }

        init_flag++;
 }

 //       cur_stat.bper= 1;
        cur_stat.pix= desiredpix;
        cur_stat.aper= InterceptProportion;

 if( visualizeyn == 1 )
 {
        visualize=true;
        if (vis) delete vis;
        vis= new GenericVisualization( paramlist, corecomm );
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

  if (bcidtry) delete bcidtry;

  return(1);
}


void StatFilter::GetStates( void )
{
PARAM   *paramptr;
int     i, numbl;
int     j,numbl_states;
char    *statename;
int     stateval;
int     match;

 CurrentTarget=       statevector->GetStateValue("TargetCode");
 CurrentOutcome=      statevector->GetStateValue("ResultCode");
 CurrentStimulusTime= statevector->GetStateValue("StimulusTime");
 CurrentFeedback=     statevector->GetStateValue("Feedback");
 CurrentIti=          statevector->GetStateValue("IntertrialInterval");



 paramptr=paramlist->GetParamPtr("BaselineCfg");
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

                if ( !(statevector->GetStateValue(statename) == stateval) )
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

int StatFilter::Resting( ClassFilter *clsf )
{
        char memotext[256];
        int i;

        if( intercept_flag > 0 )               // return to operator
        {
                intercept_flag= 0;

                // change the value of the parameter

                sprintf(memotext, "%.2f\r", ud_intercept);
                paramlist->GetParamPtr("UD_A")->SetValue( memotext );

                sprintf(memotext, "%.2f\r", ud_gain);
                paramlist->GetParamPtr("UD_B")->SetValue( memotext );


                corecomm->StartSendingParameters();

                corecomm->PublishParameter( paramlist->GetParamPtr("UD_A") );
                corecomm->PublishParameter( paramlist->GetParamPtr("UD_B") );

                corecomm->StopSendingParameters();
        }
        if( trend_flag > 0 )         // return trends to operator
        {
                trend_flag= 0;

                sprintf(memotext, "%.4f\r", cur_stat.aper);
                paramlist->GetParamPtr("InterceptProportion")->SetValue( memotext );


    //     sprintf(memotext, "%.2f\r", cur_stat.bper * desiredpix);
                sprintf(memotext, "%.2f\r", cur_stat.pix);
                paramlist->GetParamPtr("DesiredPixelsPerSec")->SetValue( memotext );


                corecomm->StartSendingParameters();

                corecomm->PublishParameter( paramlist->GetParamPtr("InterceptProportion") );
                corecomm->PublishParameter( paramlist->GetParamPtr("DesiredPixelsPerSec") );

                corecomm->StopSendingParameters();
        }
        if( weight_flag > 0 )            // return wieghts to operator
        {

                weight_flag= 0;
                for(i=0;i<clsf->n_vmat;i++)
                {
                        sprintf(memotext, "%.5f",clsf->wtmat[0][i]);
                        paramlist->GetParamPtr("MUD")->SetValue(memotext,i,4);   // 4th value is the weight
                }
                corecomm->StartSendingParameters();
                corecomm->PublishParameter( paramlist->GetParamPtr("MUD") );
                corecomm->StopSendingParameters();

        }

        return(0);              // no errors?
}

// **************************************************************************
// Function:   Process
// Purpose:    This function applies the Stat routine
// Parameters: input  - input signal for the
//             output - output signal for this filter
// Returns:    0 ... on error
//             1 ... no error
// **************************************************************************
int StatFilter::Process(      //  CalibrationFilter       *calf,
                              //  SpatialFilter           *sf
                              //  TemporalFilter          *tf,

                                ClassFilter   *clsf,
                                GenericSignal *SignalE,
                                NormalFilter  *nf,
                                GenericSignal *SignalF
                        )
{
static int recno= 1;
int i;
int     in_channel, out_channel,sample;
float   value;
char    memotext[512];
static  float old_ud_intercept=0, old_ud_gain=0, old_lr_intercept=0, old_lr_gain=0;
static int oldtarget, oldoutcome;

 // actually perform the Stat Filtering on the NormalFilter output signal

     GetStates();

     if( InterceptEstMode >0 )
     {
        sample= 0;

        for(in_channel=0; in_channel<SignalE->Channels; in_channel++)
         {
         value= SignalE->GetValue(in_channel, sample);

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
               stat->ProcRunningAvg(CurrentBaseline, in_channel, value, &cur_stat);
               stat->ProcRunningAvg(CurrentBaseline, in_channel, value, &cur_stat);
               lr_intercept=cur_stat.Intercept;
               if (cur_stat.StdDev != 0)
                  lr_gain=desiredpix/cur_stat.StdDev;

               if (( visualize ) && ((lr_intercept != old_lr_intercept) || (lr_gain != old_lr_gain)))
               {
                  //rintf(memotext, "Adjusted CH1 intercept to %.2f and slope to %.2f\r", lr_intercept, lr_gain);
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
                for(i=0;i<Ntargets;i++)       //  was   cur_stat.NumT;i++)
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

      if( WtControl > 0 )
      {
        // control Weight !!

        stat->ProcWeightControl(        Ntargets,
                                        CurrentTarget,
                                        CurrentFeedback,
                                        clsf->n_vmat,     // # vertical control elements
                                        WtControl,        // control mode (1 vs 2 dim)
                                        clsf->feature[0], // pointer to linear equation matrx
                                        clsf->wtmat[0],   // matrix of weights
                                        WtRate,
                                        WtControl
                               ) ;
         weight_flag= 1;
      }
      return(1);
}





