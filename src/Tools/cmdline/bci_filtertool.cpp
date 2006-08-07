////////////////////////////////////////////////////////////////////
// $Id$
// File:    bci_filtertool.cpp
// Date:    Jul 18, 2003
// Author:  juergen.mellinger@uni-tuebingen.de
// Description: A BCI2000 filter wrapper that reads a BCI2000
//          compliant binary stream from standard input, applies
//          a BCI2000 filter, and writes its output to the
//          standard output as a BCI2000 compliant binary stream.
// $Log$
// Revision 1.15  2006/08/07 19:04:30  mellinger
// Fixed handling of states that are requested by filters but not present in the input stream.
//
// Revision 1.14  2006/07/04 16:02:21  mellinger
// Introduced namespace "bci", put the ClassName() global function inside that namespace.
//
// Revision 1.13  2006/03/30 10:20:20  mellinger
// Added missing call to Environment::EnterNonaccessPhase().
//
// Revision 1.12  2006/02/03 13:40:53  mellinger
// Compatibility with gcc and BCB 2006.
//
// Revision 1.11  2006/01/12 20:37:14  mellinger
// Adaptation to latest revision of parameter and state related class interfaces.
//
////////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "bci_tool.h"
#include "shared/defines.h"
#include "shared/UParameter.h"
#include "shared/UState.h"
#include "shared/UGenericVisualization.h"
#include "shared/UGenericFilter.h"
#include "shared/MessageHandler.h"
#include "ClassName.h"

#define FILTER_NAME "$FILTER$"

using namespace std;
using namespace bci;

string ToolInfo[] =
{
  "",
  "tool, framework $Revision$, compiled "__DATE__,
  "Process standard input with the \"" FILTER_NAME "\" BCI2000 filter.",
  "Reads a BCI2000 compliant binary stream from standard input, applies the\n"
    FILTER_NAME " BCI2000 filter, and writes its output to standard output\n"
    "as a BCI2000 compliant binary stream.",
  "-o<file>, --operator<file>\tdirect visualization messages to <file>",
  "                          \tinstead of /dev/null",
  ""
};


class FilterWrapper : public MessageHandler
{
 public:
  FilterWrapper( ostream& arOut )
  : mrOut( arOut ),
    mpInputStatevector( NULL ),
    mpOutputStatevector( NULL ),
    mSingleStatevector( true ) {}
  ~FilterWrapper() { DisposeStatevectors(); }

  void RedirectOperator( string file ) { mOperator.open( file.c_str() ); }
  void FinishProcessing();
  static const char* FilterName();

 private:
  ostream& mrOut;
  ofstream mOperator;
  GenericSignal mOutputSignal;
  PARAMLIST mParamlist;
  STATELIST mInputStatelist,
            mOutputStatelist,
            mFilterStatelist;
  STATEVECTOR* mpInputStatevector,
             * mpOutputStatevector;
  bool      mSingleStatevector;

  virtual bool HandlePARAM(       istream& );
  virtual bool HandleSTATE(       istream& );
  virtual bool HandleVisSignal(   istream& );
  virtual bool HandleSTATEVECTOR( istream& );

  void StopRun();
  void InitializeInputStatevector();
  void InitializeOutputStatevector();
  void DisposeStatevectors();
  void SynchronizeStatevectors();
};


ToolResult
ToolInit()
{
  const char* pFilterName = FilterWrapper::FilterName();
  if( pFilterName == NULL )
    return genericError;
  for( int i = 0; ToolInfo[ i ] != ""; ++i )
  {
    size_t namePos;
    while( ( namePos = ToolInfo[ i ].find( FILTER_NAME ) ) != string::npos )
      ToolInfo[ i ].replace( namePos, string( FILTER_NAME ).length(), pFilterName );
  }
  return noError;
}

ToolResult
ToolMain( const OptionSet& arOptions, istream& arIn, ostream& arOut )
{
  FilterWrapper wrapper( arOut );
  if( arOptions.size() == 1 )
  {
    string operatorFile = arOptions.getopt( "-o|-O|--operator", "" );
    if( operatorFile == "" )
      return illegalOption;
    wrapper.RedirectOperator( operatorFile.c_str() );
  }
  while( arIn && arIn.peek() != EOF )
    wrapper.HandleMessage( arIn );
  wrapper.FinishProcessing();
  if( !arIn )
    return illegalInput;
  return noError;
}

const char*
FilterWrapper::FilterName()
{
  const char* pName = NULL;
  PARAMLIST paramlist;
  STATELIST statelist;
  Environment::EnterConstructionPhase( &paramlist, &statelist, NULL, NULL );
  GenericFilter::InstantiateFilters();
  Environment::EnterNonaccessPhase();
  GenericFilter* pFilter = GenericFilter::GetFilter<GenericFilter>();
  if( pFilter == NULL )
    cerr << "Could not create filter instance.\n"
         << "Make sure there is a filter definition with a "
         << "\"RegisterFilter\" statement linked into the executable."
         << endl;
  else
    pName = ClassName( typeid( *pFilter ) );
  return pName;
}

void
FilterWrapper::FinishProcessing()
{
  if( Environment::GetPhase() == Environment::processing )
    StopRun();
  if( Environment::GetPhase() != Environment::nonaccess )
    Environment::EnterNonaccessPhase();
}

bool
FilterWrapper::HandlePARAM( istream& arIn )
{
  FinishProcessing();

  PARAM p;
  p.ReadBinary( arIn );
  if( arIn )
    mParamlist[ p.GetName() ] = p;
  return true;
}

bool
FilterWrapper::HandleSTATE( istream& arIn )
{
  if( Environment::GetPhase() == Environment::processing )
    StopRun();

  if( mpInputStatevector != NULL )
  {
     mInputStatelist.Clear();
     DisposeStatevectors();
  }

  STATE s;
  s.ReadBinary( arIn );
  if( arIn )
    mInputStatelist.Add( s );
  return true;
}

