/******************************************************************************
 * $Id$                                                                       *
 * Program:   TMSI.EXE                                                        *
 * Module:    TMSiADC.H                                                       *
 * Comment:   Definition for the TMSiADC class                                *
 * Version:   0.04                                                            *
 * Author:    M.M.Span                                                        *
 * Copyright: (C) RUG University of Groningen                                 *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.01 - 12/10/2005 - First start                                           *
 * V0.02 - 27/10/2005 - Primary working version: entered testing phase        *
 * V0.03 - 25/01/2006 - Multiple devices (PORTI) useable                      *
 *                      Synchronizing PORTIs work in progress                 *
 * V0.04 - 03/04/2006 - Porti Synchro disbanded. Working on selecting channels*
 * V0.05 - 15/05/2006 - Using the features pull out unused channels from the  *
 *                      common reference pool                                 *
 * $Log$
 * Revision 1.1  2006/07/04 18:45:50  mellinger
 * Put files into CVS.
 *                                                                      *
 ******************************************************************************/
// MMS adaption from the TMSiDemo in driver

#ifndef TMSiADCH
#define TMSiADCH

#define DEBUG

#include "GenericADC.h"
#include "RTDevice.h"     // from TMSi driver
#include "Feature.h"      // from TMSi driver

#define MAX_DEVICE                  20 //Max number of devices supported
#define USE_MASTER_SLAVE            TRUE

#define MEASURE_MODE_NORMAL         ((ULONG)0x0)
#define MEASURE_MODE_IMPEDANCE      ((ULONG)0x1)
#define MEASURE_MODE_CALIBRATION    ((ULONG)0x2)
#define MEASURE_MODE_IMPEDANCE_EX   ((ULONG)0x3)
#define MEASURE_MODE_CALIBRATION_EX ((ULONG)0x4)
#define MAXBUFFERSIZE               4*1024*1024
#define TMSIOK  0
#define TMSIERR 1

class TMSiADC : public GenericADC
{
 public:
               TMSiADC();
  virtual      ~TMSiADC();

  void Preflight( const SignalProperties&, SignalProperties& ) const;
  void Initialize();
  void Process( const GenericSignal*, GenericSignal* );
  void Halt();

 private:
  int  WaitForData(ULONG*,ULONG);
  void StartDriver();

 private:
  RTDeviceEx *Device[MAX_DEVICE];
  RTDeviceEx *Master;

  ULONG valuesToRead;
  ULONG Index;
  ULONG BufferSize;
  ULONG NrOfSamples,Total;
  ULONG PercentFull,Overflow;
  ULONG BytesPerSample;
  ULONG BytesReturned;

  ULONG NrOfDevices;

  ULONG SignalBuffer[MAXBUFFERSIZE];

  unsigned int  BufferMulti;
  ULONG         UseMasterSlave( RTDeviceEx **Devices , ULONG Max );

  unsigned int mSoftwareCh,mHardwareCh,
               mSampleBlockSize,
               mSamplingRate;

  ULONG        srate;
};

#endif // TMSiADCH
