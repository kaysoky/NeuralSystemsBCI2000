////////////////////////////////////////////////////////////////////////////////
//
// File: DataIOFilter.h
//
// Date: Nov 11, 2003
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A filter that handles data acquisition from a GenericADC
//              and storing into a file.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef DataIOFilterH
#define DataIOFilterH

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

class GenericFileWriter;

class DataIOFilter: public GenericFilter
{
 public:
          DataIOFilter();
  virtual ~DataIOFilter();
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const;
  virtual void Initialize2( const SignalProperties& Input,
                            const SignalProperties& Output );
  virtual void StartRun();
  virtual void StopRun();

  virtual void Process( const GenericSignal* Input,
                              GenericSignal* Output );
  virtual void Resting();
  virtual void Halt();

 private:
  GenericFilter*         mpADC,
               *         mpSourceFilter;
  GenericFileWriter*     mpFileWriter;
  GenericSignal          mSignalBuffer;
  STATEVECTOR            mStatevectorBuffer;

  bool                   mVisualizeEEG,
                         mVisualizeRoundtrip;
  int                    mVisualizeSourceDecimation;
  GenericVisualization   mEEGVis,
                         mRoundtripVis;
  GenericSignal          mDecimatedSignal,
                         mRoundtripSignal,
                         mRestingSignal;
  std::vector<float>     mSourceChOffset,
                         mSourceChGain;
};

#endif // DataIOFilterH
