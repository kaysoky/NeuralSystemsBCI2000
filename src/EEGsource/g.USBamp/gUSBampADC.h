#ifndef gUSBampADCH
#define gUSBampADCH

#include "TCPStream.h"

#include <vector.h>

#include "GenericADC.h"

#define gUSBampNUMCHANNELS      16

class gUSBampADC : public GenericADC
{
 public:
               gUSBampADC();
  virtual      ~gUSBampADC();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process( const GenericSignal*, GenericSignal* );
  virtual void Halt();

 private:
  HANDLE         m_hEvent;
  vector<string> DeviceIDs;
  vector<HANDLE> hdev;
  vector<BYTE *> pBuffer;
  vector<int>    buffersize;
  vector<int>    iBytesperScan;
  vector<int>    numchans;
  vector<float>  LSB;             // how many microVolts is one A/D unit (=SourceChGain)
  int            numdevices;
  float          filterhighpass, filterlowpass, notchhighpass, notchlowpass;   // at the moment, only one filter setting for all channels and all devices
  int            filtermodelorder, filtertype, notchmodelorder, notchtype;
  string         MasterDeviceID;  // device ID for the master device (exactly one device has to be master)
  int            timeoutms;
};

#endif // gUSBampADCH

