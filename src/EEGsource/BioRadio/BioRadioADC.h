////////////////////////////////////////////////////////////////////////////////
// $Id$
// $Log$
// Revision 1.1  2006/07/04 18:44:25  mellinger
// Put files into CVS.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef BioRadioADCH
#define BioRadioADCH

#include "GenericADC.h"
#include "BR_defines.h"
#include "BR150.h"

class BioRadioADC : public GenericADC
{
 public:
               BioRadioADC();
  virtual      ~BioRadioADC();

  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process( const GenericSignal*, GenericSignal* );
  virtual void Halt();

 private:
          void GetData(int channel);
          char*PortTest(int numPort, BR150 radio);
          void ClearSampleIndices();

 private:
  int         mSamplerate;
  int         mSoftwareCh;
  int         mSampleBlockSize;
  int         mVoltageRange;
  double*     mpIndex;
  BR150       mBioRadio150;
  int         mDataRead;
  int         mTracker;
  int         mSampleIndex[ALL_CHANNELS];
  int         mComPort;
  std::string mFileLocation;

 };

#endif // BioRadioADCH

