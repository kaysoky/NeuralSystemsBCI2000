////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: BufferedADC is a base class for signal source filters that
//   provides buffering for the data packets read from the ADC, to avoid data
//   loss when data isn't read timely enough.
//   To interface with an ADC, you need to implement the following functions:
//     Constructor:
//       Define configuration parameters as done in BCI2000 filters.
//     Preflight( SignalProperties& output )
//       The standard Preflight() function as documented for BCI2000 filters
//       except that it takes a single output signal properties argument, and
//       none for input.
//       In addition to parameter checking and reporting of output dimensions,
//       check whether the ADC is available for reading data.
//     Initialize( const SignalProperties& output )
//        The standard Initialize() function, except that it only takes an
//        argument for output properties, and none for input.
//        From here, do all initialization except actually starting data
//        acquisition.
//     StartDataAcquisition()
//       Perform initialization steps necessary to read from the ADC.
//     StopDataAcquisition()
//       Perform de-initialization of the ADC.
//     AcquireData( GenericSignal& output )
//       Read data from the ADC into the GenericSignal object provided as an
//       argument.
//    The last three functions are called from a reading thread, whereas the
//    others are called from the main thread. Still, it is guaranteed that
//    they are never executed simultaneously, so you may read/write member
//    variables of your derived class without using synchronizers.
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
#include "BufferedADC.h"
#include "OSThread.h"
#include "OSMutex.h"
#include "OSEvent.h"
#include <queue>

class BufferedADC : public GenericADC, OSThread
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
  virtual void Preflight( SignalProperties& ) const = 0;
  virtual void Initialize( const SignalProperties& ) = 0;
  virtual void StartDataAcquisition() = 0;
  virtual void StopDataAcquisition() = 0;
  virtual void AcquireData( GenericSignal& ) = 0;

 private:
  virtual int Execute();

  GenericSignal             mBuffer;
  std::queue<GenericSignal> mQueue;
  OSMutex                   mQueueMutex;
  volatile bool             mStartupLock;
};

#endif // BUFFERED_ADC_H

