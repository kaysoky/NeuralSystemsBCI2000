#ifndef TaskH
#define TaskH

#include <vcl.h>
#include <ComCtrls.hpp>

#include <stdio.h>
#include <map>

#include "UBitRate.h"
#include "UGenericVisualization.h"
#include "MidiPlayer.h"
#include "WavePlayer.h"
#include "UGenericFilter.h"

#include <vcl/series.hpp>


class TTask : public GenericFilter
{
 private:
        GenericVisualization  *vis;
        int             AcousticMode;
        int             MakeMusic(short controlsignal);
        TForm           *form;
        TProgressBar    *progressbar;
        TChart          *chart;
        TLineSeries     *series;
        TMidiPlayer     midiPlayer;
        typedef std::map<std::string, TWavePlayer> WavePlayerContainer;
        WavePlayerContainer wavePlayers;

 public:
          TTask();
  virtual ~TTask();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process( const GenericSignal* Input, GenericSignal* Output );
  virtual void Halt();
};

#endif // TaskH
