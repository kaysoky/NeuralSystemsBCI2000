////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates a data acquisition thread for
//   gMOBIlab and gMOBIlab+ devices.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "gMOBIlabThread.h"
#include "gMOBIlabDriver.h"

using namespace std;

const int cBufBlocks = 32;    // Size of data ring buffer in terms of sample blocks.
const int cMaxReadBuf = 1024; // Restriction of gtec driver.

gMOBIlabThread::gMOBIlabThread( int inBlockSize, int inTimeout, HANDLE inDevice )
: OSThread( true ),
  mBlockSize( inBlockSize ),
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
  this->Resume();
}

gMOBIlabThread::~gMOBIlabThread()
{
  ::CloseHandle( mEvent );
  delete[] mpBuffer;
}

sint16
gMOBIlabThread::ExtractData()
{
  while( mReadCursor == mWriteCursor )
    ::WaitForSingleObject( mEvent, mTimeout );
  sint16 value = *reinterpret_cast<sint16*>( mpBuffer + mReadCursor );
  mReadCursor += sizeof( sint16 );
  mReadCursor %= mBufSize;
  return value;
}

int
gMOBIlabThread::Execute()
{
  enum { ok = 0, errorOccurred = 1 };

  OVERLAPPED ov;
  ::memset( &ov, 0, sizeof( ov ) );
  ov.hEvent = mEvent;
  ov.Offset = 0;
  ov.OffsetHigh = 0;

  while( !this->IsTerminating() )
  {
    DWORD bytesReceived = 0;
    while( mWriteCursor < mBufSize )
    {
      _BUFFER_ST buf;
      buf.pBuffer = reinterpret_cast<SHORT*>( mpBuffer + mWriteCursor );
      buf.size = min( mBlockSize, mBufSize - mWriteCursor );
      buf.size = min( buf.size, cMaxReadBuf );
      buf.validPoints = 0;
      if( !::GT_GetData( mDev, &buf, &ov ) )
        return errorOccurred;
      ::GetOverlappedResult( mDev, &ov, &bytesReceived, TRUE );
      mWriteCursor += bytesReceived;
    }
    if( mWriteCursor > mBufSize )
      return errorOccurred;
    mWriteCursor = 0;
  }
  return ok;
}


