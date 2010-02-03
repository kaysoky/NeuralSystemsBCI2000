//////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An base class for converting a BCI2000 file into different
//   formats. Output formats are represented as descendants implementing
//   BCIReader's purely virtual output functions.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////
#ifndef BCI_READER_H
#define BCI_READER_H

#include <set>
#include <vector>
#include <string>

#include "BCI2000FileReader.h"

struct iless
{
  bool operator()( const std::string& a, const std::string& b ) const
  { return ::stricmp( a.c_str(), b.c_str() ) < 0; };
};

typedef std::set<std::string, iless> StringSet;
typedef std::vector<std::string>     StringList;

class BCIReader
{
 public:
  BCIReader()
    {}
  virtual ~BCIReader()
    {}
  void Open( const char* inFileName );
  void Close();
  void Process( const StringList& inChannelNames,
                const StringSet&  inIgnoreStates,
                bool              inScanOnly = false );
  const StringSet& GetStates() const
    { return mStatesInFile; }

 protected:
  void Idle() const;

  struct OutputInfo
  {
          const char*       name;
          unsigned long     numChannels;
          const StringList* channelNames;
          unsigned long     blockSize;
          unsigned long     numSamples;
          float             samplingRate;
          const StringList* stateNames;
  };

  virtual void InitOutput( OutputInfo& ) = 0;
  virtual void ExitOutput() = 0;
  virtual void OutputSignal( const GenericSignal&, long inSamplePos ) = 0;
  virtual void OutputStateValue( const State&, short, long inSamplePos ) {}
  virtual void OutputStateChange( const State&, short, long inSamplePos ) {}
  virtual void OutputStateRange( const State&, short, long inBeginPos, long inEndPos ) {}

 private:
  std::string       mFileName;
  BCI2000FileReader mInputData;
  StringSet         mStatesInFile;
};

#endif // BCI_READER_H

