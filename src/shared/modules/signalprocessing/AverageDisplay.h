////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: A BCI2000 filter that computes epoch averages of its input
//   signal, and displays these in a visualization window.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef AVERAGE_DISPLAY_H
#define AVERAGE_DISPLAY_H

#include <vector>
#include "GenericVisualization.h"
#include "GenericFilter.h"
#include "Color.h"

#define SET_BASELINE

class AverageDisplay : public GenericFilter
{
  enum
  {
    maxPower = 1
  };

 public:
  AverageDisplay();
  void Preflight( const SignalProperties&, SignalProperties& ) const;
  void Initialize( const SignalProperties&, const SignalProperties& );
  void Process( const GenericSignal&, GenericSignal& );
  bool AllowsVisualization() const { return false; }

 private:
  std::vector<GenericVisualization> mVisualizations;
  std::vector<size_t> mChannelIndices;
  std::vector<size_t> mTargetCodes;

  // Indices are [ channel ][ sample ].
  std::vector<std::vector<float> > mSignalOfCurrentRun;

  // Indices are [ power ][ channel ][ targetCode ][ sample ].
  std::vector<std::vector<std::vector<std::vector<float> > > > mPowerSums;

  size_t mLastTargetCode;

#ifdef SET_BASELINE
  // Baseline stuff that should really be factored out.
  std::vector<float> mBaselines, mBaselineSamples;
#endif // SET_BASELINE
  // A list of channel colors to use for the overlayed display.
  static const RGBColor sChannelColors[];
};

#endif // AVERAGE_DISPLAY_H
