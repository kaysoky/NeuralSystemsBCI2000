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
////////////////////////////////////////////////////////////////////////////////
#ifndef DataIOFilterH
#define DataIOFilterH

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

#include <queue>
#include <fstream>

class DataIOFilter: public GenericFilter
{
 public:
          DataIOFilter();
  virtual ~DataIOFilter();
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const;
  virtual void Initialize();
  virtual void Process( const GenericSignal* Input,
                              GenericSignal* Output );
  virtual void Resting();
  virtual void Halt();

 private:
  void        ResetQueue();
  void        StartNewRecording();

  GenericFilter*                    mADC;
  typedef std::queue<GenericSignal> signalqueue_type;
  signalqueue_type                  mSignalQueue;
  std::string                       mFileName;
  std::ofstream                     mOutputFile;
  bool                              mVisualizeEEG,
                                    mVisualizeRoundtrip;
  GenericVisualization              mEEGVis,
                                    mRoundtripVis;
  GenericSignal                     mRoundtripSignal;
  mutable GenericSignal             mRestingSignal;
};

#endif // DataIOFilterH