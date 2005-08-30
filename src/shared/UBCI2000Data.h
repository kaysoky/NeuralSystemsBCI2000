#ifndef UBCI2000DataH
#define UBCI2000DataH

#include "UParameter.h"
#include "UState.h"
#include "UGenericSignal.h"

#include <vector>
#include <fstream>
#include <string>

enum
{
  BCI2000ERR_NOERR = 0,
  BCI2000ERR_FILENOTFOUND,
  BCI2000ERR_MALFORMEDHEADER,
  BCI2000ERR_NOBUFMEM,
  BCI2000ERR_CHSINCONSISTENT,
};

class BCI2000DATA
{
 public:
  enum { defaultBufSize = 50000, };

  BCI2000DATA();
  ~BCI2000DATA();

  int                Initialize( const char* filename, int buffer_size = defaultBufSize );
  int                SetRun( int );
  int                GetFirstRunNumber() const;
  int                GetLastRunNumber() const;

  int                GetHeaderLength() const      { return mHeaderLength; }
  int                GetStateVectorLength() const { return mStatevectorLength; }
  int                GetNumChannels() const       { return mChannels; }
  float              GetSamplingRate() const      { return mSamplingRate; }
  const std::string& GetFileFormatVersion() const { return mFileFormatVersion; }
  const SignalType&  GetSignalType() const        { return mSignalCache.Type(); }
  unsigned long      GetNumSamples() const        { return mSampleNumber; }
  bool               Initialized() const          { return mInitialized; }

  const PARAMLIST*   GetParamListPtr() const      { return &mParamlist; }
  const STATELIST*   GetStateListPtr() const      { return &mStatelist; }
  const STATEVECTOR* GetStateVectorPtr() const    { return mpStatevector; }

  GenericSignal::value_type
                     Value( int channel, unsigned long sample );
  GenericSignal::value_type
                     ReadValue( int channel, unsigned long sample );
  GenericSignal::value_type
                     ReadValue( int channel, unsigned long sample, int run );
  void               ReadStateVector( unsigned long sample );
  void               ReadStateVector( unsigned long sample, int run );
  // the following are functions that make the underlying file structure transparent to the programmer
  // they all look at the whole dataset as one contigous block, rather than different files
  unsigned long      GetNumSamplesTotal() const   { return mSampleNumberTotal; }
  bool               InitializedTotal() const     { return mInitializedTotal; }
  int                InitializeTotal( const char* new_filename, int buffer_size  = defaultBufSize );

  // Please, use GetSamplingRate() instead.
  int                GetSampleFrequency() const   { return mSamplingRate; }

 private:
  void               Reset();
  void               ResetTotal();

  int                ReadHeader();
  void               CalculateSampleNumber();
  void               CacheSample( unsigned long sample );
  void               ReadSample();

  int                DetermineRunNumber( unsigned long sample );


  PARAMLIST          mParamlist;
  STATELIST          mStatelist;
  STATEVECTOR*       mpStatevector;
  bool               mInitialized,
                     mInitializedTotal;

  std::string        mFilename,
                     mFileFormatVersion;
  std::ifstream      mFile;
  GenericSignal      mSignalCache;
  int                mChannels,
                     mHeaderLength,
                     mStatevectorLength,
                     mDataSize;
  float              mSamplingRate;
  std::vector<float> mSourceOffsets,
                     mSourceGains;
  unsigned long      mCachedSample;
  unsigned char*     mpFileBuffer;

  unsigned long      mSampleNumber;      // samples in this run
  std::vector<unsigned long>
                     mSampleNumberRun;   // samples in a particular run
  unsigned long      mSampleNumberTotal; // samples in this dataset
};

#endif // UBCI2000DataH
