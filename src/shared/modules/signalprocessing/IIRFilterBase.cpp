////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: An abstract base class that implements an IIR filter.
//   Subclasses will provide individual implementations for the
//   DesignFilter() member, which is supposed to translate parameter settings
//   into a rational transfer function (complex poles and zeros).
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

#include "IIRFilterBase.h"

using namespace std;

IIRFilterBase::IIRFilterBase()
{
}

IIRFilterBase::~IIRFilterBase()
{
}

void
IIRFilterBase::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
  Real          preflightGain;
  ComplexVector preflightZeros,
                preflightPoles;
  DesignFilter( Input, preflightGain, preflightZeros, preflightPoles );
  if( preflightZeros.size() != preflightPoles.size() )
    bcierr << "The numbers of zeros and poles must agree" << endl;
  Output = Input;
}

void
IIRFilterBase::Initialize( const SignalProperties& Input, const SignalProperties& /*Output*/ )
{
  Real gain;
  ComplexVector zeros, poles;
  DesignFilter( Input, gain, zeros, poles );
  mFilter.SetGain( gain )
         .SetZeros( zeros )
         .SetPoles( poles )
         .Initialize( Input.Channels() );
}

void
IIRFilterBase::StartRun()
{
  mFilter.Initialize();
}

void
IIRFilterBase::Process( const GenericSignal& Input, GenericSignal& Output )
{
  mFilter.Process( Input, Output );
}


