////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File: MatlabFilter.h
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date: Nov 30, 2005
//
// $Log$
// Revision 1.3  2006/08/08 15:25:18  mellinger
// Fixed a typing error in a Matlab expression that would lead to errors when requesting states from the constructor.
//
// Revision 1.2  2006/01/12 20:39:00  mellinger
// Adaptation to latest revision of parameter and state related class interfaces.
//
// Revision 1.1  2005/12/20 11:38:07  mellinger
// Initial version.
//
//
// Description: This BCI2000 filter calls the Matlab engine to act upon signals,
//    parameters, and states, thus providing the full BCI2000 filter interface
//    to a Matlab filter implementation.
//
//    For each BCI2000 filter member function, there is a corresponding Matlab
//    function as follows:
//
//    GenericFilter member      Matlab function syntax
//    ====================      ======================
//    Constructor               [parameters, states] = bci_Construct
//    Destructor                bci_Destruct
//    Preflight                 out_signal_dim = bci_Preflight( in_signal_dim )
//    Initialize                bci_Initialize( in_signal_dim, out_signal_dim )
//    Process                   out_signal = bci_Process( in_signal )
//    StartRun                  bci_StartRun
//    StopRun                   bci_StopRun
//    Resting                   bci_Resting
//    Halt                      bci_Halt
//
//    Existence of the above-listed Matlab functions is not mandatory.
//    The MatlabFilter uses the Matlab 'exist' command to determine whether a
//    given function is available, and will not call the Matlab engine when this
//    is not the case.
//    If either of the bci_Preflight, bci_Initialize, or bci_Process functions
//    is not available, a warning will be displayed to the user.
//
//    Parameters and states are accessible via global Matlab structs called
//    'bci_Parameters' and 'bci_States'. In Matlab, write
//
//      global bci_Parameters bci_States;
//      my_sampling_rate = bci_Parameters.SamplingRate;
//
//    Parameters may be changed from 'bci_StopRun' and 'bci_Resting', and will
//    automatically be propagated to the other modules.
//    State values may be modified from the 'bci_Process' function.
//
//    To add parameters and states to the BCI2000 list of states, the 'bci_Construct'
//    function may return non-empty cell arrays of strings in its 'parameters'
//    and 'states' return values. The strings constituting these cell arrays must
//    follow the BCI2000 parameter/state definition syntax as described in sections
//    3.2.4 and 3.2.5 of the "BCI2000 project outline" document.
//
//    BCI2000 signals are mapped to Matlab matrices with the channel index first,
//    and sample (element) index second.
//    Signal dimension arguments of bci_Preflight and bci_Initialize are
//    vectors of integers (1x2 matrices) as in '[n_channels n_elements]'.
//
//    To report errors from Matlab functions, use Matlab's error() command.
//
//    Troubleshooting:
//    If no Matlab instance opens up, execute
//      matlab /regserver
//    from the command line when logged in with administrative privileges.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "MatlabFilter.h"
#include "shared/defines.h"

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
: mBci_Process( PROCESS ),
  mVisualization( SOURCEID::TEMPORALFILT ),
  mVisualize( false )
{
  BEGIN_PARAMETER_DEFINITIONS
    "Visualize int VisualizeMatlab= 0 0 0 1 "
      "// Visualize the Matlab filter's output (boolean)",
  END_PARAMETER_DEFINITIONS

  MatlabFunction bci_Construct( CONSTRUCT );
  bci_Construct.OutputArgument( PARAM_DEFS )
               .OutputArgument( STATE_DEFS );
  if( CallMatlab( bci_Construct ) )
  {
    // Add the parameters and states requested by the Matlab bci_Construct function.
    int numParamDefs = MatlabEngine::GetScalar( "max(size(" PARAM_DEFS "))" );
    for( int i = 1; i <= numParamDefs; ++i )
    {
      ostringstream expr;
      expr << PARAM_DEFS << "{" << i << "}";
      string paramDef = MatlabEngine::GetString( expr.str() );
      if( !Parameters->Add( paramDef ) )
        bcierr << "Error in parameter definition: " << paramDef << endl;
    }
    int numStateDefs = MatlabEngine::GetScalar( "max(size(" STATE_DEFS "))" );
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
           << "Make sure that the m-files exist within the path, and contain "
           << "appropriate function definitions."
           << endl;

  // Initialize the bci_Process function for more efficient calling during Process().
  mBci_Process.InputArgument( IN_SIGNAL )
              .OutputArgument( OUT_SIGNAL );
}

MatlabFilter::~MatlabFilter()
{
  MatlabFunction bci_Destruct( DESTRUCT );
  CallMatlab( bci_Destruct );

  MatlabEngine::ClearVariable( IN_SIGNAL );
  MatlabEngine::ClearVariable( OUT_SIGNAL );
  MatlabEngine::ClearVariable( PARAMETERS );
  MatlabEngine::ClearVariable( STATES );
}

void
MatlabFilter::Preflight( const SignalProperties& inSignalProperties,
                               SignalProperties& outSignalProperties ) const
{
  MatlabEngine::ClearVariable( PARAMETERS );
  MatlabEngine::CreateGlobal( PARAMETERS );
  ParamsToMatlabWS();
  MatlabEngine::ClearVariable( STATES );
  MatlabEngine::CreateGlobal( STATES );
  StatesToMatlabWS();

  outSignalProperties = inSignalProperties;
  MatlabEngine::PutMatrix( IN_SIGNAL_DIMS, inSignalProperties );
  MatlabFunction bci_Preflight( PREFLIGHT );
  bci_Preflight.InputArgument( IN_SIGNAL_DIMS )
               .OutputArgument( OUT_SIGNAL_DIMS );
  if( CallMatlab( bci_Preflight ) )
    outSignalProperties = MatlabEngine::GetMatrix( OUT_SIGNAL_DIMS );
  MatlabEngine::ClearVariable( IN_SIGNAL_DIMS );
  MatlabEngine::ClearVariable( OUT_SIGNAL_DIMS );
}

void
MatlabFilter::Initialize2( const SignalProperties& inSignalProperties,
                           const SignalProperties& outSignalProperties )
{
  MatlabEngine::PutMatrix( IN_SIGNAL_DIMS, inSignalProperties );
  MatlabEngine::PutMatrix( OUT_SIGNAL_DIMS, outSignalProperties );
  MatlabFunction bci_Initialize( INITIALIZE );
  bci_Initialize.InputArgument( IN_SIGNAL_DIMS )
                .InputArgument( OUT_SIGNAL_DIMS );
  CallMatlab( bci_Initialize );
  MatlabEngine::ClearVariable( IN_SIGNAL_DIMS );
  MatlabEngine::ClearVariable( OUT_SIGNAL_DIMS );
  MatlabEngine::PutMatrix( IN_SIGNAL, GenericSignal( inSignalProperties ) );
  MatlabEngine::PutMatrix( OUT_SIGNAL, GenericSignal( outSignalProperties ) );

  mVisualize = ( Parameter( "VisualizeMatlab" ) != 0 );
  if( mVisualize )
  {
    mVisualization.Send( CFGID::WINDOWTITLE, "Matlab" );
    mVisualization.Send( CFGID::MINVALUE, -40 );
    mVisualization.Send( CFGID::MAXVALUE, 40 );
    mVisualization.Send( CFGID::NUMSAMPLES, 256 );
  }
}

void
MatlabFilter::Process( const GenericSignal* input, GenericSignal* output )
{
  StatesToMatlabWS();
  MatlabEngine::PutMatrix( IN_SIGNAL, *input );
  if( CallMatlab( mBci_Process ) )
    *output = MatlabEngine::GetMatrix( OUT_SIGNAL );
  else
    *output = *input;
  MatlabWSToStates();

  if( mVisualize )
    mVisualization.Send( *output );
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
  if( CallMatlab( bci_Resting ) )
    MatlabWSToParams();
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
  for( size_t i = 0; i < States->Size(); ++i )
  {
    const char* name = ( *States )[ i ].GetName();
    MatlabEngine::PutScalar( string( STATES "." ) + name, State( name ) );
  }
}

void
MatlabFilter::MatlabWSToStates()
{
  for( size_t i = 0; i < States->Size(); ++i )
  {
    const char* name = ( *States )[ i ].GetName();
    State( name ) = MatlabEngine::GetScalar( string( STATES "." ) + name );
  }
}

void
MatlabFilter::ParamsToMatlabWS() const
{
  for( size_t i = 0; i < Parameters->Size(); ++i )
  {
    PARAM& p = ( *Parameters )[ i ];
    MatlabEngine::PutCells( string( PARAMETERS "." ) + p.GetName(), p );
  }
}

void
MatlabFilter::MatlabWSToParams()
{
  for( size_t i = 0; i < Parameters->Size(); ++i )
  {
    PARAM& p = ( *Parameters )[ i ];
    MatlabEngine::StringMatrix values = MatlabEngine::GetCells( string( PARAMETERS "." ) + p.GetName() );
    size_t newRows = values.size(),
           newCols = newRows ? values.at( 0 ).size() : 1;
    if( newRows != p.GetNumRows() || newCols != p.GetNumColumns() )
      p.SetDimensions( newRows, newCols );
    for( size_t row = 0; row < newRows; ++row )
      for( size_t col = 0; col < newCols; ++col )
        if( string( p.GetValue( row, col ) ) != values.at( row ).at( col ) )
          p.SetValue( values.at( row ).at( col ).c_str(), row, col );
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
    __bcierr << "Matlab function \"" << inFunction.Name() << "\": "
             << err
             << endl;
    return false;
  }
  return true;
}

