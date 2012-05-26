////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: System-related object types for the script interpreter.
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
#ifndef SYSTEM_TYPES_H
#define SYSTEM_TYPES_H

#include "ObjectType.h"
#include <set>

namespace Interpreter {

class SystemType : public ObjectType
{
 protected:
  virtual const char* Name() const { return "System"; }
  virtual const MethodEntry* MethodTable() const { return sMethodTable; }

 public:
  static bool Get( CommandInterpreter& );
  static bool GetState( CommandInterpreter& );
  static bool GetVersion( CommandInterpreter& );
  static bool WaitFor( CommandInterpreter& );
  static bool Sleep( CommandInterpreter& );
  static bool SetConfig( CommandInterpreter& );
  static bool Start( CommandInterpreter& );
  static bool Stop( CommandInterpreter& );
  static bool Startup( CommandInterpreter& );
  static bool Shutdown( CommandInterpreter& );
  static bool Reset( CommandInterpreter& );
  static bool Quit( CommandInterpreter& );
  
 private:
  static bool DoWaitFor( const std::set<int>&, double, CommandInterpreter& );

 private:
  static const MethodEntry sMethodTable[];
  static SystemType sInstance;
};

class ConfigType : public SystemType
{
 protected:
  virtual const char* Name() const { return "Config"; }
  virtual const MethodEntry* MethodTable() const { return sMethodTable; }

 public:
  static bool Set( CommandInterpreter& );

 private:
  static const MethodEntry sMethodTable[];
  static ConfigType sInstance;
};

} // namespace

#endif // SYSTEM_TYPES_H
