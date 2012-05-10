////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A base class for interpreter object types.
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

#include "ObjectType.h"
#include "CommandInterpreter.h"

using namespace std;
using namespace Interpreter;

bool
ObjectType::Execute( const string& inVerb, CommandInterpreter& inInterpreter ) const
{
  const MethodEntry* pEntry = MethodTable();
  while( pEntry->verb )
  {
    if( 0 != ::stricmp( pEntry->verb, inVerb.c_str() ) )
      ++pEntry;
    else
      return pEntry->action( inInterpreter );
  }
  if( 0 == ::stricmp( inVerb.c_str(), "Help" ) || !inVerb.empty() && inVerb[0] == '?' )
  {
    Help( inInterpreter );
    return true;
  }
  return false;
}

void
ObjectType::Help( CommandInterpreter& inInterpreter ) const
{
  OnHelp( inInterpreter );
}

void
ObjectType::ListMethods( std::ostream& os ) const
{
  OnListMethods( os );
}

void
ObjectType::Initialize( class StateMachine& inStateMachine )
{
  for( TypeDictionary::const_iterator i = Dictionary().begin(); i != Dictionary().end(); ++i )
    ( *i )->OnInitialize( inStateMachine );
}

ObjectType*
ObjectType::ByName( const string& inName )
{
  for( TypeDictionary::const_iterator i = Dictionary().begin(); i != Dictionary().end(); ++i )
    if( 0 == ::stricmp( inName.c_str(), ( *i )->Name() ) )
      return *i;
  return 0;
}

ObjectType*
ObjectType::Next( const ObjectType* inP )
{
  ObjectType* pResult = NULL;
  TypeDictionary::const_iterator i = Dictionary().begin();
  while( pResult == NULL && i != Dictionary().end() )
  {
    if( *i == inP )
    {
      if( ++i == Dictionary().end() )
        i = Dictionary().begin();
      pResult = *i;
    }
    ++i;
  }
  return pResult;
}

void
ObjectType::OnHelp( CommandInterpreter& inInterpreter ) const
{
  if( MethodTable()->verb )
  {
    inInterpreter.Out() << "Commands for objects of type " << Name() << ": ";
    ListMethods( inInterpreter.Out() );
  }
  else
    inInterpreter.Out() << "There are no commands associated with type " << Name();
}

void
ObjectType::OnListMethods( std::ostream& os ) const
{
  const MethodEntry* pEntry = MethodTable();
  if( pEntry->verb )
    os << ( pEntry++ )->verb;
  while( pEntry->verb )
    os << ", " << ( pEntry++ )->verb;
}

ObjectType::TypeDictionary&
ObjectType::Dictionary()
{
  static TypeDictionary instance;
  return instance;
}
