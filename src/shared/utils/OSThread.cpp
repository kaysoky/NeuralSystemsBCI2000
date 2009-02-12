//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A thread class similar to the VCL's TThread.
//   To implement your own thread, create a class that inherits from
//   OSThread, and put your own functionality into its
//   Execute() function.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "OSThread.h"
#include "BCIError.h"
#include "OSError.h"

using namespace std;

OSThread::OSThread( bool inCreateSuspended )
: mHandle( NULL ),
  mThreadID( 0 ),
  mTerminating( false )
{
  int creationFlags = 0;
  if( inCreateSuspended )
    creationFlags |= CREATE_SUSPENDED;
  mHandle = ::CreateThread( NULL, 0, OSThread::StartThread, this, creationFlags, &mThreadID );
  if( mHandle == NULL )
    bcierr << OSError().Message() << endl;
}

OSThread::~OSThread()
{
  if( mHandle != NULL )
    ::TerminateThread( mHandle, 0 );
}

void
OSThread::Suspend()
{
  if( mHandle != NULL )
    ::SuspendThread( mHandle );
}

void
OSThread::Resume()
{
  if( mHandle != NULL )
    ::ResumeThread( mHandle );
}

void
OSThread::Terminate()
{
  mTerminating = true;
  if( mHandle != NULL )
    while( !::PostThreadMessage( mThreadID, WM_QUIT, 0, 0 )
            && ::GetLastError() != ERROR_INVALID_THREAD_ID )
      ::Sleep( 0 );
}

int
OSThread::Execute()
{
  MSG msg;
  int result = 0;
  while( 1 == ( result = ::GetMessage( &msg, NULL, 0, 0 ) ) )
  {
    ::TranslateMessage( &msg );
    ::DispatchMessage( &msg );
  }
  return result;
}


DWORD WINAPI
OSThread::StartThread( void* inInstance )
{
  OSThread* this_ = reinterpret_cast<OSThread*>( inInstance );
  this_->mResult = this_->Execute();
  ::CloseHandle( this_->mHandle );
  this_->mHandle = NULL;
  return this_->mResult;
}
