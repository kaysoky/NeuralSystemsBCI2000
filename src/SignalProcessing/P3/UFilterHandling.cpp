//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "StatFilter.h"
#include "UFilterHandling.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


// **************************************************************************
// Function:   FILTERS
// Purpose:    This is the constructor for the FILTERS class
//             it creates all filters we might want to use
// Parameters: plist - pointer to a list of parameters
//             slist - pointer to a list of states
// Returns:    N/A
//             sets the variable error to true, if there was an error
// **************************************************************************
FILTERS::FILTERS(PARAMLIST *plist, STATELIST *slist)
{
char line[512];

 error.SetErrorMsg("");

 was_error=false;
 calfilter=new CalibrationFilter(plist, slist);
 if (!calfilter) was_error=true;
 spatfilter= new SpatialFilter(plist, slist);
 if(!spatfilter) was_error=true;
 tempfilter= new P3TemporalFilter(plist, slist);
 if(!tempfilter) was_error= true;
 classfilter= new ClassFilter(plist, slist );
 if(!classfilter) was_error= true;
 normalfilter= new NormalFilter( plist, slist );
 if(!normalfilter) was_error= true;
 statfilter= new StatFilter( plist, slist );
 if(!statfilter) was_error= true;

 strcpy(line, "Filtering int NumControlSignals= 2 1 1 128    // the number of transmitted control signals");
 plist->AddParameter2List(line, strlen(line));
 strcpy(line, "Filtering int MaxChannels= 256 10 1 256       // maximum number of channels in signals B,C");
 plist->AddParameter2List(line, strlen(line));
 strcpy(line, "Filtering int MaxElements= 256 10 1 256       // maximum number of elements in signals B,C");
 plist->AddParameter2List(line, strlen(line));

 SignalA=SignalB=SignalC=SignalD=SignalE=SignalF=NULL;
}


// **************************************************************************
// Function:   ~FILTERS
// Purpose:    This is the deconstructor for the FILTERS class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
FILTERS::~FILTERS()
{
 if (calfilter) delete calfilter;
 calfilter=NULL;
 if(spatfilter) delete spatfilter;
 spatfilter=NULL;
 if(tempfilter) delete tempfilter;
 tempfilter= NULL;
 if(classfilter) delete classfilter;
 classfilter= NULL;
 if(normalfilter) delete normalfilter;
 normalfilter= NULL;
 if(statfilter) delete statfilter;
 statfilter= NULL;

 if (SignalA) delete SignalA;
 if (SignalC) delete SignalC;
 if (SignalD) delete SignalD;
 if (SignalE) delete SignalE;
 if (SignalB) delete SignalB;
 if (SignalF) delete SignalF;
 SignalA=SignalB=SignalC=SignalD=SignalE=SignalF=NULL;
}


