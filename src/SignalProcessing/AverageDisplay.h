#ifndef AverageDisplayH
#define AverageDisplayH

#include <vector>
#include "UGenericVisualization.h"
#include "UGenericFilter.h"

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
  bool mITI;

#ifdef SET_BASELINE
  // Baseline stuff that should really be factored out.
  std::vector<float> mBaselines, mBaselineSamples;
#endif // SET_BASELINE
};

#endif // AverageDisplayH
