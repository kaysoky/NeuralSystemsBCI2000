////////////////////////////////////////////////////////////////////////////////
//
// File: UGenericSignal.cpp
//
// Description: This file declares a SignalProperties base class and a BasicSignal
//   class template deriving from it with the signal's numerical type as the
//   template argument.
//   Two classes, GenericSignal and GenericIntSignal, are derived from a float
//   and int instantiation of this template. With a compatibility flag set
//   (SIGNAL_BACK_COMPAT) existing code should compile with minimal changes.
//
//   For the future, the following name transitions might be considered:
//     BasicSignal --> GenericSignal
//     GenericSignal --> FloatSignal
//     GenericIntSignal --> IntSignal
//   as the latter two don't have anything generic about them any more.
//
// Changes: June 28, 2002, juergen.mellinger@uni-tuebingen.de
//          - Rewrote classes from scratch but kept old class interface.
//          Mar 28, 2003, juergen.mellinger@uni-tuebingen.de
//          - Added depth member to SignalProperties to unify GenericSignal
//            and GenericIntSignal into one signal type.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "UGenericSignal.h"

bool
SignalProperties::SetNumElements( size_t inChannel, size_t inElements )
{
  bool elementsTooBig = ( inElements > maxElements );
  if( elementsTooBig )
    elements.at( inChannel ) = maxElements;
  else
    elements.at( inChannel ) = inElements;
  return elementsTooBig;
}

bool
SignalProperties::operator>=( const SignalProperties& sp ) const
{
  if( elements.size() < sp.elements.size() )
    return false;
  for( size_t i = 0; i < sp.elements.size(); ++i )
    if( elements[ i ] < sp.elements[ i ] )
      return false;
  return true;
}

bool
SignalProperties::operator<=( const SignalProperties& sp ) const
{
  if( sp.elements.size() < elements.size() )
    return false;
  for( size_t i = 0; i < elements.size(); ++i )
    if( sp.elements[ i ] < elements[ i ] )
      return false;
  return true;
}

void
GenericSignal::SetChannel( const short *inSource, size_t inChannel )
{
  for( size_t i = 0; i < elements.at( inChannel ); ++i )
    Value[ inChannel ][ i ] = ( float )inSource[ i ];
}

const GenericSignal&
GenericSignal::operator=( const GenericIntSignal& inRHS )
{
  SetProperties( inRHS );
  for( size_t i = 0; i < inRHS.Channels(); ++i )
    for( size_t j = 0; j < inRHS.GetNumElements( i ); ++j )
      SetValue( i, j, ( float )inRHS.GetValue( i, j ) );
  return *this;
}

