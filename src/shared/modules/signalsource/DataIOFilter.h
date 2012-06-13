////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that handles data acquisition from a GenericADC,
//   storing through a GenericFileWriter, and signal calibration into muV.
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
#ifndef DATA_IO_FILTER_H
#define DATA_IO_FILTER_H

#include "GenericFilter.h"
#include "GenericVisualization.h"
#include "EventQueue.h"

#include <vector>
#include <queue>

class GenericFileWriter;
class GenericADC;

class DataIOFilter: public GenericFilter
{
 public:
          DataIOFilter();
  virtual ~DataIOFilter();
  virtual void Publish();
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const;
  virtual void Initialize( const SignalProperties& Input,
                           const SignalProperties& Output );
  virtual void StartRun();
  virtual void StopRun();

  virtual void Process( const GenericSignal& Input,
                              GenericSignal& Output );
  virtual void Resting();
  virtual void Halt();

  virtual bool AllowsVisualization() const { return false; }

 private:
  static void Downsample( const GenericSignal& Input,
                                GenericSignal& Output );
  static void CopyBlock( const GenericSignal& Input,
                               GenericSignal& Output,
                               int            block );
  void EvaluateTiming( double inRoundtrip );
  void ProcessBCIEvents();
  void ResetStates( int kind );

  GenericADC*            mpADC;
  GenericFilter*         mpSourceFilter;
  GenericFileWriter*     mpFileWriter;
  GenericSignal          mOutputBuffer;
  StateVector            mStatevectorBuffer;

  bool                   mVisualizeSource,
                         mVisualizeTiming,
                         mEvaluateTiming;
  int                    mVisualizeSourceDecimation,
                         mVisualizeSourceBufferSize;
  GenericVisualization   mSourceVis,
                         mTimingVis;
  GenericSignal          mDecimatedSignal,
                         mTimingSignal;
  mutable GenericSignal  mADCInput,
                         mInputBuffer,
                         mVisSourceBuffer;
  mutable int            mBlockCount;
  std::vector<double>    mSourceChOffset,
                         mSourceChGain,
                         mTimingBuffer;
  EventQueue             mBCIEvents;
  double                 mBlockDuration;
  int                    mSampleBlockSize;
  size_t                 mTimingBufferCursor;
};

#endif // DATA_IO_FILTER_H
