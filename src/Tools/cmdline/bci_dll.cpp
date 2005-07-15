////////////////////////////////////////////////////////////////////
// File:        bci_dll.cpp
// Date:        Jul 12, 2005
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: Provides a basic structure for dynamically loaded
//              libraries that contain BCI2000 filters.
//
//              The interface has two levels:
//              A base level, where input and output are sent in
//              BCI2000 stream format, and a more convenient one
//              intended for use with, e.g., Matlab.
//
//              In the present preliminary implementation of the
//              convenience interface, data will be translated into
//              BCI2000 binary stream format before processing, and
//              results will be translated back from stream format
//              after processing.
//
////////////////////////////////////////////////////////////////////
#include "bci_dll.h"

#include "bci_filterwrapper.h"
#include "UState.h"
#include "UGenericVisualization.h"

#include <iostream>
#include <strstream> // strstreambuf is obsolete but fits our
                     // purposes better than stringbuf because
                     // it allows direct manipulation of the
                     // underlying buffer (which is normally a bad
                     // idea but much more efficient here).

using namespace std;

class omemstream : public ostream
{
 public:
  omemstream() : ostream( new strstreambuf ) {}
  ~omemstream() { delete rdbuf(); }

  void clear()
  {
    dynamic_cast<strstreambuf*>( rdbuf() )->freeze( false );
    ostream::clear();
    delete rdbuf( new strstreambuf );
  }
  char* str()
  { return dynamic_cast<strstreambuf*>( rdbuf() )->str(); }
  int pcount()
  { return dynamic_cast<strstreambuf*>( rdbuf() )->pcount(); }
};

static istream sIn(  new strstreambuf );
static omemstream sOut;
static omemstream sVis;
ostream sErr( new strstreambuf );

static FilterWrapper sFilterWrapper( sOut, sVis );
static STATELIST     sStatelist;

char*
DLLEXPORT
GetInfo()
{
  sOut.clear();
  sOut << sFilterWrapper.FilterName() << '\n'
       << "BCI2000 filter DLL framework compiled " __DATE__ "\n"
       << ends;
  return sOut.str();
}

char*
DLLEXPORT
GetError()
{
  strstreambuf* sb = dynamic_cast<strstreambuf*>( sErr.rdbuf() );
  sb->freeze( false );
  sErr << '\0';
  return sb->str();
}

int
DLLEXPORT
ProcessStreamData( BCIDLL_bufferDesc* inInput,
                   BCIDLL_bufferDesc* outOutput,
                   BCIDLL_bufferDesc* outVis,
                   BCIDLL_bufferDesc* outError )
{
  sOut.clear();
  sVis.clear();

  dynamic_cast<strstreambuf*>( sErr.rdbuf() )->freeze( false );
  sErr.clear();
  delete sErr.rdbuf( new strstreambuf );

  if( inInput )
  {
    delete sIn.rdbuf( new strstreambuf( inInput->data, inInput->length ) );
    while( sIn && sIn.peek() != EOF )
      sFilterWrapper.HandleMessage( sIn );
  }
  else
  {
    delete sIn.rdbuf( new strstreambuf );
    sFilterWrapper.FinishProcessing();
  }
  if( outOutput )
  {
    outOutput->data = sOut.str();
    outOutput->length = sOut.pcount();
  }
  if( outVis )
  {
    outVis->data = sVis.str();
    outVis->length = sVis.pcount();
  }
  if( outError )
  {
    strstreambuf* sb = dynamic_cast<strstreambuf*>( sErr.rdbuf() );
    outError->data = sb->str();
    outError->length = sb->pcount();
  }
  return bool( sIn );
}

int
DLLEXPORT
FinishProcessing( BCIDLL_bufferDesc* outOutput,
                  BCIDLL_bufferDesc* outVis,
                  BCIDLL_bufferDesc* outError )
{
  return ProcessStreamData( NULL, outOutput, outVis, outError );
}

/*
function:  ProcessState
purpose:   Processes a state given as a BCI2000 state definition line.
arguments: Pointer to a NULL terminated state line string.
returns:   True if no error occurred.
*/
int DLLEXPORT
ProcessState( char* inStateLine )
{
  STATE state;
  if( !state.ReadFromStream( istrstream( inStateLine ) ) )
  {
    sErr << "Error parsing state definition line" << endl;
    return false;
  }
  omemstream os;
  MessageHandler::PutMessage( os, state );
  BCIDLL_bufferDesc input = { os.str(), os.pcount() };
  bool success = ProcessStreamData( &input, NULL, NULL, NULL );
  if( success )
    sStatelist.AddState2List( &state );
  return success;
}

