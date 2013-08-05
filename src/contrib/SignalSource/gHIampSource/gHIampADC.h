////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: griffin.milsap@gmail.com
// Description: gHIampADC header
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
#ifndef INCLUDED_GHIAMPADC_H 
#define INCLUDED_GHIAMPADC_H

#include "gHIampDevice.h"
#include "BufferedADC.h"
#include "PrecisionTime.h"
#include <map>
#include <set>
#include <string>

class gHIampADC : public BufferedADC
{
 public:
           gHIampADC();
  virtual ~gHIampADC();
  virtual void OnPreflight( SignalProperties& Output ) const;
  virtual void OnInitialize( const SignalProperties& Output );
  virtual void OnStartAcquisition();
  virtual void OnStopAcquisition();
  virtual void DoAcquire( GenericSignal& );
  virtual void OnHalt();

 private:
  bool DetermineFilterNumber( int& oFilterNumber ) const;
  bool DetermineNotchNumber( int& oFilterNumber ) const;

  typedef std::map< int, std::set< int > > ModeMap;
  ModeMap ParseModes( std::string modes ) const;

  size_t mMasterIdx;
  gHIampDeviceContainer mDevices;

  // A little class for parsing SourceChList
  class SrcCh
  {
   public:
    explicit SrcCh( std::string s );
    int Amp() { return mAmp; }
    int Channel() { return mChannel; }
    bool IsDigital() { return mDigital; }
   
   private:
    int mAmp, mChannel;
    bool mDigital;
  };
};

#endif // INCLUDED_GHIAMPADC_H
