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
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "DataIOFilter.h"

#include "defines.h"
#include "GenericADC.h"
#include "GenericFileWriter.h"
#include "UBCIError.h"
#include "BCIDirectry.h"
#include "UBCItime.h"
#include "MeasurementUnits.h"
#include "ClassName.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <set>

using namespace std;
using namespace bci;

RegisterFilter( DataIOFilter, 0 );


DataIOFilter::DataIOFilter()
: mpADC( GenericFilter::PassFilter<GenericADC>() ),
  mpSourceFilter( NULL ),
  mpFileWriter( NULL ),
  mStatevectorBuffer( *States, true ),
  mVisualizeEEG( false ),
  mVisualizeSourceDecimation( 1 ),
  mVisualizeRoundtrip( false ),
  mEEGVis( SOURCEID::EEGDISP, VISTYPE::GRAPH ),
  mRoundtripVis( SOURCEID::ROUNDTRIP, VISTYPE::GRAPH ),
  mRoundtripSignal( 2, 1 )
{
  BEGIN_PARAMETER_DEFINITIONS
    // Parameters required to interpret a data file are listed here
    // to enforce their presence:
    "Source int SoftwareCh= 16 "
       "16 1 128 // the number of digitized and stored channels",
    "Source int SampleBlockSize= 32 "
       "5 1 128 // the number of samples transmitted at a time",
    "Source int SamplingRate= 256 "
       "128 1 4000 // the sample rate",
    "Source list ChannelNames= 0 "
       "% % % // list of channel names",
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
    "Storage string FileInitials= ..\\data a z "
      "// path to top level data directory (directory)",
    "Storage string SubjectName= Name Name a z "
      "// subject alias",
    "Storage string SubjectSession= 001 001 0 0 "
      "// three-digit session number",
    "Storage string SubjectRun= 00 00 0 0 "
      "// two-digit run number",

    // Visualization of data as far as managed by the DataIOFilter:
    "Visualize int VisualizeRoundtrip= 1 1 0 1 "
      "// visualize roundtrip time (0=no, 1=yes) (boolean)",
    "Visualize int VisualizeSource= 1 1 0 1 "
      "// visualize raw brain signal (0=no, 1=yes) (boolean)",
    "Visualize int VisualizeSourceDecimation= 1 1 0 1 "
      "// decimation factor for raw brain signal",
    "Visualize int VisualizeSourceTime= 2 2 0 5 "
      "// how much time in Source visualization",
    "Visualize int SourceMin= -100muV -100muV 0 0 "
      "// raw signal vis Min Value",
    "Visualize int SourceMax= 100muV 100muV 0 0 "
      "// raw signal vis Max Value",
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
    "Running 1 0 0 0",
    "Recording 1 1 0 0", // "Recording" must have an initial value of 1 to avoid
                         // it being set to zero by the other modules' initialization
                         // code.
    "SourceTime 16 0 0 0",
    "StimulusTime 16 0 0 0",
  END_STATE_DEFINITIONS

  // Find available GenericFileWriter descendants and determine which one to use.
  typedef set<GenericFileWriter*> writerSet;
  set<GenericFileWriter*> availableFileWriters;
  for( GenericFileWriter* p = GenericFilter::PassFilter<GenericFileWriter>();
       p != NULL;
       p = GenericFilter::PassFilter<GenericFileWriter>() )
    availableFileWriters.insert( p );

  if( !availableFileWriters.empty() )
  {
    string fileFormat = "BCIDat";
    if( Parameters->Exists( "FileFormat" ) )
      fileFormat = string( Parameter( "FileFormat" ) );

    string writerName = fileFormat + "FileWriter";
    for( writerSet::const_iterator i = availableFileWriters.begin();
         mpFileWriter == NULL && i != availableFileWriters.end();
         ++i )
      if( writerName == ClassName( typeid( **i ) ) )
        mpFileWriter = *i;
        
    if( mpFileWriter == NULL )
      bcierr << "Could not identify writer component for file format "
             << "\"" << fileFormat << "\""
             << endl;

    availableFileWriters.erase( mpFileWriter );
    for( writerSet::const_iterator i = availableFileWriters.begin();
         i != availableFileWriters.end();
         ++i )
      delete *i;
  }
  if( mpFileWriter != NULL )
    mpFileWriter->Publish();

  // Check whether the next filter in the chain is a notch filter.
  GenericFilter* filter = GenericFilter::GetFilter<GenericFilter>();
  if( string( "NotchFilter" ) == ClassName( typeid( *filter ) ) )
    mpSourceFilter = GenericFilter::PassFilter<GenericFilter>();
}


