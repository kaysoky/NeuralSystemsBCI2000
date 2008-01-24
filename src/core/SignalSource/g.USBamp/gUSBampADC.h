////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: schalk@wadsworth.org
// Description: BCI2000 Source Module for gUSBamp devices.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef GUSBAMP_ADC_H
#define GUSBAMP_ADC_H

#include "GenericADC.h"
#include "GenericVisualization.h"

// Number of buffers for g.USBamp acquisition
#define NUM_BUFS        1

#include <windows.h>
#include <vector>
#include <string>

class gUSBampADC : public GenericADC
{
 public:
               gUSBampADC();
  virtual      ~gUSBampADC();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual void Halt();

 private:
  OVERLAPPED     m_ov[20];
  HANDLE         m_hEvent[20];
  std::vector<std::string> m_DeviceIDs;
  std::vector<HANDLE>      m_hdev;
  std::vector<BYTE *>      m_pBuffer[NUM_BUFS];
  std::vector<int>         m_buffersize;
  std::vector<int>         m_iBytesperScan;
  std::vector<int>         m_numchans;
  std::vector<float>       m_LSB; // how many microVolts is one A/D unit (=SourceChGain)
  int            DetectAutoMode() const;
  int            DetermineFilterNumber() const;
  int            DetermineNotchNumber() const;
  GenericVisualization mVis;
  int            m_numdevices;
  float          m_filterhighpass, m_filterlowpass, m_notchhighpass, m_notchlowpass;   // at the moment, only one filter setting for all channels and all devices
  int            m_filtermodelorder, m_filtertype, m_notchmodelorder, m_notchtype;
  std::string    mMasterDeviceID;  // device ID for the master device (exactly one device has to be master)
  int            m_timeoutms;
  bool           mFloatOutput;
  bool           m_digitalinput;
  bool 			 m_digitalOutput;
  int            m_acqmode;         // normal, calibrate, or impedance
  bool 			 m_digitalOut1;
  int            current_buffer;
};

#endif // GUSBAMP_ADC_H



