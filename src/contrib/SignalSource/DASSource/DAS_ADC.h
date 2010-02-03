////////////////////////////////////////////////////////////////////////////////
//
// File: DAS_ADC.h
//
// Author: juergen.mellinger@uni-tuebingen.de
//         original version by thilo.hinterberger@uni-tuebingen.de
//
// Date: Sep 18, 2003
//
// Description: A source class that interfaces to the
//              ComputerBoards/Measurement Computing Universal Library.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef DAS_ADCH
#define DAS_ADCH

#include "GenericADC.h"
#include "DASQueue.h"

class TDAS_ADC : public GenericADC
{
 public:
  TDAS_ADC();
  virtual ~TDAS_ADC(){}
  virtual void Preflight( const SignalProperties&,
                                SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&,
                           const SignalProperties& );
  virtual void Process(   const GenericSignal&,
                                GenericSignal& );
  virtual void Halt();

 private:
  DASQueue mInputQueue;
};

#endif // DAS_ADCH
