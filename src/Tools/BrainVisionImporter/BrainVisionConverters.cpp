//////////////////////////////////////////////////////////////////////////////
//
// File: BrainVisionConverters.cpp
//
// Author: Juergen Mellinger
//
// Date: May 29, 2002
//
// Description: Classes for getting BCI files into BrainVision's Analyzer.
//
//////////////////////////////////////////////////////////////////////////////
#ifdef __BORLANDC__
#include <vcl.h>
#define VCL
#pragma hdrstop
#endif // __BORLANDC__

#include "BrainVisionConverters.h"
#include "UGenericSignal.h"
#include "UState.h"

#include <string>
#include <time.h>

#define PROGRAM_INFO "BCI 2000 to BrainVision file converter -- mailto:juergen.mellinger@uni-tuebingen.de"

using namespace std;

string ExtractFileName( const string& inPath )
{
  size_t namePos = inPath.find_last_of( "/\\:" );
  if( namePos == string::npos )
    namePos = 0;
  else
    ++namePos;
  return inPath.substr( namePos );
}

TBrainVisionGDRConverter::TBrainVisionGDRConverter()
: curMarker( 0 )
{
}

TBrainVisionGDRConverter::~TBrainVisionGDRConverter()
{
    ExitOutput();
}

void
TBrainVisionGDRConverter::InitOutput( TOutputInfo& inInfo )
{
    curMarker = 0;

    const string bciExtension = ".dat";
    string  headerFileName = ".vhdr",
            dataFileName = ".vraw",
            markerFileName = ".vmrk",
            baseName( inInfo.name ),
            lowerBaseName( baseName );
    for( string::iterator i = lowerBaseName.begin(); i != lowerBaseName.end(); ++i )
        *i = tolower( *i );
    int lengthDiff = baseName.length() - bciExtension.length();
    if( ( lengthDiff > 0 ) && ( lowerBaseName.substr( lengthDiff ) == bciExtension ) )
        baseName = baseName.substr( 0, lengthDiff );

    headerFileName = baseName + headerFileName;
    dataFileName = baseName + dataFileName;
    markerFileName = baseName + markerFileName;

    headerFile.open( headerFileName.c_str() );
    markerFile.open( markerFileName.c_str() );
    dataFile.open( dataFileName.c_str(), ios_base::out | ios_base::binary );
    bool success = headerFile.is_open() && markerFile.is_open() && dataFile.is_open();
    if( !success )
    {
        bcierr << "Could not open " << baseName << " for writing.\n\n"
               << "Make sure you have write access to the folder containing "
               << "the BCI2000 files." << endl;
        return;
    }

    time_t  now;
    time( &now );
    const char* username = getenv( "USERNAME" );
    if( !username )
      username = getenv( "USER" );
    if( !username )
      username = "<unknown user>";
    const char* hostname = getenv( "COMPUTERNAME" );
    if( !hostname )
      hostname = getenv( "HOSTNAME" );
    if( !hostname )
      hostname = "<unknown host>";
    string userinfo = string( username ) + "@" + hostname;

    headerFile  << "Brain Vision Data Exchange Header File Version 1.0" << endl
                << "; Created " << ctime( &now )
                << "; by " << userinfo << endl
                << "; from " << inInfo.name << endl
                << "; using " << PROGRAM_INFO << endl
                << endl
                << "[Common Infos]" << endl
                << "DataFile=" << ExtractFileName( dataFileName ) << endl
                << "MarkerFile=" << ExtractFileName( markerFileName ) << endl
                << "DataFormat=BINARY" << endl
                << "DataOrientation=MULTIPLEXED" << endl
                << "DataType=TIMEDOMAIN" << endl
                << "NumberOfChannels=" << inInfo.numChannels << endl
                << "SamplingInterval=" << 1e6 / inInfo.samplingRate << endl
                << endl
                << "[Binary Infos]" << endl
                << "BinaryFormat=IEEE_FLOAT_32" << endl
                << endl
                << "[Channel Infos]" << endl;

    for( unsigned long channel = 0; channel < inInfo.numChannels; ++channel )
    {
        headerFile  << "Ch" << channel + 1 << "=";
        headerFile  << ( *inInfo.channelNames )[ channel ];
        headerFile  << "," /* reference Channel name */
                    << ",1" << endl;
    }

    markerFile  << "Brain Vision Data Exchange Marker File Version 1.0" << endl
                << "; Created " << ctime( &now )
                << "; by " << userinfo << endl
                << "; from " << inInfo.name << endl
                << "; using " << PROGRAM_INFO << endl
                << endl
                << "[Common Infos]" << endl
                << "DataFile=" << ExtractFileName( dataFileName ) << endl
                << endl
                << "[Marker Infos]" << endl;

    if( !headerFile )
      bcierr << "Error writing " << headerFileName << endl;
    if( !markerFile )
      bcierr << "Error writing " << markerFileName << endl;
}

void
TBrainVisionGDRConverter::ExitOutput()
{
    dataFile.close();
    headerFile.close();
    markerFile.close();
}

void
TBrainVisionGDRConverter::OutputSignal( const GenericSignal& inSignal, long /*inSamplePos*/ )
{
    Idle();
    for( size_t sample = 0; sample < inSignal.Elements(); ++sample )
      for( size_t channel = 0; channel < inSignal.Channels(); ++channel )
      {
        float value = inSignal( channel, sample );
        dataFile.write( ( const char* )&value, sizeof( value ) );
      }

    if( !dataFile )
      bcierr << "Error writing data file" << endl;
}

void
TBrainVisionGDRConverter::OutputStateChange( const STATE& inState, short inValue, long inSamplePos )
{
    Idle();
#if 0
    ++curMarker;
    markerFile  << "Mk" << curMarker << "="
                << "BCIStateChange,";
    markerFile  << inState.GetName()
                << " " << inValue << ",";
    markerFile  << inSamplePos << ","
                << 1 << ","
                << 0 << '\n';
#endif
}

void
TBrainVisionGDRConverter::OutputStateRange( const STATE& inState, short inValue, long inBeginPos, long inEndPos )
{
    Idle();

    if( ( inEndPos < inBeginPos ) || ( inBeginPos < 0 ) )
    {
      bcierr << "Bad file format" << endl;
      return;
    }

    ++curMarker;
    markerFile  << "Mk" << curMarker << "="
                << "BCIState,";

    if( inState.GetLength() > 1 )
        markerFile  << inState.GetName()
                    << " " << inValue << ",";
    else
        markerFile  << inState.GetName() << ",";

    markerFile  << inBeginPos << ","
                << inEndPos - inBeginPos << ","
                << 0 << '\n';
}
