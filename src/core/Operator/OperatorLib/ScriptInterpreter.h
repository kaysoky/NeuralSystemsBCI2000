////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates interpretation of operator scripts.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
#ifndef SCRIPT_INTERPRETER_H
#define SCRIPT_INTERPRETER_H

#include "StateMachine.h"
#include <string>
#include <iostream>

class ScriptInterpreter
{
 public:
  ScriptInterpreter( class StateMachine& );
  virtual ~ScriptInterpreter()
    {}

  // Methods
  //  Interpret the argument script as a sequence of scripting commands.
  bool Execute( const char* script );

 private:
  bool ExecuteLine( const std::string& line );
  bool ExecuteCommand( const std::string& command );

  bool Execute_Load( std::istream& );
  bool Execute_Set( std::istream& );
  bool Execute_Insert( std::istream& );
  bool Execute_System( std::istream& );
  bool Execute_SetConfig( std::istream& );
  bool Execute_Start( std::istream& );
  bool Execute_Stop( std::istream& );
  bool Execute_Quit( std::istream& );

  bool ApplyVisPropertySet( const std::string& setID );


  int mLine;
  class StateMachine& mrStateMachine;
};

#endif // SCRIPT_INTERPRETER_H
