////////////////////////////////////////////////////////////////////////////////
//
// File: RDAClientADC.cpp
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date: Jan 3, 2003
//
// Description: A source class that interfaces to the BrainAmp RDA socket
//              interface.
//
////////////////////////////////////////////////////////////////////////////////
#include <vcl.h>
#pragma hdrstop

#include <string>
#include <sstream>
#define bcierr std::ostringstream()
#define bciout std::ostringstream()
#include "RDAClientADC.h"

#pragma package(smart_init)

#ifdef BCI_2000_STRICT
// **************************************************************************
// Function:   GetNewADC
// Purpose:    This static member function of the GenericADC class is meant to
//             be implemented along with each subclass of GenericADC.
//             Its sole purpose is to make subclassing transparent for the
//             code in EEGSource/UMain.cpp .
// Parameters: Pointers to parameter and state lists.
// Returns:    A generic pointer to an instance of the respective default
//             ADC class.
// **************************************************************************
GenericADC*
GenericADC::GetNewADC( PARAMLIST* inParamList, STATELIST* inStateList )
{
  return new RDAClientADC( inParamList, inStateList );
}
#endif // BCI_2000_STRICT

// **************************************************************************
// Function:   ADInit
// Purpose:    This function initializes the RDAClientADC.
//             It is called each time the operator first starts,
//             or suspends and then resumes, the system
//             (i.e., each time the system goes into the main
//             data acquisition loop (fMain->MainDataAcqLoop())
// Parameters: N/A
// Returns:    Error value
// **************************************************************************
int RDAClientADC::ADInit()
{
  int err = noError;
#ifdef BCI_2000_STRICT
  sourceSignal = GenericIntSignal( 0, 0 );
#endif // BCI_2000_STRICT
  size_t sampleBlockSize = 0;
  hostName = "";

  const PARAM* param = NULL;
  const char*  value = NULL;
#pragma option push
#pragma warn -pia // Don't complain about possible unwanted assignments
  if( ( param = paramlist->GetParamPtr( "SampleBlockSize" ) ) && ( value = param->GetValue() ) )
    sampleBlockSize = atoi( value );
  else
    err = parameterError;
  if( ( param = paramlist->GetParamPtr( "HostName" ) ) && ( value = param->GetValue() ) )
    hostName = value;
  else
    err = parameterError;
#pragma option pop
  if( err != noError )
  {
    bcierr << "Could not obtain all parameter values needed.";
    return err;
  }

  inputQueue.open( hostName.c_str() );
  if( !inputQueue.is_open() )
    return connectionError;

  {
    std::ostringstream os;
    softwareCh = inputQueue.info().numChannels + 1;
    os << "Source int SoftwareCh= "
       << softwareCh << " "
       << softwareCh
       << " 1 129 // number of digitized and stored channels including marker channel (readonly)";
    paramlist->AddParameter2List( os.str().c_str() );
  }
  {
    std::ostringstream os;
    samplingRate = 1e6 / inputQueue.info().samplingInterval;
    os << "Source int SamplingRate= "
       << samplingRate << " "
       << samplingRate
       << " 1 10000 // sampling rate in S/s (readonly)";
    paramlist->AddParameter2List( os.str().c_str() );
  }
  {
    size_t numChannels = inputQueue.info().numChannels;
#ifdef BCI_2000_STRICT
    std::ostringstream osOffset, osGain;
    osOffset << "Filtering floatlist SourceChOffset= " << numChannels + 1;
    osGain   << "Filtering floatlist SourceChGain= " << numChannels + 1;
    for( size_t i = 0; i < numChannels; ++i )
    {
      osOffset << " 0";
      osGain   << ' ' << inputQueue.info().channelResolutions[ i ];
    }
    osOffset << " 0 -500 500 // offset for channels in A/D units (readonly)";
    osGain   << " 1 -500 500 // gain for each channel (A/D units -> muV) (readonly)";
    paramlist->AddParameter2List( osOffset.str().c_str() );
    paramlist->AddParameter2List( osGain.str().c_str() );
#else // BCI_2000_STRICT
#ifndef HIDE_MARKER_CHANNEL
    ++numChannels;
#endif // HIDE_MARKER_CHANNEL
    std::ostringstream osAmp, osList;
    osAmp << "Calibrator floatlist CAMaxAmplitude= " << numChannels;
    osList << "Calibrator intlist CAChList= " << numChannels;
    for( size_t i = 0; i < inputQueue.info().numChannels; ++i )
    {
      osAmp << ' ' << inputQueue.info().channelResolutions[ i ] * 32000;
      osList << ' ' << i;
    }
#ifndef HIDE_MARKER_CHANNEL
    osAmp << " 32000";
    osList << ' ' << numChannels - 1;
#endif // HIDE_MARKER_CHANNEL

    osAmp << " 1 10000 // Assignment of maximum +/-amplitude in uV (readonly)";
    osList << " 0 " << numChannels -1 << " // Assignment of calibrated channels (readonly)";
    paramlist->AddParameter2List( osAmp.str().c_str() );
    paramlist->AddParameter2List( osList.str().c_str() );
#endif // BCI_2000_STRICT
  }

  // Check whether block sizes are sub-optimal.
  size_t sourceBlockSize = inputQueue.info().blockDuration / inputQueue.info().samplingInterval;
  if( sampleBlockSize % sourceBlockSize != 0 && sourceBlockSize % sampleBlockSize != 0 )
    bciout << "Non-integral ratio in source block sizes.";

  inputQueue.close();
  inputQueue.clear();

#ifdef BCI_2000_STRICT
 sourceSignal = GenericIntSignal( softwareCh, sampleBlockSize );
#endif // BCI_2000_STRICT

  return noError;
}


