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
#include <iomanip>

# include <iostream>

using namespace std;

const char* bciDataExtension = ".dat",
          * bciParameterExtension = ".prm";

RegisterFilter( DataIOFilter, 0 );

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
    "Storage string FileInitials= c:\\data a z "
      "// Initials of file name",
    "Storage string SubjectName= Name Name a z "
      "// subject alias",
    "Storage string SubjectSession= 001 001 0 999 "
      "// session number (max. 3 characters)",
    "Storage string SubjectRun= 00 00 0 99 "
      "// digit run number (max. 2 characters)",
    "Storage string StorageTime= 16:15 Time a z "
      "// time of beginning of data storage",
#if 0 // The value of AutoIncrementRunNo is not read from anywhere.
    "Storage int AutoIncrementRunNo= 1 1 0 1 "
      "// 0: no auto increment 1: auto increment at Initialize)",
#endif
    "Storage int SavePrmFile= 0 1 0 1 "
      "// 0/1: don't save/save additional parameter file",

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
    "Running 1 0 0 0",   // published w/default value of 1 (system is suspended)
    "Recording 1 0 0 0", // published w/default value of 0 (NO recording)
    "SourceTime 16 0 0 0",
    "StimulusTime 16 0 0 0",
  END_STATE_DEFINITIONS
}

DataIOFilter::~DataIOFilter()
{
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
  string baseFileName = ConstructFileName();
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

void DataIOFilter::StartNewRecording()
{
  string baseFileName = ConstructFileName(),
         dataFileName = baseFileName + bciDataExtension;
  mOutputFile.close();
  mOutputFile.clear();
  mOutputFile.open( dataFileName.c_str(), ios::out | ios::binary );

  // Write the header.
  const int lengthDigits = 5;
  mOutputFile << "HeaderLen= ";
  ios::pos_type lengthPos = mOutputFile.tellp();
  mOutputFile << setw( lengthDigits + 1 ) << setfill( ' ' ) << " "
              << "SourceCh= " << ( int )Parameter( "SoftwareCh" ) << " "
              << "StatevectorLen= " << Statevector->GetStateVectorLength()
              << "\r\n"
              << "[ State Vector Definition ] \r\n"
              << *States
              << "[ Parameter Definition ] \r\n"
              << *Parameters
              << "\r\n";
  ios::pos_type dataBegin = mOutputFile.tellp();
  mOutputFile.seekp( lengthPos );
  mOutputFile << dataBegin;
  mOutputFile.seekp( dataBegin );

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
}

void DataIOFilter::Initialize()
{
  mSignalQueue = signalqueue_type();
  StartNewRecording();
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
    // of what we expect for the second signal (the time between completions of
    // the ADC::Process()).
    int roundtripMax = 2000 * Parameter( "SampleBlockSize" ) / Parameter( "SamplingRate" );
    mRoundtripVis.Send( CFGID::MAXVALUE, roundtripMax );
    mRoundtripVis.Send( CFGID::graphType, CFGID::polyline );
    mRoundtripVis.Send( CFGID::showBaselines, true );
  }
}

void DataIOFilter::Process( const GenericSignal* Input,
                                  GenericSignal* Output )
{
  bool visualizeRoundtrip = false;
  if( !mSignalQueue.empty() )
  {
    {
      const GenericSignal& signal = mSignalQueue.front();
      for( size_t j = 0; j < signal.MaxElements(); ++j )
      {
        for( size_t i = 0; i < signal.Channels(); ++i )
        {
          uint16 value = 0;
          if( j < signal.GetNumElements( i ) )
            value = signal( i, j );
          mOutputFile.put( value & 0xff ).put( value >> 8 );
        }
        mOutputFile.write( Statevector->GetStateVectorPtr(), Statevector->GetStateVectorLength() );
      }
      if( !mOutputFile )
        bcierr << "Error writing to file" << endl;
    }
    mSignalQueue.pop();
    visualizeRoundtrip = mVisualizeRoundtrip;
  }

  mADC->Process( Input, Output );
  BCITIME now = BCITIME::GetBCItime_ms();
  if( visualizeRoundtrip )
  {
    BCITIME sourceTime = ( short )State( "SourceTime" ),
            stimulusTime = ( short )State( "StimulusTime" );
    mRoundtripSignal( 0, 0 ) = stimulusTime - sourceTime;
    mRoundtripSignal( 1, 0 ) = now - sourceTime;
    mRoundtripVis.Send( &mRoundtripSignal );
  }
  State( "SourceTime" ) = now;
  mSignalQueue.push( *Output );

  if( mVisualizeEEG )
    mEEGVis.Send( Output );
}

void DataIOFilter::Resting()
{
  mSignalQueue = signalqueue_type();
  mADC->Process( NULL, &mRestingSignal );
  if( mVisualizeEEG )
    mEEGVis.Send( &mRestingSignal );
}

void DataIOFilter::Halt()
{
  mOutputFile.close();
  mOutputFile.clear();
  mSignalQueue = signalqueue_type();
  mADC->Halt();
}

string
DataIOFilter::ConstructFileName() const
{
  BCIDtry bcidtry;
  bcidtry.SetDir( Parameter( "FileInitials" ) );
  bcidtry.SetName( Parameter( "SubjectName" ) );
  ostringstream subjectSession;
  subjectSession << setw( 3 ) << setfill( '0' ) << ( int )Parameter( "SubjectSession" );
  bcidtry.SetSession( subjectSession.str().c_str() );

  string dirName = bcidtry.ProcSubDir(),
         fileName = string( Parameter( "SubjectName" ) ) + "S"
                    + string( Parameter( "SubjectSession" ) ) + "R";
  int existingMaxRun = bcidtry.GetLargestRun( dirName.c_str() ),
      newRun = Parameter( "SubjectRun" );
  if( newRun <= existingMaxRun )
    newRun = existingMaxRun + 1;
  Parameter( "SubjectRun" ) = newRun;

  ostringstream subjectRun;
  subjectRun << setw( 2 ) << setfill( '0' ) << newRun;
  fileName += subjectRun.str();
  return dirName + BCIDtry::DirSeparator + fileName;
}

