////////////////////////////////////////////////////////////////////////////////
// $Id: $
// Authors: griffin.milsap@gmail.com
// Description: A class which manages a gHIamp device
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
#ifndef INCLUDED_GHIAMPDEVICE_H 
#define INCLUDED_GHIAMPDEVICE_H

#include <windows.h>
#include <gHIamp.h>
#include <vector>
#include "GenericSignal.h"

class gHIampDevice 
{
 public:
  gHIampDevice( HANDLE device ) { Init( device ); }
  ~gHIampDevice() { Cleanup(); }
  void Init( HANDLE device );
  void Cleanup();

  // Device control
  bool MapAnalogChannel( unsigned int devicech, unsigned int sourcech, bool err = true );
  bool MapDigitalChannel( unsigned int devicech, unsigned int sourcech );
  int MapAllAnalogChannels( int startch, int numch );
  void SetNotch( int iNotchNo );
  void SetFilter( int iFilterNo );
  bool SetRefChan( int devicech );
  void SetConfiguration( int iSampleRate, int iSampleBlockSize );
  void BeginAcquisition();
  void GetData( GenericSignal& Output );
  void EndAcquisition();

  // Getters
  std::string Serial() { return std::string( mSerial ); }

  HANDLE                 mDevice;
  GT_HIAMP_CONFIGURATION mConfig;

 private:
  bool        mConfigured;
  char        mSerial[16]; 
  int         mQueueIndex,
              mNumChannels,
              mNumScans,
              mRefIdx;
  BYTE**      mpBuffers;
  OVERLAPPED* mpOverlapped;
  size_t      mBufferSizeBytes;
  std::map< int, int > mAnalogChannelMap;
  std::map< int, int > mDigitalChannelMap;
};

class gHIampDeviceContainer : public std::vector< gHIampDevice >
{
 public:
  bool Detect();
  void Close();
  void Remove( gHIampDeviceContainer::iterator& itr );
};

#endif // INCLUDED_GHIAMPDEVICE_H