// **************************************************************************
// Function:   ADReadDataBlock
// Purpose:    This function is called within fMain->MainDataAcqLoop().
//             It fills its argument signal with values
//             and does not return until a full block of data is acquired.
// Parameters: Pointer to a signal to be filled with values.
// Returns:    Error value
// **************************************************************************
int RDAClientADC::ADReadDataBlock( GenericIntSignal* SourceSignal )
{
  if( !inputQueue.is_open() )
  {
    inputQueue.open( hostName.c_str() );
    if( !inputQueue.is_open() )
      return connectionError;
  }

  for( size_t sample = 0; sample < SourceSignal->MaxElements; ++sample )
    for( size_t channel = 0; channel < SourceSignal->Channels; ++channel )
    {
      if( !inputQueue )
        return connectionError;
      SourceSignal->Value[channel][sample] = inputQueue.front();
      inputQueue.pop();
    }

#ifndef BCI_2000_STRICT
  // This should go into the framework code.
  statevector->SetStateValue( "SourceTime", BCITIME::GetBCItime_ms() );
#endif // BCI_2000_STRICT
  return noError;
}

// **************************************************************************
// Function:   ADShutdown
// Purpose:    This routine shuts down data acquisition
// Parameters: N/A
// Returns:    noError
// **************************************************************************
int RDAClientADC::ADShutdown()
{
  inputQueue.close();
  inputQueue.clear();
  return noError;
}

// **************************************************************************
// Function:   RDAClientADC
// Purpose:    The RDAClientADC constructor.
//             It fills the provided list of parameters and states
//             with the parameters and states it requests from the operator.
// Parameters: plist - the list of parameters
//             slist - the list of states
// Returns:    N/A
// **************************************************************************
RDAClientADC::RDAClientADC( PARAMLIST *plist, STATELIST *slist )
: paramlist( plist ),
  statelist( slist ),
  softwareCh( 0 ),
  samplingRate( 0.0 ),
  sourceSignal( 0, 0 )
{
  const char* params[] =
  {
    "Source int SoftwareCh= 33 33 1 129"
            " // the number of digitized and stored channels including marker channel",
    "Source int SampleBlockSize= 20 20 1 128"
            " // the number of samples transmitted at a time, incoming blocks are always 40ms",
    "Source int SamplingRate= 250 250 1 4000"
            " // the sample rate",
    "Source string HostName= localhost"
            " // the name of the host to connect to",
  };
  const size_t numParams = sizeof( params ) / sizeof( *params );
  for( size_t i = 0; i < numParams; ++i )
    paramlist->AddParameter2List( params[ i ] );

  // This should go into the framework code.
  statelist->AddState2List( "SourceTime 16 0 0 0\n" );
}

// **************************************************************************
// Function:   ~RDAClientADC
// Purpose:    The RDAClientADC destructor.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
RDAClientADC::~RDAClientADC()
{
  ADShutdown();
}

