////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: A BCI2000 filter that applies a short-term FFT to its input
//   signal.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef FFT_FILTER_H
#define FFT_FILTER_H

#include <vector>
#include "GenericVisualization.h"
#include "GenericFilter.h"

#include "FFTLibWrap.h"

class FFTFilter : public GenericFilter
{
 public:
  FFTFilter();
  virtual ~FFTFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual void Resting();
  virtual bool AllowsVisualization() const { return false; }

 private:
  bool mVisualizeFFT;
  enum eFFTOutputSignal
  {
    eInput = 0,
    ePower,
    eHalfcomplex,
  } mFFTOutputSignal;
  std::vector<size_t> mFFTInputChannels;
  int mFFTWindowLength;
  enum eFFTWindow
  {
    eNone = 0,
    eHamming,
    eHann,
    eBlackman,
  } mFFTWindow;

  void ResetValueBuffers( size_t );
  void DetermineSignalProperties( SignalProperties&, int ) const;

  SignalProperties                  mVisProperties;
  std::vector<GenericVisualization> mVisualizations;
  std::vector<GenericSignal>        mSpectra;
  std::vector<std::vector<float> >  mValueBuffers;
  std::vector<float>                mWindow;

  FFTLibWrapper mFFT;
};

#endif // FFT_FILTER_H
