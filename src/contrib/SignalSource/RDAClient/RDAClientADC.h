////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A source class that interfaces to the BrainAmp RDA socket
//              interface.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef RDA_CLIENT_ADC_H
#define RDA_CLIENT_ADC_H

#include <string>
#include "GenericADC.h"
#include "RDAQueue.h"

class RDAClientADC : public GenericADC
{
 public:
                RDAClientADC();
  virtual       ~RDAClientADC();
  virtual void  Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void  Initialize( const SignalProperties&, const SignalProperties& );
  virtual void  Process( const GenericSignal&, GenericSignal& );
  virtual void  Halt();

 private:
  std::string  mHostName;
  RDAQueue     mInputQueue;
};

#endif // RDA_CLIENT_ADC_H


