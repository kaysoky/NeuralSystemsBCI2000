///////////////////////////////////////////////////////////////////////////////
// $Id$
// File:   load_bcidat.cpp
// Author: juergen.mellinger@uni-tuebingen.de
// Date:   Jan 16, 2006
// Description: A Matlab (mex) subroutine that reads BCI2000 .dat files into
//  Matlab workspace variables.
//
//  [ signal, states, parameters ] = load_bcidat( 'filename1', 'filename2', ... )
//
//  loads signal, state, and parameter data from the files whose names are given
//  as function arguments.
//
//  Examples for loading multiple files:
//    files = dir( '*.dat' );
//    [ signal, states, parameters ] = load_bcidat( files.name );
//
//    files = struct( 'name', uigetfile( 'MultiSelect', 'on' ) );
//    [ signal, states, parameters ] = load_bcidat( files.name );
//
//
//  For multiple files, number of channels, states, and signal type must be
//  consistent.
//
//  Signal data will be in raw A/D units, and will be represented by the
//  smallest Matlab data type that accomodates them.
//
//  The 'states' output variable will be a Matlab struct with BCI2000 state
//  names as struct member names, and the number of state value entries matching
//  the first dimension of the 'signal' output variable.
//
//  The 'parameters' output variable will be a Matlab struct with BCI2000
//  parameter names as struct member names.
//  Individual parameters are represented as cell arrays of strings, and may
//  be converted into numeric matrices by Matlab's str2double function.
//  If multiple files are given, parameter values will match the parameters
//  contained in the first file.
//
// $Log$
// Revision 1.6  2006/08/10 15:36:23  mellinger
// Extended parameter translation into Matlab; introduced partial file reading.
//
// Revision 1.5  2006/05/17 15:42:11  mellinger
// Fixed comment/help text.
//
// Revision 1.4  2006/01/19 15:25:10  mellinger
// Fixed potential memory leaks.
//
// Revision 1.3  2006/01/18 20:21:24  mellinger
// Allowed for multiple input files.
//
// Revision 1.2  2006/01/17 17:39:44  mellinger
// Fixed list of project files.
//
// Revision 1.1  2006/01/17 17:15:47  mellinger
// Initial version.
//
///////////////////////////////////////////////////////////////////////////////
#pragma hdrstop

#include "mex.h"
#include "UBCI2000Data.h"
#include "mexutils.h"
#include <sstream>
#include <limits>
#include <cmath>

using namespace std;

typedef signed short int16;
typedef signed int   int32;
typedef float        float32;

struct StateInfo
{
  int    location,
         length;
  int16* data;
};

struct FileInfo
{
  int          begin, // range of samples to read
               end;
  BCI2000DATA* data;
};

struct FileContainer : public vector<FileInfo>
{
  public:
    FileContainer() {}
    ~FileContainer()
    { for( iterator i = begin(); i != end(); ++i ) delete i->data; }
  private:
    FileContainer( const FileContainer& );
    const FileContainer& operator=( const FileContainer& );
};

template<typename T>
void
ReadSignal( FileContainer& inFiles, mxArray* ioSignal )
{
  T* data = reinterpret_cast<T*>( mxGetData( ioSignal ) );
  int sampleOffset = 0,
      totalSamples = 0;
  for( FileContainer::iterator i = inFiles.begin(); i != inFiles.end(); ++i )
    totalSamples += i->end - i->begin;

  for( FileContainer::iterator i = inFiles.begin(); i != inFiles.end(); ++i )
  {
    BCI2000DATA* file = i->data;
    int numSamples = i->end - i->begin,
        numChannels = file->GetNumChannels();
    for( int sample = 0; sample < numSamples; ++sample )
      for( int channel = 0; channel < numChannels; ++channel )
        data[ totalSamples * channel + sample + sampleOffset ]
          = file->ReadValue( channel, sample + i->begin );
    sampleOffset += numSamples;
  }
}

