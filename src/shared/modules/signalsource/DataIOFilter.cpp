////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that handles data acquisition from a GenericADC,
//   storing through a GenericFileWriter, and signal calibration into muV.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "DataIOFilter.h"

#include "defines.h"
#include "GenericADC.h"
#include "GenericFileWriter.h"
#include "BCIError.h"
#include "BCIException.h"
#include "BCIEvent.h"
#include "BCIDirectory.h"
#include "PrecisionTime.h"
#include "ClassName.h"
#include "MeasurementUnits.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <set>
#include <numeric>

using namespace std;
using namespace bci;

RegisterFilter( DataIOFilter, 0 );


DataIOFilter::DataIOFilter()
: mpADC( GenericFilter::PassFilter<GenericADC>() ),
  mpSourceFilter( NULL ),
  mpFileWriter( NULL ),
  mVisualizeSource( false ),
  mVisualizeTiming( false ),
  mVisualizeSourceDecimation( 1 ),
  mVisualizeSourceBufferSize( 1 ),
  mSourceVis( "SRCD" ),
  mTimingVis( "RNDT" ),
  mTimingSignal( 3, 1 ),
  mBlockCount( 0 ),
  mBlockDuration( 0 ),
  mSampleBlockSize( 0 ),
  mTimingBufferCursor( 0 ),
  mEvaluateTiming( true )
{
  BCIEvent::SetEventQueue( &mBCIEvents );

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
    if( Parameters->Exists( "FileFormat" ) ) {
      fileFormat = string( Parameter( "FileFormat" ) );
    }

    string writerName = fileFormat + "FileWriter";
    for( writerSet::const_iterator i = availableFileWriters.begin();
         mpFileWriter == NULL && i != availableFileWriters.end();
         ++i )
      if( writerName == ClassName( typeid( **i ) ) )
        mpFileWriter = *i;

    if( !mpFileWriter )
      bcierr << "Could not identify writer component for file format "
             << "\"" << fileFormat << "\""
             << endl;

    availableFileWriters.erase( mpFileWriter );
    for( writerSet::const_iterator i = availableFileWriters.begin();
         i != availableFileWriters.end();
         ++i )
      delete *i;
  }
  // Check whether the next filter in the chain is of type SourceFilter.
  // If this is the case, use it for preprocessing _before_ writing into the file.
  GenericFilter* filter = GenericFilter::GetFilter<GenericFilter>();
  if( filter && string( "SourceFilter" ) == ClassName( typeid( *filter ) ) )
    mpSourceFilter = GenericFilter::PassFilter<GenericFilter>();
}

void
DataIOFilter::Publish()
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
    "Visualize:Source%20Signal int VisualizeSourceDecimation= auto auto % % "
      "// decimation factor for raw brain signal",
    "Visualize:Source%20Signal int VisualizeSourceBufferSize= auto auto % % "
      "// number of blocks to aggregate before sending to operator",
    "Visualize:Source%20Signal int VisualizeSourceTime= 2s 2s 0 % "
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
  
  if( mpADC )
    mpADC->CallPublish();

  if( mpFileWriter )
    mpFileWriter->CallPublish();
    
  if( mpSourceFilter )
    mpSourceFilter->CallPublish();
}


DataIOFilter::~DataIOFilter()
{
  Halt();
  delete mpADC;
  delete mpSourceFilter;
  delete mpFileWriter;
  BCIEvent::SetEventQueue( NULL );
}


