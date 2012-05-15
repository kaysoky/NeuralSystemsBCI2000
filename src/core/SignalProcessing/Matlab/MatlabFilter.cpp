////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
//         jeremy.hill@tuebingen.mpg.de
// Description: This BCI2000 filter calls the Matlab engine to act upon signals,
//    parameters, and states, thus providing the full BCI2000 filter interface
//    to a Matlab filter implementation.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "MatlabFilter.h"
#include "defines.h"
#include "FileUtils.h"

using namespace std;

RegisterFilter( MatlabFilter, 2.C );

#define MATLAB_NAME( x )   "bci_"#x  // How variables and functions are named in Matlab.

// Matlab function names
#define CONSTRUCT   MATLAB_NAME( Construct )
#define DESTRUCT    MATLAB_NAME( Destruct )
#define PREFLIGHT   MATLAB_NAME( Preflight )
#define INITIALIZE  MATLAB_NAME( Initialize )
#define PROCESS     MATLAB_NAME( Process )
#define START_RUN   MATLAB_NAME( StartRun )
#define STOP_RUN    MATLAB_NAME( StopRun )
#define RESTING     MATLAB_NAME( Resting )
#define HALT        MATLAB_NAME( Halt )

// Matlab variable names
#define PARAM_DEFS      MATLAB_NAME( ParamDefs )
#define STATE_DEFS      MATLAB_NAME( StateDefs )
#define IN_SIGNAL       MATLAB_NAME( InSignal )
#define IN_SIGNAL_DIMS  MATLAB_NAME( InSignalDims )
#define OUT_SIGNAL      MATLAB_NAME( OutSignal )
#define OUT_SIGNAL_DIMS MATLAB_NAME( OutSignalDims )
#define PARAMETERS      MATLAB_NAME( Parameters )
#define STATES          MATLAB_NAME( States )

MatlabFilter::MatlabFilter()
: mpBci_Process( NULL )
{
  MatlabEngine::Open();
  if( !MatlabEngine::IsOpen() )
  {
    bcierr << "Could not connect to Matlab engine. "
           << "Please make sure that Matlab is available on your machine.\n"
           << "On Windows, Matlab's bin/win32 directory must be on your system's %PATH% variable, "
           << "and \"Matlab /regserver\" must have been executed with administrative privileges."
           << endl;
  }
  else
  {
    // Configure matlab engine behavior as specified by --MatlabStayOpen.
    mMatlabStayOpen = OptionalParameter( "MatlabStayOpen", closeEngine );
    // Change matlab's working directory to the directory specified by --MatlabWD.
    string wd = OptionalParameter( "MatlabWD", "." );
    wd = FileUtils::AbsolutePath( wd );
    MatlabEngine::PutString( "bci_WD", wd );
    CallMatlab( MatlabFunction( "cd" ).InputArgument( "bci_WD" ) );
    MatlabEngine::ClearVariable( "bci_WD" );
    
    MatlabFunction bci_Construct( CONSTRUCT );
    bci_Construct.OutputArgument( PARAM_DEFS )
                 .OutputArgument( STATE_DEFS );
    if( CallMatlab( bci_Construct ) )
    {
      // Add the parameters and states requested by the Matlab bci_Construct function.
      int numParamDefs = static_cast<int>( MatlabEngine::GetScalar( "max(size(" PARAM_DEFS "))" ) );
      for( int i = 1; i <= numParamDefs; ++i )
      {
        ostringstream expr;
        expr << PARAM_DEFS << "{" << i << "}";
        string paramDef = MatlabEngine::GetString( expr.str() );
        if( !Parameters->Add( paramDef ) )
          bcierr << "Error in parameter definition: " << paramDef << endl;
      }
      int numStateDefs = static_cast<int>( MatlabEngine::GetScalar( "max(size(" STATE_DEFS "))" ) );
      for( int i = 1; i <= numStateDefs; ++i )
      {
        ostringstream expr;
        expr << STATE_DEFS << "{" << i << "}";
        string stateDef = MatlabEngine::GetString( expr.str() );
        if( !States->Add( stateDef ) )
          bcierr << "Error in state definition: " << stateDef << endl;
      }
    }
    MatlabEngine::ClearVariable( PARAM_DEFS );
    MatlabEngine::ClearVariable( STATE_DEFS );

    // Issue a warning to indicate potential mis-configuration if there is no
    // Preflight, Initialize, or Process function available for Matlab.
    const char* essentialFunctions[] =
    {
      PREFLIGHT,
      INITIALIZE,
      PROCESS,
    };
    ostringstream oss;
    for( size_t i = 0; i < sizeof( essentialFunctions ) / sizeof( *essentialFunctions ); ++i )
      if( !MatlabFunction( essentialFunctions[ i ] ).Exists() )
        oss << ", " << essentialFunctions[ i ];
    if( !oss.str().empty() )
      bciout << "The following functions could not be found in the Matlab path:\n"
             << oss.str().substr( 2 ) << ".\n"
             << "Make sure that the m-files exist within path or working directory, "
             << "and contain appropriate function definitions.\n"
             << "Consider using the --MatlabWD command line option to set Matlab's "
             << "working directory at startup"
             << endl;

    // Initialize the bci_Process function for more efficient calling during Process().
    mpBci_Process = new MatlabFunction( PROCESS );
    mpBci_Process->InputArgument( IN_SIGNAL )
                  .OutputArgument( OUT_SIGNAL );
  }
}

