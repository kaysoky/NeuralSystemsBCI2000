////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: schalk@wadsworth.org
// Description: BCI2000 Source Module for gMOBIlab devices.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef GMOBILAB_ADC_H
#define GMOBILAB_ADC_H

#include "GenericADC.h"
#include "gMOBIlabThread.h"

class gMOBIlabADC : public GenericADC
{
 public:
               gMOBIlabADC();
  virtual      ~gMOBIlabADC();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual void Halt();

 private:
  HANDLE          mDev;
  gMOBIlabThread* mpAcquisitionThread;
};

#endif // GMOBILAB_ADC_H