void
DataIOFilter::Preflight( const SignalProperties& Input,
                               SignalProperties& Output ) const
{
  // Parameter existence and range.
  Parameter( "SignalSourceIP" );
  Parameter( "ApplicationIP" );
  PreflightCondition( Parameter( "SamplingRate" ).InHertz() > 0 );
  PreflightCondition( Parameter( "SampleBlockSize" ).InHertz() > 0 );

  bool sourceChOffsetConsistent = ( Parameter( "SourceChOffset" )->NumValues() >= Parameter( "SourceCh" ) );
  if( !sourceChOffsetConsistent )
  {
    bcierr << "The number of entries in the SourceChOffset parameter (currently "
           << Parameter( "SourceChOffset" )->NumValues()
           << ") must match the SourceCh parameter (currently "
           << Parameter( "SourceCh" )
           << ")"
           << endl;
  }

  bool sourceChGainConsistent = ( Parameter( "SourceChGain" )->NumValues() >= Parameter( "SourceCh" ) );
  if( !sourceChGainConsistent )
  {
    bcierr << "The number of entries in the SourceChGain parameter (currently "
           << Parameter( "SourceChGain" )->NumValues()
           << ") must match the SourceCh parameter (currently "
           << Parameter( "SourceCh" )
           << ")"
           << endl;
  }

  if( Parameter( "VisualizeSource" ) == 1 )
  {
    int SampleBlockSize = Parameter( "SampleBlockSize" );
    PreflightCondition( SampleBlockSize > 0 );

    if( Parameter( "VisualizeSourceDecimation" ) != string( "auto" ) )
    {
      int VisualizeSourceDecimation = Parameter( "VisualizeSourceDecimation" );
      if( VisualizeSourceDecimation <= 0 )
        bcierr << "The VisualizeSourceDecimation parameter must be greater 0"
               << endl;
      if( Parameter( "VisualizeSourceBufferSize" ) != string( "auto" ) )
      {
        int VisualizeSourceBufferSize = Parameter( "VisualizeSourceBufferSize" ),
            effectiveSampleBlockSize = SampleBlockSize * VisualizeSourceBufferSize;
        if( effectiveSampleBlockSize % VisualizeSourceDecimation != 0 )
          bcierr << "The VisualizeSourceDecimation parameter "
                 << "(now " << VisualizeSourceDecimation << ") "
                 << "must be a divider of VisualizeSourceDecimation times sample block size "
                 << "(now " << effectiveSampleBlockSize << ")"
                 << endl;
      }
    }
    if( Parameter( "VisualizeSourceBufferSize" ) != string( "auto" ) )
    {
      int VisualizeSourceBufferSize = Parameter( "VisualizeSourceBufferSize" );
      if( VisualizeSourceBufferSize <= 0 )
        bcierr << "The VisualizeSourceBufferSize parameter must be greater 0"
               << endl;
    }
    Parameter( "SourceMin" );
    Parameter( "SourceMax" );
  }
  else
  {
    Parameter( "VisualizeSourceDecimation" );
    Parameter( "VisualizeSourceBufferSize" );
  }

  // Sub-filter preflight/signal properties.
  // The ADC and file writer filters must have a position string greater than
  // that of the DataIOFilter.
  SignalProperties adcInput;
  adcInput.SetChannels( Parameter( "SourceCh" ) )
          .SetElements( Parameter( "SampleBlockSize" ) )
          .SetUpdateRate( Parameter( "SamplingRate" ).InHertz() / Parameter( "SampleBlockSize" ) );
  if( sourceChOffsetConsistent && sourceChGainConsistent )
  {
    for( int ch = 0; ch < adcInput.Channels(); ++ch )
      adcInput.ValueUnit( ch ).SetGain( Parameter( "SourceChGain" )( ch ) * 1e-6 )
                              .SetOffset( Parameter( "SourceChOffset" )( ch ) )
                              .SetSymbol( "V" );
  }
  adcInput.ElementUnit().SetOffset( 0 )
                        .SetGain( 1.0 / Parameter( "SamplingRate" ).InHertz() )
                        .SetSymbol( "s" );
  Output = adcInput;
  mADCInput.SetProperties( adcInput );

  if( !mpADC )
    bcierr << "Expected an ADC filter instance to be present" << endl;
  else
    mpADC->CallPreflight( adcInput, Output );

  // Fix update and sampling rates in case Output has been reset by the ADC's Preflight() function:
  if( Output.ElementUnit().Symbol().empty() )
    Output.ElementUnit().SetOffset( 0 )
                        .SetGain( 1.0 / Parameter( "SamplingRate" ).InHertz() )
                        .SetSymbol( "s" );
  Output.SetUpdateRate( Parameter( "SamplingRate" ).InHertz() / Parameter( "SampleBlockSize" ) );

  if( !mpADC->IsRealTimeSource() )
    if( OptionalParameter( "EvaluateTiming", 1 ) == 0 )
      bciout << "WARNING: the EvaluateTiming parameter is false, so realtime operation will not be enforced" << endl;

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
  {
    bcierr << "Expected a file writer filter instance to be present" << endl;
  }
  else
  {
    SignalProperties writerOutput;
    mpFileWriter->CallPreflight( Output, writerOutput );
    if( !writerOutput.IsEmpty() )
      bcierr << "Expected empty output signal from file writer" << endl;
  }

  // Signal properties.
  if( !Input.IsEmpty() )
    bcierr << "Expected empty input signal" << endl;

  // Channel labels.
  set<string> names;
  LabelIndex& outputLabels = Output.ChannelLabels();
  int namesFromParam = min( Output.Channels(), Parameter( "ChannelNames" )->NumValues() );
  for( int i = 0; i < namesFromParam; ++i )
  {
    string name = Parameter( "ChannelNames" )( i );
    if( names.find( name ) == names.end() )
      names.insert( name );
    else
      bcierr << "Duplicate name: \"" << name << "\" in ChannelNames parameter" << endl;
    outputLabels[i] = name;
  }
  for( int i = namesFromParam; i < Output.Channels(); ++i )
  {
    ostringstream oss;
    oss << "Ch" << i + 1;
    outputLabels[i] = oss.str();
  }
  mInputBuffer.SetProperties( Output );

  if( !PhysicalUnit().SetSymbol( "s" ).IsPhysical( Parameter( "VisualizeSourceTime" ) ) )
    bciout << "The VisualizeSourceTime parameter is specified without a trailing \"s\". "
           << "This will lead to undesired results in future versions of BCI2000. "
           << "Please update your parameter files to prepare for the change."
           << endl;
  double numSamplesInDisplay = Parameter( "VisualizeSourceTime" )/*.InSeconds()*/ * Parameter( "SamplingRate" ).InHertz();
  Output.ElementUnit().SetRawMin( 0 )
                      .SetRawMax( numSamplesInDisplay - 1 );
  // We output calibrated signals in float32 format.
  Output.SetType( SignalType::float32 );
  Output.ValueUnit().SetOffset( 0 )
                    .SetGain( 1e-6 )
                    .SetSymbol( "V" );
  double rangeMin = Output.ValueUnit().PhysicalToRaw( Parameter( "SourceMin" ) ),
         rangeMax = Output.ValueUnit().PhysicalToRaw( Parameter( "SourceMax" ) );
  Output.ValueUnit().SetRawMin( rangeMin )
                    .SetRawMax( rangeMax );
}