MatlabFilter::~MatlabFilter()
{
  if( MatlabEngine::IsOpen() )
  {
    { // Make sure bci_Destruct is out of scope when calling MatlabEngine::Close().
      MatlabFunction bci_Destruct( DESTRUCT );
      CallMatlab( bci_Destruct );
    }

    delete mpBci_Process;

    if( mMatlabStayOpen != dontClear )
    {
      MatlabEngine::ClearVariable( IN_SIGNAL );
      MatlabEngine::ClearVariable( OUT_SIGNAL );
      MatlabEngine::ClearVariable( PARAMETERS );
      MatlabEngine::ClearVariable( STATES );
    }
    if( mMatlabStayOpen == closeEngine )
      MatlabEngine::Close();
  }
}

void
MatlabFilter::Preflight( const SignalProperties& Input,
                               SignalProperties& Output ) const
{
  int matlabStayOpen = OptionalParameter( "MatlabStayOpen", closeEngine );
  switch( matlabStayOpen )
  {
    case closeEngine:
    case clearVariables:
    case dontClear:
      break;
    default:
      bcierr << "Undefined value of MatlabStayOpen\n"
             << "Possible values are:\n"
             << " 0: close engine;\n"
             << " 1: keep engine open, clear variables;\n"
             << " 2: keep engine open, keep variables."
             << endl;
  }

  MatlabEngine::ClearVariable( PARAMETERS );
  MatlabEngine::CreateGlobal( PARAMETERS );
  ParamsToMatlabWS();
  MatlabEngine::ClearVariable( STATES );
  MatlabEngine::CreateGlobal( STATES );
  StatesToMatlabWS();

  Output = Input;
  MatlabEngine::PutMatrix( IN_SIGNAL_DIMS, Input );
  MatlabFunction bci_Preflight( PREFLIGHT );
  bci_Preflight.InputArgument( IN_SIGNAL_DIMS )
               .OutputArgument( OUT_SIGNAL_DIMS );
  if( CallMatlab( bci_Preflight ) )
    Output = MatlabEngine::GetMatrix( OUT_SIGNAL_DIMS );
  MatlabEngine::ClearVariable( IN_SIGNAL_DIMS );
  MatlabEngine::ClearVariable( OUT_SIGNAL_DIMS );
}

