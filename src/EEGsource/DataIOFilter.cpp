////////////////////////////////////////////////////////////////////////////////
//
// File: DataIOFilter.cpp
//
// Date: Nov 11, 2003
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A filter that handles data acquisition from a GenericADC
//              and storing into a file.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "DataIOFilter.h"

#include "defines.h"
#include "GenericADC.h"
#include "UBCIError.h"
#include "BCIDirectry.h"
#include "UBCItime.h"

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

RegisterFilter( DataIOFilter, 0 );


static const char* bciDataExtension = ".dat",
                 * bciParameterExtension = ".prm";


DataIOFilter::DataIOFilter()
: mADC( GenericFilter::PassFilter<GenericADC>() ),
  mVisualizeEEG( false ),
  mVisualizeRoundtrip( false ),
  mEEGVis( SOURCEID::EEGDISP, VISTYPE::GRAPH ),
  mRoundtripVis( SOURCEID::ROUNDTRIP, VISTYPE::GRAPH ),
  mRoundtripSignal( 2, 1 )
{
  // The ADC filter must have a position string less than that of the
  // DataIOFilter.
  if( !mADC )
    bcierr << "Expected an ADC filter instance to be present" << endl;

  BEGIN_PARAMETER_DEFINITIONS
    // Parameters required to interpret a data file are listed here
    // to enforce their presence:
    "Source int SoftwareCh= 16 "
       "16 1 128 // the number of digitized and stored channels",
    "Source int SampleBlockSize= 32 "
       "5 1 128 // the number of samples transmitted at a time",
    "Source int SamplingRate= 256 "
       "128 1 4000 // the sample rate",
    "Filtering floatlist SourceChOffset= 16 "
      "0 0 0 0 "
      "0 0 0 0 "
      "0 0 0 0 "
      "0 0 0 0 "
      "0 -500 500 // offset for channels in A/D units",
    "Filtering floatlist SourceChGain= 16 "
      "0.003 0.003 0.003 0.003 "
      "0.003 0.003 0.003 0.003 "
      "0.003 0.003 0.003 0.003 "
      "0.003 0.003 0.003 0.003 "
      "0.003 -500 500 // gain for each channel (A/D units -> muV)",
    "Filtering floatlist SourceChTimeOffset= 1 "
      "-1 "
      "0 0 1 // time offset for all source channels (not used if -1)",

    // Storage related parameters:
    "Storage string FileInitials= c:\\data a z "
      "// path to top level data directory",
    "Storage string SubjectName= Name Name a z "
      "// subject alias",
    "Storage string SubjectSession= 001 001 0 0 "
      "// three-digit session number",
    "Storage string SubjectRun= 00 00 0 0 "
      "// two-digit run number",
    "Storage string StorageTime= 16:15 Time a z "
      "// time of beginning of data storage",
    "Storage int SavePrmFile= 0 1 0 1 "
      "// 0/1: don't save/save additional parameter file",

    // Visualization of data as far as managed by the DataIOFilter:
    "Visualize int VisualizeRoundtrip= 1 1 0 1 "
      "// visualize roundtrip time (0=no, 1=yes)",
    "Visualize int VisualizeSource= 1 1 0 1 "
      "// visualize raw brain signal (0=no, 1=yes)",
    "Visualize int VisualizeSourceDecimation= 1 1 0 1 "
      "// decimation factor for raw brain signal",
    "Visualize int VisualizeSourceTime= 2 2 0 5 "
      "// how much time in Source visualization",
    "Visualize int SourceMin= 0 0 -8092 0 "
      "// raw signal vis Min Value",
    "Visualize int SourceMax= 4096 4096 0 16386 "
      "// raw signal vis Max Value",
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
    "Running 1 0 0 0",
    "Recording 1 0 0 0",
    "SourceTime 16 0 0 0",
    "StimulusTime 16 0 0 0",
  END_STATE_DEFINITIONS
}


DataIOFilter::~DataIOFilter()
{
  Halt();
  delete mADC;
}


