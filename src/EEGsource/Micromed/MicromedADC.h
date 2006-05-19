#ifndef MicromedADCH
#define MicromedADCH

#include "TCPStream.h"
#include "GenericADC.h"
#include "UBCITime.h"
#include "MicromedNetRead.h"




class MicromedADC : public GenericADC
{
 public:
               MicromedADC();
  virtual      ~MicromedADC();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process( const GenericSignal*, GenericSignal* );
  virtual void Halt();

 private:

  bool   ProcessDataMsg(CAcqMessage *pMsg, int packetnr, GenericSignal *);
  server_tcpsocket MmServerSocket;
  tcpstream MmServer;
  int    samplerate;
  int    mSignalType;
  int    num_channels;
  int    BCIblocksize;
  int    MMblocksize;
  int    bytespersample;

};

#endif // MicromedADCH

