////////////////////////////////////////////////////////////////////////////////
//
// File: RDAClientADC.h
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date: Jan 3, 2003
//
// Description: A source class that interfaces to the BrainAmp RDA socket
//              interface.
//
// Changes:     Apr 3, 2003: Adaptations to the changes introduced by the error
//              handling facilities.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef RDACLIENTADCH
#define RDACLIENTADCH

#include <string>
#include "GenericADC.h"
#include "RDAQueue.h"

class RDAClientADC : public GenericADC
{
 public:
                RDAClientADC();
  virtual       ~RDAClientADC();
  virtual void  Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void  Initialize();
  virtual void  Process( const GenericSignal*, GenericSignal* );
  virtual void  Halt();

 private:
  std::string  hostName;
  RDAQueue     inputQueue;
};

#endif // RDACLIENTADCH


