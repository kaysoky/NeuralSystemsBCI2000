#ifndef AverageDisplayH
#define AverageDisplayH

#include <vector>
#include "UGenericVisualization.h"
#include "UGenericFilter.h"
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
  void Initialize();
  void Process( const GenericSignal*, GenericSignal* );

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

#endif // AverageDisplayH
