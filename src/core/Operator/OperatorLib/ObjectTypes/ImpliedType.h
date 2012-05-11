////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A script interpreter type that handles global commands.
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
#ifndef IMPLIED_TYPE_H
#define IMPLIED_TYPE_H

#include "ObjectType.h"

namespace Interpreter {

class ImpliedType : public ObjectType
{
 protected:
  virtual const char* Name() const { return ""; }
  virtual const MethodEntry* MethodTable() const { return sMethodTable; }
  virtual void OnHelp( CommandInterpreter& ) const;

 public:
  static bool Get( CommandInterpreter& );
  static bool Set( CommandInterpreter& );

  static bool SetConfig( CommandInterpreter& );
  static bool Start( CommandInterpreter& );
  static bool Stop( CommandInterpreter& );
  static bool Startup( CommandInterpreter& );
  static bool Shutdown( CommandInterpreter& );
  static bool Reset( CommandInterpreter& );
  static bool Quit( CommandInterpreter& );

  static bool System( CommandInterpreter& );
  static bool Echo( CommandInterpreter& );
  static bool Ls( CommandInterpreter& );
  static bool Cd( CommandInterpreter& );
  static bool Pwd( CommandInterpreter& );

  static bool Wait( CommandInterpreter& );
  static bool Sleep( CommandInterpreter& );

  static bool Version( CommandInterpreter& );

  static bool Log( CommandInterpreter& );
  static bool Warn( CommandInterpreter& );
  static bool Error( CommandInterpreter& );

  static bool Square( CommandInterpreter& );
  

 private:
  static const MethodEntry sMethodTable[];
  static ImpliedType sInstance;
};

class ExecutableType : public ObjectType
{
 protected:
  virtual const char* Name() const { return "Executable"; }
  virtual const MethodEntry* MethodTable() const { return sMethodTable; }

 public:
  static bool Start( CommandInterpreter& );

 private:
  static const MethodEntry sMethodTable[];
  static ExecutableType sInstance;
};

} // namespace

#endif // IMPLIED_TYPE_H
