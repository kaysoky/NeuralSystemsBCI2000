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

 was_error=false;
 calfilter=new CalibrationFilter(plist, slist);
 if (!calfilter) was_error=true;
 spatfilter= new SpatialFilter(plist, slist);
 if(!spatfilter) was_error=true;
 peakdetector= new PeakDetector(plist, slist);
 if(!peakdetector) was_error= true;
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
 if(peakdetector) delete peakdetector;
 peakdetector= NULL;
 if(classfilter) delete classfilter;
 classfilter= NULL;
 if(normalfilter) delete normalfilter;
 normalfilter= NULL;
 if(statfilter) delete statfilter;
 statfilter= NULL;

 if (SignalA) delete SignalA;
 if (SignalB) delete SignalB;
 if (SignalC) delete SignalC;
 if (SignalD) delete SignalD;
 if (SignalE) delete SignalE;
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
int     maxchannels, maxelements;
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
  maxchannels=atoi(plist->GetParamPtr("MaxChannels")->GetValue());
  maxelements=atoi(plist->GetParamPtr("MaxElements")->GetValue());
  ME= atoi(plist->GetParamPtr("NumControlSignals")->GetValue());
  MF= ME;
  MA= atoi(plist->GetParamPtr("TransmitCh")->GetValue());
  MB= MA;
  NA= atoi(plist->GetParamPtr("SampleBlockSize")->GetValue());
  NB= NA;
  NC= NA;
  MC= atoi(plist->GetParamPtr("SpatialFilteredChannels")->GetValue());
  MD= MC;
  SignalB=new GenericSignal( MB, NB );
  SignalC=new GenericSignal( MC, NC );


  // now, here place the code to initalize your filter
 calfilter->Initialize(plist, svector, corecomm);
 spatfilter->Initialize(plist, svector, corecomm);
 peakdetector->Initialize(plist, svector, corecomm);
 ND= peakdetector->nBins;
 classfilter->Initialize(plist, svector, corecomm);
 normalfilter->Initialize( plist, svector, corecomm);
 statfilter->Initialize( plist, svector, corecomm);

  SignalD=new GenericSignal(2, ND);
  SignalE=new GenericSignal(ME, 1);
  SignalF=new GenericSignal(MF, 1);

#if 0 // jm Oct 21, 2002
  SignalB->Channels= MB;
  SignalB->MaxElements= NB;
  SignalC->Channels= MC;
  SignalC->MaxElements= NC;
#endif
  }
 catch(...)
  { returnval=0; }

 if (!SignalB || !SignalC || !SignalD || !SignalE || !SignalF)
    returnval=0;

 return(returnval);
}


int FILTERS::Resting( char *buf)
{
  statfilter->Resting(classfilter);
  return 0;
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
#if 0 // jm Oct 21, 2002
 SignalA->Channels= MA;
 SignalA->MaxElements= NA;
#endif

 returnval=1;

 // now, here place the code to let your filters process the signals
 calfilter->Process(SignalA, SignalB);
 spatfilter->Process(SignalB, SignalC);
 peakdetector->Process(SignalC, SignalD);
 classfilter->Process(SignalD, SignalE);
 normalfilter->Process(SignalE, SignalF);
 statfilter->Process(classfilter, SignalE, normalfilter, SignalF);

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



