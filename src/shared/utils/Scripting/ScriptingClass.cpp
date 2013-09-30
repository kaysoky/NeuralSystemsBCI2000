//////////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A ScriptingClass object represents properties and functions of a
//   scripting object type, and provides a scripting engine with the ability to
//   manipulate instances of that type.
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
//////////////////////////////////////////////////////////////////////////////////////
#if 0

#ifndef SCRIPTING_CLASS_H
#define SCRIPTING_CLASS_H

#include <string>
#include <vector>
#include "Uncopyable.h"

class ScriptingClass : Uncopyable
{
  public:
    virtual ~ScriptingClass() {}

    class Value;

    typedef Value (*Getter)( void* );
    typedef bool (*Setter)( void*, const Value& );
    typedef Value (*Function)( void*, const Value& );

    Value Properties() const;
    Getter GetProperty( const std::string& name ) const;
    Setter SetProperty( const std::string& name ) const;

    Value Functions() const;
    Function Call( const std::string& name ) const;

    class Value
    {
      public:
        typedef double Number;
        typedef std::string String;

        Value();
        Value( Number );
        Value( String );
        const String& AsString() const;
        Number AsNumber() const;
        bool IsNull() const;

        Value( const Value& );
        Value& operator=( const Value& );

        Value& Assign( const Value& );
        Value& Append( const Value& );
        Value& Next();

      private:
        enum { null, number, string } mKind;
        mutable Number mNumber;
        mutable String mString;
    };

  protected:
    ScriptingClass& Inherit( const ScriptingClass& );

    class Property;
    class Function;

  private:
    std::vector<ScriptingClass*> mParents;
};

#endif // SCRIPTING_CLASS_H

#endif
