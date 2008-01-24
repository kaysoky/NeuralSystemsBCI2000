////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that handles data acquisition from a GenericADC,
//   storing through a GenericFileWriter, and signal calibration into muV.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "DataIOFilter.h"

#include "defines.h"
#include "GenericADC.h"
#include "GenericFileWriter.h"
#include "BCIError.h"
#include "BCIDirectory.h"
#include "PrecisionTime.h"
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
  mVisualizeSource( false ),
  mVisualizeSourceDecimation( 1 ),
  mVisualizeTiming( false ),
  mSourceVis( "SRCD" ),
  mTimingVis( "RNDT" ),
  mTimingSignal( 3, 1 )
{
  BEGIN_PARAMETER_DEFINITIONS
    // Parameters required to interpret a data file are listed here
    // to enforce their presence:
    "Source:Signal%20Properties int SourceCh= 16 "
       "16 1 % // number of digitized and stored channels",
    "Source:Signal%20Properties int SampleBlockSize= 32 "
       "32 1 % // number of samples transmitted at a time",
    "Source:Signal%20Properties int SamplingRate= 256Hz "
       "256Hz 1 % // sample rate",
    "Source:Signal%20Properties list ChannelNames= 0 "
       "% % % // list of channel names",
    "Source:Signal%20Properties floatlist SourceChOffset= 16 "
      "0 0 0 0 "
      "0 0 0 0 "
      "0 0 0 0 "
      "0 0 0 0 "
      "0 % % // Offset for channels in A/D units",
    "Source:Signal%20Properties floatlist SourceChGain= 16 "
      "0.003 0.003 0.003 0.003 "
      "0.003 0.003 0.003 0.003 "
      "0.003 0.003 0.003 0.003 "
      "0.003 0.003 0.003 0.003 "
      "0.003 % % // gain for each channel (A/D units -> muV)",

    // Storage related parameters are listed here to enforce their presence
    // even if not used by all FileWriter classes.
    "Storage:Data%20Location string DataDirectory= ..\\data ..\\data % % "
      "// path to top level data directory (directory)",
    "Storage:Session string SubjectName= Name Name % % "
      "// subject alias",
    "Storage:Session string SubjectSession= 001 001 % % "
      "// three-digit session number",
    "Storage:Session string SubjectRun= 00 00 % % "
      "// two-digit run number",
    "Storage:Documentation string ID_System= % % % % "
      "// BCI2000 System Code",
    "Storage:Documentation string ID_Amp= % % % % "
      "// BCI2000 Amp Code",
    "Storage:Documentation string ID_Montage= % % % % "
      "// BCI2000 Cap Montage Code",

    // Visualization of data as far as managed by the DataIOFilter:
    "Visualize:Timing int VisualizeTiming= 1 1 0 1 "
      "// visualize system timing (0=no, 1=yes) (boolean)",
    "Visualize:Source%20Signal int VisualizeSource= 1 1 0 1 "
      "// visualize raw brain signal (0=no, 1=yes) (boolean)",
    "Visualize:Source%20Signal int VisualizeSourceDecimation= 1 1 1 % "
      "// decimation factor for raw brain signal",
    "Visualize:Source%20Signal int VisualizeSourceTime= 2 2 0 % "
      "// how much time in Source visualization",
    "Visualize:Source%20Signal int SourceMin= -100muV -100muV % % "
      "// raw signal vis Min Value",
    "Visualize:Source%20Signal int SourceMax= 100muV 100muV % % "
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
    string fileFormat = "BCI2000";
    if( Parameters->Exists( "FileFormat" ) )
      fileFormat = Parameter( "FileFormat" );

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
  // If this is the case, use it for preprocessing _before_ writing into the file.
  GenericFilter* filter = GenericFilter::GetFilter<GenericFilter>();
  if( string( "SourceFilter" ) == ClassName( typeid( *filter ) ) )
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
  Parameter( "SignalSourceIP" );
  Parameter( "ApplicationIP" );
  PreflightCondition( Parameter( "SamplingRate" ) > 0 );

  if( Parameter( "SourceChOffset" )->NumValues() < Parameter( "SourceCh" )
      || Parameter( "SourceChGain" )->NumValues() < Parameter( "SourceCh" ) )
    bcierr << "The number of entries in the SourceChOffset and SourceChGain "
           << "parameters must match the SourceCh parameter."
           << endl;

  if( Parameter( "VisualizeSource" ) == 1 )
  {
    int SampleBlockSize = Parameter( "SampleBlockSize" ),
        VisualizeSourceDecimation = Parameter( "VisualizeSourceDecimation" );
    PreflightCondition( SampleBlockSize > 0 );
    if( VisualizeSourceDecimation <= 0 )
      bcierr << "The VisualizationSourceDecimation parameter must be greater 0"
             << endl;
    else if( SampleBlockSize % VisualizeSourceDecimation != 0 )
      bcierr << "The VisualizationSourceDecimation parameter "
             << "(now " << VisualizeSourceDecimation << ") "
             << "must be a divider of the sample block size "
             << "(now " << SampleBlockSize << ")"
             << endl;
    Parameter( "SourceMin" );
    Parameter( "SourceMax" );
  }

  // Sub-filter preflight/signal properties.
  // The ADC and file writer filters must have a position string greater than
  // that of the DataIOFilter.
  if( !mpADC )
    bcierr << "Expected an ADC filter instance to be present" << endl;
  else
  {
    mpADC->CallPreflight( Input, Output );
    mRestingSignal.SetProperties( Output );
  }

  if( mpSourceFilter )
  {
    SignalProperties sourceFilterInput( Output );
    mpSourceFilter->CallPreflight( sourceFilterInput, Output );
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

  // We output calibrated signals in float32 format.
  Output.SetType( SignalType::float32 );
  Output.ElementUnit().SetOffset( 0 )
                      .SetGain( 1.0 / Parameter( "SamplingRate" ) )
                      .SetSymbol( "s" );
  int numSamplesInDisplay = Parameter( "VisualizeSourceTime" ) * Parameter( "SamplingRate" );
  Output.ElementUnit().SetRawMin( 0 )
                      .SetRawMax( numSamplesInDisplay - 1 );
  Output.ValueUnit().SetOffset( 0 )
                    .SetGain( 1e-6 )
                    .SetSymbol( "V" );
  float rangeMin = Output.ValueUnit().PhysicalToRaw( Parameter( "SourceMin" ) ),
        rangeMax = Output.ValueUnit().PhysicalToRaw( Parameter( "SourceMax" ) );
  Output.ValueUnit().SetRawMin( rangeMin )
                    .SetRawMax( rangeMax );
  if( Parameters->Exists( "ChannelNames" ) && Parameter( "ChannelNames" )->NumValues() > 0 )
  {
    LabelIndex& outputLabels = Output.ChannelLabels();
    for( int i = 0; i < min( Output.Channels(), Parameter( "ChannelNames" )->NumValues() ); ++i )
      outputLabels[ i ] = Parameter( "ChannelNames" )( i );
  }
}


void
DataIOFilter::Initialize( const SignalProperties& Input,
                          const SignalProperties& Output )
{
  const SignalProperties& adcOutput = mRestingSignal.Properties();
  State( "Recording" ) = 0;
  mSignalBuffer = GenericSignal( 0, 0 );
  mpADC->CallInitialize( Input, adcOutput );
  if( mpSourceFilter )
  {
    mpSourceFilter->CallInitialize( adcOutput, adcOutput );
    mpSourceFilter->CallStartRun();
  }
  mpFileWriter->CallInitialize( adcOutput, SignalProperties( 0, 0 ) );

  // Calibration is handled by the DataIOFilter as well.
  mSourceChOffset.resize( mRestingSignal.Channels(), 0.0 );
  mSourceChGain.resize( mRestingSignal.Channels(), 1.0 );
  for( int channel = 0; channel < mRestingSignal.Channels(); ++channel )
  {
    mSourceChOffset[ channel ] = Parameter( "SourceChOffset" )( channel );
    mSourceChGain[ channel ] = Parameter( "SourceChGain" )( channel );
  }

  // Configure visualizations.
  mVisualizeSource = ( Parameter( "VisualizeSource" ) == 1 );

  mVisualizeSourceDecimation = Parameter( "VisualizeSourceDecimation" );
  SignalProperties d = Output;
  d.SetName( "Source Signal" )
   .SetElements( Output.Elements() / mVisualizeSourceDecimation )
   .SetType( SignalType::float24 )
   .ElementUnit().SetGain( Output.ElementUnit().Gain() * mVisualizeSourceDecimation )
                 .SetRawMax( Output.ElementUnit().RawMax() / mVisualizeSourceDecimation - 1 );
  mDecimatedSignal.SetProperties( d );
  if( mVisualizeSource )
    mSourceVis.Send( mDecimatedSignal.Properties() );

  mVisualizeTiming = ( Parameter( "VisualizeTiming" ) == 1 );
  if( mVisualizeTiming )
  {
    bool measureStimulus = ( Parameter( "SignalSourceIP" ) == Parameter( "ApplicationIP" ) );
    float blockDuration = Parameter( "SampleBlockSize" ) / Parameter( "SamplingRate" );
    SignalProperties p = mTimingSignal.Properties();
    p.SetChannels( measureStimulus ? 3 : 2 );
    p.SetName( "Timing" );
    // Roundtrip values are in ms, and we want a range that is twice the value
    // of what we expect for the second signal (the time between subsequent
    // completions of the ADC's Process()).
    p.ValueUnit().SetRawMin( 0 ).SetRawMax( 2000 * blockDuration )
                 .SetOffset( 0 ).SetGain( 1e-3 ).SetSymbol( "s" );
    p.ChannelLabels()[ 0 ] = ":Block";
    p.ChannelLabels()[ 1 ] = ":Roundtrip";
    if( measureStimulus )
      p.ChannelLabels()[ 2 ] = ":Stimulus";
    p.ElementUnit().SetRawMin( 0 ).SetRawMax( 127 )
                   .SetOffset( 0 ).SetGain( blockDuration ).SetSymbol( "s" );
    mTimingSignal.SetProperties( p );
    mTimingVis.Send( mTimingSignal.Properties() );

    RGBColor colors[] = { RGBColor::White, RGBColor::LtGray, RGBColor::DkGray, ColorList::End };
    mTimingVis.Send( CfgID::ChannelColors, ColorList( colors ) )
              .Send( CfgID::ShowBaselines, true );
  }
  mTimingVis.Send( CfgID::Visible, mVisualizeTiming );
}


void
DataIOFilter::StartRun()
{
  mpADC->CallStartRun();
  mpFileWriter->CallStartRun();

  // Initialize time stamps with the current time to get a correct roundtrip
  // time, and a zero stimulus delay, for the first block.
  PrecisionTime now = PrecisionTime::Now();
  State( "SourceTime" ) = now;
  State( "StimulusTime" ) = now;
  State( "Recording" ) = 1;
}


void
DataIOFilter::StopRun()
{
  mpADC->CallStopRun();
  mpFileWriter->CallStopRun();
  mSignalBuffer = GenericSignal( 0, 0 );
  State( "Recording" ) = 0;
}


void
DataIOFilter::Process( const GenericSignal& Input,
                             GenericSignal& Output )
{
  PrecisionTime functionEntry = PrecisionTime::Now();
  // Moving the save-to-file code to the beginning of Process() implies
  // that the time spent on i/o operations will only reduce the
  // time spent waiting for A/D data, and thus not enter into the
  // roundtrip time.
  // In between, the signal is buffered in a data member.
  // The BCI2000 standard requires that the state vector saved with a data block
  // is the one that existed when the data came out of the ADC.
  // So we also need to buffer the state vector between calls to Process().
  bool visualizeTiming = false;
  if( !mSignalBuffer.Properties().IsEmpty() )
  {
    if( State( "Recording" ) == 1 )
      mpFileWriter->Write( mSignalBuffer, mStatevectorBuffer );
    visualizeTiming = mVisualizeTiming;
  }
  mpADC->CallProcess( Input, Output );
  PrecisionTime now = PrecisionTime::Now();
  if( mpSourceFilter )
  {
    GenericSignal sourceFilterInput( Output );
    mpSourceFilter->CallProcess( sourceFilterInput, Output );
  }
  if( visualizeTiming )
  {
    PrecisionTime sourceTime = static_cast<short>( State( "SourceTime" ) ),
                  stimulusTime = static_cast<short>( State( "StimulusTime" ) );
    mTimingSignal( 0, 0 ) = now - sourceTime; // sample block duration
    mTimingSignal( 1, 0 ) = functionEntry - sourceTime; // roundtrip
    if( mTimingSignal.Channels() > 2 )
      mTimingSignal( 2, 0 ) = stimulusTime - sourceTime; // source-to-stimulus delay
    mTimingVis.Send( mTimingSignal );
  }
  mStatevectorBuffer = *Statevector;
  mSignalBuffer = Output;
  State( "SourceTime" ) = now;

  for( int i = 0; i < Output.Channels(); ++i )
    for( int j = 0; j < Output.Elements(); ++j )
      Output( i, j )  = ( Output( i, j ) - mSourceChOffset[ i ] ) * mSourceChGain[ i ];

  if( mVisualizeSource )
  {
    Downsample( Output, mDecimatedSignal );
    mSourceVis.Send( mDecimatedSignal );
  }
}

void
DataIOFilter::Resting()
{
  static GenericSignal adcInput( 0, 0 );
  mpADC->CallProcess( adcInput, mRestingSignal );
  if( mpSourceFilter )
  {
    GenericSignal sourceFilterInput( mRestingSignal );
    mpSourceFilter->CallProcess( sourceFilterInput, mRestingSignal );
  }
  if( mVisualizeSource )
  {
    Downsample( mRestingSignal, mDecimatedSignal );
    for( int i = 0; i < mDecimatedSignal.Channels(); ++i )
      for( int j = 0; j < mDecimatedSignal.Elements(); ++j )
        mDecimatedSignal( i, j )
          = ( mDecimatedSignal( i, j ) - mSourceChOffset[ i ] )
          * mSourceChGain[ i ];
    mSourceVis.Send( mDecimatedSignal );
  }
}

void
DataIOFilter::Halt()
{
  mSignalBuffer = GenericSignal( 0, 0 );
  if( mpADC )
    mpADC->CallHalt();
  if( mpFileWriter )
    mpFileWriter->CallHalt();
}

void
DataIOFilter::Downsample( const GenericSignal& Input, GenericSignal& Output )
{
  int decimationFactor = Input.Elements() / Output.Elements();
  for( int ch = 0; ch < Output.Channels(); ++ch )
    for( int outsample = 0; outsample < Output.Elements(); ++outsample )
    {
      GenericSignal::ValueType value = 0.0;
      for( int insample = outsample * decimationFactor; insample < ( outsample + 1 ) * decimationFactor; ++insample )
        value += Input( ch, insample );
      Output( ch, outsample ) = value / decimationFactor;
    }
}