DataIOFilter::~DataIOFilter()
{
  Halt();
  delete mpADC;
  delete mpSourceFilter;
  delete mpFileWriter;
}


void
DataIOFilter::Preflight( const SignalProperties& Input,
                               SignalProperties& Output ) const
{
  // Parameter existence and range.
  PreflightCondition( Parameter( "SamplingRate" ) > 0 );
  int SampleBlockSize = Parameter( "SampleBlockSize" ),
      VisualizeSourceDecimation = Parameter( "VisualizeSourceDecimation" );
  PreflightCondition( SampleBlockSize > 0 );
  PreflightCondition( VisualizeSourceDecimation > 0 );
  if( SampleBlockSize % VisualizeSourceDecimation != 0 )
    bcierr << "The VisualizationSourceDecimation parameter "
           << "(now " << VisualizeSourceDecimation << ") "
           << "must be a divider of the sample block size "
           << "(now " << SampleBlockSize << ")"
           << endl;

  // Sub-filter preflight/signal properties.
  // The ADC and file writer filters must have a position string greater than
  // that of the DataIOFilter.
  if( !mpADC )
    bcierr << "Expected an ADC filter instance to be present" << endl;
  else
    mpADC->Preflight( Input, Output );

  if( mpSourceFilter )
  {
    SignalProperties sourceFilterInput( Output );
    mpSourceFilter->Preflight( sourceFilterInput, Output );
    if( Output != sourceFilterInput )
      bcierr << ClassName( typeid( *mpSourceFilter ) )
             << " input and output signal properties must match"
             << " when used as a source filter"
             << endl;
  }

  if( !mpFileWriter )
    bcierr << "Expected a file writer filter instance to be present" << endl;
  else
  {
    SignalProperties writerOutput;
    mpFileWriter->Preflight( Output, writerOutput );
    if( !writerOutput.IsEmpty() )
      bcierr << "Expected empty output signal from file writer" << endl;
  }
  // Signal properties.
  if( !Input.IsEmpty() )
    bcierr << "Expected empty input signal" << endl;
}


void
DataIOFilter::Initialize2( const SignalProperties& inputProperties,
                           const SignalProperties& outputProperties )
{
  State( "Recording" ) = 0;
  mSignalBuffer = GenericSignal( 0, 0 );
  mRestingSignal.SetProperties( outputProperties );
  mpADC->Initialize2( inputProperties, outputProperties );
  if( mpSourceFilter )
  {
    mpSourceFilter->Initialize2( outputProperties, outputProperties );
    mpSourceFilter->StartRun();
  }
  mpFileWriter->Initialize2( outputProperties, SignalProperties( 0, 0 ) );

  // Configure visualizations.
  mVisualizeEEG = ( Parameter( "VisualizeSource" ) == 1 );
  mVisualizeSourceDecimation = Parameter( "VisualizeSourceDecimation" );
  mSourceChOffset.resize( mRestingSignal.Channels(), 0.0 );
  mSourceChGain.resize( mRestingSignal.Channels(), 1.0 );
  for( size_t channel = 0; channel < mRestingSignal.Channels(); ++channel )
  {
    mSourceChOffset[ channel ] = OptionalParameter( 0, "SourceChOffset", channel );
    mSourceChGain[ channel ] = OptionalParameter( 1, "SourceChGain", channel );
  }
  if( mVisualizeEEG )
  {
    mEEGVis.Send( CFGID::WINDOWTITLE, "Source Signal" );
    int numSamplesInDisplay = ( Parameter( "VisualizeSourceTime" ) * Parameter( "SamplingRate" ) )
                               / mVisualizeSourceDecimation;
    mEEGVis.Send( CFGID::NUMSAMPLES, numSamplesInDisplay );
    ostringstream oss;
    oss << ( 1.0 / Parameter( "SamplingRate" ) * mVisualizeSourceDecimation ) << "s";
    mEEGVis.Send( CFGID::sampleUnit, oss.str() );
    // This is a hack to keep compatibility with old parameter files; it will
    // not treat individual offset/gain settings for different channels properly.
    // Ideally, the offset/gain parameters should not be used here, and the parameter
    // values should be treated as muV regardless of whether this is stated
    // explicitly or not.
    float minValue = MeasurementUnits::ReadAsVoltage( Parameter( "SourceMin" ) ),
          maxValue = MeasurementUnits::ReadAsVoltage( Parameter( "SourceMax" ) );
    mEEGVis.Send( CFGID::MINVALUE, ( minValue - mSourceChOffset[ 0 ] ) * mSourceChGain[ 0 ] );
    mEEGVis.Send( CFGID::MAXVALUE, ( maxValue - mSourceChOffset[ 0 ] ) * mSourceChGain[ 0 ] );
    mEEGVis.Send( CFGID::valueUnit, "1muV" );
  }
  mDecimatedSignal.SetProperties( SignalProperties(
    mRestingSignal.Channels(),
    mRestingSignal.Elements() / mVisualizeSourceDecimation,
    SignalType::float24
  ) );

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
    mRoundtripVis.Send( CFGID::valueUnit, "1ms" );
  }
}


