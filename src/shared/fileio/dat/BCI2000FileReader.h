////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: Class that provides an interface to the data stored in a
//              BCI2000 data file.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef BCI2000_FILE_READER_H
#define BCI2000_FILE_READER_H

#include "Param.h"
#include "ParamList.h"
#include "ParamRef.h"
#include "State.h"
#include "StateList.h"
#include "StateVector.h"
#include "StateRef.h"
#include "GenericSignal.h"

#include <vector>
#include <fstream>
#include <string>

class BCI2000FileReader
{
 public:
  static const int cDefaultBufSize = 50 * 1024;

  enum
  {
    NoError = 0,
    FileOpenError,
    MalformedHeader,
    
    NumErrors
  };

 public:
  BCI2000FileReader();
  explicit BCI2000FileReader( const char* fileName );
  ~BCI2000FileReader();

 private:
  BCI2000FileReader( const BCI2000FileReader& );
  BCI2000FileReader& operator=( const BCI2000FileReader& );

 public:
  // State
  virtual int   ErrorState() const
                { return mErrorState; }
  virtual bool  IsOpen() const
                { return mInitialized; }

  // File access
  virtual BCI2000FileReader&
                Open( const char* fileName, int bufferSize = cDefaultBufSize );
  virtual long  NumSamples() const
                { return mNumSamples; }
  float SamplingRate() const
        { return mSamplingRate; }
  const class SignalProperties&
        SignalProperties() const
        { return mSignalProperties; }
  const std::string&
        FileFormatVersion() const
        { return mFileFormatVersion; }

  // Parameter/State access
  //  Accessor functions consistent with the Environment class interface.
  const ParamList*   Parameters() const
                     { return &mParamlist; }
  const StateList*   States() const
                     { return &mStatelist; }
  const class StateVector* StateVector() const
                     { return mpStatevector; }
  const ParamRef     Parameter( const std::string& name ) const;
  const StateRef     State( const std::string& name ) const;

  int   HeaderLength() const
        { return mHeaderLength; }
  int   StateVectorLength() const
        { return mStatevectorLength; }

  // Data access
  virtual GenericSignal::ValueType
        RawValue( int channel, long sample );
  GenericSignal::ValueType
        CalibratedValue( int channel, long sample );
  virtual BCI2000FileReader&
        ReadStateVector( long sample );

 protected:
  void               Reset();

 private:
  void               ReadHeader();
  void               CalculateNumSamples();
  const char*        BufferSample( long sample );

 private:
  ParamList          mParamlist;
  StateList          mStatelist;
  class StateVector* mpStatevector;
  bool               mInitialized;

  std::ifstream      mFile;
  std::string        mFilename,
                     mFileFormatVersion;

  ::SignalProperties mSignalProperties;
  SignalType         mSignalType;
  int                mDataSize;

  int                mChannels,
                     mHeaderLength,
                     mStatevectorLength;
  float              mSamplingRate;
  std::vector<GenericSignal::ValueType> mSourceOffsets,
                                        mSourceGains;

  unsigned long      mNumSamples;

  char*              mpBuffer;
  int                mBufferSize,
                     mBufferBegin,
                     mBufferEnd;

  int                mErrorState;
};

#endif // BCI2000_FILE_READER_H
