/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------
//  #define USE_LOGFILE
//---------------------------------------------------------------------------
#include <math.h>
#include <stdio.h>
#include "Statistics.h"


//  FILE *statf;

void get_oc( float *l, float *q, int n  )
{

	switch( n )
	{
                case 2:
                        l[0]= -1; l[1]= 1;
                        q[0]= 0;  q[1]= 0;
                        break;
		case 3:
			l[0]= -1; l[1]=  0; l[2]=  1;
			q[0]=  1; q[1]= -2; q[2]=  1;
			break;
		case 4:
			l[0]= -3; l[1]= -1; l[2]=  1; l[3]=  3;
			q[0]=  1; q[1]= -1; q[2]= -1; q[3]=  1;
			break;
		case 5:
			l[0]= -2; l[1]= -1; l[2]=  0; l[3]=  1; l[4]=  2;
			q[0]=  2; q[1]= -1; q[2]= -2; q[3]= -1; q[4]=  2;
			break;
		case 6:
			l[0]= -5; l[1]= -3; l[2]= -1; l[3]=  1; l[4]=  3; l[5]=  5;
			q[0]=  5; q[1]= -1; q[2]= -4; q[3]= -4; q[4]= -1; q[5]=  1;
			break;
	}
}


CIRCBUF::CIRCBUF()
{
int i;

 for (i=0; i<MAX_TRIALS; i++)
  {
  trialdata[i]=NULL;
  trialnum[i]=-1000;
  trialsamples[i]=0;
  }

 trials=0;

 NewTrial(0);
}


CIRCBUF::~CIRCBUF()
{
int i;

 for (i=0; i<MAX_TRIALS; i++)
  {
  if (trialdata[i])
     DeleteTrial(trialnum[i]);
  }
}


void CIRCBUF::PushVal(float val)
{
int i;

 // if we already have a trial with this number, delete it first
 for (i=0; i<MAX_TRIALS; i++)
  if (trialdata[i])
     if (trialnum[i] == trials)
        {
        trialdata[i][trialsamples[i]]=val;
        trialsamples[i]++;
        if (trialsamples[i] >= MAX_SAMPLESPERTRIAL)
           trialsamples[i]=MAX_SAMPLESPERTRIAL-1;
        }
}


void CIRCBUF::NextTrial()
{
 // delete the oldest trial (the one we don't need anymore based on maxtrials)
 DeleteTrial(trials-maxtrials-1);

 trials++;

 // now create space for a new trial
 NewTrial(trials);
}


// calculates the average of all trials in the buffer
float CIRCBUF::CalculateAllTrialAverage()
{
int     trial, sample;
float   accum, trialaccum;
int     trialcount, count;

 accum=0;
 count=0;

 // go through all the trials in the buffer
 for (trial=0; trial<MAX_TRIALS; trial++)
  {
  // if a trial exists at this position
  if (trialdata[trial])
     {
     trialaccum=0;
     trialcount=0;
     // calculate the mean of this trial
     for (sample=0; sample<trialsamples[trial]; sample++)
      {
      trialaccum+=trialdata[trial][sample];
      trialcount++;
      }
     // take the mean of this trial to calculate the overall mean (= the mean of the trial mean)
     if (trialcount > 0)
        {
        accum+=trialaccum/(float)trialcount;
        count++;
        }
     }
  }

 if (count == 0)
    return(0.0);

 return(accum/(float)count);
}


// calculates the average standard deviation of all trials in the buffer
float CIRCBUF::CalculateAllTrialStdDev()
{
int     trial, sample;
float   accum, trialaccum, trialaccumsq, res;
int     trialcount, count;

 accum=0;
 count=0;

 // go through all the trials in the buffer
 for (trial=0; trial<MAX_TRIALS; trial++)
  {
  // if a trial exists at this position
  if (trialdata[trial])
     {
     trialaccum=trialaccumsq=0;
     trialcount=0;
     // calculate the mean of this trial
     for (sample=0; sample<trialsamples[trial]; sample++)
      {
      trialaccum+=trialdata[trial][sample];
      trialaccumsq+=trialdata[trial][sample]*trialdata[trial][sample];
      trialcount++;
      }
     // take the stddev of this trial to calculate the overall stddev (= the mean of the trial's stddevs)
     if (trialcount > 1)
        {
        res=((float)trialcount*trialaccumsq-(float)trialaccum*trialaccum)/((float)trialcount*((float)trialcount-1));  // stddev=sqrt((n*sum(x^2)-(sum(x))^2)/(n*(n-1)))
        if (res < 0.0001) res=0.0001;  // avoid sqrt(-x)
        accum+=sqrt(res);
        count++;
        }
     }
  }

 if (count == 0)
    return(0);

 return(accum/(float)count);
}