void
DataIOFilter::Initialize( const SignalProperties& /*Input*/,
                          const SignalProperties& Output )
{
  const SignalProperties& adcInput = mADCInput.Properties(),
                        & adcOutput = mInputBuffer.Properties();

  mBlockDuration = 1e3 / adcInput.UpdateRate();
  mSampleBlockSize = adcInput.Elements();

  State( "Recording" ) = 0;
  mOutputBuffer = GenericSignal( 0, 0 );
  mpADC->CallInitialize( adcInput, adcOutput );
  if( mpSourceFilter )
  {
    mpSourceFilter->CallInitialize( adcOutput, adcOutput );
    mpSourceFilter->CallStartRun();
  }
  mpFileWriter->CallInitialize( adcOutput, SignalProperties( 0, 0 ) );

  // Calibration is handled by the DataIOFilter as well.
  mSourceChOffset.resize( mInputBuffer.Channels(), 0.0 );
  mSourceChGain.resize( mInputBuffer.Channels(), 1.0 );
  for( int channel = 0; channel < mInputBuffer.Channels(); ++channel )
  {
    mSourceChOffset[ channel ] = Parameter( "SourceChOffset" )( channel );
    mSourceChGain[ channel ] = Parameter( "SourceChGain" )( channel );
  }

  // Configure visualizations.
  mVisualizeSource = ( Parameter( "VisualizeSource" ) == 1 );

  if( Parameter( "VisualizeSourceBufferSize" ) == string( "auto" ) )
  {
    // Choose visualization buffer size for an update rate of 30Hz.
    mVisualizeSourceBufferSize = static_cast<int>( ::floor( 1000.0 / mBlockDuration / 30.0 ) );
    if( mVisualizeSourceBufferSize < 1 )
      mVisualizeSourceBufferSize = 1;
    bcidbg( 5 ) << "Choosing a VisualizeSourceBufferSize of "
                << mVisualizeSourceBufferSize << endl;
  }
  else
  {
    mVisualizeSourceBufferSize = Parameter( "VisualizeSourceBufferSize" );
  }
  SignalProperties v = Output;
  v.SetElements( mVisualizeSourceBufferSize * v.Elements() );
  mVisSourceBuffer.SetProperties( v );

  if( Parameter( "VisualizeSourceDecimation" ) == string( "auto" ) )
  {
    mVisualizeSourceDecimation = static_cast<int>( ::floor( adcInput.SamplingRate() / 256.0 ) );
    if( mVisualizeSourceDecimation < 1 )
      mVisualizeSourceDecimation = 1;
    int numSamplesBuffered = static_cast<int>( mVisualizeSourceBufferSize * mSampleBlockSize );
    while( numSamplesBuffered % mVisualizeSourceDecimation )
      --mVisualizeSourceDecimation;
    bcidbg( 5 ) << "Choosing a VisualizeSourceDecimation value of "
                << mVisualizeSourceDecimation << endl;
  }
  else
  {
    mVisualizeSourceDecimation = Parameter( "VisualizeSourceDecimation" );
  }
  mBlockCount = 0;

  SignalProperties d = Output;
  int newElements = Output.Elements() * mVisualizeSourceBufferSize / mVisualizeSourceDecimation;
  double trueDecimation = ( Output.Elements() * mVisualizeSourceBufferSize ) / newElements;
  d.SetName( "Source Signal" )
   .SetElements( newElements )
   .SetType( SignalType::float32 )
   .ElementUnit().SetGain( Output.ElementUnit().Gain() * trueDecimation )
                 .SetRawMax( Output.ElementUnit().RawMax() / trueDecimation );
  mDecimatedSignal.SetProperties( d );
  if( mVisualizeSource )
    mSourceVis.Send( mDecimatedSignal.Properties() );

  mVisualizeTiming = ( Parameter( "VisualizeTiming" ) == 1 );

  if( !mpADC->IsRealTimeSource() )
    mEvaluateTiming = ( OptionalParameter( "EvaluateTiming", 1 ) != 0 );

  bool measureStimulus = ( Parameter( "SignalSourceIP" ) == Parameter( "ApplicationIP" ) );
  SignalProperties p = mTimingSignal.Properties();
  p.SetChannels( measureStimulus ? 3 : 2 );
  p.SetName( "Timing" );
  // Roundtrip values are in ms, and we want a range that is twice the value
  // of what we expect for the second signal (the time between subsequent
  // completions of the ADC's Process()).
  p.ValueUnit().SetRawMin( 0 ).SetRawMax( 2 * mBlockDuration )
               .SetOffset( 0 ).SetGain( 1e-3 ).SetSymbol( "s" );
  p.ChannelLabels()[ 0 ] = ":Block";
  p.ChannelLabels()[ 1 ] = ":Roundtrip";
  if( measureStimulus )
    p.ChannelLabels()[ 2 ] = ":Stimulus";
  p.ElementUnit().SetRawMin( 0 ).SetRawMax( 127 )
                 .SetOffset( 0 ).SetGain( mBlockDuration / 1e3 ).SetSymbol( "s" );
  mTimingSignal.SetProperties( p );

  if( mVisualizeTiming )
  {
    mTimingVis.Send( mTimingSignal.Properties() );

    RGBColor colors[] = { RGBColor::White, RGBColor::LtGray, RGBColor::DkGray, ColorList::End };
    mTimingVis.Send( CfgID::ChannelColors, ColorList( colors ) )
              .Send( CfgID::ShowBaselines, true )
              .Send( CfgID::AutoScale, "off" );
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

  // Reset timing evaluation buffer.
  mTimingBuffer.resize( 0 );
  mTimingBuffer.resize( static_cast<size_t>( MeasurementUnits::TimeInSampleBlocks( "10s" ) ), 0 );
  mTimingBufferCursor = 0;
}


void
DataIOFilter::StopRun()
{
  mpADC->CallStopRun();
  mpFileWriter->CallStopRun();
  mOutputBuffer = GenericSignal( 0, 0 );
  State( "Recording" ) = 0;
}


void
DataIOFilter::Process( const GenericSignal& /*Input*/,
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
  if( !mOutputBuffer.Properties().IsEmpty() )
  {
    if( State( "Recording" ) == 1 )
      mpFileWriter->CallWrite( mOutputBuffer, mStatevectorBuffer );
    visualizeTiming = mVisualizeTiming;
  }

  PrecisionTime prevSourceTime = static_cast<PrecisionTime::NumType>( State( "SourceTime" ) );

  mpADC->CallProcess( mADCInput, mInputBuffer );
  if( State( "SourceTime" ) == prevSourceTime ) // GenericADC::Process() did not set the time stamp
    State( "SourceTime" ) = PrecisionTime::Now();

  PrecisionTime sourceTime = static_cast<PrecisionTime::NumType>( State( "SourceTime" ) ),
                stimulusTime = static_cast<PrecisionTime::NumType>( State( "StimulusTime" ) );
  mTimingSignal( 0, 0 ) = PrecisionTime::SignedDiff( sourceTime, prevSourceTime ); // sample block duration
  if( mTimingSignal( 0, 0 ) < 0 )
  {
    if( mEvaluateTiming )
      bciout << "Time measurement appears to be unreliable on your system. "
             << "You cannot use BCI2000 time stamps for timing evaluation."
             << endl;
    mEvaluateTiming = false;
  }
  mTimingSignal( 1, 0 ) = PrecisionTime::SignedDiff( functionEntry, prevSourceTime ); // roundtrip
  if( mTimingSignal.Channels() > 2 )
    mTimingSignal( 2, 0 ) = PrecisionTime::SignedDiff( stimulusTime, prevSourceTime ); // source-to-stimulus delay
  if( mEvaluateTiming )
    EvaluateTiming( mTimingSignal( 1, 0 ) );
  if( visualizeTiming )
    mTimingVis.Send( mTimingSignal );

  if( mpSourceFilter )
  {
    GenericSignal sourceFilterInput( mInputBuffer );
    mpSourceFilter->CallProcess( sourceFilterInput, mInputBuffer );
  }

  ResetStates( State::EventKind );
  ProcessBCIEvents();
  mStatevectorBuffer = *Statevector;
  mOutputBuffer = mInputBuffer;
  ResetStates( State::StateKind );

  for( int i = 0; i < Output.Channels(); ++i )
    for( int j = 0; j < Output.Elements(); ++j )
      Output( i, j )  = ( mInputBuffer( i, j ) - mSourceChOffset[ i ] ) * mSourceChGain[ i ];

  CopyBlock( Output, mVisSourceBuffer, mBlockCount++ );
  bool doSend = ( mBlockCount == mVisualizeSourceBufferSize );
  if( doSend )
    mBlockCount = 0;
  if( mVisualizeSource && doSend )
  {
    Downsample( mVisSourceBuffer, mDecimatedSignal );
    mSourceVis.Send( mDecimatedSignal );
  }
}

void
DataIOFilter::Resting()
{
  mpADC->CallProcess( mADCInput, mInputBuffer );
  if( mpSourceFilter )
  {
    GenericSignal sourceFilterInput( mInputBuffer );
    mpSourceFilter->CallProcess( sourceFilterInput, mInputBuffer );
  }

  for( int i = 0; i < mInputBuffer.Channels(); ++i )
    for( int j = 0; j < mInputBuffer.Elements(); ++j )
      mInputBuffer( i, j ) = ( mInputBuffer( i, j ) - mSourceChOffset[ i ] ) * mSourceChGain[ i ];

  CopyBlock( mInputBuffer, mVisSourceBuffer, mBlockCount++ );
  bool doSend = ( mBlockCount == mVisualizeSourceBufferSize );
  if( doSend )
    mBlockCount = 0;
  if( mVisualizeSource && doSend )
  {
    Downsample( mVisSourceBuffer, mDecimatedSignal );
    mSourceVis.Send( mDecimatedSignal );
  }
}

void
DataIOFilter::Halt()
{
  mOutputBuffer = GenericSignal( 0, 0 );
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
        value += Input( ch, insample ) / decimationFactor;
      Output( ch, outsample ) = value;
    }
}

