/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef NeuroscanADCH
#define NeuroscanADCH

#include "SockStream.h"

#include "GenericADC.h"
#include "NeuroscanNetRead.h"

class NeuroscanADC : public GenericADC
{
 public:
               NeuroscanADC();
  virtual      ~NeuroscanADC();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual void Halt();

 private:
  bool          ProcessDataMsg(CAcqMessage *pMsg, GenericSignal *);
  void          SendCommand(unsigned short controlcode, unsigned short command);
  sockstream    *server;
  client_tcpsocket *c_tcpsocket;
  int           num_channels, num_markerchannels, blocksize, samplingrate, bitspersample;
  float         LSB;
};

#endif // NeuroscanADCH