// calculates the total number of samples in all the trials
int CIRCBUF::GetAllTrialSampleNum()
{
int trial, count;

 count=0;
 for (trial=0; trial<MAX_TRIALS; trial++)
  if (trialdata[trial])
     count+=trialsamples[trial];

 return(count);
}


void CIRCBUF::NewTrial(int newtrialnum)
{
int i;

 // if we already have a trial with this number, delete it first
 for (i=0; i<MAX_TRIALS; i++)
  if (trialdata[i])
     if (trialnum[i] == newtrialnum)
        DeleteTrial(newtrialnum);

 // now, find an empty spot and create this trial
 i=0;
 while (i < MAX_TRIALS)
  {
  if (trialdata[i] == NULL)
     {
     trialdata[i]=new float[MAX_SAMPLESPERTRIAL];
     trialnum[i]=newtrialnum;
     trialsamples[i]=0;
     i=MAX_TRIALS;
     }
  i++;
  }
}


void CIRCBUF::DeleteTrial(int trial2delete)
{
int i;

 for (i=0; i<MAX_TRIALS; i++)
  if (trialdata[i])
     if (trialnum[i] == trial2delete)
        {
        trialnum[i]=-1000;
        delete [] trialdata[i];
        trialdata[i]=NULL;
        trialsamples[i]=0;
        }
}


STATISTICS::STATISTICS()
{
int     i, t, u;

        outcome_type= 1;                // assume initialy 1
        sign= 1;


 for(i=0;i<MAXDIM;i++)
 {
        oldBLstate[i]= -1;
 }

 for(i=0;i<MAX_BLSTATES;i++)
 {

        targbuf[i]= new CIRCBUF;        // buffers for target % Correct
 }

 for (i=0; i<MAX_CONTROLSIG; i++)
  {
  CurAvg[i]=0;                          // set the average to start with to zero
  for (t=0; t<MAX_BLSTATES; t++)
   circbuf[i][t]=new CIRCBUF;
   sig_mean[i]= 0;                     // estimate of signal means
   use_flag[i]= 0;

   for(t=0;t<MAX_BLSTATES;t++)
   {
        targuse[i][t]= 0;
        targval[i][t]= 0;
        targval2[i][t]= 0;
   }
  }


    current_yintercept= 0;
    current_xintercept= 0;

//  statf= fopen( "c:/current/log/statistics.asc","w+");
//  fprintf(statf,"Opening statistics log \n");
}


STATISTICS::~STATISTICS()
{
int     i, j, t, u;

 for (i=0; i<MAX_CONTROLSIG; i++)
  for (t=0; t<MAX_BLSTATES; t++)
   if (circbuf[i][t])
      {
      delete circbuf[i][t];
      circbuf[i][t]=NULL;
      }

      for(i=0;i<MAX_BLSTATES;i++)
      {
        if( targbuf[i])
        {
                delete targbuf[i];
                targbuf[i]= NULL;
        }
      }
//        fprintf(statf,"Closing sfile \n");
//        fclose(statf);
}


void STATISTICS::SetNumMaxTrials(int trials)
{
int     i, t, u;

 for (i=0; i<MAX_CONTROLSIG; i++)
  for (t=0; t<MAX_BLSTATES; t++)
   if (circbuf[i][t])
      circbuf[i][t]->maxtrials=trials;
}

void STATISTICS::SetDTWinMaxTrials( int trials )
{
        int i,t;
        for (t=0; t<MAX_BLSTATES; t++)
        if (targbuf[t])
                targbuf[t]->maxtrials=trials-1;

}



int   STATISTICS::GetNumBLstates(int controlsigno)
{
int t;
int count;

 count=0;
 for (t=0; t<MAX_BLSTATES; t++)
  if (circbuf[controlsigno][t])
     if (circbuf[controlsigno][t]->GetAllTrialSampleNum() > 0)
        count++;

 return(count);
}