void DataIOFilter::Preflight( const SignalProperties& Input,
                                    SignalProperties& Output ) const
{
  // Parameter existence and range.
  PreflightCondition( Parameter( "SamplingRate" ) > 0 );
  PreflightCondition( Parameter( "SampleBlockSize" ) > 0 );
  PreflightCondition( Parameter( "VisualizeSourceDecimation" ) > 0 );

  // File accessibility.
  string baseFileName = BCIDirectory()
    .SubjectDirectory( Parameter( "FileInitials" ) )
    .SubjectName( Parameter( "SubjectName" ) )
    .SessionNumber( Parameter( "SubjectSession" ) )
    .RunNumber( Parameter( "SubjectRun" ) )
    .CreatePath()
    .FilePath();

  {
    string dataFileName = baseFileName + bciDataExtension;

    // Does the data file exist?
    ifstream dataRead( dataFileName.c_str() );
    if( dataRead.is_open() )
      bcierr << "Data file " << dataFileName << " already exists, "
             << "will not be touched." << endl;
    else
    {
      // It does not exist, can we write to it?
      ofstream dataWrite( dataFileName.c_str() );
      if( !dataWrite.is_open() )
        bcierr << "Cannot write to file " << dataFileName << endl;
      else
      {
        dataWrite.close();
        ::remove( dataFileName.c_str() );
      }
    }
  }
  if( Parameter( "SavePrmFile" ) == 1 )
  {
    string paramFileName =  baseFileName + bciParameterExtension;
    ifstream paramRead( paramFileName.c_str() );
    if( paramRead.is_open() )
      bcierr << "Parameter file " << paramFileName << " already exists, "
             << "will not be touched." << endl;
    else
    {
      ofstream paramWrite( paramFileName.c_str() );
      if( !paramWrite.is_open() )
        bcierr << "Cannot write to file " << paramFileName << endl;
      else
      {
        paramWrite.close();
        ::remove( paramFileName.c_str() );
      }
    }
  }

  // Sub-filter preflight/signal properties.
  mADC->Preflight( Input, Output );
  mRestingSignal.SetProperties( Output );

  // Signal properties.
  if( Input.Channels() > 0 )
    bcierr << "Expected empty input signal" << endl;
  if( Output.GetDepth() != 2 )
    bcierr << "Expected short integer signal in ADC output" << endl;
}


void DataIOFilter::Initialize()
{
  mOutputFile.close();
  mOutputFile.clear();
  State( "Recording" ) = 0;
  mStatevectorBuffer.resize( 0 );
  mSignalBuffer = GenericSignal( 0, 0 );
  mADC->Initialize();

  // Configure visualizations.
  mVisualizeEEG = ( Parameter( "VisualizeSource" ) == 1 );
  if( mVisualizeEEG )
  {
    mEEGVis.Send( CFGID::WINDOWTITLE, "Source Signal" );
    int numSamplesInDisplay = ( Parameter( "VisualizeSourceTime" ) * Parameter( "SamplingRate" ) )
                               / Parameter( "VisualizeSourceDecimation" );
    mEEGVis.Send( CFGID::NUMSAMPLES, numSamplesInDisplay );
    mEEGVis.Send( CFGID::MINVALUE, ( const char* )Parameter( "SourceMin" ) );
    mEEGVis.Send( CFGID::MAXVALUE, ( const char* )Parameter( "SourceMax" ) );
  }

  mVisualizeRoundtrip = ( Parameter( "VisualizeRoundtrip" ) == 1 );
  if( mVisualizeRoundtrip )
  {
    mRoundtripVis.Send( CFGID::WINDOWTITLE, "Roundtrip" );
    mRoundtripVis.Send( CFGID::NUMSAMPLES, 128 );
    mRoundtripVis.Send( CFGID::MINVALUE, 0 );
    // Roundtrip values are in ms, and we want a range that is twice the value
    // of what we expect for the second signal (the time between subsequent
    // completions of the ADC's Process()).
    int roundtripMax = 2000 * Parameter( "SampleBlockSize" ) / Parameter( "SamplingRate" );
    mRoundtripVis.Send( CFGID::MAXVALUE, roundtripMax );
    mRoundtripVis.Send( CFGID::graphType, CFGID::polyline );
    mRoundtripVis.Send( CFGID::showBaselines, true );
  }
}


