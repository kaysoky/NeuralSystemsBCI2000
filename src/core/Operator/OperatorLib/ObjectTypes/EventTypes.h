////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: Event object types for the script interpreter.
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
#ifndef EVENT_TYPES_H
#define EVENT_TYPES_H

#include "ObjectType.h"

class State;

namespace Interpreter {

class EventType : public ObjectType
{
 protected:
  virtual const char* Name() const { return "Event"; }
  virtual const MethodEntry* MethodTable() const { return sMethodTable; }

 public:
  static bool Set( CommandInterpreter& );
  static bool Get( CommandInterpreter& );
  static bool Insert( CommandInterpreter& );
  static bool List( CommandInterpreter& );
  static bool Exists( CommandInterpreter& );

 private:
  static State& GetEvent( CommandInterpreter& );

  static const MethodEntry sMethodTable[];
  static EventType sInstance;
};

class EventsType : public ObjectType
{
 protected:
  virtual const char* Name() const { return "Events"; }
  virtual const MethodEntry* MethodTable() const { return sMethodTable; }

 public:
  static bool Insert( CommandInterpreter& );
  static bool List( CommandInterpreter& );
  static bool Clear( CommandInterpreter& );

 private:
  static const MethodEntry sMethodTable[];
  static EventsType sInstance;
};

} // namespace

#endif // EVENT_TYPES_H
