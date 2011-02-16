////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A BCI2000 filter wrapper that reads a BCI2000
//          compliant binary stream from standard input, applies
//          a BCI2000 filter, and writes its output to the
//          standard output as a BCI2000 compliant binary stream.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
////////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "bci_tool.h"
#include "Param.h"
#include "ParamList.h"
#include "State.h"
#include "StateList.h"
#include "StateVector.h"
#include "GenericVisualization.h"
#include "GenericFilter.h"
#include "MessageHandler.h"
#include "ClassName.h"
#include "Version.h"

#define FILTER_NAME "$FILTER$"

using namespace std;
using namespace bci;

string ToolInfo[] =
{
  "",
  BCI2000_VERSION,
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
  FilterWrapper( ostream& arOut );
  ~FilterWrapper();

  void RedirectOperator( string file ) { mOperator.open( file.c_str() ); }
  void FinishProcessing();
  static const char* FilterName();

 private:
  ostream& mrOut;
  ofstream mOperator;
  SignalProperties* mpInputProperties;
  GenericSignal mOutputSignal;
  ParamList mParamlist;
  StateList mInputStatelist,
            mOutputStatelist,
            mFilterStatelist;
  StateVector* mpInputStatevector,
             * mpOutputStatevector;
  bool      mSingleStatevector;

  virtual bool HandleParam( istream& );
  virtual bool HandleState( istream& );
  virtual bool HandleVisSignalProperties( istream& );
  virtual bool HandleVisSignal( istream& );
  virtual bool HandleStateVector( istream& );

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
  ToolResult result = noError;
  try
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
  }
  catch( const char* s )
  {
    bcierr << s << endl;
  }
  catch( const exception& e )
  {
    bcierr << "caught exception "
           << typeid( e ).name() << " (" << e.what() << "),\n"
           << "terminating module"
           << endl;
  }
  if( bcierr__.Flushes() > 0 || !arIn )
    result = illegalInput;
  return result;
}

FilterWrapper::FilterWrapper( ostream& arOut )
: mrOut( arOut ),
  mpInputProperties( NULL ),
  mpInputStatevector( NULL ),
  mpOutputStatevector( NULL ),
  mSingleStatevector( true )
{
}

FilterWrapper::~FilterWrapper()
{
  DisposeStatevectors();
  delete mpInputProperties;
}

const char*
FilterWrapper::FilterName()
{
  const char* pName = NULL;
  ParamList paramlist;
  StateList statelist;
  EnvironmentBase::EnterConstructionPhase( &paramlist, &statelist, NULL, NULL );
  GenericFilter::InstantiateFilters();
  EnvironmentBase::EnterNonaccessPhase();
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
  if( Environment::Phase() == Environment::processing )
    StopRun();
  if( Environment::Phase() != Environment::nonaccess )
    EnvironmentBase::EnterNonaccessPhase();
}

bool
FilterWrapper::HandleParam( istream& arIn )
{
  FinishProcessing();

  Param p;
  p.ReadBinary( arIn );
  if( arIn )
    mParamlist[ p.Name() ] = p;
  return true;
}

bool
FilterWrapper::HandleState( istream& arIn )
{
  if( Environment::Phase() == Environment::processing )
    StopRun();

  if( mpInputStatevector != NULL )
  {
     mInputStatelist.Clear();
     DisposeStatevectors();
  }

  State s;
  s.ReadBinary( arIn );
  if( arIn )
    mInputStatelist.Add( s );
  return true;
}

bool
FilterWrapper::HandleStateVector( istream& arIn )
{
  if( mpInputStatevector == NULL )
  {
    InitializeInputStatevector();
    for( int i = 0; i < mInputStatelist.Size(); ++i )
      PutMessage( mrOut, mInputStatelist[ i ] );
  }
  mpInputStatevector->ReadBinary( arIn );
  SynchronizeStatevectors();
  if( !mpInputStatevector->StateValue( "Running" )
      && Environment::Phase() == Environment::processing )
    StopRun();
  return true;
}

bool
FilterWrapper::HandleVisSignalProperties( istream& arIn )
{
  delete mpInputProperties;
  mpInputProperties = NULL;
  VisSignalProperties v;
  if( v.ReadBinary( arIn ) && v.SourceID().empty() )
    mpInputProperties = new SignalProperties( v.SignalProperties() );
  return mpInputProperties != NULL;
}

