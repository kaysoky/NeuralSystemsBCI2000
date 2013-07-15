////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A BufferedADC that gets input from a sound card.
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
#ifndef SOUNDCARD_ADC_H
#define SOUNDCARD_ADC_H

#include "BufferedADC.h"
#include "OSEvent.h"
#include "portaudio.h"
#include <map>


class SoundcardADC : public BufferedADC
{
 public:
  SoundcardADC();
  ~SoundcardADC();

  void OnAutoConfig();
  void OnPreflight( SignalProperties& Output ) const;
  void OnInitialize( const SignalProperties& Output );
  void OnStartAcquisition();
  void OnStopAcquisition();
  void DoAcquire( GenericSignal& Output );

 protected:
   bool UseAcquisitionThread() const { return mpBuffer; }

 private:
  enum { auto_ = 0, callback = 1, blocking = 2 };
  PaStreamParameters StreamParameters() const;
  unsigned long NativeBlockSize() const;
  static int AcquisitionCallback( const void*, void*, unsigned long,
    const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* );

  typedef std::map<int,PaHostApiIndex> ApiList;
  ApiList mHostApis;

  PaDeviceIndex mDevice;
  PaStream* mpStream;

  float* mpBuffer;
  struct {
    GenericSignal* pOutput;
    int outputIdx;
  } mCallbackState;
};

#endif // SOUNDCARD_ADC_H
