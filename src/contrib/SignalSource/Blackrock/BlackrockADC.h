////////////////////////////////////////////////////////////////////////////////
// $Id: $
// Authors: griffin.milsap@gmail.com
// Description: Implementation of a source module for Blackrock systems
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

#ifndef INCLUDED_BLACKROCKADC_H
#define INCLUDED_BLACKROCKADC_H

#include "BufferedADC.h"
#include "PrecisionTime.h"
#include "OSMutex.h"

#include <cbsdk.h>
#include <queue>
#include <map>

class BlackrockADC : public BufferedADC
{
 public:
           BlackrockADC();
  virtual ~BlackrockADC();
  virtual void OnHalt();
  virtual void OnPreflight( SignalProperties& Output ) const;
  virtual void OnInitialize( const SignalProperties& Output );
  virtual void StartRun();
  virtual void OnStartAcquisition();
  virtual void DoAcquire( GenericSignal& Output );
  virtual void OnStopAcquisition();
  virtual void StopRun();

 private:
  bool Connect() const;
  static void DataCallback( UINT32 iInstance, const cbSdkPktType iType, const void* iData, void* iBlackrockADC );
  bool CreateChannelList( std::string iParameter, int iMaxEntries, std::vector< int > &oVector ) const; 
  void Disconnect() const;
  static bool CereLinkError( cbSdkResult res );

  OSMutex mDataMutex;
  std::map< int, std::queue< INT16 > > mDataBuffer;
  std::vector< int > mAnalogCh;
  int mFramesQueued;
  int mSampleRateEnum;
  int mFilterEnum;
  int mSampleBlockSize;
};

#endif // INCLUDED_BLACKROCKADC_H
