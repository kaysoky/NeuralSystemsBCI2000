////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates a data acquisition thread for
//   gMOBIlab and gMOBIlab+ devices.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
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