void STATISTICS::SetIntercept(int controlsigno, float intercept)
{
 CurAvg[controlsigno]=intercept;
}


void STATISTICS::SetGain(int controlsigno, float gain)
{
 CurStdDev[controlsigno]=gain;
}

void STATISTICS::SetOutcomeDirection( int dir )
{
        outcome_type= dir;
}


int   STATISTICS::GetNumTrendstates()
{
int t;
int count;

 count=0;
 for (t=0; t<MAX_BLSTATES; t++)
  if (targbuf[t])
     if (targbuf[t]->GetAllTrialSampleNum() > 0)
        count++;

 return(count);
}

void STATISTICS::ProcTrendControl(int dim, int Ntargets, int numBLstate, int target, int outcome,
                         short adapt, TRIALSTAT *trialstat, float lrate, int mode )
{
        static int oldBLupdate=-1;
        float val;
        int i;
        static int o_sign= 1;
        static int nblstates= 0;
        float l[MAX_BLSTATES];
        float q[MAX_BLSTATES];
        float actual_lrate;

        float quad_mean;
        int tflag;

        trialstat->trial_flag= 0;

        if ((oldBLstate[dim] > -1) && (numBLstate == -1))        // end of any of the defined BL periods
        {
                if( target < 1 )
                  throw __FUNC__ ": invalid target index";
                targval[dim][target-1]= adapt;
                targval2[dim][target-1]= abs( adapt );
                targuse[dim][target-1]++;

                if( oldBLupdate > -1 )                                // do once for all dimensions
                {

                        if( outcome_type == 1 )                       // direction = 1 for pc
                        {
                                if( target == outcome )   val= 1.0;
                                else                      val= 0.0;
                                o_sign= +1;
                        }
                        else if( outcome_type == 2 )           // direction = 2 for time
                        {
                                val= (float)outcome;
                                o_sign= -1;
                        }

                        targbuf[oldBLstate[dim]]->PushVal(val);
                }



                nblstates= GetNumTrendstates();

                if( nblstates >= trialstat->NumT ) trialstat->NumT= nblstates;

                for(i=0;i<nblstates;i++)
                {
                        trialstat->TargetPC[i]=  targbuf[i]->CalculateAllTrialAverage();
                }

                if( oldBLupdate > -1 )    targbuf[oldBLstate[dim]]->NextTrial();     // do once for all dimensions


                trialstat->lin= 0.0;

                if( mode > 2 )
                {
                        trialstat->quad= 0.0;

                        quad_mean= 0.0;
                        for(i=0;i<nblstates;i++)
                        {
                                quad_mean+= targval2[dim][i];
                        }

                        if( nblstates > 0 ) quad_mean/= nblstates;

                        for(i=0;i<nblstates;i++)
                        {
                                quadval[i]= targval2[dim][i] - quad_mean;
                        }
                }

                tflag= 1;

                for(i=0;i<nblstates;i++)
                {

                        if( targuse[dim][i]== 0 ) tflag= 0;

                        trialstat->lin+= targval[dim][i]  * trialstat->TargetPC[i];             // product of dimension val and outcome
//  fprintf(statf,"lin= %8.4f  targval[%2d]= %2d  TargetPC= %8.4f \n", trialstat->lin,i,targval[dim][i],trialstat->TargetPC[i]);
                        if( mode > 2 )
                        {
                                trialstat->quad+= quadval[i] * trialstat->TargetPC[i];
                        }

                }


                actual_lrate= lrate * (float)sign * (float)o_sign;                 // what is sign?  must be outcome_direction !!

                if( tflag > 0 )
                {
                        trialstat->aper+= trialstat->aper * trialstat->lin  * actual_lrate;

//  fprintf(statf,"dim= %2d  sign= %2d  aper= %8.4f  \n",dim,sign,trialstat->aper);

                        if( mode > 2 )
                                trialstat->pix+= trialstat->pix  * trialstat->quad * actual_lrate;


                        trialstat->trial_flag= 1;  
                }

        }

        oldBLupdate= numBLstate;
        oldBLstate[dim]= numBLstate;
}

