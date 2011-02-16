////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates a data acquisition thread for
//   gMOBIlab and gMOBIlab+ devices.
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
#ifndef GMOBILAB_THREAD_H
#define GMOBILAB_THREAD_H

#include "defines.h"
#include "OSThread.h"

class gMOBIlabThread : public OSThread
{
 public:
  gMOBIlabThread( int inBlockSize, int inTimeout, HANDLE inDevice );
  virtual ~gMOBIlabThread();

  sint16 ExtractData();

 private:
  virtual int Execute();

  int    mBlockSize,
         mTimeout,
         mBufSize,
         mWriteCursor,
         mReadCursor;
  uint8* mpBuffer;
  HANDLE mEvent,
         mDev;
};

#endif // GMOBILAB_THREAD_H