void
DataIOFilter::CopyBlock( const GenericSignal& Input, GenericSignal& Output, int inBlock )
{
  int blockSize = Input.Elements();
  for( int ch = 0; ch < Input.Channels(); ++ch )
    for( int sample = 0; sample < Input.Elements(); ++sample )
      Output( ch, sample + blockSize * inBlock ) = Input( ch, sample );
}

void
DataIOFilter::EvaluateTiming( double inRoundtrip )
{
  if( !mTimingBuffer.empty() )
  {
    mTimingBuffer[ mTimingBufferCursor++ ] = inRoundtrip;
    if( mTimingBufferCursor == mTimingBuffer.size() / 2 || mTimingBufferCursor == mTimingBuffer.size() )
    {
      double avgRoundtrip = accumulate( mTimingBuffer.begin(), mTimingBuffer.end(), 0.0 ) / mTimingBuffer.size();
      avgRoundtrip = avgRoundtrip / mBlockDuration;
      if( avgRoundtrip >= 1.1 )
        bcierr << "Roundtrip time consistently exceeds block duration (currently "
               << avgRoundtrip * 100
               << "%)"
               << endl;
      else if( avgRoundtrip >= 1.0 )
        bciout << "Roundtrip time exceeds block duration (currently "
               << avgRoundtrip * 100
               << "%)"
               << endl;
      else if( avgRoundtrip >= 0.75 )
        bciout << "Roundtrip time approaches block duration (currently "
               << avgRoundtrip * 100
               << "%)"
               << endl;
    }
    mTimingBufferCursor %= mTimingBuffer.size();
  }
}

