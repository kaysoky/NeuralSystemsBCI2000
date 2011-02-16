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
#include "PCHIncludes.h"
#pragma hdrstop

#include "gMOBIlabThread.h"
#include "gMOBIlabDriver.h"

#include <algorithm>

using namespace std;

const size_t cBufBlocks = 32;      // Size of data ring buffer in terms of sample blocks.
const size_t cMaxReadBuf = 1024;   // Restriction of gtec driver.
const int cMinDataTimeout = 1000;  // Minimum data timeout in ms.
const int cMaxVoidLoopCount = 3;   // Maximum number of zero reads in data acquisition loop.

gMOBIlabThread::gMOBIlabThread( int inBlockSize, int inTimeout, HANDLE inDevice )
: mBlockSize( inBlockSize ),
  mTimeout( inTimeout ),
  mBufSize( inBlockSize * cBufBlocks ),
  mWriteCursor( 0 ),
  mReadCursor( 0 ),
  mpBuffer( NULL ),
  mEvent( NULL ),
  mDev( inDevice )
{
  mEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL );
  mpBuffer = new uint8[mBufSize];
  this->Start();
}

gMOBIlabThread::~gMOBIlabThread()
{
  ::CloseHandle( mEvent );
  delete[] mpBuffer;
}

sint16
gMOBIlabThread::ExtractData()
{
  while( mReadCursor == mWriteCursor && mEvent != NULL )
    ::WaitForSingleObject( mEvent, mTimeout );
  sint16 value = *reinterpret_cast<sint16*>( mpBuffer + mReadCursor );
  mReadCursor += sizeof( sint16 );
  mReadCursor %= mBufSize;
  return value;
}

int
gMOBIlabThread::Execute()
{
  enum { ok = 0, error = 1 } result = ok;

  OVERLAPPED ov;
  ::memset( &ov, 0, sizeof( ov ) );
  ov.hEvent = mEvent;
  ov.Offset = 0;
  ov.OffsetHigh = 0;

  while( !this->IsTerminating() && result == ok )
  {
    int voidLoopCount = 0;
    DWORD bytesReceived = 0;
    while( mWriteCursor < mBufSize && result == ok )
    {
      _BUFFER_ST buf;
      buf.pBuffer = reinterpret_cast<SHORT*>( mpBuffer + mWriteCursor );
      buf.size = min<int>( mBlockSize, mBufSize - mWriteCursor );
      buf.size = min<int>( buf.size, cMaxReadBuf );
      buf.validPoints = 0;
      if( !::GT_GetData( mDev, &buf, &ov ) )
        result = error;
      int dataTimeout = max( cMinDataTimeout, 2 * mTimeout );
      if( WAIT_OBJECT_0 != ::WaitForSingleObject( mEvent, dataTimeout ) )
        result = error;
      if( !::GetOverlappedResult( mDev, &ov, &bytesReceived, FALSE ) )
        result = error;
      if( bytesReceived == 0 && ++voidLoopCount > cMaxVoidLoopCount )
        result = error;
      mWriteCursor += bytesReceived;
    }
    if( mWriteCursor > mBufSize )
      result = error;
    mWriteCursor = 0;
  }
  if( result != ok )
  {
    ::CloseHandle( mEvent );
    mEvent = NULL;
  }
  return result;
}


