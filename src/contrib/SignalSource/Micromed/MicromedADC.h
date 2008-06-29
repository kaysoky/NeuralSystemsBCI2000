/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef MicromedADCH
#define MicromedADCH

#include "SockStream.h"
#include "GenericADC.h"
#include "PrecisionTime.h"
#include "MicromedNetRead.h"




class MicromedADC : public GenericADC
{
 public:
               MicromedADC();
  virtual      ~MicromedADC();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual void Halt();

 private:

  bool   ProcessDataMsg(CAcqMessage *pMsg, int packetnr, GenericSignal *);
  server_tcpsocket MmServerSocket;
  sockstream MmServer;
  int    samplerate;
  int    mSignalType;
  int    num_channels;
  int    BCIblocksize;
  int    MMblocksize;
  int    bytespersample;
  int    conditionmask;
  int    mTimerID;
  int    mTimerDelay;
  

};

#endif // MicromedADCH

