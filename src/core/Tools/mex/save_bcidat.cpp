///////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A Matlab (mex) subroutine that writes data files from
//  Matlab workspace variables. File format is deduced from the output
//  file's extension, which may be .dat, .edf, or .gdf. When no extension
//  is recognized, the BCI2000 dat file format is used.
//
//  save_bcidat( 'filename', signal, states, parameters )
//
//  Saves signal, state, and parameter data into the named file.
//  Signal data is always interpreted as raw data, i.e. it will be written
//  into the output file unchanged.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////////////
#pragma hdrstop

#include "mex.h"
#include "mexutils.h"
#include "BCI2000OutputFormat.h"
#include "EDFOutputFormat.h"
#include "GDFOutputFormat.h"
#include "StateList.h"
#include "StateVector.h"
#include "BCIError.h"
#include <sstream>
#include <fstream>


using namespace std;

struct StateInfo
{
  int   mLength, mLocation;
  void* mpData;
  void ( StateInfo::*mfpCopyState )( long, StateVector& ) const;

  template<typename T>
  void CopyState( long inBlockStart, StateVector& outStatevector ) const
  {
    T* data = reinterpret_cast<T*>( mpData ) + inBlockStart;
    for( int sample = 0; sample < outStatevector.Samples(); ++sample )
      outStatevector.SetStateValue( mLocation, mLength, sample, data[sample] );
  }
};


template<typename T>
void
ReadSignal( void* inData, long inNumSamples, long inSampleOffset, GenericSignal& outSignal )
{
  for( int ch = 0; ch < outSignal.Channels(); ++ch )
  {
    T* data = reinterpret_cast<T*>( inData ) + ch * inNumSamples + inSampleOffset;
    for( int sample = 0; sample < outSignal.Elements(); ++sample )
      outSignal( ch, sample ) = data[ sample ];
  }
}


class FilterWrapper
{ // FilterWrapper classes are friends to the Environment class.
 public:
  FilterWrapper()
    : mpOutputFormat( NULL )
    {}
  ~FilterWrapper()
    { Cleanup(); }

  ParamList& Parameters()
    { return mParameters; }
  StateList& States()
    { return mStates; }

  template<class T>
  void Instantiate()
  {
    EnvironmentBase::EnterConstructionPhase( &mParameters, &mStates, NULL, NULL );
    delete mpOutputFormat;
    mpOutputFormat = new T;
    EnvironmentBase::ErrorContext( "Publish", mpOutputFormat );
    mpOutputFormat->Publish();
    EnvironmentBase::ErrorContext( "" );
    EnvironmentBase::EnterNonaccessPhase();
    mStates.AssignPositions();
  }

  void Preflight( const SignalProperties& inProperties, const StateVector& inStatevector )
  {
    EnvironmentBase::EnterPreflightPhase( &mParameters, &mStates, NULL, NULL );
    EnvironmentBase::ErrorContext( "Preflight", mpOutputFormat );
    mpOutputFormat->Preflight( inProperties, inStatevector );
    EnvironmentBase::ErrorContext( "" );
    EnvironmentBase::EnterNonaccessPhase();
  }

  void Initialize( const SignalProperties& inProperties, const StateVector& inStatevector )
  {
    EnvironmentBase::EnterInitializationPhase( &mParameters, &mStates, NULL, NULL );
    EnvironmentBase::ErrorContext( "Initialize", mpOutputFormat );
    mpOutputFormat->Initialize( inProperties, inStatevector );
    EnvironmentBase::ErrorContext( "" );
    EnvironmentBase::EnterNonaccessPhase();
  }

  void StartRun( ostream& os, const string& inName )
  {
    EnvironmentBase::EnterStartRunPhase( &mParameters, &mStates, NULL, NULL );
    EnvironmentBase::ErrorContext( "StartRun", mpOutputFormat );
    mpOutputFormat->StartRun( os, inName );
    EnvironmentBase::ErrorContext( "" );
    EnvironmentBase::EnterNonaccessPhase();
  }

  void Write( ostream& os, const GenericSignal& inSignal, StateVector& inStatevector )
  {
    EnvironmentBase::EnterProcessingPhase( &mParameters, &mStates, &inStatevector, NULL );
    EnvironmentBase::ErrorContext( "Write", mpOutputFormat );
    mpOutputFormat->Write( os, inSignal, inStatevector );
    EnvironmentBase::ErrorContext( "" );
    EnvironmentBase::EnterNonaccessPhase();
  }

