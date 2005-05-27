#ifndef TaskH
#define TaskH

#include <vcl.h>
#include <ComCtrls.hpp>
#include <Series.hpp>

#include <vector>
#include "MidiPlayer.h"
#include "WavePlayer.h"
#include "UGenericVisualization.h"
#include "UGenericFilter.h"

class TTask : public GenericFilter
{
 public:
          TTask();
  virtual ~TTask();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process( const GenericSignal* Input, GenericSignal* Output );
  virtual void Halt();

 private:
  int MakeMusic( short Controlsignal );

  enum
  {
    ultra_low = 0,
    low,
    medium,
    high,
    ultra_high,
  };

  enum
  {
    amNone,
    amMidi,
    amWave,
  };

  int                     mAcousticMode;
  TMidiPlayer             mMidiPlayer;
  typedef std::vector<TWavePlayer> WavePlayerContainer;
  WavePlayerContainer     mWavePlayers;
  GenericVisualization    mVis;

  // VCL object pointers.
  TForm*                  mpForm;
  TProgressBar*           mpProgressbar;
  TChart*                 mpChart;
  TLineSeries*            mpSeries;
};

#endif // TaskH
