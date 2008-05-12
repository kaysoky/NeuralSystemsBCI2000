////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that handles data acquisition from a GenericADC,
//   storing through a GenericFileWriter, and signal calibration into muV.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef DATA_IO_FILTER_H
#define DATA_IO_FILTER_H

#include "GenericFilter.h"
#include "GenericVisualization.h"

class GenericFileWriter;

class DataIOFilter: public GenericFilter
{
 public:
          DataIOFilter();
  virtual ~DataIOFilter();
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

  GenericFilter*         mpADC,
               *         mpSourceFilter;
  GenericFileWriter*     mpFileWriter;
  GenericSignal          mOutputBuffer;
  StateVector            mStatevectorBuffer;

  bool                   mVisualizeSource,
                         mVisualizeTiming;
  int                    mVisualizeSourceDecimation;
  GenericVisualization   mSourceVis,
                         mTimingVis;
  GenericSignal          mDecimatedSignal,
                         mTimingSignal;
  mutable GenericSignal  mInputBuffer;
  std::vector<float>     mSourceChOffset,
                         mSourceChGain;
};

#endif // DATA_IO_FILTER_H