void
MatlabFilter::Initialize( const SignalProperties& Input,
                          const SignalProperties& Output )
{
  // Re-configure matlab engine behavior as specified by the MatlabStayOpen parameter.
  mMatlabStayOpen = OptionalParameter( "MatlabStayOpen", closeEngine );

  MatlabEngine::PutMatrix( IN_SIGNAL_DIMS, Input );
  MatlabEngine::PutMatrix( OUT_SIGNAL_DIMS, Output );
  MatlabFunction bci_Initialize( INITIALIZE );
  bci_Initialize.InputArgument( IN_SIGNAL_DIMS )
                .InputArgument( OUT_SIGNAL_DIMS );
  CallMatlab( bci_Initialize );
  MatlabEngine::ClearVariable( IN_SIGNAL_DIMS );
  MatlabEngine::ClearVariable( OUT_SIGNAL_DIMS );
  MatlabEngine::PutMatrix( IN_SIGNAL, GenericSignal( Input ) );
  MatlabEngine::PutMatrix( OUT_SIGNAL, GenericSignal( Output ) );
}

void
MatlabFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
  StatesToMatlabWS();
  MatlabEngine::PutMatrix( IN_SIGNAL, Input );
  if( CallMatlab( *mpBci_Process ) )
    Output = MatlabEngine::GetMatrix( OUT_SIGNAL );
  else
    Output = Input;
  MatlabWSToStates();
}

void
MatlabFilter::StartRun()
{
  MatlabFunction bci_StartRun( START_RUN );
  if( bci_StartRun.Exists() )
  {
    ParamsToMatlabWS();
    CallMatlab( bci_StartRun );
  }
}

void
MatlabFilter::StopRun()
{
  MatlabFunction bci_StopRun( STOP_RUN );
  if( CallMatlab( bci_StopRun ) )
    MatlabWSToParams();
}

void
MatlabFilter::Resting()
{
  MatlabFunction bci_Resting( RESTING );
  CallMatlab( bci_Resting );
}

void
MatlabFilter::Halt()
{
  MatlabFunction bci_Halt( HALT );
  CallMatlab( bci_Halt );
}

void
MatlabFilter::StatesToMatlabWS() const
{
  for( int i = 0; i < States->Size(); ++i )
  {
    const string& name = ( *States )[ i ].Name();
    MatlabEngine::PutScalar( string( STATES "." ) + name, State( name ) );
  }
}

void
MatlabFilter::MatlabWSToStates()
{
  for( int i = 0; i < States->Size(); ++i )
  {
    const string& name = ( *States )[ i ].Name();
    State( name ) = static_cast<int>( MatlabEngine::GetScalar( string( STATES "." ) + name ) );
  }
}

void
MatlabFilter::ParamsToMatlabWS() const
{
  for( int i = 0; i < Parameters->Size(); ++i )
  {
    Param& p = ( *Parameters )[ i ];
    MatlabEngine::PutCells( string( PARAMETERS "." ) + p.Name(), p );
  }
}

void
MatlabFilter::MatlabWSToParams()
{
  for( int i = 0; i < Parameters->Size(); ++i )
  {
    Param& p = ( *Parameters )[ i ];
    MatlabEngine::StringMatrix values = MatlabEngine::GetCells( string( PARAMETERS "." ) + p.Name() );
    int newRows = static_cast<int>( values.size() ),
        newCols = static_cast<int>( newRows ? values.at( 0 ).size() : 1 );
    if( newRows != p.NumRows() || newCols != p.NumColumns() )
      p.SetDimensions( newRows, newCols );
    for( int row = 0; row < newRows; ++row )
      for( int col = 0; col < newCols; ++col )
        if( string( p.Value( row, col ) ) != values.at( row ).at( col ) )
          p.Value( row, col ) = values.at( row ).at( col );
  }
}

bool
MatlabFilter::CallMatlab( MatlabFunction& inFunction ) const
{
  if( !inFunction.Exists() )
    return false;
  string err = inFunction.Execute();
  if( !err.empty() )
  {
    bcierr__ << "Matlab function \"" << inFunction.Name() << "\": "
             << err
             << endl;
    return false;
  }
  return true;
}

