////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: GenericSignal is the BCI2000 type representing filter input and
//              output data.
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

#include "GenericSignal.h"

#include "LengthField.h"
#include "BinaryData.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <cstring>
#include <inttypes.h>

using namespace std;
using namespace bci;

template<class T> class StaticObject : Uncopyable
{
 public:
  T& operator()() { return Access(); }
  const T& operator()() const { return Access(); }
 private:
  static T& Access()
  {
    static T t;
    return t;
  }
};

namespace
{

typedef LazyArray<GenericSignal::ValueType> Array;
struct SharedMemory : Array::Memory
{
  SharedMemory( const SharedPointer<OSSharedMemory>& p, size_t count )
    : Memory( reinterpret_cast<ValueType*>( p->Memory() ), count ),
      mpShm( p )
    {}
  SharedPointer<OSSharedMemory> mpShm;
};

typedef SharedPointer<OSSharedMemory> ShmPtr;
struct ShmPool_ : private map<string, ShmPtr>
{
  ShmPtr& New( size_t count )
  {
    OSSharedMemory* p = new OSSharedMemory( count * sizeof( GenericSignal::ValueType ) );
    return (*this)[p->Name()] = ShmPtr( p );
  }
  ShmPtr& Get( const string& name )
  {
    ShmPtr& s = (*this)[name];
    if( !s )
      s = ShmPtr( new OSSharedMemory( name ) );
    return s;
  }
};

StaticObject<ShmPool_> ShmPool;

}

const GenericSignal::ValueType GenericSignal::NaN = numeric_limits<ValueType>::quiet_NaN();

GenericSignal::GenericSignal()
{
  SetProperties( mProperties );
}

GenericSignal::~GenericSignal()
{
}

GenericSignal::GenericSignal( size_t inChannels, size_t inElements, SignalType::Type inType )
{
  SetProperties( SignalProperties( inChannels, inElements, inType ) );
}

GenericSignal::GenericSignal( size_t inChannels, size_t inElements, SignalType inType )
{
  SetProperties( SignalProperties( inChannels, inElements, inType ) );
}

GenericSignal::GenericSignal( const SignalProperties& inProperties )
{
  SetProperties( inProperties );
}

GenericSignal::GenericSignal( const SignalProperties& inProperties, ValueType inValue )
{
  SetProperties( inProperties );
  for( int ch = 0; ch < Channels(); ++ch )
    for( int el = 0; el < Elements(); ++el )
      SetValue( ch, el, inValue );
}

GenericSignal&
GenericSignal::SetProperties( const SignalProperties& inSp )
{
  if( inSp.Channels() != mProperties.Channels() || inSp.Elements() != mProperties.Elements() )
  {
    size_t newSize = inSp.Channels() * inSp.Elements();
    Array newValues;
    if( mSharedMemory )
    {
      if( mSharedMemory->Protocol() == "file://" )
        throw std_runtime_error( "Cannot resize shared memory if tied to a file" );
      mSharedMemory = ShmPool().New( newSize );
      SharedPointer<Array::Memory> pShm( new SharedMemory( mSharedMemory, newSize ) );
      newValues = Array( pShm );
    }
    else
      newValues = Array( newSize );

    for( int ch = 0; ch < min( mProperties.Channels(), inSp.Channels() ); ++ch )
    {
      for( int el = 0; el < min( mProperties.Elements(), inSp.Elements() ); ++el )
        newValues[inSp.LinearIndex( ch, el )] = mValues[mProperties.LinearIndex( ch, el )];
      for( int el = mProperties.Elements(); el < inSp.Elements(); ++el )
        newValues[inSp.LinearIndex( ch, el )] = 0;
    }
    for( int ch = mProperties.Channels(); ch < inSp.Channels(); ++ch )
      for( int el = 0; el < inSp.Elements(); ++el )
        newValues[inSp.LinearIndex( ch, el )] = 0;
    mValues = newValues;
  }
  mProperties = inSp;
  return *this;
}

ostream&
GenericSignal::WriteToStream( ostream& os ) const
{
  string indent( os.width(), ' ' );

  os << '\n' << indent << "SignalProperties { ";
  mProperties.WriteToStream( os );
  os << '\n' << indent << "}";
  if( mSharedMemory )
    os << '\n' << indent << "SharedMemory { "
       << mSharedMemory->Name()
       << '\n' << indent << "}";
  os << setprecision( 7 );
  for( int j = 0; j < Elements(); ++j )
  {
    os << '\n' << indent;
    for( int i = 0; i < Channels(); ++i )
    {
      os << setw( 14 )
         << Value( i, j )
         << ' ';
    }
  }
  return os;
}

ostream&
GenericSignal::WriteBinary( ostream& os ) const
{
  SignalType type = Type();
  type.SetShared( mSharedMemory );
  type.WriteBinary( os );
  LengthField<2> channelsField( Channels() ),
                 elementsField( Elements() );
  channelsField.WriteBinary( os );
  elementsField.WriteBinary( os );
  if( type.Shared() )
  {
    MemoryFence();
    os.write( mSharedMemory->Name().c_str(), mSharedMemory->Name().length() + 1 );
  }
  else
    for( int i = 0; i < Channels(); ++i )
      for( int j = 0; j < Elements(); ++j )
        WriteValueBinary( os, i, j );
  return os;
}