void
DataIOFilter::StartRun()
{
  mpADC->StartRun();
  mpFileWriter->StartRun();

  // Initialize time stamps with the current time to get a correct roundtrip
  // time, and a zero stimulus delay, for the first block.
  BCITIME now = BCITIME::GetBCItime_ms();
  State( "SourceTime" ) = now;
  State( "StimulusTime" ) = now;
  State( "Recording" ) = 1;
}


void
DataIOFilter::StopRun()
{
  mpADC->StopRun();
  mpFileWriter->StopRun();
  mSignalBuffer = GenericSignal( 0, 0 );
  State( "Recording" ) = 0;
}


void
DataIOFilter::Process( const GenericSignal* Input,
                             GenericSignal* Output )
{
  // Moving the save-to-file code to the beginning of Process() implies
  // that the time spent on i/o operations will only reduce the
  // time spent waiting for A/D data, and thus not enter into the
  // roundtrip time.
  // In between, the signal is buffered in a data member.
  // The BCI2000 standard requires that the state vector saved with a data block
  // is the one that existed when the data came out of the ADC.
  // So we also need to buffer the state vector between calls to Process().
  bool visualizeRoundtrip = false;
  if( !mSignalBuffer.GetProperties().IsEmpty() )
  {
    if( State( "Recording" ) == 1 )
      mpFileWriter->Write( mSignalBuffer, mStatevectorBuffer );
    visualizeRoundtrip = mVisualizeRoundtrip;
  }
  mpADC->Process( Input, Output );
  if( mpSourceFilter )
  {
    GenericSignal sourceFilterInput( *Output );
    mpSourceFilter->Process( &sourceFilterInput, Output );
  }
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
  mStatevectorBuffer = *Statevector;
  mSignalBuffer = *Output;

  if( mVisualizeEEG )
  {
    for( size_t i = 0; i < mDecimatedSignal.Channels(); ++i )
      for( size_t j = 0; j < mDecimatedSignal.Elements(); ++j )
        mDecimatedSignal( i, j )
          = ( ( *Output )( i, j * mVisualizeSourceDecimation ) - mSourceChOffset[ i ] )
          * mSourceChGain[ i ];
    mEEGVis.Send( &mDecimatedSignal );
  }
}

void
DataIOFilter::Resting()
{
  mpADC->Process( NULL, &mRestingSignal );
  if( mpSourceFilter )
  {
    GenericSignal sourceFilterInput( mRestingSignal );
    mpSourceFilter->Process( &sourceFilterInput, &mRestingSignal );
  }
  if( mVisualizeEEG )
  {
    for( size_t i = 0; i < mDecimatedSignal.Channels(); ++i )
      for( size_t j = 0; j < mDecimatedSignal.Elements(); ++j )
        mDecimatedSignal( i, j )
          = ( mRestingSignal( i, j * mVisualizeSourceDecimation ) - mSourceChOffset[ i ] )
          * mSourceChGain[ i ];
    mEEGVis.Send( &mDecimatedSignal );
  }
}

void
DataIOFilter::Halt()
{
  mSignalBuffer = GenericSignal( 0, 0 );
  if( mpADC )
    mpADC->Halt();
  if( mpFileWriter )
    mpFileWriter->Halt();
}

