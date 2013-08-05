////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A C++ wrapper for PortAudio streams, providing a functional
//   blocking interface independently of the host api.
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
#ifndef PORTAUDIO_STREAM_H
#define PORTAUDIO_STREAM_H

#include "portaudio.h"
#include "GenericSignal.h"
#include "OSEvent.h"
#include "OSMutex.h"
#include <inttypes.h>
#include <string>

class PortAudioStream
{
 public:
  PortAudioStream( PaDeviceIndex, const SignalProperties& );
  ~PortAudioStream();
  const SignalProperties& Properties() const
    { return mProperties; }
  const std::string& Error() const
    { return mError; }

  bool Start();
  bool Stop();
  bool Read( GenericSignal& );

 private:
  void InitHostApi( PaHostApiTypeId );
  void CreateStreamObject();
  void DeleteStreamObject();
  double SamplingRate() const;
  unsigned long NativeBlockSize();
  bool IsError( PaError );

  struct
  {
    PaHostApiTypeId id;
    void* privateData;
    PaStreamCallback* callback;
    PaError (*Init)( PaStream*, int, void** );
    PaError (*Cleanup)( PaStream*, void* );
    PaError (*Measure)( PaStream*, unsigned long* );
    PaError (*Start)( PaStream*, void* );
    PaError (*Stop)( PaStream*, void* );
    PaError (*ReadBegin)( PaStream*, void*, const int16_t** );
    PaError (*ReadEnd)( PaStream*, void* );
  } mHostApi;

  SignalProperties mProperties;
  std::string mError;

  PaStreamParameters mParameters;
  PaStream* mpStream;

  static int sInstanceCount;

 private:
  // PortAudio favors its callback interface over its blocking interface, so we use the callback
  // interface by default.
  static int StreamCallback( const void*, void*, unsigned long, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* );
  static PaError PaStart( PaStream*, void* );
  static PaError PaStop( PaStream*, void* );
  static PaError PaReadSync( PaStream*, void*, const int16_t** );
  static PaError PaReadBegin( PaStream*, void*, const int16_t** );
  static PaError PaReadEnd( PaStream*, void* );

  const void* mpBuffer;
  int16_t* mpAllocatedBuffer;
  int mTimeout;
  OSEvent mBufferPrepared, mBufferDone;
};

#endif // PORTAUDIO_ADC_H
