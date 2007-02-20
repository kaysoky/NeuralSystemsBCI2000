/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef FFTFilterH
#define FFTFilterH

#include <vector>
#include "UGenericVisualization.h"
#include "UGenericFilter.h"

#include "fftlib/FFTLibWrap.h"

class FFTFilter : public GenericFilter
{
 public:
  FFTFilter();
  virtual ~FFTFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process( const GenericSignal*, GenericSignal* );
  virtual void Resting();

 private:
  bool mVisualizeFFT;
  enum eFFTOutputSignal
  {
    eInput = 0,
    ePower,
    eHalfcomplex,
  } mFFTOutputSignal;
  std::vector<size_t> mFFTInputChannels;
  int mFFTSize;
  enum eFFTWindow
  {
    eNone = 0,
    eHamming,
    eHann,
    eBlackman,
  } mFFTWindow;

  void ResetValueBuffers( size_t );

  std::vector<GenericVisualization> mVisualizations;
  std::vector<GenericSignal>        mSpectra;
  std::vector<std::vector<float> >  mValueBuffers;
  std::vector<float>                mWindow;

  FFTLibWrapper mFFT;
};

#endif // FFTFilterH


