////////////////////////////////////////////////////////////////////////////////
// $Id: gMOBIlabADC.h 1439 2007-07-19 09:44:43Z mellinger $
// Author: schalk@wadsworth.org
// Description: BCI2000 Source Module for gMOBIlab devices.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef gMOBIlabPlus_ADC_H
#define gMOBIlabPlus_ADC_H

#include "gMOBIlabplus.h"
#include "GenericADC.h"

#include <windows.h>
#include <string>


class gMOBIlabPlusADC : public GenericADC
{
 public:
               gMOBIlabPlusADC();
  virtual      ~gMOBIlabPlusADC();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual void Halt();

 private:
  HANDLE        hEvent;
  HANDLE        hDev;
  short         *buffer;
  _BUFFER_ST    myBuffer;
  int           bufsize;
  OVERLAPPED    ov;
  int           numAChans;
  int           numDChans;
  int           numChans;

  int mEnableDigInput;
  bool mEnableDigOut;

};

#endif // GMOBILAB_ADC_H

