//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A wrapper for semaphore objects.
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
#ifndef OS_SEMAPHORE_H
#define OS_SEMAPHORE_H

#ifdef _WIN32
# include <windows.h>
#else
# include <semaphore.h>
#endif // _WIN32

#include "Uncopyable.h"

class OSSemaphore : private Uncopyable
{
 public:
  OSSemaphore( int count = 1 );
  virtual ~OSSemaphore();

  bool Acquire() const;
  bool Release() const;

 private:
#ifdef _WIN32
  HANDLE mHandle;
#else // _WIN32
  sem_t mSemaphore;
#endif // _WIN32
};

#endif // OS_SEMAPHORE_H
