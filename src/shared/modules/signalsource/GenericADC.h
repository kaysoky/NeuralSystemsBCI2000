////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: GenericADC defines the virtual function interface for signal
//   source filters, and thus is the base class for all ADC filter classes.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef GENERIC_ADC_H
#define GENERIC_ADC_H

#include "GenericFilter.h"

class GenericADC : public GenericFilter
{
 protected:
  GenericADC() {}

 public:
  virtual ~GenericADC() {}
  // GenericFilter inherited functions.
  virtual void Preflight( const SignalProperties&,
                                SignalProperties& ) const = 0;
  virtual void Initialize( const SignalProperties&,
                           const SignalProperties& ) = 0;
  virtual void StartRun() {}
  virtual void StopRun() {}
  virtual void Process(   const GenericSignal&,
                                GenericSignal& ) = 0;
  virtual void Halt() = 0;

  virtual bool AllowsVisualization() const { return false; }
};

#endif // GENERIC_ADC_H

