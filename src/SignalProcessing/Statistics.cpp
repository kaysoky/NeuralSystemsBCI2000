#undef USE_LOGFILE
//---------------------------------------------------------------------------
#pragma hdrstop
#include <math.h>
#include <stdio.h>
#include "Statistics.h"

#ifdef USE_LOGFILE
FILE *sfile;
#endif // USE_LOGFILE

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

        sign= 1;                // assume initialy 1

 for (i=0; i<MAX_CONTROLSIG; i++)
  {
  CurAvg[i]=0;                          // set the average to start with to zero
  for (t=0; t<MAX_BLSTATES; t++)
   circbuf[i][t]=new CIRCBUF;
  }

  for(i=0;i<MAX_BLSTATES;i++)
        targbuf[i]= new CIRCBUF;        // buffers for target % Correct

    current_intercept= 0;
}


STATISTICS::~STATISTICS()
{
int     i, t, u;

 for (i=0; i<MAX_CONTROLSIG; i++)
  for (t=0; t<MAX_BLSTATES; t++)
   if (circbuf[i][t])
      {
      delete circbuf[i][t];
      circbuf[i][t]=NULL;
      }

      for(i=0;i<MAX_BLSTATES;i++)
        if( targbuf[i])
        {
                delete targbuf[i];
                targbuf[i]= NULL;
        }
        fclose(sfile);
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
        int t;
        for (t=0; t<MAX_BLSTATES; t++)
        if (targbuf[t])
                targbuf[t]->maxtrials=trials;

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


void STATISTICS::ProcTrendControl(int Ntargets, int numBLstate, int target, int outcome, TRIALSTAT *trialstat, float lrate , float qrate)
{
        static int oldBLstate=-1;
        float val;
        static int oldtarget= -1;
        static int oldoutcome= 0;
        int i;

        static int nblstates= 0;
        float l[MAX_BLSTATES];
        float q[MAX_BLSTATES];
        float actual_lrate;

        trialstat->trial_flag= 0;

        if ((oldBLstate > -1) && (numBLstate == -1))        // end of any of the defined BL periods
        {

                if( target == outcome )   val= 1.0;
                else                      val= 0.0;

                targbuf[oldBLstate]->PushVal(val);

                nblstates= GetNumTrendstates();
                nblstates= Ntargets;

                if( nblstates >= trialstat->NumT ) trialstat->NumT= nblstates;

                for(i=0;i<nblstates;i++)
                {
                        trialstat->TargetPC[i]=  targbuf[i]->CalculateAllTrialAverage();
                }

                targbuf[oldBLstate]->NextTrial();

                get_oc(l,q,nblstates);

                trialstat->lin= 0;
                trialstat->quad= 0;

                for(i=0;i<nblstates;i++)
                {
                        trialstat->lin+= l[i]  * trialstat->TargetPC[i];
                        trialstat->quad+= q[i] * trialstat->TargetPC[i];
                }

                actual_lrate= lrate * (float)sign;

                trialstat->aper+= trialstat->aper * trialstat->lin  * actual_lrate;
         //  trialstat->bper-= trialstat->bper * trialstat->quad * qrate;
                trialstat->pix-= trialstat->pix * trialstat->quad * qrate;

                trialstat->trial_flag= 1;

#ifdef USE_LOGFILE
                fprintf(sfile,"oldtarget= %2d outcome= %2d val= %4.2f \n",oldtarget,outcome,val);
#endif // USE_LOGFILE

        }
        oldoutcome= outcome;

        oldBLstate=numBLstate;
        oldtarget= target;
}

void STATISTICS::ProcRunningAvg( int numBLstate, int controlsigno, float val, TRIALSTAT *trialstat )
{
static int oldBLstate=-1;
int     t;
float   accavg, accstddev;

 accavg=accstddev=0;
 // the end of any BL interval ?
 if ((oldBLstate > -1) && (numBLstate == -1))        // end of any of the defined BL periods
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
    circbuf[controlsigno][oldBLstate]->NextTrial();
    }

 // are we within any of the BL periods ?
 // if so, store the current value in the respective circular buffer

 if (numBLstate > -1)
    circbuf[controlsigno][numBLstate]->PushVal(val);

 oldBLstate=numBLstate;

 trialstat->Intercept=CurAvg[controlsigno];
 trialstat->StdDev=CurStdDev[controlsigno];

 if( trialstat->Intercept > 0 ) sign= +1;
 else                           sign= -1;

 current_intercept= trialstat->Intercept;

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

void STATISTICS::ProcWeightControl(     int ntargs,             // number of targets
                                        int targ,
                                        int feedback,
                                        int nh,                 // # horizontal elements
                                        int ctr,                // control mode
                                        float *elements,        // pointer to lineq
                                        float *wts,             // pointer to weights
                                        float rate,
                                        int use                 // use of wts
                                  )
{
        static int oldfeedback= 0;
        static float wt_buf[128];
        int i;
        int target;
        float predicted;
        float err;
        static float mean= 0;
        static int use_flag= 0;
        static int count= 0;

        switch( ntargs )                      //  target should have 0 mean?
        {
                case 2:
                        if( targ == 1 )     target= -1;
                        else if( targ == 2 )target=  1;
                        else target= 0;
                        break;
                case 3:
                        if( targ == 1 )      target= -1;
                        else if( targ == 2 ) target=  0;
                        else if( targ == 3 ) target=  1;
                        else targ= 0;
                        break;
                case 4:
                        if( targ == 1 )      target= -2;
                        else if( targ == 2 ) target= -1;
                        else if( targ == 3 ) target=  1;
                        else if( targ == 4 ) target=  2;
                        else target= 0;
                        break;
                default:    return;
        }


        if( use_flag == 0 )
        {
                for(i=0;i<nh;i++)                // transfer weights
                        wt_buf[i]= wts[i];

                use_flag= 1;
        }

        if(feedback == 1 )
        {
                if( oldfeedback == 0 )
                {

                }

                predicted= 0;

                for( i=0;i<nh;i++)       // apply filter
                {
                        predicted+=  wt_buf[i] * elements[i];
                }

                predicted-= mean;         // include mean in model

                err= (float)target - predicted;

                for(i=0;i<nh;i++)         // update weights
                {
                        wt_buf[i]+= elements[i] * err * rate;
                }
                mean= current_intercept;  // += 1.0 * err * rate;        // update mean

                                                                        // Print to file
                fprintf(sfile,"%5d %2d %2d %8.4f ",count,use,target,err);
                count++;
                for(i=0;i<nh;i++)
                {
                        fprintf(sfile," %7.3f  %7.4f ",elements[i],wt_buf[i]);
                }
                fprintf(sfile,"%8.4f \n",mean);                  // mean and linefeed

          }
          else if( feedback == 0 )
          {
                if( (oldfeedback == 1) &&  (use == 2 ) )      // reset weights at trial end
                {
                        for(i=0;i<nh;i++)
                                wts[i]= wt_buf[i];
                }
          }
          oldfeedback= feedback;
}


