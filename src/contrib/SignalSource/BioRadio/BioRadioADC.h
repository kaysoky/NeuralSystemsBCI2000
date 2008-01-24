////////////////////////////////////////////////////////////////////////////////
// $Id$
// $Log$
// Revision 1.2  2006/07/05 15:21:19  mellinger
// Formatting and naming changes.
//
// Revision 1.1  2006/07/04 18:44:25  mellinger
// Put files into CVS.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
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
  virtual       ~BioRadioADC();

  virtual void  Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void  Initialize( const SignalProperties&, const SignalProperties& );
  virtual void  Process( const GenericSignal&, GenericSignal& );
  virtual void  Halt();

 private:
          void  GetData(int channel);
          void  ClearSampleIndices();

 private:
  int           mSamplerate;
  int           mSourceCh;
  int           mSampleBlockSize;
  int           mVoltageRange;
  const double* mpIndex;
  BR150         mBioRadio150;
  int           mDataRead;
  int           mTracker;
  int           mSampleIndex[ALL_CHANNELS];
  int           mComPort;
  std::string   mFileLocation;
 };

#endif // BioRadioADCH

