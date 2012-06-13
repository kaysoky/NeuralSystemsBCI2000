////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: See the header file for a description.
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

#include "StandaloneFilters.h"
#include "IdentityFilter.h"
#include "BCIError.h"

using namespace std;

StandaloneChain::StandaloneChain()
: EnvironmentBase( mParameters, mStates, mStatevector ),
  mChainState( uninitialized ),
  mSignals( 1 )
{
}

StandaloneChain::~StandaloneChain()
{
  for( size_t i = 0; i < mFilters.size(); ++i )
    delete mFilters[i];
}

bool
StandaloneChain::SetConfig( const SignalProperties& inS )
{
  if( mFilters.empty() )
    AddFilter<IdentityFilter>();
  Lock<BCIError::OutStream> lock( bcierr__ );
  bcierr__.Clear();
  if( mChainState == running )
    bcierr << "Still in running state" << endl;
  if( bcierr__.Empty() )
    DoPreflightInitialize( inS );
  bool success = bcierr__.Empty();
  bcierr__.Clear();
  if( success )
    mChainState = suspended;
  return success;
}

void
StandaloneChain::Start()
{
  if( mChainState != suspended )
    bcierr << "Not in suspended state" << endl;
  else
  {
    for( size_t i = 0; i < mFilters.size(); ++i )
      mFilters[i]->CallStartRun();
    mChainState = running;
  }
}

void
StandaloneChain::Stop()
{
  if( mChainState != running )
    bcierr << "Not in running state" << endl;
  else
  {
    for( size_t i = 0; i < mFilters.size(); ++i )
    {
      mFilters[i]->CallStopRun();
      mFilters[i]->CallResting();
      mFilters[i]->CallHalt();
    }
    mChainState = suspended;
  }
}

void
StandaloneChain::Process( const GenericSignal& inS )
{
  if( mChainState != running )
    bcierr << "Not in running state" << endl;
  else
  {
    const GenericSignal* pInput = &inS;
    GenericSignal* pOutput = NULL;
    for( size_t i = 0; i < mFilters.size(); ++i )
    {
      pOutput = &mSignals[i];
      mFilters[i]->CallProcess( *pInput, *pOutput );
      pInput = pOutput;
    }
  }
}

void
StandaloneChain::DoAddFilter( GenericFilter* inpFilter )
{
  if( mChainState != uninitialized )
    bcierr << "Cannot add filters after initialization" << endl;
  mFilters.push_back( inpFilter );
  inpFilter->CallPublish();
  mStates.AssignPositions();
  mStatevector = StateVector( mStates, 1 );
}

void
StandaloneChain::DoPreflightInitialize( const SignalProperties& inS )
{
  mSignals.clear();
  const SignalProperties *pInput = &inS;
  SignalProperties Output;
  for( size_t i = 0; i < mFilters.size(); ++i )
  {
    Output = *pInput;
    mFilters[i]->CallPreflight( *pInput, Output );
    mSignals.push_back( GenericSignal( Output ) );
    pInput = &mSignals.back().Properties();
  }
  if( bcierr__.Empty() )
  {
    const SignalProperties *pInput = &inS,
                           *pOutput = NULL;
    for( size_t i = 0; i < mFilters.size(); ++i )
    {
      pOutput = &mSignals[i].Properties();
      mFilters[i]->CallInitialize( *pInput, *pOutput );
      pInput = pOutput;
    }
  }
}

