//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A thread class similar to the VCL's TThread.
//   To implement your own thread, create a class that inherits from
//   OSThread, and put your own functionality into its
//   Execute() function.
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
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "OSThread.h"
#include "BCIError.h"
#include "OSError.h"

#if !_WIN32
# include <unistd.h>
#endif // !_WIN32

using namespace std;

#if _WIN32

OSThread::OSThread()
: mHandle( NULL ),
  mThreadID( 0 ),
  mResult( 0 ),
  mTerminating( false )
{
}

OSThread::~OSThread()
{
  if( mHandle != NULL )
    ::TerminateThread( mHandle, 0 );
}

void
OSThread::Start()
{
  mHandle = ::CreateThread( NULL, 0, OSThread::StartThread, this, 0, &mThreadID );
  if( mHandle == NULL )
    bcierr << OSError().Message() << endl;
}

void
OSThread::Terminate()
{
  mTerminating = true;
}

bool 
OSThread::IsTerminated() const
{
  return mHandle == NULL;
}

void
OSThread::Sleep( int inMs )
{
  ::Sleep( inMs );
}

#else // _WIN32

OSThread::OSThread()
: mTerminated( false ),
  mResult( 0 ),
  mTerminating( false )
{
}

OSThread::~OSThread()
{
  if( !mTerminated )
    ::pthread_kill( mThread, SIGHUP );
}

void
OSThread::Start()
{
  ::pthread_create( &mThread, NULL, OSThread::StartThread, this );
}

void
OSThread::Terminate()
{
  mTerminating = true;
}

bool 
OSThread::IsTerminated() const
{
  return mTerminated;
}

void
OSThread::Sleep( int inMs )
{
  ::usleep( inMs * 1000 );
}

#endif // _WIN32


#if _WIN32

DWORD WINAPI
OSThread::StartThread( void* inInstance )
{
  OSThread* this_ = reinterpret_cast<OSThread*>( inInstance );
  this_->mResult = this_->Execute();
  ::CloseHandle( this_->mHandle );
  this_->mHandle = NULL;
  return this_->mResult;
}

#else // _WIN32

void*
OSThread::StartThread( void* inInstance )
{
  OSThread* this_ = reinterpret_cast<OSThread*>( inInstance );
  this_->mResult = this_->Execute();
  this_->mTerminated = true;
  return &this_->mResult;
}

#endif // _WIN32
