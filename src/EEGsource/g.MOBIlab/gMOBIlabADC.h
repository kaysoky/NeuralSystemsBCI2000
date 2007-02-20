/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef gMOBIlabADCH
#define gMOBIlabADCH

#include "TCPStream.h"

#include <vector.h>

#include "spa20a.h"

#include "GenericADC.h"

class gMOBIlabADC : public GenericADC
{
 public:
               gMOBIlabADC();
  virtual      ~gMOBIlabADC();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process( const GenericSignal*, GenericSignal* );
  virtual void Halt();

 private:
  HANDLE        hEvent;
  HANDLE        hDev;
  short         *buffer;
  _BUFFER_ST    myBuffer;
  int           bufsize;
  OVERLAPPED    ov;
  int           numchans;
};

#endif // gMOBIlabADCH



