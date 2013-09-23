////////////////////////////////////////////////////////////////////////////////
// $Id: StimBoxFilter.h 4536 2013-08-05 14:30:13Z mellinger $
// Author: griffin.milsap@gmail.com
// Description: A filter which controls a g.STIMbox
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
#ifndef STIM_BOX_FILTER_H
#define STIM_BOX_FILTER_H

#include "GenericFilter.h"
#include "Expression/Expression.h"
#include "OSThread.h"

#include "gSTIMbox.imports.h"

#include <map>
#include <vector>

class StimBoxFilter : public GenericFilter
{
 public:
  // Constructor/Destructor
  StimBoxFilter();
  ~StimBoxFilter();

  friend class StimBoxThread;

 protected:
  // Virtual Interface
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const;
  virtual void Initialize( const SignalProperties& Input,
                           const SignalProperties& Output );
  virtual void Process( const GenericSignal& Input,
                              GenericSignal& Output );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Halt();

 private:
  // Private member methods
  void DisablePorts();

  // Private member variables
  int mPortNumber;
  HANDLE mStimBox;
  std::map< int, Expression > mDigitalOutputExpMap;
  std::vector< int >    mOutputPorts,
                        mInputPorts,
                        mModeselektor,
                        mOutputPortStates;
  std::vector< double > mOutputPortFreqs;

  class StimBoxThread : public OSThread
  {
   public:
             StimBoxThread( StimBoxFilter* inFilt ) : mpFilter( inFilt ) {}
    virtual ~StimBoxThread() {}
    virtual int Execute();

    OSMutex& GetLock() { return mDataLock; }

   private:
    OSMutex mDataLock;
    StimBoxFilter *mpFilter;

  } *mpStimBoxThread;
};

#endif // STIM_BOX_FILTER_H
