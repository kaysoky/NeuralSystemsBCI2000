////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates interpretation of operator scripts.
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
#ifndef SCRIPT_INTERPRETER_H
#define SCRIPT_INTERPRETER_H

#include <vector>
#include <sstream>
#include <iostream>
#include <stack>
#include <set>
#include "StateMachine.h"
#include "OSMutex.h"

class ScriptInterpreter
{
 public:
  // Begin: Interface to users
  ScriptInterpreter( class StateMachine& );
  virtual ~ScriptInterpreter();
  // Properties
  //  The result of the last executed scripting command.
  std::string Result() const
    { return mResultStream.str(); }
  // Methods
  //  Interpret the argument script as a sequence of scripting commands.
  bool Execute( const char* script );
  bool Execute( const std::string& s )
    { return Execute( s.c_str() ); }
  // Initialize a state machine for use with scripts..
  static void Initialize( class StateMachine& );
  // End: Interface to users

 protected:
  // Begin: Interface to descendants
  //  Re-implement this function to direct error messages somewhere else than
  //  into the BCI_OnScriptError callback.
  virtual void OnScriptError( const std::string& );
  // End: Interface to descendants

 public:
  // Begin: Interface to ObjectType instances.
  //  GetToken() reads a single string, which may be quoted and URL-encoded.
  std::string GetToken();
  //  GetRemainder() reads the remainder of the command line.
  std::string GetRemainder();
  //  Unget() undoes a read operation. Without a previous read operation, it does nothing.
  void Unget();

  typedef std::vector< std::vector<std::string> > ArgumentList;
  static void ParseArguments( std::string&, ArgumentList& );

  std::ostream& Out()
    { return mResultStream; }
  class StateMachine& StateMachine() const
    { return mrStateMachine; }

  // Log and error message capturing.
  void CaptureLog( int messageCallbackID )
    { OSMutex::Lock lock( mMutex ); mLogCapture.insert( messageCallbackID ); }
  void DontCaptureLog()
    { OSMutex::Lock lock( mMutex ); mLogCapture.clear(); }
  void FlushCapturedLog()
    { OSMutex::Lock lock( mMutex ); Out() << mLogBuffer; mLogBuffer.clear(); }
  // HandleLogMessage() is called by StateMachine to report a message.
  void HandleLogMessage( int messageCallbackID, const std::string& );

  class LogStream : public std::ostream
  {
   public:
    LogStream( class StateMachine& s ) : std::ostream( &mBuffer ), mBuffer( s ) {}
    LogStream( const LogStream& l ) : std::ostream( &mBuffer ), mBuffer( l.mBuffer ) {}
    ~LogStream() { flush(); }
    template<typename T> std::ostream& operator<<( T t ) const { return const_cast<LogStream&>( *this ) << t; }
    template<typename T> std::ostream& operator<<( T t ) { return static_cast<std::ostream&>( *this ) << t; }
   private:
    class LogBuffer : public std::stringbuf
    {
     public:
      LogBuffer( class StateMachine& s ) : mrStateMachine( s ) {}
      LogBuffer( const LogBuffer& l ) : mrStateMachine( l.mrStateMachine ) {}
     private:
      virtual int sync();
      class StateMachine& mrStateMachine;
    } mBuffer;
  };
  LogStream Log()
    { return LogStream( mrStateMachine ); }
  // End: Interface to ObjectType instances.

 private:
  bool ExecuteLine( const std::string& line );
  bool ExecuteCommand( const std::string& command );

 private:
  std::ostringstream mResultStream;
  std::istringstream mInputStream;
  std::stack<std::istream::pos_type> mPosStack;

  class StateMachine& mrStateMachine;
  int mLine;

  OSMutex mMutex;

  std::set<int> mLogCapture;
  std::string mLogBuffer;
};

#endif // SCRIPT_INTERPRETER_H
