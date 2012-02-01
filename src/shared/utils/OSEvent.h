//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A wrapper for event objects.
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
///////////////////////////////////////////////////////////////////////
#ifndef OS_EVENT_H
#define OS_EVENT_H

#if _WIN32
# include <windows.h>
#else // _WIN32
# include <pthread.h>
#endif // _WIN32

#include "Uncopyable.h"

class OSEvent : private Uncopyable
{
 public:
  static const int cInfiniteTimeout = -1;

  OSEvent();
  virtual ~OSEvent();

  bool Set();
  bool Reset();
  bool Wait( int timeout_ms = cInfiniteTimeout );

 private:
#if _WIN32
  HANDLE mHandle;
#else // _WIN32
  pthread_cond_t  mCond;
  pthread_mutex_t mMutex;
  bool            mSignaled;
#endif // _WIN32
};

#endif // OS_EVENT_H