void
DataIOFilter::ProcessBCIEvents()
{
  // When translating event time stamps into sample positions, we assume a block's
  // source time stamp to match the subsequent block's first sample.
  PrecisionTime sourceTime = static_cast<PrecisionTime::NumType>( Statevector->StateValue( "SourceTime" ) );

  while( !mBCIEvents.IsEmpty()
         && PrecisionTime::SignedDiff( mBCIEvents.FrontTimeStamp(), sourceTime ) <= 0 )
  {
    int offset = static_cast<int>( 
      ( ( mBlockDuration - ( PrecisionTime::UnsignedDiff( sourceTime, mBCIEvents.FrontTimeStamp() ) + 1 ) ) * mSampleBlockSize ) / mBlockDuration
    );
    istringstream iss( mBCIEvents.FrontDescriptor() );
    string name;
    iss >> name;
    State::ValueType value;
    iss >> value;
    int duration;
    if( !( iss >> duration ) )
      duration = -1;

    int kind = ( *States )[name].Kind();
    if( kind != State::EventKind )
      throw bciexception( "Trying to set state \"" << name << "\" from an event. "
                          "This state was not defined as an event state. "
                          "Use BEGIN_EVENT_DEFINITIONS/END_EVENT_DEFINITIONS to define states "
                          "as event states."
                        );

#if BCIDEBUG
    bcidbg( 10 ) << "Setting State \"" << name
                 << "\" to " << value
                 << " at offset " << offset
                 << " with duration " << duration
                 << " from event:\n" << mBCIEvents.FrontDescriptor()
                 << endl;
#endif // BCIDEBUG

    offset = max( offset, 0 );
    if( duration < 0 )
    { // No duration given -- set the state at the current and following positions.
      Statevector->SetStateValue( name, offset, value );
    }
    else if( duration == 0 )
    { // Set the state at a single position only.
      // For zero duration events, avoid overwriting a previous event by
      // moving the current one if possible, and reposting if not.
      while( offset <= mSampleBlockSize && Statevector->StateValue( name, offset ) != 0 )
        ++offset;
      if( offset == mSampleBlockSize )
      { // Re-post the event to be processed in the next block
        mBCIEvents.PushBack( mBCIEvents.FrontDescriptor(), mBCIEvents.FrontTimeStamp() );
      }
      else
      {
        Statevector->SetStateValue( name, offset, value );
        Statevector->SetStateValue( name, offset + 1, 0 );
      }
    }
    else
    {
      bcierr__ << "Event durations > 0 are currently unsupported "
               << "(" << iss.str() << ")"
               << endl;
    }
    mBCIEvents.PopFront();
  }
}

void
DataIOFilter::ResetStates( int inKind )
{ // For all states of the given kind, reset values to the value at the carry over position.
  int nextSample = Statevector->Samples() - 1;
  for( int state = 0; state < States->Size(); ++state )
  {
    const ::State& s = ( *States )[state];
    if( s.Kind() == inKind )
    {
      int location = s.Location(),
          length = s.Length();
      State::ValueType value = ( *Statevector )( nextSample ).StateValue( location, length );
      for( int sample = 0; sample < nextSample ; ++sample )
        ( *Statevector )( sample ).SetStateValue( location, length, value );
    }
  }
}