  void StopRun( ostream& os )
  {
    EnvironmentBase::EnterStopRunPhase( &mParameters, &mStates, NULL, NULL );
    EnvironmentBase::ErrorContext( "StopRun", mpOutputFormat );
    mpOutputFormat->StopRun( os );
    EnvironmentBase::ErrorContext( "" );
    EnvironmentBase::EnterNonaccessPhase();
  }

  void Cleanup()
  {
    delete mpOutputFormat;
    mpOutputFormat = NULL;
  }

 private:
  ParamList            mParameters;
  StateList            mStates;
  GenericOutputFormat* mpOutputFormat;
};


void
mexFunction( int nargout, mxArray* varargout[],
             int nargin,  const mxArray* varargin[] )
{
  std::ios_base::Init();
  bcierr__.Reset();

  if( PrintVersion( __FILE__, nargin, varargin ) )
    return;

  TypeCheck();

  if( nargin < 1 || !mxIsChar( varargin[0] ) )
    bcierr__ << "No file name given." << endl;
  const char* outputFileName = mxArrayToString( varargin[0] );

  if( nargin < 2 || !mxIsNumeric( varargin[1] ) )
    bcierr__ << "No signal data given." << endl;

  const mxArray* pSignal = varargin[1];
  int nDim = mxGetNumberOfDimensions( pSignal );
  const int* dims = mxGetDimensions( pSignal );
  if( nDim != 2 )
    bcierr__ << "Signal data must have two dimensions." << endl;
  unsigned long totalSamples = dims[0];

  void ( *fpReadSignal )( void*, long, long, GenericSignal& ) = NULL;
  SignalType type = SignalType::int16;
  switch( mxGetClassID( pSignal ) )
  {
    case mxDOUBLE_CLASS:
      fpReadSignal = ReadSignal<double>;
      type = SignalType::float32;
      break;

    case mxSINGLE_CLASS:
      fpReadSignal = ReadSignal<float32>;
      type = SignalType::float32;
      break;

    case mxINT8_CLASS:
      fpReadSignal = ReadSignal<int8>;
      type = SignalType::int16;
      break;

    case mxUINT8_CLASS:
      fpReadSignal = ReadSignal<uint8>;
      type = SignalType::int16;
      break;

    case mxINT16_CLASS:
      fpReadSignal = ReadSignal<int16>;
      type = SignalType::int16;
      break;

    case mxUINT16_CLASS:
      fpReadSignal = ReadSignal<uint16>;
      type = SignalType::int32;
      break;

    case mxINT32_CLASS:
      fpReadSignal = ReadSignal<int32>;
      type = SignalType::int32;
      break;

    default:
      bcierr << "Cannot handle signal data in "
             << mxGetClassName( pSignal )
             << " format."
             << endl;
  }
  void* pSignalData = mxGetData( pSignal );
  SignalProperties properties;
  properties.SetChannels( dims[1] )
            .SetType( type );

  if( nargin < 3 || !mxIsStruct( varargin[2] ) )
    bcierr__ << "No state information given. "
             << "Please provide state information in the format "
             << "returned by load_bcidat."
             << endl;

  FilterWrapper wrapper;
  typedef map<string, StateInfo> StateInfoMap;
  StateInfoMap stateInfo;
  const mxArray* pStates = varargin[2];
  int numFields = mxGetNumberOfFields( pStates );
  for( int i = 0; i < numFields; ++i )
  {
    const char* name = mxGetFieldNameByNumber( pStates, i );
    const mxArray* pField = mxGetFieldByNumber( pStates, 0, i );
    const int* dims = mxGetDimensions( pField );
    if( dims[0] < totalSamples )
      bcierr__ << "Too little samples in state \"" << name << "\": "
               << "state variables should provide a value for each sample point."
               << endl;
    if( dims[1] != 1 )
      bcierr__ << "State variables should provide a single value per sample point."
               << endl;

    StateInfo entry;
    entry.mLocation = 0;

    switch( mxGetClassID( pField ) )
    {
      case mxINT8_CLASS:
      case mxUINT8_CLASS:
        entry.mLength = 8;
        entry.mfpCopyState = &StateInfo::CopyState<uint8>;
        break;

      case mxINT16_CLASS:
      case mxUINT16_CLASS:
        entry.mLength = 16;
        entry.mfpCopyState = &StateInfo::CopyState<uint16>;
        break;

      case mxINT32_CLASS:
      case mxUINT32_CLASS:
        entry.mLength = 32;
        entry.mfpCopyState = &StateInfo::CopyState<uint32>;
        break;

      case mxSINGLE_CLASS:
        entry.mLength = 32;
        entry.mfpCopyState = &StateInfo::CopyState<float32>;
        break;

      case mxDOUBLE_CLASS:
        entry.mLength = 32;
        entry.mfpCopyState = &StateInfo::CopyState<double>;
        break;

      default:
        bcierr__ << "Cannot handle state data in "
                 << mxGetClassName( pField )
                 << " format."
                 << endl;
    }
    entry.mpData = mxGetData( pField );
    stateInfo[name] = entry;

    ostringstream stateline;
    stateline << name << ' ' << entry.mLength << " 0 0 0";
    wrapper.States().Add( stateline.str() );
  };

  if( nargin < 4 || !mxIsStruct( varargin[3] ) )
    bcierr__ << "No parameter information given. "
             << "Please provide parameter information in the format "
             << "returned by load_bcidat."
             << endl;

  StructToParamlist( varargin[3], wrapper.Parameters() );
  ParamList& Params = wrapper.Parameters();

  if( nargin > 4 )
    bciout__ << "Ignored additional argument(s)."
             << endl;

  int sampleBlockSize = 1;
  if( !Params.Exists( "SampleBlockSize" ) )
  {
    Params.Add( "save_bcidat int SampleBlockSize= 1 % % % //" );
  }
  else
  {
    sampleBlockSize = ::atoi( Params["SampleBlockSize"].Value().c_str() );
    if( sampleBlockSize <= 0 )
      sampleBlockSize = 1;
  }
  properties.SetElements( sampleBlockSize );

  if( !Params.Exists( "SourceCh" ) )
  {
    ostringstream oss;
    oss << "save_bcidat int SourceCh= "
        << properties.Channels();
    Params.Add( oss.str() );
  }

  if( !Params.Exists( "SamplingRate" ) )
    Params.Add( "save_bcidat float SamplingRate= 1" );

  // Choose an output format depending on the file name's extension.
  enum { bcidat, edf, gdf } format = bcidat;
  int p = ::strlen( outputFileName );
  while( outputFileName[p] != '.' )
    --p;
  if( p >= 0 )
  {
    if( !::stricmp( outputFileName + p, ".edf" ) )
      format = edf;
    else if( !::stricmp( outputFileName + p, ".gdf" ) )
      format = gdf;
  }
  switch( format )
  {
    case edf:
      wrapper.Instantiate<EDFOutputFormat>();
      break;
    case gdf:
      wrapper.Instantiate<GDFOutputFormat>();
      break;
    default:
      wrapper.Instantiate<BCI2000OutputFormat>();
  }

  StateVector statevector( wrapper.States(), properties.Elements() );
  for( int i = 0; i < wrapper.States().Size(); ++i )
  {
    State& s = wrapper.States()[i];
    stateInfo[s.Name()].mLocation = s.Location();
  }
  wrapper.Preflight( properties, statevector );
  wrapper.Initialize( properties, statevector );

  ofstream outputFile( outputFileName, ios::out | ios::binary );
  if( !outputFile.is_open() )
    bcierr__ << "Could not open file \""
             << outputFileName
             << "\" for output."
             << endl;
  wrapper.StartRun( outputFile, outputFileName );

  GenericSignal signal( properties );
  for( long blockStart = 0; blockStart + sampleBlockSize <= totalSamples; blockStart += sampleBlockSize )
  {
    fpReadSignal( pSignalData, totalSamples, blockStart, signal );
    for( StateInfoMap::const_iterator i = stateInfo.begin(); i != stateInfo.end(); ++i )
      ((i->second).*(i->second.mfpCopyState))( blockStart, statevector );
    wrapper.Write( outputFile, signal, statevector );
  }
  wrapper.StopRun( outputFile );
  wrapper.Cleanup();
}