void
mexFunction( int nargout, mxArray* varargout[],
             int nargin,  const mxArray* varargin[] )
{
  mxAssert(
    sizeof( int16 ) == 2 && sizeof( int32 ) == 4 && sizeof( float32 ) == 4,
    "Numeric types don't agree with this function's assumptions."
  );

  if( nargin < 1 )
    mexErrMsgTxt( "No file name given." );

  // Open the files.
  FileContainer files;
  int i = 0;
  while( i < nargin )
  {
    if( mxGetClassID( varargin[ i ] ) != mxCHAR_CLASS )
    {
      ostringstream oss;
      oss << "File name expected in argument " << i + 1 << ".";
      mexErrMsgTxt( oss.str().c_str() );
    }
    char* fileName = mxArrayToString( varargin[ i ] );
    if( fileName == NULL )
      mexErrMsgTxt( "Out of memory when reading file name." );

    BCI2000DATA* file = new BCI2000DATA;
    FileInfo fileInfo =
    {
      0, numeric_limits<int>::max(),
      file
    };
    files.push_back( fileInfo );
    int result = file->Initialize( fileName );
    if( result == BCI2000ERR_FILENOTFOUND )
      result = file->Initialize( ( string( fileName ) + ".dat" ).c_str() );
    switch( result )
    {
      case BCI2000ERR_NOERR:
        break;

      case BCI2000ERR_FILENOTFOUND:
        {
          ostringstream oss;
          oss << "File \"" << fileName << "\" does not exist.";
          mexErrMsgTxt( oss.str().c_str() );
        }
        break;

      default:
        {
          ostringstream oss;
          oss << "Could not open \"" << fileName << "\" as a BCI2000 data file.";
          mexErrMsgTxt( oss.str().c_str() );
        }
    }

    int samplesInFile = file->GetNumSamples(),
        begin = 0,
        end = numeric_limits<int>::max();

    if( ( i + 1 < nargin )
         && ( mxGetClassID( varargin[ i + 1 ] ) == mxDOUBLE_CLASS ) )
    {
      // If a file name is followed by a numeric argument, we interpret it as a sample range.
      // [1 0] and [0 0] are considered an empty range.
      ++i;
      const int* dim = mxGetDimensions( varargin[ i ] );
      int numEntries = dim[ 0 ] * dim[ 1 ];
      double* range = mxGetPr( varargin[ i ] );
      for( int j = 0; j < numEntries; ++j )
        if( floor( abs( range[ j ] ) ) != range[ j ] )
          mexErrMsgTxt( "Nonnegative integers expected in range vector." );

      if( numEntries > 0 )
        begin = range[ 0 ] - 1;
      if( numEntries > 1 )
        end = range[ 1 ];
    }
    if( begin == -1 && end == 0 ) // The [0 0] case.
      begin = 0;
    if( end > samplesInFile )
      end = samplesInFile;
    if( begin >= samplesInFile )
      begin = end;
    if( begin < 0 || ( end - begin ) < 0 )
    {
      ostringstream oss;
      oss << "Invalid sample range specified for file \"" << fileName << "\".";
      mexErrMsgTxt( oss.str().c_str() );
    }
    if( begin > end )
      end = begin;
    files.rbegin()->begin = begin;
    files.rbegin()->end = end;

    mxFree( fileName );
    __bcierr.clear();
    ++i;
  }

  int totalSamples = files[ 0 ].end - files[ 0 ].begin,
      numChannels = files[ 0 ].data->GetNumChannels();
  SignalType dataType = files[ 0 ].data->GetSignalType();
  mxClassID classID = mxUNKNOWN_CLASS;
  switch( dataType )
  {
    case SignalType::int16:
      classID = mxINT16_CLASS;
      break;

    case SignalType::int32:
      classID = mxINT32_CLASS;
      break;

    case SignalType::float32:
      classID = mxSINGLE_CLASS;
      break;

    default:
      classID = mxDOUBLE_CLASS;
  }

  // Assess whether the files are compatible.
  const STATELIST* statelist = files[ 0 ].data->GetStateListPtr();
  for( size_t i = 1; i < files.size(); ++i )
  {
    totalSamples += files[ i ].end - files[ i ].begin;
    if( files[ i ].data->GetNumChannels() != numChannels )
      mexErrMsgTxt( "All input files must have identical numbers of channels." );

    if( files[ i ].data->GetSignalType() != dataType )
      classID = mxDOUBLE_CLASS;

    if( nargout > 1 ) // The caller wants us to read state information.
      for( size_t j = 0; j < statelist->Size(); ++j )
        if( !files[ i ].data->GetStateListPtr()->Exists( ( *statelist )[ j ].GetName() ) )
          mexErrMsgTxt( "Incompatible state information across input files." );
  }

  // Read EEG data into the first output argument.
  int dim[] = { totalSamples, numChannels };
  mxArray* signal = mxCreateNumericArray( 2, dim, classID, mxREAL );
  if( signal == NULL )
    mexErrMsgTxt( "Out of memory when allocating space for the signal variable." );
  switch( classID )
  {
    case mxINT16_CLASS:
      ReadSignal<int16>( files, signal );
      break;

    case mxINT32_CLASS:
      ReadSignal<int32>( files, signal );
      break;

    case mxSINGLE_CLASS:
      ReadSignal<float32>( files, signal );
      break;

    case mxDOUBLE_CLASS:
      ReadSignal<double>( files, signal );
      break;

    default:
      mexErrMsgTxt( "Unsupported class ID" );
  }
  __bcierr.clear();
  varargout[ 0 ] = signal;

  // Read state data if appropriate.
  if( nargout > 1 )
  {
    const STATELIST* mainStatelist = files[ 0 ].data->GetStateListPtr();
    int numStates = mainStatelist->Size();
    char** stateNames = MexAlloc<char*>( numStates );
    StateInfo* stateInfo = MexAlloc<StateInfo>( numStates );
    for( int i = 0; i < numStates; ++i )
    {
      const STATE& s = ( *mainStatelist )[ i ];
      stateNames[ i ] = reinterpret_cast<char*>( mxMalloc( strlen( s.GetName() ) + 1 ) );
      strcpy( stateNames[ i ], s.GetName() );
    }
    mxArray* states = mxCreateStructMatrix(
      1, 1, numStates, const_cast<const char**>( stateNames )
    );
    for( int i = 0; i < numStates; ++i )
    {
      mxArray* stateArray = mxCreateNumericMatrix(
        totalSamples, 1, mxINT16_CLASS, mxREAL
      );
      if( stateArray == NULL )
        mexErrMsgTxt( "Out of memory when allocating space for state variables." );
      mxSetFieldByNumber( states, 0, i, stateArray );
      stateInfo[ i ].data = reinterpret_cast<int16*>( mxGetData( stateArray ) );
    }
    for( FileContainer::iterator file = files.begin(); file != files.end(); ++file )
    { // Locations and lengths are not necessarily compatible across files, so we must
      // create StateInfos for each file individually.
      const STATEVECTOR* statevector = file->data->GetStateVectorPtr();
      const STATELIST& curStatelist = statevector->Statelist();
      for( int i = 0; i < numStates; ++i )
      {
        const STATE& s = curStatelist[ stateNames[ i ] ];
        stateInfo[ i ].location = s.GetLocation();
        stateInfo[ i ].length = s.GetLength();
      }
      for( int sample = file->begin; sample < file->end; ++sample )
      { // Iterating over samples in the outer loop will avoid scanning
        // the file multiple times.
        file->data->ReadStateVector( sample );
        for( int i = 0; i < numStates; ++i )
          *stateInfo[ i ].data++
            = statevector->GetStateValue( stateInfo[ i ].location, stateInfo[ i ].length );
      }
    }

    __bcierr.clear();
    varargout[ 1 ] = states;
  }

  // Read parameters if appropriate.
  if( nargout > 2 )
  {
    varargout[ 2 ] = ParamlistToStruct( *files[ 0 ].data->GetParamListPtr() );
    __bcierr.clear();
  }
}

