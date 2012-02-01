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
#include "SysCommand.h"
#include "Uncopyable.h"

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
  "binary",
  "          --operator=<file>     Direct visualization messages to <file>",
  ""
};


class FilterWrapper : public MessageHandler, private Uncopyable
{
 public:
  FilterWrapper( istream& in, ostream& out, ostream& op );
  ~FilterWrapper();

  static string FilterName();
  void Run();

 private:
  virtual bool HandleParam( istream& );
  virtual bool HandleState( istream& );
  virtual bool HandleVisSignalProperties( istream& );
  virtual bool HandleVisSignal( istream& );
  virtual bool HandleStateVector( istream& );

  void FinishProcessing();
  void StopRun();
  void OutputParameterChanges();
  void InitializeInputStatevector();
  void InitializeOutputStatevector();
  void DisposeStatevectors();
  void SynchronizeStatevectors();

 private:
  istream& mrIn;
  ostream& mrOut,
         & mrOperator;
  SignalProperties* mpInputProperties;
  GenericSignal mOutputSignal;
  ParamList mParamlist;
  StateList mInputStatelist,
            mOutputStatelist,
            mFilterStatelist;
  StateVector* mpInputStatevector,
             * mpOutputStatevector;
  bool      mSingleStatevector;

};


ToolResult
ToolInit()
{
  string filterName = FilterWrapper::FilterName();
  for( int i = 0; ToolInfo[ i ] != ""; ++i )
  {
    size_t namePos;
    while( ( namePos = ToolInfo[ i ].find( FILTER_NAME ) ) != string::npos )
      ToolInfo[ i ].replace( namePos, string( FILTER_NAME ).length(), filterName );
  }
  return noError;
}

ToolResult
ToolMain( OptionSet& arOptions, istream& arIn, ostream& arOut )
{
  ToolResult result = noError;
  ofstream operatorOut;
  if( arOptions.size() == 1 )
  {
    string operatorFile = arOptions.getopt( "--operator", "" );
    if( operatorFile == "" )
      return illegalOption;
    operatorOut.open( operatorFile.c_str() );
  }
  FilterWrapper wrapper( arIn, arOut, operatorOut );
  wrapper.Run();
  if( bcierr__.Flushes() > 0 || !arIn )
    result = illegalInput;
  return result;
}

FilterWrapper::FilterWrapper( istream& arIn, ostream& arOut, ostream& arOp )
: mrIn( arIn ),
  mrOut( arOut ),
  mrOperator( arOp ),
  mpInputProperties( NULL ),
  mpInputStatevector( NULL ),
  mpOutputStatevector( NULL ),
  mSingleStatevector( true )
{
  GenericVisualization::SetOutputStream( &mrOperator );
}

FilterWrapper::~FilterWrapper()
{
  DisposeStatevectors();
  delete mpInputProperties;
}

string
FilterWrapper::FilterName()
{
  string name = "<n/a>";
  ParamList paramlist;
  StateList statelist;
  EnvironmentBase::EnterConstructionPhase( &paramlist, &statelist, NULL );
  GenericFilter::InstantiateFilters();
  EnvironmentBase::EnterNonaccessPhase();
  GenericFilter* pFilter = GenericFilter::GetFilter<GenericFilter>();
  if( pFilter == NULL )
    cerr << "Could not create filter instance.\n"
         << "Make sure there is a filter definition with a "
         << "\"RegisterFilter\" statement linked into the executable."
         << endl;
  else
    name = ClassName( typeid( *pFilter ) );
  return name;
}

void
FilterWrapper::FinishProcessing()
{
  if( Environment::Phase() == Environment::processing )
    StopRun();
  if( Environment::Phase() != Environment::nonaccess )
    EnvironmentBase::EnterNonaccessPhase();
}

void
FilterWrapper::Run()
{
  while( mrIn && mrIn.peek() != EOF )
    HandleMessage( mrIn );
  FinishProcessing();
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
          EnvironmentBase::EnterConstructionPhase( &filterParams, &mFilterStatelist, NULL );
          GenericFilter::InstantiateFilters();
          if( bcierr__.Flushes() > 0 )
          {
            arIn.setstate( ios::failbit );
            break;
          }
          // Make sure the filter's parameters get their properties from the filter
          // rather than the input stream.
          for( int i = 0; i < filterParams.Size(); ++i )
          {
            const string& name = filterParams[i].Name();
            if( mParamlist.Exists( name ) )
              filterParams[i].AssignValues( mParamlist[name] );
            mParamlist[name] = filterParams[i];
          }
        }
        /* no break */
      case Environment::construction:
        if( mpInputStatevector == NULL )
          InitializeInputStatevector();
        InitializeOutputStatevector();
        for( int i = 0; i < mOutputStatelist.Size(); ++i )
          PutMessage( mrOut, mOutputStatelist[ i ] );
        EnvironmentBase::EnterPreflightPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector );
        if( mpInputProperties != NULL
            && inputSignal.Channels() == mpInputProperties->Channels()
            && inputSignal.Elements() == mpInputProperties->Elements() )
        {
          mpInputProperties->SetUpdateRate( 1.0 / MeasurementUnits::SampleBlockDuration() );
          GenericFilter::PreflightFilters( *mpInputProperties, outputProperties );
        }
        else
        {
          delete mpInputProperties;
          mpInputProperties = NULL;
          SignalProperties inputProperties( inputSignal.Properties() );
          inputProperties.SetUpdateRate( 1.0 / MeasurementUnits::SampleBlockDuration() );
          GenericFilter::PreflightFilters( inputProperties, outputProperties );
        }
        mOutputSignal.SetProperties( outputProperties );
        if( bcierr__.Flushes() > 0 )
        {
          arIn.setstate( ios::failbit );
          break;
        }
        /* no break */
      case Environment::preflight:
        EnvironmentBase::EnterInitializationPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector );
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
        EnvironmentBase::EnterStartRunPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector );
        GenericFilter::StartRunFilters();
        EnvironmentBase::EnterNonaccessPhase();
        EnvironmentBase::EnterProcessingPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector );
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
  EnvironmentBase::EnterStopRunPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector );
  GenericFilter::StopRunFilters();
  EnvironmentBase::EnterNonaccessPhase();
  OutputParameterChanges();
  EnvironmentBase::EnterRestingPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector );
  GenericFilter::RestingFilters();
  EnvironmentBase::EnterNonaccessPhase();
  OutputParameterChanges();
  EnvironmentBase::EnterRestingPhase( &mParamlist, &mOutputStatelist, mpOutputStatevector );
}

void
FilterWrapper::OutputParameterChanges()
{
  ParamList changedParameters;
  for( int i = 0; i < mParamlist.Size(); ++i )
    if( mParamlist[ i ].Changed() )
      changedParameters.Add( mParamlist[ i ] );

  if( !changedParameters.Empty() )
  {
    bool success = MessageHandler::PutMessage( mrOut, changedParameters )
                && MessageHandler::PutMessage( mrOut, SysCommand::EndOfParameter );
    if( !success )
      bcierr << "Could not publish changed parameters" << endl;
  }
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

