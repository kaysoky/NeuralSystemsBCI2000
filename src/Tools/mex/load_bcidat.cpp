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
//  For multiple files, number of channels, states, and signal type must be
//  consistent.
//
//  The 'states' output variable will be a Matlab struct with BCI2000 state
//  names as struct member names, and the number of state value entries matching
//  the first dimension of the 'signal' output variable.
//
//  The 'parameters' output variable will be a Matlab struct with BCI2000
//  parameter names as struct member names.
//  Individual parameters are represented as cell arrays of strings, and may
//  be converted into numeric matrices by Matlab's str2double function.
//  If multiple files are given, parameter values will match the first files'
//  parameters.
//
//
// $Log$
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
#include <sstream>

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

struct FileContainer : public vector<BCI2000DATA*>
{
  public:
    FileContainer() {}
    ~FileContainer()
    { for( iterator i = begin(); i != end(); ++i ) delete *i; }
  private:
    FileContainer( const FileContainer& );
    const FileContainer& operator=( const FileContainer& );
};

template<typename T>
T*
MexAlloc( int inElements )
{
  // mxCalloc'ed memory will be freed automatically on return from mexFunction().
  return reinterpret_cast<T*>( mxCalloc( inElements, sizeof( T ) ) );
}

template<typename T>
void
ReadSignal( FileContainer& inFiles, mxArray* ioSignal )
{
  T* data = reinterpret_cast<T*>( mxGetData( ioSignal ) );
  int sampleOffset = 0,
      totalSamples = 0;
  for( FileContainer::iterator i = inFiles.begin(); i != inFiles.end(); ++i )
    totalSamples += ( *i )->GetNumSamples();

  for( FileContainer::iterator i = inFiles.begin(); i != inFiles.end(); ++i )
  {
    BCI2000DATA* file = *i;
    int numSamples = file->GetNumSamples(),
        numChannels = file->GetNumChannels();
    for( int sample = 0; sample < numSamples; ++sample )
      for( int channel = 0; channel < numChannels; ++channel )
        data[ totalSamples * channel + sample + sampleOffset ]
          = file->ReadValue( channel, sample );
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
  for( int i = 0; i < nargin; ++i )
  {
    files.push_back( new BCI2000DATA );
    char* fileName = mxArrayToString( varargin[ i ] );
    if( fileName == NULL )
      mexErrMsgTxt( "Out of memory when reading file name." );
    int result = files[ i ]->Initialize( fileName );
    if( result == BCI2000ERR_FILENOTFOUND )
      result = files[ i ]->Initialize( ( string( fileName ) + ".dat" ).c_str() );
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
    mxFree( fileName );
    __bcierr.clear();
  }

  int totalSamples = files[ 0 ]->GetNumSamples(),
      numChannels = files[ 0 ]->GetNumChannels();
  SignalType dataType = files[ 0 ]->GetSignalType();
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
  const STATELIST* statelist = files[ 0 ]->GetStateListPtr();
  for( size_t i = 1; i < files.size(); ++i )
  {
    totalSamples += files[ i ]->GetNumSamples();
    if( files[ i ]->GetNumChannels() != numChannels )
      mexErrMsgTxt( "All input files must have identical numbers of channels." );

    if( files[ i ]->GetSignalType() != dataType )
      classID = mxDOUBLE_CLASS;

    if( nargout > 1 ) // The caller wants us to read state information.
      for( size_t j = 0; j < statelist->Size(); ++j )
        if( !files[ i ]->GetStateListPtr()->Exists( ( *statelist )[ j ].GetName() ) )
          mexErrMsgTxt( "Incompatible state information in input files." );
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
    const STATELIST* mainStatelist = files[ 0 ]->GetStateListPtr();
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
    { // StateRefs are not necessarily compatible across files, so we must
      // create StateRefs for each file individually.
      const STATEVECTOR* statevector = ( *file )->GetStateVectorPtr();
      const STATELIST& curStatelist = statevector->Statelist();
      for( int i = 0; i < numStates; ++i )
      {
        const STATE& s = curStatelist[ stateNames[ i ] ];
        stateInfo[ i ].location = s.GetLocation();
        stateInfo[ i ].length = s.GetLength();
      }
      for( size_t sample = 0; sample < ( *file )->GetNumSamples(); ++sample )
      { // Iterating over samples in the outer loop will avoid scanning
        // the file multiple times.
        ( *file )->ReadStateVector( sample );
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
    const PARAMLIST* paramlist = files[ 0 ]->GetParamListPtr();
    int numParams = paramlist->Size();
    char** paramNames = MexAlloc<char*>( numParams );
    for( int i = 0; i < numParams; ++i )
    {
      const PARAM& param = ( *paramlist )[ i ];
      paramNames[ i ] = MexAlloc<char>( strlen( param.GetName() ) + 1 );
      strcpy( paramNames[ i ], param.GetName() );
    }
    mxArray* params = mxCreateStructMatrix(
      1, 1, numParams, const_cast<const char**>( paramNames )
    );
    for( int i = 0; i < numParams; ++i )
    {
      const PARAM& p = ( *paramlist )[ i ];
      mxArray* paramArray = mxCreateCellMatrix( p.GetNumRows(), p.GetNumColumns() );
      if( paramArray == NULL )
        mexErrMsgTxt( "Out of memory when allocating space for state variables." );
      int cell = 0;
      for( size_t col = 0; col < p.GetNumColumns(); ++col )
        for( size_t row = 0; row < p.GetNumRows(); ++row )
          mxSetCell( paramArray, cell++, mxCreateString( p.GetValue( row, col ) ) );
      mxSetFieldByNumber( params, 0, i, paramArray );
    }
    __bcierr.clear();
    varargout[ 2 ] = params;
  }
}
