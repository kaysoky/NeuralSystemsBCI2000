//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A thread-safe event queue, implemented as a linked
//   list.
//   An event is defined by a string description, and a time stamp.
//   Any thread may insert/remove events at any time.
//   We assume new/delete to be thread-safe for structs without
//   constructors.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "EventQueue.h"

void
EventQueue::PushBack( const char* inDescriptor, PrecisionTime inTimeStamp )
{
  int descLen = ::strlen( inDescriptor ) + 1;
  Entry* pEntry = new Entry;
  pEntry->mpDescriptor = new char[ descLen ];
  ::memcpy( pEntry->mpDescriptor, inDescriptor, descLen );
  pEntry->mTimeStamp = inTimeStamp;
  pEntry->mpNext = NULL;
  Lock();
  if( mpBack != NULL )
    mpBack->mpNext = pEntry;
  mpBack = pEntry;
  if( mpFront == NULL )
    mpFront = mpBack;
  Unlock();
}

void
EventQueue::PopFront()
{
  Lock();
  Entry* pEntry = mpFront;
  mpFront = pEntry->mpNext;
  if( mpFront == NULL )
    mpBack = NULL;
  Unlock();
  delete[] pEntry->mpDescriptor;
  delete pEntry;
}

