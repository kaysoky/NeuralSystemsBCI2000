//////////////////////////////////////////////////////////////////////////////
//
// File: BrainVisionConverters.h
//
// Author: Juergen Mellinger
//
// Date: May 29, 2002
//
// Description: Classes for getting BCI files into BrainVision's Analyzer.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BrainVisionConvertersH
#define BrainVisionConvertersH

#include <fstream>
#include <string>
#include "BCIReader.h"

std::string ExtractFileName( const std::string& );

// This one uses BrainVision's Generic Data Reader.
class TBrainVisionGDRConverter : public TBCIReader
{
  public:
                  TBrainVisionGDRConverter();
    virtual       ~TBrainVisionGDRConverter();

  private:
    virtual void  InitOutput( TOutputInfo& );
    virtual void  ExitOutput();
    virtual void  OutputSignal( const GenericSignal&, long inSamplePos );
    virtual void  OutputStateChange( const STATE&, short, long inSamplePos );
    virtual void  OutputStateRange( const STATE&, short, long inBeginPos, long inEndPos );

    long          curMarker;
    std::ofstream dataFile,
                  headerFile,
                  markerFile;
};

// This one uses the BrainVision Analyzer's automation interface.
class TBrainVisionAutoConverter : public TBCIReader
{
  public:
                 TBrainVisionAutoConverter();
    virtual      ~TBrainVisionAutoConverter();

  private:
    virtual void InitOutput( TOutputInfo& );
    virtual void ExitOutput();
    virtual void OutputSignal( const GenericSignal&, long inSamplePos );
    virtual void OutputStateChange( const STATE&, short, long inSamplePos );
    virtual void OutputStateRange( const STATE&, short, long inBeginPos, long inEndPos );
};



#endif // BrainVisionConvertersH


