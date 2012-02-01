////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: BufferedADC is a base class for signal source filters that
//   provides buffering for data packets read from the ADC, to avoid data
//   loss when data isn't read timely enough.
//   To interface with an ADC, you need to implement the following functions:
//     Constructor:
//       Define configuration parameters as done in BCI2000 filters.
//     OnPreflight( SignalProperties& output )
//       The standard Preflight() function as documented for BCI2000 filters
//       except that it takes a single output signal properties argument, and
//       none for input.
//       In addition to parameter checking and reporting of output dimensions,
//       check whether the ADC is available for reading data.
//     OnInitialize( const SignalProperties& output )
//        The standard Initialize() function, except that it only takes an
//        argument for output properties, and none for input.
//        From here, do all initialization except actually starting data
//        acquisition.
//     OnProcess()
//        Called at the beginning of the standard Process() function.
//        Override this method when you need to act on state variables,
//        e.g. to set a digital output on the amplifier.
//     OnStartAcquisition()
//       Perform initialization steps necessary to read from the ADC.
//     OnStopAcquisition()
//       Perform de-initialization of the ADC.
//     DoAcquire( GenericSignal& output )
//       Read data from the ADC into the GenericSignal object provided as an
//       argument.
//    The latter three functions are called from a data acquisition thread, whereas
//    remaining ones are called from the main thread. Still, it is guaranteed that
//    functions of this interface are not executed concurrently, so you may
//    read/write member variables of your derived class without using synchronizers.
//    An important exception are OnProcess() and DoAcquire(), which may execute
//    concurrently.
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
#ifndef BUFFERED_ADC_H
#define BUFFERED_ADC_H

#include "GenericADC.h"
#include "OSThread.h"
#include "OSEvent.h"
#include "OSMutex.h"
#include "PrecisionTime.h"
#include <vector>

class BufferedADC : public GenericADC, private OSThread
{
 protected:
  BufferedADC();

 public:
  virtual ~BufferedADC();
  // GenericFilter inherited functions.
  virtual void Preflight( const SignalProperties&,
                                SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&,
                           const SignalProperties& );
  virtual void StartRun() {}
  virtual void StopRun() {}
  virtual void Process( const GenericSignal&,
                              GenericSignal& );
  virtual void Halt();

 protected:
  // Virtual data acquisition interface.
  virtual void OnPreflight( SignalProperties& ) const = 0;
  virtual void OnInitialize( const SignalProperties& ) = 0;
  virtual void OnProcess() {}
  virtual void OnStartAcquisition() = 0;
  virtual void OnStopAcquisition() = 0;
  virtual void DoAcquire( GenericSignal& ) = 0;
  virtual void OnHalt() {}

 private:
  virtual int Execute();

  std::vector<GenericSignal> mBuffer;
  std::vector<PrecisionTime> mTimeStamps;
  size_t                     mReadCursor,
                             mWriteCursor;
  OSMutex                    mMutex;
  OSEvent                    mAcquisitionDone;
};

#endif // BUFFERED_ADC_H

