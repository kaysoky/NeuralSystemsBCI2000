//////////////////////////////////////////////////////////////////////////////
//
// File: BCIReader.h
//
// Author: Juergen Mellinger
//
// Date: May 29, 2002
//
// Description: An base class for converting a BCI file with purely virtual
//              output functions (e.g., for output into a file, or directly
//              into an application via automation interfaces).
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BCIReaderH
#define BCIReaderH

#include <set>
#include <vector>
#include <string>

struct iless
{
  bool operator()( const std::string& a, const std::string& b )
  { return ::stricmp( a.c_str(), b.c_str() ) < 0; };
};

typedef std::set<std::string, iless> TStrSet;
typedef std::vector<std::string> TStrList;

class GenericSignal;
class STATE;

class TBCIReader
{
  public:
                    TBCIReader()  {}
    virtual         ~TBCIReader() {}
             void   Open( const char* inFileName );
             void   Close();
             void   Process( const TStrList&    inChannelNames,
                             const TStrSet&     inIgnoreStates,
                                   bool         inScanOnly = false );
    const TStrSet&  GetStates() const { return statesInFile; }

  protected:
            void    Idle() const;

  private:
    struct TOutputInfo
    {
        const char*     name;
        unsigned long   numChannels;
        const TStrList* channelNames;
        unsigned long   blockSize;
        unsigned long   numSamples;
        float           samplingRate;
        const TStrList* stateNames;
    };

    virtual void InitOutput( TOutputInfo& ) = 0;
    virtual void ExitOutput() = 0;
    virtual void OutputSignal( const GenericSignal&, long inSamplePos ) = 0;
    virtual void OutputStateValue( const STATE&, short, long inSamplePos ) {}
    virtual void OutputStateChange( const STATE&, short, long inSamplePos ) {}
    virtual void OutputStateRange( const STATE&, short, long inBeginPos, long inEndPos ) {}

    std::string   fileName;
    std::ifstream inputStream;
    TStrSet       statesInFile;
};

#endif // BCIReaderH
