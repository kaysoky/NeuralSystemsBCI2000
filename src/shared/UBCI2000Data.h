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
  const SignalType&  GetSignalType() const        { return mSignalType; }
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
  int                GetSampleFrequency() const   { return static_cast<int>( mSamplingRate ); }

 private:
  void               Reset();
  void               ResetTotal();

  int                ReadHeader();
  void               CalculateSampleNumber();
  const char*        BufferSample( unsigned long sample );

  int                DetermineRunNumber( unsigned long sample );

  static bool                      IsBigEndianMachine();
  static GenericSignal::value_type ReadValueInt16_LittleEndian( const char* );
  static GenericSignal::value_type ReadValueInt32_LittleEndian( const char* );
  static GenericSignal::value_type ReadValueFloat32_LittleEndian( const char* );
  static GenericSignal::value_type ReadValueInt16_BigEndian( const char* );
  static GenericSignal::value_type ReadValueInt32_BigEndian( const char* );
  static GenericSignal::value_type ReadValueFloat32_BigEndian( const char* );

  PARAMLIST          mParamlist;
  STATELIST          mStatelist;
  STATEVECTOR*       mpStatevector;
  bool               mInitialized,
                     mInitializedTotal;

  std::ifstream      mFile;
  std::string        mFilename,
                     mFileFormatVersion;

  SignalType         mSignalType;
  int                mDataSize;
  GenericSignal::value_type ( *mfpReadValueBinary )( const char* );

  int                mChannels,
                     mHeaderLength,
                     mStatevectorLength;
  float              mSamplingRate;
  std::vector<float> mSourceOffsets,
                     mSourceGains;

  unsigned long      mSampleNumber;      // samples in this run
  std::vector<unsigned long>
                     mSampleNumberRun;   // samples in a particular run
  unsigned long      mSampleNumberTotal; // samples in this dataset

  char*              mpBuffer;
  int                mBufferSize,
                     mBufferBegin,
                     mBufferEnd;
};

#endif // UBCI2000DataH