/*
function:  ProcessParameter
purpose:   Processes a parameter given as a BCI2000 parameter line.
arguments: Pointer to a NULL terminated parameter line string.
returns:   True if no error occurred.
*/
int DLLEXPORT
ProcessParameter( char* inParameterLine )
{
  PARAM param;
  if( !param.ReadFromStream( istrstream( inParameterLine ) ) )
  {
    sErr << "Error parsing parameter definition line" << endl;
    return false;
  }
  omemstream os;
  MessageHandler::PutMessage( os, param );
  BCIDLL_bufferDesc input = { os.str(), os.pcount() };
  return ProcessStreamData( &input, NULL, NULL, NULL );
}

/*
function:  ProcessData
purpose:   Processes signal and state vector data.
arguments: Pointers holding signal buffer, signal dimensions,
           state vector data buffer, state vector length.
           On return, buffers will point to output data.
returns:   True if no error occurred.
*/
int DLLEXPORT
ProcessData( double** ioSignal, long* ioChannels, long* ioElements,
             unsigned char** ioStatevector, long* ioStatevectorLength )
{
  // Put the input signal into a GenericSignal.
  GenericSignal signal( *ioChannels, *ioElements, SignalType::float32 );
  double* dataPtr = *ioSignal;
  for( long element = 0; element < *ioElements; ++element )
    for( long channel = 0; channel < *ioChannels; ++channel )
      signal( channel, element ) = *dataPtr++;

  // Put state vector data into an appropriate STATEVECTOR.
  STATEVECTOR statevector( &sStatelist, true );
  if( statevector.GetStateVectorLength() != *ioStatevectorLength )
  {
    sErr << "State vector length does not match: expected "
         << statevector.GetStateVectorLength() << ", got "
         << *ioStatevectorLength << "."
         << endl;
    return false;
  }
  ::memcpy( statevector.GetStateVectorPtr(), *ioStatevector, *ioStatevectorLength );

  // Write signal and state vector into a buffer in BCI2000 stream format, and process it.
  omemstream os;
  MessageHandler::PutMessage( os, signal );
  MessageHandler::PutMessage( os, statevector );
  BCIDLL_bufferDesc input = { os.str(), os.pcount() },
                    output = { NULL, 0 };
  if( !ProcessStreamData( &input, &output, NULL, NULL ) )
    return false;

  // Read the resulting stream format data back into the GenericSignal and STATEVECTOR objects.
  istrstream is( output.data, output.length );
  class OutputConverter : public MessageHandler
  {
   public:
    OutputConverter( GenericSignal& signal, STATEVECTOR& statevector )
    : mrSignal( signal ), mrStatevector( statevector )
    {}
    virtual bool HandleVisSignal( istream& is )
    {
      VisSignal s;
      if( s.ReadBinary( is ) && s.GetSourceID() == 0 )
        mrSignal = s;
      return true;
    }
    virtual bool HandleSTATEVECTOR( istream& is )
    {
      mrStatevector.ReadBinary( is );
      return true;
    }
   private:
    GenericSignal& mrSignal;
    STATEVECTOR&   mrStatevector;
  } outputConverter( signal, statevector );

  while( is.peek() != EOF )
    outputConverter.HandleMessage( is );

  // Store the contents of the GenericSignal object into a double array.
  static double* signalBuffer = NULL;
  delete [] signalBuffer;
  *ioChannels = signal.Channels();
  *ioElements = signal.Elements();
  signalBuffer = new double[ *ioChannels * *ioElements ];
  *ioSignal = signalBuffer;
  dataPtr = *ioSignal;
  for( long element = 0; element < *ioElements; ++element )
    for( long channel = 0; channel < *ioChannels; ++channel )
       *dataPtr++ = signal( channel, element );

  // Store the contents of the STATEVECTOR object into a char array.
  static char* statevectorBuffer = NULL;
  delete [] statevectorBuffer;
  *ioStatevectorLength = statevector.GetStateVectorLength();
  statevectorBuffer = new char[ *ioStatevectorLength ];
  *ioStatevector = statevectorBuffer;
  ::memcpy( statevectorBuffer, statevector.GetStateVectorPtr(), *ioStatevectorLength );

  return true;
}