istream&
GenericSignal::ReadBinary( istream& is )
{
  SignalType     type;
  LengthField<2> channels,
                 elements;
  type.ReadBinary( is );
  channels.ReadBinary( is );
  elements.ReadBinary( is );
  if( type.Shared() )
  {
    string name;
    getline( is, name, '\0' );
    SetProperties( SignalProperties( channels, elements, type ) );
    AttachToSharedMemory( name );
    MemoryFence();
  }
  else
  {
    SetProperties( SignalProperties( channels, elements, type ) );
    for( int i = 0; i < Channels(); ++i )
      for( int j = 0; j < Elements(); ++j )
        ReadValueBinary( is, i, j );
  }
  return is;
}

ostream&
GenericSignal::WriteValueBinary( ostream& os, size_t i, size_t j ) const
{
  switch( Type() )
  {
    case SignalType::int16:
      BinaryData<int16_t, LittleEndian>( Value( i, j ) ).Put( os );
      break;

    case SignalType::float24:
      PutValue_float24( os, Value( i, j ) );
      break;

    case SignalType::float32:
      BinaryData<float, LittleEndian>( Value( i, j ) ).Put( os );
      break;

    case SignalType::int32:
      BinaryData<int32_t, LittleEndian>( Value( i, j ) ).Put( os );
      break;

    default:
      os.setstate( os.failbit );
  }
  return os;
}

istream&
GenericSignal::ReadValueBinary( istream& is, size_t i, size_t j )
{
  switch( Type() )
  {
    case SignalType::int16:
      Value( i, j ) = BinaryData<int16_t, LittleEndian>( is );
      break;

    case SignalType::float24:
      Value( i, j ) = GetValue_float24( is );
      break;

    case SignalType::float32:
      Value( i, j ) = BinaryData<float, LittleEndian>( is );
      break;

    case SignalType::int32:
      Value( i, j ) = BinaryData<int32_t, LittleEndian>( is );
      break;

    default:
      is.setstate( is.failbit );
  }
  return is;
}

void
GenericSignal::PutValue_float24( std::ostream& os, ValueType value )
{
  int mantissa,
      exponent;
  if( value == 0.0 )
  {
    mantissa = 0;
    exponent = 1;
  }
  else
  {
    exponent = static_cast<int>( ::ceil( ::log10( ::fabs( value ) ) ) );
    mantissa = static_cast<int>( value / ::pow( 10.0, exponent ) ) * 10000;
    exponent -= 4;
  }
  os.put( mantissa & 0xff ).put( mantissa >> 8 );
  os.put( exponent & 0xff );
}

GenericSignal::ValueType
GenericSignal::GetValue_float24( std::istream& is )
{
  signed short mantissa = is.get();
  mantissa |= is.get() << 8;
  signed char exponent = is.get();
  return mantissa * ::pow( 10.0, exponent );
}

GenericSignal&
GenericSignal::AssignFrom( const GenericSignal& s )
{
  SetProperties( s.Properties() );
  AssignValues( s );
  return *this;
}

GenericSignal&
GenericSignal::AssignValues( const GenericSignal& s )
{
  bool mismatch = s.Channels() != Channels() || s.Elements() != Elements();
  if( mismatch )
    SetProperties( SignalProperties( s.Channels(), s.Elements() ) );

  if( !mSharedMemory && !s.mSharedMemory )
    mValues.ShallowAssignFrom( s.mValues );
  else
    mValues.DeepAssignFrom( s.mValues );
  return *this;
}

bool
GenericSignal::ShareAcrossModules()
{
  if( !mSharedMemory && mValues.Count() != 0 )
  {
    mSharedMemory = ShmPool().New( mValues.Count() );
    SharedPointer<Array::Memory> pShm( new SharedMemory( mSharedMemory, mValues.Count() ) );
    Array newValues( pShm );
    newValues.DeepAssignFrom( mValues );
    mValues.ShallowAssignFrom( newValues );
  }
  return mSharedMemory;
}

void
GenericSignal::AttachToSharedMemory( const string& inName )
{
  if( !mSharedMemory || mSharedMemory->Name() != inName )
  {
    mSharedMemory = ShmPool().Get( inName );
    SharedPointer<Array::Memory> pShm( new SharedMemory( mSharedMemory, mValues.Count() ) );
    mValues = Array( pShm );
  }
}

// GenericChannel
GenericChannel& 
GenericChannel::operator=( const GenericChannel& inChannel )
{
  for( size_t i = 0; i < size(); i++ )
    ( *this )[i] = inChannel[i];
  return ( *this );
}

// GenericElement
GenericElement&
GenericElement::operator=( const GenericElement& inElement )
{
  for( size_t i = 0; i < size(); i++ )
    ( *this )[i] = inElement[i];
  return ( *this );
}