void DataIOFilter::StartRun()
{
  string baseFileName = BCIDirectory()
                        .SubjectDirectory( Parameter( "FileInitials" ) )
                        .SubjectName( Parameter( "SubjectName" ) )
                        .SessionNumber( Parameter( "SubjectSession" ) )
                        .RunNumber( Parameter( "SubjectRun" ) )
                        .FilePath(),
         dataFileName = baseFileName + bciDataExtension;
  mOutputFile.close();
  mOutputFile.clear();
  mOutputFile.open( dataFileName.c_str(), ios::out | ios::binary );

  // Write the header.
  //
  // Because the header contains its own length in ASCII format, it is a bit
  // tricky to get this right if we don't want to imply a fixed width for the
  // HeaderLen field.
  ostringstream header;
  header << " "
         << "SourceCh= " << ( int )Parameter( "SoftwareCh" ) << " "
         << "StatevectorLen= " << Statevector->GetStateVectorLength()
         << "\r\n"
         << "[ State Vector Definition ] \r\n";
  States->WriteBinary( header );
  header << "[ Parameter Definition ] \r\n";
  Parameters->WriteBinary( header );
  header << "\r\n";

  const string headerBegin = "HeaderLen= ";
  size_t fieldLength = 5; // Follow the old scheme
                          // (5 characters for the header length field),
                          // but allow for a longer HeaderLen field
                          // if necessary.
  ostringstream headerLengthField;
  do
  { // This trial-and-error scheme is invariant against changes in number
    // formatting and character set (unicode, hex, ...).
    size_t headerLength = headerBegin.length() + fieldLength + header.str().length();
    headerLengthField.str( "" );
    headerLengthField << headerBegin
                      << setfill( ' ' ) << setw( fieldLength )
                      << headerLength;
  } while( headerLengthField
           && ( headerLengthField.str().length() - headerBegin.length() != fieldLength++ ) );

  mOutputFile.write( headerLengthField.str().data(), headerLengthField.str().size() );
  mOutputFile.write( header.str().data(), header.str().size() );

  if( !mOutputFile )
    bcierr << "Error writing to file " << dataFileName << endl;

  if( Parameter( "SavePrmFile" ) == 1 )
  {
    string paramFileName =  baseFileName + bciParameterExtension;
    ofstream file( paramFileName.c_str() );
    if( !( file << *Parameters << flush ) )
      bcierr << "Error writing parameters to file "
             << paramFileName
             << endl;
  }

  mADC->StartRun();

  // Initialize time stamps with the current time to get a correct roundtrip
  // time, and a zero stimulus delay, for the first block.
  BCITIME now = BCITIME::GetBCItime_ms();
  State( "SourceTime" ) = now;
  State( "StimulusTime" ) = now;
  State( "Recording" ) = 1;
}


void DataIOFilter::StopRun()
{
  mOutputFile.close();
  mOutputFile.clear();
  mADC->StopRun();
  mSignalBuffer = GenericSignal( 0, 0 );
  State( "Recording" ) = 0;
}

void DataIOFilter::Process( const GenericSignal* Input,
                                  GenericSignal* Output )
{
  // Moving the save-to-file code to the beginning of Process() implies
  // that the time spent on i/o operations will only reduce the
  // time spent waiting for A/D data, and thus not enter into the
  // roundtrip time.
  // In between, the signal is written to a queue which should never contain
  // more than one element.
  // The BCI2000 standard requires that the state vector saved with a data block
  // is the one that existed when the data came out of the ADC.
  // So we also need to buffer the state vector between calls to Process().
  bool visualizeRoundtrip = false;
  if( mSignalBuffer > SignalProperties( 0, 0 ) )
  {
    for( size_t j = 0; j < mSignalBuffer.MaxElements(); ++j )
    {
      for( size_t i = 0; i < mSignalBuffer.Channels(); ++i )
      {
        uint16 value = 0;
        if( j < mSignalBuffer.GetNumElements( i ) )
          value = mSignalBuffer( i, j );
        mOutputFile.put( value & 0xff ).put( value >> 8 );
      }
      mOutputFile.write( mStatevectorBuffer.data(), mStatevectorBuffer.size() );
    }
    if( !mOutputFile )
      bcierr << "Error writing to file" << endl;
    State( "Recording" ) = ( mOutputFile ? 1 : 0 );

    visualizeRoundtrip = mVisualizeRoundtrip;
  }

  mADC->Process( Input, Output );
  BCITIME now = BCITIME::GetBCItime_ms();
  if( visualizeRoundtrip )
  {
    BCITIME sourceTime = static_cast<short>( State( "SourceTime" ) ),
            stimulusTime = static_cast<short>( State( "StimulusTime" ) );
    mRoundtripSignal( 0, 0 ) = stimulusTime - sourceTime;
    mRoundtripSignal( 1, 0 ) = now - sourceTime;
    mRoundtripVis.Send( &mRoundtripSignal );
  }
  State( "SourceTime" ) = now;
  mStatevectorBuffer = string(
    reinterpret_cast<const char*>( Statevector->GetStateVectorPtr() ),
    Statevector->GetStateVectorLength() );
  mSignalBuffer = *Output;

  if( mVisualizeEEG )
    mEEGVis.Send( Output );
}


void DataIOFilter::Resting()
{
  mADC->Process( NULL, &mRestingSignal );
  if( mVisualizeEEG )
    mEEGVis.Send( &mRestingSignal );
}


void DataIOFilter::Halt()
{
  mOutputFile.close();
  mOutputFile.clear();
  mSignalBuffer = GenericSignal( 0, 0 );
  mADC->Halt();
}