bool
FilterWrapper::HandleSTATEVECTOR( istream& arIn )
{
  if( mpInputStatevector == NULL )
    InitializeInputStatevector();
  mpInputStatevector->ReadBinary( arIn );
  SynchronizeStatevectors();
  if( !mpInputStatevector->GetStateValue( "Running" )
      && Environment::GetPhase() == Environment::processing )
    StopRun();
  return true;
}

bool
FilterWrapper::HandleVisSignal( istream& arIn )
{
  VisSignal s;
  if( s.ReadBinary( arIn ) && s.GetSourceID() == 0 )
  {
    const GenericSignal& inputSignal = s;
    SignalProperties outputProperties;
    switch( Environment::GetPhase() )
    {
      case Environment::nonaccess:
        {
          GenericFilter::DisposeFilters();

          PARAMLIST filterParams;
          Environment::EnterConstructionPhase( &filterParams, &mFilterStatelist, NULL, &mOperator );
          GenericFilter::InstantiateFilters();
          if( __bcierr.flushes() > 0 )
          {
            arIn.setstate( ios::failbit );
            break;
          }
          // Add the filter's parameters with their default values to the parameter
          // list as far as they are missing from the input stream.
          for( size_t i = 0; i < filterParams.Size(); ++i )
            if( !mParamlist.Exists( filterParams[ i ].GetName() ) )
              mParamlist[ filterParams[ i ].GetName() ] = filterParams[ i ];
        }
        /* no break */
      case Environment::construction:
        if( mpInputStatevector == NULL )
          InitializeInputStatevector();
        InitializeOutputStatevector();
        Environment::EnterPreflightPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector, &mOperator );
        GenericFilter::PreflightFilters( inputSignal.GetProperties(), outputProperties );
        mOutputSignal.SetProperties( outputProperties );
        if( __bcierr.flushes() > 0 )
        {
          arIn.setstate( ios::failbit );
          break;
        }
        /* no break */
      case Environment::preflight:
        Environment::EnterInitializationPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector, &mOperator );
        GenericFilter::InitializeFilters();
        for( size_t i = 0; i < mParamlist.Size(); ++i )
          PutMessage( mrOut, mParamlist[ i ] );
        if( __bcierr.flushes() > 0 )
        {
          arIn.setstate( ios::failbit );
          break;
        }
        /* no break */
      case Environment::initialization:
      case Environment::resting:
        /* no break */
        Environment::EnterStartRunPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector, &mOperator );
        GenericFilter::StartRunFilters();
        Environment::EnterProcessingPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector, &mOperator );
        /* no break */
      case Environment::processing:
        {
          bool wasRunning = mpOutputStatevector->GetStateValue( "Running" );
          GenericFilter::ProcessFilters( &inputSignal, &mOutputSignal );
          if( __bcierr.flushes() > 0 )
          {
            arIn.setstate( ios::failbit );
            break;
          }
          bool isRunning = mpOutputStatevector->GetStateValue( "Running" );
          if( isRunning || wasRunning )
            if( mpOutputStatevector->Length() > 0 )
              PutMessage( mrOut, *mpOutputStatevector );
          if( isRunning )
            PutMessage( mrOut, mOutputSignal );
        }
        break;
      default:
        bcierr << "Unknown Environment phase" << endl;
        arIn.setstate( ios::failbit );
    }
  }
  return arIn;
}

void
FilterWrapper::StopRun()
{
  Environment::EnterStopRunPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector, &mrOut );
  GenericFilter::StopRunFilters();
  Environment::EnterNonaccessPhase();
  Environment::EnterRestingPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector, &mrOut );
  GenericFilter::RestingFilters();
  Environment::EnterNonaccessPhase();
  Environment::EnterRestingPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector, &mOperator );
}

void
FilterWrapper::InitializeInputStatevector()
{
  delete mpInputStatevector;
  mpInputStatevector = new STATEVECTOR( mInputStatelist, true );
}

void
FilterWrapper::InitializeOutputStatevector()
{
  if( !mSingleStatevector )
    delete mpOutputStatevector;

  mSingleStatevector = mFilterStatelist.Empty();
  if( mSingleStatevector )
  {
    mOutputStatelist = mInputStatelist;
    mpOutputStatevector = mpInputStatevector;
  }
  else
  {
    for( size_t i = 0; i < mInputStatelist.Size(); ++i )
      mOutputStatelist.Add( mInputStatelist[ i ] );
    for( size_t i = 0; i < mFilterStatelist.Size(); ++i )
      mOutputStatelist.Add( mFilterStatelist[ i ] );
    mpOutputStatevector = new STATEVECTOR( mOutputStatelist, false );
  }
  SynchronizeStatevectors();
  for( size_t i = 0; i < mOutputStatelist.Size(); ++i )
    PutMessage( mrOut, mOutputStatelist[ i ] );
}

void
FilterWrapper::SynchronizeStatevectors()
{
  if( !mSingleStatevector )
    // Copy state values by name if input and output statevectors differ.
    for( size_t i = 0; i < mInputStatelist.Size(); ++i )
    {
      const char* name = mInputStatelist[ i ].GetName();
      STATE::value_type value = mpInputStatevector->GetStateValue( name );
      mpOutputStatevector->SetStateValue( name, value );
    }
}

void
FilterWrapper::DisposeStatevectors()
{
  delete mpInputStatevector;
  mpInputStatevector = NULL;
  if( !mSingleStatevector )
    delete mpOutputStatevector;
  mpOutputStatevector = NULL;
  mSingleStatevector = true;
}


