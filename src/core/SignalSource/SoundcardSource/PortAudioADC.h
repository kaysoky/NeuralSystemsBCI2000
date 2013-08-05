////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A BufferedADC that gets input through the PortAudio interface.
//
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
#ifndef PORTAUDIO_ADC_H
#define PORTAUDIO_ADC_H

#include "BufferedADC.h"
#include "PortAudioStream.h"
#include <map>


class PortAudioADC : public BufferedADC
{
 public:
  PortAudioADC();
  ~PortAudioADC();

  void OnAutoConfig();
  void OnPreflight( SignalProperties& Output ) const;
  void OnInitialize( const SignalProperties& Output );
  void OnHalt();
  void OnStartAcquisition();
  void OnStopAcquisition();
  void DoAcquire( GenericSignal& Output );

 private:
  typedef std::map<int,PaHostApiIndex> ApiList;
  ApiList mHostApis;

  PaDeviceIndex mDevice;
  PortAudioStream* mpStream;
  static int sInstanceCount;
};

#endif // PORTAUDIO_ADC_H
