////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: An abstract base class that implements an IIR filter.
//   Subclasses will provide individual implementations for the
//   DesignFilter() member, which is supposed to translate parameter settings
//   into a rational transfer function (complex poles and zeros).
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
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
  DesignFilter( preflightGain, preflightZeros, preflightPoles );
  if( preflightZeros.size() != preflightPoles.size() )
    bcierr << "The numbers of zeros and poles must agree" << endl;
  Output = Input;
}

void
IIRFilterBase::Initialize( const SignalProperties& Input, const SignalProperties& /*Output*/ )
{
  Real gain;
  ComplexVector zeros, poles;
  DesignFilter( gain, zeros, poles );
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


