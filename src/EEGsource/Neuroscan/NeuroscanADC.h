#ifndef NeuroscanADCH
#define NeuroscanADCH

#include "TCPStream.h"

#include "GenericADC.h"
#include "NeuroscanNetRead.h"

class NeuroscanADC : public GenericADC
{
 public:
               NeuroscanADC();
  virtual      ~NeuroscanADC();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process( const GenericSignal*, GenericSignal* );
  virtual void Halt();

 private:
  bool          ProcessDataMsg(CAcqMessage *pMsg, GenericSignal *);
  void          SendCommand(unsigned short controlcode, unsigned short command);
  tcpstream     *server;
  client_tcpsocket *c_tcpsocket;
  int           num_channels, num_markerchannels, blocksize, samplingrate, bitspersample;
  float         LSB;
};

#endif // NeuroscanADCH

