////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: Class that provides an interface to the data stored in BCI2000
//   data files belonging to a session.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef BCI2000_SESSION_READER_H
#define BCI2000_SESSION_READER_H

#include "BCI2000FileReader.h"

class BCI2000SessionReader: public BCI2000FileReader
{
 public:
  static const int cDefaultBufSize = BCI2000FileReader::cDefaultBufSize;
  enum
  {
    InconsistentFiles = BCI2000FileReader::NumErrors,

    NumErrors
  };

 public:
  BCI2000SessionReader();
  explicit BCI2000SessionReader( const char* fileName );
  ~BCI2000SessionReader();

 private:
  BCI2000SessionReader( const BCI2000SessionReader& );
  BCI2000SessionReader& operator=( const BCI2000SessionReader& );

 public:
  // State
  virtual int   ErrorState() const;
  virtual bool  IsOpen() const
                { return mInitialized; }

  // File access
  virtual BCI2000SessionReader&
                Open( const char* fileName, int bufferSize = cDefaultBufSize );
  virtual long  NumSamples() const
                { return mNumSamples.empty() ? 0 : *mNumSamples.rbegin(); }

  // Data access
  virtual GenericSignal::ValueType
               RawValue( int channel, long sample );
  virtual BCI2000SessionReader&
               ReadStateVector( long sample );

 private:
  void Reset();
  long SetSampleRun( long sample );

 private:
  bool               mInitialized;
  int                mBufSize;
  std::vector<std::string> mRunNames;
  std::vector<long>  mNumSamples;  // samples in a particular run
  int                mCurRun;
  int                mErrorState;
};

#endif // BCI2000_SESSION_READER_H