void STATISTICS::ProcRunningAvg( int numBLstate, int controlsigno, float val, TRIALSTAT *trialstat )
{

static int oldBLstate[3]= {-1,-1,-1};
int     t;
float   accavg, accstddev;

if( controlsigno > 2 ) return;
 accavg=accstddev=0;
 // the end of any BL interval ?
 if ((oldBLstate[controlsigno] > -1) && (numBLstate == -1))        // end of any of the defined BL periods
    {
    for (t=0; t<MAX_BLSTATES; t++)
     {
     accavg+=circbuf[controlsigno][t]->CalculateAllTrialAverage();
     accstddev+=circbuf[controlsigno][t]->CalculateAllTrialStdDev();
     }
    if (GetNumBLstates(controlsigno) > 0)
       {
       CurAvg[controlsigno]=accavg/(float)GetNumBLstates(controlsigno);           // the intercept for this control signal is the average of the averages of all BL periods
       CurStdDev[controlsigno]=accstddev/(float)GetNumBLstates(controlsigno);       // the gain for this control signal is the average of the averages of all BL periods
       trialstat->update_flag++;
       }
    circbuf[controlsigno][oldBLstate[controlsigno]]->NextTrial();
    }

 // are we within any of the BL periods ?
 // if so, store the current value in the respective circular buffer

 if (numBLstate > -1)
    circbuf[controlsigno][numBLstate]->PushVal(val);

 oldBLstate[controlsigno]=numBLstate;

 trialstat->Intercept=CurAvg[controlsigno];
 trialstat->StdDev=CurStdDev[controlsigno];

 if( trialstat->Intercept > 0 ) sign= +1;
 else                           sign= -1;

 if( controlsigno == 0 )      current_yintercept= trialstat->Intercept;
 else if( controlsigno == 1 ) current_xintercept= trialstat->Intercept;

 return;
}

void STATISTICS::SetTrendControl( int bufno, float val, int ntimes )
{
        int i;

        for(i=0;i<ntimes;i++)
                targbuf[ bufno ]->PushVal( val );
}

void STATISTICS::SetWeightControl( FILE *filehndl )
{
        sfile= filehndl;
}

void STATISTICS::ProcWeightControl(     int target,             // targets value to predict
                                        int feedback,
                                        int nh,                 // # elements
                                        int use,                // control mode
                                        float *elements,        // pointer to lineq
                                        float *wts,             // pointer to weights
                                        float rate,
                                        int chan
                                  )
{
        int i;
        float predicted;
        float err;
        static int count= 0;

        if( use_flag[chan] == 0 )
        {
                for(i=0;i<nh;i++)                // transfer weights
                        wt_buf[chan][i]= wts[i];

                use_flag[chan]= 1;
                fprintf(sfile,"Transfering Weights \n");
        for(i=0;i<nh;i++)
                fprintf(sfile,"     wt_buf[%2d][%2d]= %8.4f \n",chan, i, wt_buf[chan][i]);
                fprintf(sfile,"Rate= %10.8f \n",rate);
        }


        if(feedback == 1 )
        {
                if( oldfeedback[chan] == 0 )
                {

                }

                predicted= 0;

                for( i=0;i<nh;i++)       // apply filter
                {
                        predicted+=  wt_buf[chan][i] * elements[i];
                }

                predicted-= sig_mean[chan];         // include mean in model

                err= (float)target - predicted;

                for(i=0;i<nh;i++)         // update weights
                {
                        wt_buf[chan][i]+= elements[i] * err * rate;
                }
                if( chan == 1 )
                        sig_mean[chan]= current_yintercept;  // += 1.0 * err * rate;        // update mean
                else if( chan == 2 )
                        sig_mean[chan]= current_xintercept;

                                                                        // Print to file
                fprintf(sfile,"Chan %2d %5d %2d %2d %8.4f %2d",chan,count,use,target,err,nh);
                count++;
                for(i=0;i<nh;i++)
                {
                        fprintf(sfile," %7.3f  %7.4f ",elements[i],wt_buf[chan][i]);
                }
                fprintf(sfile,"%8.4f \n",sig_mean[chan]);                  // mean and linefeed

          }

          else if( feedback == 0 )
          {
                if( (oldfeedback[chan] == 1) &&  (use == 2 ) )      // reset weights at trial end
                {
                        for(i=0;i<nh;i++)
                                wts[i]= wt_buf[chan][i];
                }
          }
          oldfeedback[chan]= feedback;
}


