////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: Utility functions for executing processes.
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

#include "ProcessUtils.h"
#include "ThreadUtils.h"
#include "FileUtils.h"
#include <string>
#include <iostream>

#if _WIN32
# include <Windows.h>
#else // _WIN32
# include <cstdio>
# include <cstring>
# include <spawn.h>
# include <vector>
# include <fcntl.h>
# include <sys/stat.h>
# include <semaphore.h>
# if __APPLE__
#  include <crt_externs.h>
#  define environ (*_NSGetEnviron())
# else // __APPLE__
extern char** environ;
# endif // __APPLE__
#endif // _WIN32

using namespace std;

bool
ProcessUtils::ExecuteSynchronously( const string& inExecutable, const string& inArguments, ostream& outStream, int& outExitCode  )
{
  bool success = false;
  string command = inExecutable + " " + inArguments;
  static const int bufferSize = 512;
  char buffer[bufferSize];

#if _WIN32
  // In Windows, popen() does not work unless a console exists in the application.
  // Thus, we need to use CreatePipe() in conjunction with CreateProcess().
  // The remaining code ensures that pipe handles are set up to be closed automatically
  // when the created process terminates. This is important because ReadFile() would
  // hang otherwise.
  // MS KB Article ID 190351: How to spawn console processes with redirected standard handles.
  BOOL result = TRUE;
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof( sa );
  sa.bInheritHandle = TRUE;
  sa.lpSecurityDescriptor = NULL;
  HANDLE pipeReadTmp = NULL,
         pipeRead = NULL,
         pipeWrite = NULL,
         pipeWrite2 = NULL;
  result &= ::CreatePipe( &pipeReadTmp, &pipeWrite, &sa, 0 );
  result &= ::DuplicateHandle( ::GetCurrentProcess(), pipeWrite, ::GetCurrentProcess(), &pipeWrite2, 0, TRUE, DUPLICATE_SAME_ACCESS );
  result &= ::DuplicateHandle( ::GetCurrentProcess(), pipeReadTmp, ::GetCurrentProcess(), &pipeRead, 0, FALSE, DUPLICATE_SAME_ACCESS );
  result &= ::CloseHandle( pipeReadTmp );

  PROCESS_INFORMATION procInfo;
  ::ZeroMemory( &procInfo, sizeof( procInfo ) );
  STARTUPINFO startInfo;
  ::ZeroMemory( &startInfo, sizeof( startInfo ) );
  startInfo.cb = sizeof( startInfo );
  startInfo.hStdError = pipeWrite2;
  startInfo.hStdOutput = pipeWrite;
  startInfo.hStdInput = ::GetStdHandle( STD_INPUT_HANDLE );
  startInfo.wShowWindow = SW_SHOWNA;
  startInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

  result &= ::CreateProcessA( NULL, const_cast<char*>( command.c_str() ), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &startInfo, &procInfo );
  result &= ::CloseHandle( pipeWrite );
  result &= ::CloseHandle( pipeWrite2 );

  DWORD dwExitCode;
  while( ( result &= ::GetExitCodeProcess( procInfo.hProcess, &dwExitCode ) ) && dwExitCode == STILL_ACTIVE )
  {
    DWORD bytesRead;
    while( ::ReadFile( pipeRead, buffer, bufferSize, &bytesRead, NULL ) && bytesRead > 0 )
      outStream << string( buffer, bytesRead );
  }
  outExitCode = dwExitCode;
  ::CloseHandle( pipeRead );
  ::CloseHandle( procInfo.hProcess );
  ::CloseHandle( procInfo.hThread );
  success = ( result == TRUE );

#else // _WIN32
  FILE* pipe = ::popen( command.c_str(), "r" );
  if( pipe != NULL )
  {
    while( ::fgets( buffer, bufferSize, pipe ) )
      outStream << buffer;
    outExitCode = ::pclose( pipe );
    success = true;
  }

#endif // _WIN32

  return success;
}