bool
FilterWrapper::HandleVisSignal( istream& arIn )
{
  VisSignal s;
  if( s.ReadBinary( arIn ) && s.SourceID().empty() )
  {
    const GenericSignal& inputSignal = s;
    SignalProperties outputProperties;
    switch( Environment::Phase() )
    {
      case Environment::nonaccess:
        {
          GenericFilter::DisposeFilters();

          ParamList filterParams;
          mFilterStatelist.Clear();
          EnvironmentBase::EnterConstructionPhase( &filterParams, &mFilterStatelist, NULL, &mOperator );
          GenericFilter::InstantiateFilters();
          if( bcierr__.Flushes() > 0 )
          {
            arIn.setstate( ios::failbit );
            break;
          }
          // Add the filter's parameters with their default values to the parameter
          // list as far as they are missing from the input stream.
          for( int i = 0; i < filterParams.Size(); ++i )
            if( !mParamlist.Exists( filterParams[ i ].Name() ) )
              mParamlist[ filterParams[ i ].Name() ] = filterParams[ i ];
        }
        /* no break */
      case Environment::construction:
        if( mpInputStatevector == NULL )
          InitializeInputStatevector();
        InitializeOutputStatevector();
        for( int i = 0; i < mOutputStatelist.Size(); ++i )
          PutMessage( mrOut, mOutputStatelist[ i ] );
        EnvironmentBase::EnterPreflightPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector, &mOperator );
        if( mpInputProperties != NULL
            && inputSignal.Channels() == mpInputProperties->Channels()
            && inputSignal.Elements() == mpInputProperties->Elements() )
        {
          GenericFilter::PreflightFilters( *mpInputProperties, outputProperties );
        }
        else
        {
          delete mpInputProperties;
          mpInputProperties = NULL;
          GenericFilter::PreflightFilters( inputSignal.Properties(), outputProperties );
        }
        mOutputSignal.SetProperties( outputProperties );
        if( bcierr__.Flushes() > 0 )
        {
          arIn.setstate( ios::failbit );
          break;
        }
        /* no break */
      case Environment::preflight:
        EnvironmentBase::EnterInitializationPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector, &mOperator );
        GenericFilter::InitializeFilters();
        for( int i = 0; i < mParamlist.Size(); ++i )
          PutMessage( mrOut, mParamlist[ i ] );
        PutMessage( mrOut, outputProperties );
        if( bcierr__.Flushes() > 0 )
        {
          arIn.setstate( ios::failbit );
          break;
        }
        /* no break */
      case Environment::initialization:
      case Environment::resting:
        /* no break */
        EnvironmentBase::EnterStartRunPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector, &mOperator );
        GenericFilter::StartRunFilters();
        EnvironmentBase::EnterNonaccessPhase();
        EnvironmentBase::EnterProcessingPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector, &mOperator );
        /* no break */
      case Environment::processing:
        {
          GenericFilter::ProcessFilters( inputSignal, mOutputSignal );
          if( bcierr__.Flushes() > 0 )
          {
            arIn.setstate( ios::failbit );
            break;
          }
          PutMessage( mrOut, *mpOutputStatevector );
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
  EnvironmentBase::EnterStopRunPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector, &mrOut );
  GenericFilter::StopRunFilters();
  EnvironmentBase::EnterNonaccessPhase();
  EnvironmentBase::EnterRestingPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector, &mrOut );
  GenericFilter::RestingFilters();
  EnvironmentBase::EnterNonaccessPhase();
  EnvironmentBase::EnterRestingPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector, &mOperator );
}

void
FilterWrapper::InitializeInputStatevector()
{
  delete mpInputStatevector;
  mpInputStatevector = new StateVector( mInputStatelist );
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
    for( int i = 0; i < mInputStatelist.Size(); ++i )
      mOutputStatelist.Add( mInputStatelist[ i ] );
    for( int i = 0; i < mFilterStatelist.Size(); ++i )
      mOutputStatelist.Add( mFilterStatelist[ i ] );
    mOutputStatelist.AssignPositions();
    mpOutputStatevector = new StateVector( mOutputStatelist );
  }
  SynchronizeStatevectors();
  for( int i = 0; i < mOutputStatelist.Size(); ++i )
    PutMessage( mrOut, mOutputStatelist[ i ] );
}

void
FilterWrapper::SynchronizeStatevectors()
{
  if( !mSingleStatevector )
    // Copy state values by name if input and output statevectors differ.
    for( int i = 0; i < mInputStatelist.Size(); ++i )
    {
      const string& name = mInputStatelist[ i ].Name();
      State::ValueType value = mpInputStatevector->StateValue( name );
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