// **************************************************************************
// Function:   Initialize
// Purpose:    Initialize all the filters
// Parameters: plist - pointer to the PARAMETERLIST
//             svector - pointer to the STATEVECTOR
//             opsocket - pointer to the open socket connection to the operator
// Returns:    0 - any filter had an error on Initialize()
//             1 - no error
// **************************************************************************
int FILTERS::Initialize(PARAMLIST *plist, STATEVECTOR *svector, CORECOMM *corecomm)
{
int     res, returnval;
// int     maxchannels, maxelements;
// int m_mat;              // # of spatially filtered channels
// int f_mat;              // # of frequency bins

 returnval=1;

 if (SignalB) delete SignalB;
 if (SignalC) delete SignalC;
 if (SignalD) delete SignalD;
 if (SignalE) delete SignalE;
 if (SignalF) delete SignalF;
 SignalB=SignalC=SignalD=SignalE=SignalF=NULL;

 try
  {
  // maxchannels=atoi(plist->GetParamPtr("MaxChannels")->GetValue());
  // maxelements=atoi(plist->GetParamPtr("MaxElements")->GetValue());
  ME= atoi(plist->GetParamPtr("NumControlSignals")->GetValue());
  MF= ME;
  MA= atoi(plist->GetParamPtr("TransmitCh")->GetValue());
  MB= MA;
  NA= atoi(plist->GetParamPtr("SampleBlockSize")->GetValue());
  NB= NA;
  NC= NA;
  MC= atoi(plist->GetParamPtr("SpatialFilteredChannels")->GetValue());
  SignalB=new GenericSignal( MB, NB );
  SignalC=new GenericSignal( MC, NC );

  //
  // now, here place the code to initalize your filter(s)
  //

  // initialize the calibration filter
  res= calfilter->Initialize(plist, svector, corecomm);
  if (res == 0)
     {
     returnval= 0;
     error.CopyError(&(calfilter->error));
     }

  // initialize the spatial filter
  res= spatfilter->Initialize(plist, svector, corecomm);
  if (res == 0)
     {
     returnval= 0;
     error.CopyError(&(spatfilter->error));
     }

  // initialize the P3 temporal filter
  res= tempfilter->Initialize(plist, svector, corecomm);
  if (res == 0)
     {
     returnval= 0;
     error.CopyError(&(tempfilter->error));
     }
  MD= tempfilter->numchannels;
  ND= tempfilter->numsamplesinerp;

  // initialize the classifier
  res= classfilter->Initialize(plist, svector, corecomm, ND);
  if (res == 0)
     {
     returnval= 0;
     error.CopyError(&(classfilter->error));
     }

  // initialize the normalizer
  res= normalfilter->Initialize( plist, svector, corecomm);
  if (res == 0)
     {
     returnval= 0;
     error.CopyError(&(normalfilter->error));
     }

  // initialize statistics
  res= statfilter->Initialize( plist, svector, corecomm);
  if (res == 0)
     {
     returnval= 0;
     error.CopyError(&(statfilter->error));
     }

  SignalD=new GenericSignal(MD, ND);
  SignalE=new GenericSignal(ME, 1);
  SignalF=new GenericSignal(MF, 1);

#if 0 // jm 6/28/02
  SignalB->Channels= MB;
  SignalB->MaxElements= NB;

  SignalC->Channels= MC;
  SignalC->MaxElements= NC;
#endif
  }
 catch(...)
  {
  error.SetErrorMsg("Signal Processing: FILTERS::Initialize() threw an exception");
  returnval=0;
  }

 // in case we could not generate any of these five signals
 if (!SignalB || !SignalC || !SignalD || !SignalE || !SignalF)
    {
    error.SetErrorMsg("Signal Processing: Could not create SignalB, C, D, E, or F");
    returnval= 0;
    }

 return(returnval);
}


int FILTERS::Resting( char *buf)
{
 statfilter->Resting(classfilter);

 return(0);
}


// **************************************************************************
// Function:   Process
// Purpose:    Process all the filters
// Parameters: buf - char * to the buffer received from the EEGsource
// Returns:    0 - any filter had an error on Process()
//             1 - no error
// **************************************************************************
int FILTERS::Process(char *buf)
{
int res, returnval;

 // dynamically create Signal A from the input
 if (SignalA) delete SignalA;
 SignalA=CreateGenericSignal(MA, NA, buf);
#if 0 // jm 6/28/02
 SignalA->Channels= MA;
 SignalA->MaxElements= NA;
#endif

 returnval=1;

 // now, here place the code to let your filters process the signals

 res=calfilter->Process(SignalA, SignalB);
 if (res == 0)
    {
    returnval= 0;
    error.CopyError(&(calfilter->error));
    }

 res=spatfilter->Process(SignalB, SignalC);
 if (res == 0)
    {
    returnval= 0;
    error.CopyError(&(spatfilter->error));
    }

 res=tempfilter->Process(SignalC, SignalD);
 if (res == 0)
    {
    returnval= 0;
    error.CopyError(&(tempfilter->error));
    }

 res=classfilter->Process(SignalD, SignalE);
 if (res == 0)
    {
    returnval= 0;
    error.CopyError(&(classfilter->error));
    }

 res= normalfilter->Process(SignalE, SignalF);
 if (res == 0)
    {
    returnval= 0;
    error.CopyError(&(normalfilter->error));
    }

 res= statfilter->Process(classfilter, SignalE, normalfilter, SignalF);
 if (res == 0)
    {
    returnval= 0;
    error.CopyError(&(statfilter->error));
    }

 return(returnval);
}


// **************************************************************************
// Function:   CreateGenericSignal
// Purpose:    creates a GenericSignal from the incoming data stream
// Parameters: transmitchannels - number of incoming channels
//             samples - number of incoming samples per channel
//             buf - char * to the incoming data of size sizeof(short)*transmitchannels*samples
//                   and in the format described in the BCI2000 project outline
// Returns:    pointer to a newly generated GenericSignal
//             the calling routing will have to free this Signal
// **************************************************************************
GenericSignal *FILTERS::CreateGenericSignal(int transmitchannels, int samples, char *buf)
{
short   *dataptr;
int     ch;
GenericSignal *signal;

 signal=new GenericSignal(transmitchannels, samples);
 for (ch=0; ch<transmitchannels; ch++)
  {
  dataptr=(short *)(&buf[sizeof(short)*ch*(int)samples]);
  signal->SetChannel(dataptr, ch);
  }

 return(signal);
}