bool
ProcessUtils::ExecuteAsynchronously( const string& inExecutable, const string& inArguments, int& outExitCode )
{
  bool success = false;
  outExitCode = 0;
  string executable = inExecutable;

#if _WIN32

  string extension = ".exe";
  if( executable.length() < extension.length()
    || ::stricmp( extension.c_str(), executable.substr( executable.length() - extension.length() ).c_str() ) )
    executable += extension;

  // When CreateProcess() is used to start up a core module, listening sockets in the Operator module are not closed properly.
  // ShellExecute() seems not to have this problem.
  SHELLEXECUTEINFOA info = { 0 };
  info.cbSize = sizeof( info );
  info.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
  info.hwnd = NULL;
  info.lpVerb = "open";
  info.lpFile = executable.c_str();
  info.lpParameters = inArguments.c_str();
  info.nShow = SW_SHOWNA;
  success = ::ShellExecuteExA( &info );
  if( success )
  {
    if( WAIT_TIMEOUT == ::WaitForInputIdle( info.hProcess, 60000 ) )
    {
      success = false;
      outExitCode = 0;
    }
    else
    {
      DWORD dwExitCode = 0;
      ::GetExitCodeProcess( info.hProcess, &dwExitCode );
      if( STILL_ACTIVE != dwExitCode )
      {
        outExitCode = dwExitCode;
        success = ( dwExitCode == 0 );
      }
    }
    ::CloseHandle( info.hProcess );
  }

#else // _WIN32

  char* pArgs = new char[inArguments.length() + 1];
  ::strcpy( pArgs, inArguments.c_str() );
  vector<char*> vArgs;
  char* p = pArgs;
  bool inQuotes = false,
       inArg = false;
  while( *p != '\0' )
  {
    if( *p == '\"' )
      inQuotes = !inQuotes;
    if( ::isspace( *p ) && !inQuotes )
    {
        *p = 0;
        inArg = false;
    }
    else if( !inArg )
    {
      vArgs.push_back( p );
      inArg = true;
    }
    ++p;
  }
  char** pArgv = new char*[vArgs.size() + 2];
  pArgv[0] = const_cast<char*>( executable.c_str() );
  pArgv[vArgs.size() + 1] = NULL;
  for( size_t i = 0; i < vArgs.size(); ++i )
    pArgv[i+1] = vArgs[i];

  outExitCode = ::posix_spawnp( NULL, executable.c_str(), NULL, NULL, pArgv, environ );
  success = ( outExitCode == 0 );

  delete[] pArgv;
  delete[] pArgs;

#endif // _WIN32

  return success;
}

void
ProcessUtils::GoIdle()
{
#if _WIN32
  MSG msg;
  HWND window = ::CreateWindowA( "STATIC", NULL, 0, 0, 0, 0, 0, NULL, NULL, ::GetModuleHandle( NULL ), NULL );
  while( ::PeekMessage( &msg, window, 0, 0, PM_REMOVE ) )
    ;
  ::DestroyWindow( window );
#endif // _WIN32
}

namespace {

class GlobalID
{
 public:
   GlobalID( const string& name, int timeout );
   ~GlobalID();
   bool Owned() const { return mHandle != NULL; }
   static void Cleanup( const string& name );
   
 private:
   bool TryCreate( const string& inName );
   void Destroy();
   void* mHandle;
};

GlobalID::GlobalID( const string& inName, int inTimeout )
: mHandle( NULL )
{
  const int cTimeResolution = 100; // ms
  int timeElapsed = 0;
  while( !TryCreate( inName ) && timeElapsed < inTimeout )
  {
    ThreadUtils::SleepFor( cTimeResolution );
    timeElapsed += cTimeResolution;
  }
}

GlobalID::~GlobalID()
{
  Destroy();
}


bool
GlobalID::TryCreate( const std::string& inName )
{
  if( mHandle )
    return true;

#if _WIN32

  HANDLE mutex = ::CreateMutexA( NULL, true, inName.c_str() );
  if( ::GetLastError() == ERROR_ALREADY_EXISTS )
    ::CloseHandle( mutex );
  else
    mHandle = reinterpret_cast<void*>( mutex );

#else // _WIN32

  string name = "/" + inName;
  sem_t* pSemaphore = ::sem_open( name.c_str(), O_CREAT | O_EXCL, 0666, 0 );
  if( pSemaphore != SEM_FAILED )
  {
    ::sem_close( pSemaphore );
    mHandle = new string( name );
  }

#endif // _WIN32

  return mHandle != NULL;
}

void
GlobalID::Destroy()
{
  if( !mHandle )
    return;

#if _WIN32

  HANDLE mutex = reinterpret_cast<HANDLE>( mHandle );
  ::ReleaseMutex( mutex );
  ::CloseHandle( mutex );

#else // _WIN32

  string* pName = reinterpret_cast<string*>( mHandle );
  ::sem_unlink( pName->c_str() );
  delete pName;

#endif // _WIN32
  
  mHandle = NULL;
}

void
GlobalID::Cleanup( const string& inName )
{
#if !_WIN32
  ::sem_unlink( inName.c_str() );
#endif // _WIN32
}

} // namespace

bool
ProcessUtils::AssertSingleInstance( int inArgc, char** inArgv, const std::string& inID, int inTimeout )
{
  string name = inID.empty() ? FileUtils::ApplicationTitle() : inID;
  for( int i = 1; i < inArgc; ++i )
    if( !::stricmp( inArgv[i], "--AllowMultipleInstances" ) )
    {
      GlobalID::Cleanup( name );
      return true;
    }
  static const GlobalID sInstance( name, inTimeout );
  return sInstance.Owned();
}

