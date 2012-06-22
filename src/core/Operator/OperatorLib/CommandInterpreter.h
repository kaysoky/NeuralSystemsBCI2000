////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that interprets individual operator scripting commands,
//   and performs command substitution.
//   The CommandInterpreter class implements three interfaces:
//   1) A client interface that provides an Execute() method, and access
//      to the last command's result.
//      The Execute() method is intentionally not thread safe. Use multiple
//      CommandInterpreter instances to execute scripting commands concurrently.
//   2) An internal "callback" interface to ObjectType descendant classes
//      which perform actual command execution.
//   3) A "listener" interface to the StateMachine class. This allows to
//      capture operator log messages.
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
#ifndef COMMAND_INTERPRETER_H
#define COMMAND_INTERPRETER_H

#include <vector>
#include <sstream>
#include <stack>
#include <set>
#include "StateMachine.h"
#include "OSMutex.h"
#include "ArithmeticExpression.h"
#include "EnvVariable.h"

class CommandInterpreter
{
 public:
  // Begin: Interface to users
  CommandInterpreter( class StateMachine& );
  CommandInterpreter( CommandInterpreter& );
  virtual ~CommandInterpreter();
  // Properties
  //  The name of the local variable holding the result of the last scripting command.
  static const char* ResultName()
     { return "Result"; }
  //  The result of the last executed scripting command.
  std::string Result() const
    { return mResultStream.str(); }
  //  A tag that is inserted in the output before reporting the exit
  //  code of a command. When no tag is found, the exit code may be
  //  assumed to be zero.
  static const char* ExitCodeTag()
    { return "\\ExitCode: "; }
  //  A tag that is inserted in the output to report that the system is
  //  going to quit.
  static const char* TerminationTag()
    { return "\\Terminating"; }
  // Methods
  // Interpret the argument as a single scripting command.
  int Execute( const std::string& );
  // Perform command substitution on the argument.
  std::string SubstituteCommands( const std::string& );
  // End: Interface to users

 public:
  // Begin: Interface to ObjectType instances.
  //  GetToken() reads a single string, which may be quoted and URL-encoded.
  std::string GetToken();
  std::string GetOptionalToken();
  //  GetRemainder() reads the remainder of the command line.
  std::string GetRemainder();
  std::string GetOptionalRemainder();
  //  Unget() undoes a read operation. Without a previous read operation, it does nothing.
  void Unget();
  //  WriteLine() writes a line to the output stream associated with a command
  //  interpreter, ReadLine() reads a line from the input stream associated with
  //  a command interpreter.
  bool WriteLine( const std::string& );
  bool ReadLine( std::string& );
  //  Set handler functions to be called by the two above functions.
  //  Handlers are pointers rather than virtual functions -- this way,
  //  they may be copied from one CommandInterpreter instance to the next.
  typedef bool (*WriteLineFunc)( void*, const std::string& );
  CommandInterpreter& WriteLineHandler( WriteLineFunc, void* );
  typedef bool (*ReadLineFunc)( void*, std::string& );
  CommandInterpreter& ReadLineHandler( ReadLineFunc, void* );

  typedef std::vector< std::vector<std::string> > ArgumentList;
  static void ParseArguments( std::string&, ArgumentList& );

  std::ostream& Out()
    { return mResultStream; }

  class StateMachine& StateMachine() const
    { return mrStateMachine; }

  ArithmeticExpression::VariableContainer& ExpressionVariables()
    { return mExpressionVariables; }

  struct VariableContainer : std::map<std::string, std::string>
  {
    bool Exists( const std::string& key )
      { return find( key ) != end(); }
  };
  VariableContainer& LocalVariables()
    { return mLocalVariables; }

  // Log and error message capturing.
  void CaptureLog( int messageCallbackID )
    { OSMutex::Lock lock( mMutex ); mLogCapture.insert( messageCallbackID ); }
  void DontCaptureLog()
    { OSMutex::Lock lock( mMutex ); mLogCapture.clear(); }
  void FlushCapturedLog()
    { OSMutex::Lock lock( mMutex ); Out() << mLogBuffer; mLogBuffer.clear(); }

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
  //  Allow for background processing (e.g., from long-running commands).
  //  Returns an estimate for the time spent inside the Background() call, in ms.
  int Background( int sleepTimeMs = 1 );
  // End: Interface to ObjectType instances.

  // Begin: StateMachine listener interface.
  //  HandleLogMessage() is called by StateMachine to report a message.
  void HandleLogMessage( int messageCallbackID, const std::string& );
  //  Set the abort flag (e.g., to interrupt long-running loops in scripts)
  void Abort();
  // End: StateMachine listener interface.
 private:
  CommandInterpreter& operator=( const CommandInterpreter& );
  void Init();
  void AddListener( CommandInterpreter& );
  void RemoveListener( CommandInterpreter& );
  std::string GetVariable( const std::string& inName );
  int EvaluateResult( const std::string& inCommand );
  static bool OnWriteLineDefault( void*, const std::string& );
  bool InputFailed();

 private:
  std::ostringstream mResultStream;
  std::istringstream mInputStream;
  std::stack<std::istream::pos_type> mPosStack;

  class StateMachine& mrStateMachine;
  ArithmeticExpression::VariableContainer mExpressionVariables;
  VariableContainer mLocalVariables;

  OSMutex mMutex;
  volatile bool mAbort;
  std::set<CommandInterpreter*> mListeners;
  CommandInterpreter* mpParent;

  std::set<int> mLogCapture;
  std::string mLogBuffer;

  WriteLineFunc mWriteLineFunc;
  void* mpWriteLineData;
  ReadLineFunc mReadLineFunc;
  void* mpReadLineData;
};

#endif // SCRIPT_INTERPRETER_H
