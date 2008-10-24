#ifndef ModularEEGADCH
#define ModularEEGADCH

#include "GenericADC.h"
#include "modular_eeg_parser.h"

class ModularEEGADC : public GenericADC
{
 public:
               ModularEEGADC();
  virtual      ~ModularEEGADC();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual void Halt();

 private:
  int    samplerate;
  short  comport, protocol;
  bool   modulateamplitude;
  int mCount;
  ser_t devicehandle;
};

#endif // ModularEEGADCH

