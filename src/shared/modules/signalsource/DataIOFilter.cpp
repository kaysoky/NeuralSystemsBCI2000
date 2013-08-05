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
#include "BCIStream.h"
#include "BCIException.h"
#include "BCIEvent.h"
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
using namespace StatisticalObserver;

static double CalibrateTo = 1e-6; // muV

RegisterFilter( DataIOFilter, 0 );

enum TimingChannelIdx
{
  Block,
  BlockNom,
  BlockAvg,
  Roundtrip,
  Stimulus,

  NumTimingChannels
};

DataIOFilter::DataIOFilter()
: mpADC( GenericFilter::PassFilter<GenericADC>() ),
  mpSourceFilter( NULL ),
  mpFileWriter( NULL ),
  mpFileWriterInput( NULL ),
  mVisualizeSource( false ),
  mVisualizeTiming( false ),
  mVisualizeSourceDecimation( 1 ),
  mVisualizeSourceBufferSize( 1 ),
  mSourceVis( "SRCD" ),
  mTimingVis( "RNDT" ),
  mTimingSignal( NumTimingChannels, 1 ),
  mBlockCount( 0 ),
  mIsFirstBlock( false ),
  mBlockDuration( 0 ),
  mSampleBlockSize( 0 )
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
      "% % % // number of digitized and stored channels",
    "Source:Signal%20Properties int SampleBlockSize= 32 "
      "% % % // number of samples transmitted at a time",
    "Source:Signal%20Properties int SamplingRate= 256Hz "
      "% % % // sample rate",
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
      "0.003 % % // gain for each channel (A/D units per physical unit)",

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
    "Visualize:Source%20Signal int SourceMin= auto % % % "
      "// raw signal vis Min Value",
    "Visualize:Source%20Signal int SourceMax= auto % % % "
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
DataIOFilter::AutoConfig( const SignalProperties& Input )
{
  if( !mpADC )
    bcierr << "Expected an ADC filter instance to be present";
  else
    mpADC->CallAutoConfig( Input );

  Parameter( "SamplingRate" ) = 1;
  Parameter( "SampleBlockSize" ) = 1;
  Parameter( "ChannelNames" )->SetNumValues( 0 );

  MeasurementUnits::Initialize( *Parameters );
}

void
DataIOFilter::Preflight( const SignalProperties& Input,
                               SignalProperties& Output ) const
{
  // Parameter existence and range.
  OptionalParameter( "SignalSourceIP" );
  OptionalParameter( "ApplicationIP" );
  PreflightCondition( Parameter( "SamplingRate" ).InHertz() > 0 );

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
  SignalProperties adcOutput;
  AdjustProperties( adcOutput );
  // Channel labels.
  set<string> names;
  LabelIndex& outputLabels = adcOutput.ChannelLabels();
  int namesFromParam = min( adcOutput.Channels(), Parameter( "ChannelNames" )->NumValues() );
  for( int i = 0; i < namesFromParam; ++i )
  {
    string name = Parameter( "ChannelNames" )( i );
    if( names.find( name ) == names.end() )
      names.insert( name );
    else
      bcierr << "Duplicate name: \"" << name << "\" in ChannelNames parameter" << endl;
    outputLabels[i] = name;
  }
  for( int i = namesFromParam; i < adcOutput.Channels(); ++i )
  {
    ostringstream oss;
    oss << "Ch" << i + 1;
    outputLabels[i] = oss.str();
  }
  // Calibration
  bool unitsPresent = false;
  for( int ch = 0; ch < adcOutput.Channels(); ++ch )
  {
    PhysicalUnit& u = adcOutput.ValueUnit( ch );
    u.SetOffset( Parameter( "SourceChOffset" )( ch ) )
     .SetGainWithSymbol( Parameter( "SourceChGain" )( ch ) );
    unitsPresent |= !u.Symbol().empty();
  }
  if( !unitsPresent )
  {
    for( int ch = 0; ch < adcOutput.Channels(); ++ch )
    {
      PhysicalUnit& u = adcOutput.ValueUnit( ch );
        u.SetGain( u.Gain() * 1e-6 ).SetSymbol( "V" );
    }
  }
  Output = adcOutput;
  if( mpADC )
    mpADC->CallPreflight( adcOutput, Output );
  // Fixup Output properties without destroying changes made by the ADC filter.
  AdjustProperties( Output );
  for( int ch = 0; ch < Output.Channels(); ++ch )
  {
    PhysicalUnit& a = adcOutput.ValueUnit( ch ),
                & u = Output.ValueUnit( ch );
    u.SetOffset( a.Offset() )
     .SetGain( a.Gain() )
     .SetSymbol( a.Symbol() );
  }
  adcOutput = Output;
  mADCOutput.SetProperties( adcOutput );
  if( !mpADC->IsRealTimeSource() )
    if( OptionalParameter( "EvaluateTiming", 1 ) == 0 )
      bciwarn << "The EvaluateTiming parameter is false, so realtime operation will not be enforced" << endl;

  if( !mpSourceFilter )
  {
    mSourceFilterOutput = GenericSignal();
    mpFileWriterInput = &mADCOutput;
  }
  else
  {
    mpSourceFilter->CallAutoConfig( adcOutput );
    SignalProperties sourceFilterOutput( adcOutput );
    mpSourceFilter->CallPreflight( adcOutput, sourceFilterOutput );
    mSourceFilterOutput.SetProperties( sourceFilterOutput );
    mpFileWriterInput = &mSourceFilterOutput;
  }

  if( !mpFileWriter )
  {
    bcierr << "Expected a file writer filter instance to be present" << endl;
  }
  else
  {
    SignalProperties writerOutput;
    mpFileWriter->CallAutoConfig( mpFileWriterInput->Properties() );
    mpFileWriter->CallPreflight( mpFileWriterInput->Properties(), writerOutput );
    if( !writerOutput.IsEmpty() )
      bcierr << "Expected empty output signal from file writer" << endl;
  }

  // Signal properties.
  if( !Input.IsEmpty() )
    bcierr << "Expected empty input signal" << endl;

  for( int ch = 0; ch < Output.Channels(); ++ch )
  {
    PhysicalUnit& u = Output.ValueUnit( ch );
    double physMin = u.RawToPhysicalValue( u.RawMin() ),
           physMax = u.RawToPhysicalValue( u.RawMax() );
    u.SetOffset( 0 )
     .SetGain( CalibrateTo );
    u.SetRawMin( u.PhysicalToRawValue( physMin ) )
     .SetRawMax( u.PhysicalToRawValue( physMax ) );
  }
  Output.SetType( SignalType::float32 );

  // Range.
  if( Parameter( "SourceMin" ) != "auto" )
  {
    double rangeMin = Output.ValueUnit().PhysicalToRaw( Parameter( "SourceMin" ) );
    for( int ch = 0; ch < Output.Channels(); ++ch )
      Output.ValueUnit( ch ).SetRawMin( rangeMin );
  }
  if( Parameter( "SourceMax" ) != "auto" )
  {
    double rangeMax = Output.ValueUnit().PhysicalToRaw( Parameter( "SourceMax" ) );
    for( int ch = 0; ch < Output.Channels(); ++ch )
      Output.ValueUnit( ch ).SetRawMax( rangeMax );
  }

  double numSamplesInDisplay = Parameter( "VisualizeSourceTime" ).InSeconds() * Parameter( "SamplingRate" ).InHertz();
  if( !PhysicalUnit().SetSymbol( "s" ).IsPhysical( Parameter( "VisualizeSourceTime" ) ) )
    bciwarn << "The VisualizeSourceTime parameter specifies time without unit. "
            << "Throughout BCI2000, time specifications without unit are now consistently interpreted "
            << "as being given in sample blocks.\n"
            << "If your source display appears strange, try appending the letter \"s\" to the "
            << "VisualizeSourceTime parameters value.";
  Output.ElementUnit().SetRawMin( 0 )
                      .SetRawMax( numSamplesInDisplay - 1 );
}

void
DataIOFilter::Initialize( const SignalProperties& /*Input*/,
                          const SignalProperties& Output )
{
  mOutputBuffer.SetProperties( Output );
  const SignalProperties& adcOutput = mADCOutput.Properties();
  mBlockDuration = 1e3 / adcOutput.UpdateRate();
  mSampleBlockSize = adcOutput.Elements();

  State( "Recording" ) = 0;
  mpADC->CallInitialize( adcOutput, adcOutput );
  if( mpSourceFilter )
    mpSourceFilter->CallInitialize( adcOutput, mSourceFilterOutput.Properties() );
  mpFileWriter->CallInitialize( mpFileWriterInput->Properties(), SignalProperties( 0, 0 ) );

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
    mVisualizeSourceDecimation = static_cast<int>( ::floor( adcOutput.SamplingRate() / 256.0 ) );
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
  mIsFirstBlock = true;

  SignalProperties d = Output;
  int newElements = Output.Elements() * mVisualizeSourceBufferSize / mVisualizeSourceDecimation;
  if( newElements < 1 )
    newElements = 1;
  double trueDecimation = ( Output.Elements() * mVisualizeSourceBufferSize ) / newElements;
  d.SetName( "Source Signal" )
   .SetElements( newElements )
   .SetType( SignalType::float32 )
   .ElementUnit().SetGain( Output.ElementUnit().Gain() * trueDecimation )
                 .SetRawMax( Output.ElementUnit().RawMax() / trueDecimation );
  mDecimatedSignal.SetProperties( d );
  if( mVisualizeSource )
    mSourceVis.Send( mDecimatedSignal.Properties() );
  mSourceVis.Send( CfgID::Visible, mVisualizeSource );

  mVisualizeTiming = ( Parameter( "VisualizeTiming" ) == 1 );

  bool measureStimulus = ( Parameter( "SignalSourceIP" ) == Parameter( "ApplicationIP" ) );
  SignalProperties p = mTimingSignal.Properties();
  p.SetChannels( measureStimulus ? NumTimingChannels : NumTimingChannels - 1 );
  p.SetName( "Timing" );
  // Roundtrip values are in ms, and we want a range that is twice the value
  // of what we expect for the second signal (the time between subsequent
  // completions of the ADC's Process()).
  p.ValueUnit().SetRawMin( 0 ).SetRawMax( 2 * mBlockDuration )
               .SetOffset( 0 ).SetGain( 1e-3 ).SetSymbol( "s" );
  p.ChannelLabels()[Block] = ":Block";
  p.ChannelLabels()[BlockNom] = ":%20%20nominal";
  p.ChannelLabels()[BlockAvg] = ":%20%20average";
  p.ChannelLabels()[Roundtrip] = ":Roundtrip";
  if( measureStimulus )
    p.ChannelLabels()[Stimulus] = ":Stimulus";
  p.ElementUnit().SetRawMin( 0 ).SetRawMax( 127 )
                 .SetOffset( 0 ).SetGain( mBlockDuration / 1e3 ).SetSymbol( "s" );
  mTimingSignal = GenericSignal( p, GenericSignal::NaN );
  mTimingSignal( BlockNom, 0 ) = mBlockDuration;

  mTimingObserver.Observer().SetWindowLength( static_cast<size_t>( MeasurementUnits::TimeInSampleBlocks( "10s" ) ) );
  mTimingObserver.SetEnabled( mpADC->IsRealTimeSource() || ( OptionalParameter( "EvaluateTiming", 1 ) != 0 ) );

  if( mVisualizeTiming )
  {
    mTimingVis.Send( mTimingSignal.Properties() );
    ColorList colors( NumTimingChannels );
    colors[Block] = RGBColor::White;
    colors[BlockAvg] = RGBColor::Red;
    colors[BlockNom] = RGBColor::Aqua;
    colors[Roundtrip] = RGBColor::LtGray;
    colors[Stimulus] = RGBColor::DkGray;

    mTimingVis.Send( CfgID::ChannelColors, colors )
              .Send( CfgID::ShowBaselines, true )
              .Send( CfgID::AutoScale, "off" );
  }
  mTimingVis.Send( CfgID::Visible, mVisualizeTiming );
  if( mpSourceFilter )
    mpSourceFilter->CallStartRun();
}


void
DataIOFilter::StartRun()
{
  mpADC->CallStartRun();
  mpFileWriter->CallStartRun();

  mStatevectorBuffer = *Statevector;
  mStatevectorBuffer.SetStateValue( "Recording", 0 );

  State( "Recording" ) = 1;
}


void
DataIOFilter::StopRun()
{
  mpADC->CallStopRun();
  mpFileWriter->CallStopRun();

  State( "Recording" ) = 0;

  mTimingSignal( Roundtrip, 0 ) = GenericSignal::NaN;
  if( mTimingSignal.Channels() > Stimulus )
    mTimingSignal( Stimulus, 0 ) = GenericSignal::NaN;
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
  if( mStatevectorBuffer.StateValue( "Recording" ) )
  {
    mpFileWriter->CallWrite( *mpFileWriterInput, mStatevectorBuffer );
    PrecisionTime prevSourceTime = static_cast<PrecisionTime::NumType>( State( "SourceTime" ) ),
                  stimulusTime = static_cast<PrecisionTime::NumType>( State( "StimulusTime" ) );
    mTimingSignal( Roundtrip, 0 ) = PrecisionTime::SignedDiff( functionEntry, prevSourceTime );
    if( mTimingSignal.Channels() > Stimulus )
      mTimingSignal( Stimulus, 0 ) = PrecisionTime::SignedDiff( stimulusTime, prevSourceTime );
  }

  AcquireData();
  Output = mOutputBuffer;
  ResetStates( State::EventKind );
  ProcessBCIEvents();
  mStatevectorBuffer = *Statevector;
  ResetStates( State::StateKind );
}

void
DataIOFilter::Resting()
{
  AcquireData();
}

void
DataIOFilter::Halt()
{
  if( mpADC )
    mpADC->CallHalt();
  if( mpSourceFilter )
    mpSourceFilter->CallHalt();
  if( mpFileWriter )
    mpFileWriter->CallHalt();
}

void
DataIOFilter::AdjustProperties( SignalProperties& p ) const
{
  p.SetChannels( Parameter( "SourceCh" ) )
   .SetElements( Parameter( "SampleBlockSize" ) )
   .SetUpdateRate( Parameter( "SamplingRate" ).InHertz() / Parameter( "SampleBlockSize" ) );
  p.ElementUnit().SetOffset( 0 )
                 .SetGain( 1.0 / Parameter( "SamplingRate" ).InHertz() )
                 .SetSymbol( "s" );
}

void
DataIOFilter::AcquireData()
{
  PrecisionTime prevSourceTime = static_cast<PrecisionTime::NumType>( State( "SourceTime" ) );
  mpADC->CallProcess( mADCOutput, mADCOutput );
  if( !mpADC->SetsSourceTime() )
    State( "SourceTime" ) = PrecisionTime::Now();

  if( mpSourceFilter )
    mpSourceFilter->CallProcess( mADCOutput, *mpFileWriterInput );

  for( int ch = 0; ch < mpFileWriterInput->Channels(); ++ch )
  {
    const PhysicalUnit& u = mpFileWriterInput->Properties().ValueUnit( ch );
    for( int el = 0; el < mOutputBuffer.Elements(); ++el )
      mOutputBuffer( ch, el ) = u.RawToPhysicalValue( (*mpFileWriterInput)( ch, el ) ) / CalibrateTo;
  }

  CopyBlock( mOutputBuffer, mVisSourceBuffer, mBlockCount++ );
  bool doSend = ( mBlockCount == mVisualizeSourceBufferSize );
  if( doSend )
    mBlockCount = 0;
  if( mVisualizeSource && doSend )
  {
    Downsample( mVisSourceBuffer, mDecimatedSignal );
    mSourceVis.Send( mDecimatedSignal );
  }

  if( !mIsFirstBlock )
  {
    PrecisionTime sourceTime = static_cast<PrecisionTime::NumType>( State( "SourceTime" ) );
    mTimingSignal( Block, 0 ) = PrecisionTime::SignedDiff( sourceTime, prevSourceTime );
    mTimingObserver.Observe( mTimingSignal );
    if( mVisualizeTiming )
      mTimingVis.Send( mTimingSignal );
  }
  mIsFirstBlock = false;
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
      throw std_invalid_argument(
        "Trying to set state \"" << name << "\" from an event. "
        "This state was not defined as an event state. "
        "Use BEGIN_EVENT_DEFINITIONS/END_EVENT_DEFINITIONS to define states "
        "as event states."
      );

    bcidbg( 10 ) << "Setting State \"" << name
                 << "\" to " << value
                 << " at offset " << offset
                 << " with duration " << duration
                 << " from event:\n" << mBCIEvents.FrontDescriptor()
                 << endl;

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

// DataIOFilter::TimingObserver
DataIOFilter::TimingObserver::TimingObserver()
: mState( 0 ), mBuffer( 2 ), mObserver( Mean )
{
}

ObserverBase& DataIOFilter::TimingObserver::Observer()
{
  return mObserver;
}

void DataIOFilter::TimingObserver::SetEnabled( bool inEnabled )
{
  mObserver.Clear();
  mState = inEnabled ? 1 : 0;
}

void DataIOFilter::TimingObserver::Observe( GenericSignal& ioSignal )
{
  enum
  {
    enabled = 1,

    rtripApproaching = 2,
    rtripExceeding = 4,
    rtripTooLarge = 6,
    rtrip = rtripApproaching | rtripExceeding | rtripTooLarge,

    blockLow = 8,
    blockHigh = 16,
    block = blockLow | blockHigh,
  };
  if( ioSignal( Block, 0 ) < 0 )
  {
    bciwarn << "Time measurement appears to be unreliable on your system. "
            << "You cannot use BCI2000 time stamps for timing evaluation."
            << endl;
    mState = 0;
  }

  mBuffer[0] = ioSignal( Block, 0 );
  mBuffer[1] = ioSignal( Roundtrip, 0 );
  mObserver.AgeBy( 1 );
  mObserver.Observe( mBuffer );
  VectorPtr mean = mObserver.Mean();
  ioSignal( BlockAvg, 0 ) = mean()[0];
  if( !( mState & enabled ) || mObserver.Age() < mObserver.WindowLength() )
    return;

  double avgBlockDuration = mean()[0] / ioSignal( BlockNom, 0 ),
         avgRoundtrip = mean()[1] / ioSignal( BlockNom, 0 );

  int newState = mState & enabled;

  if( avgRoundtrip >= 1.1 )
    newState |= rtripTooLarge;
  else if( avgRoundtrip >= 1.0 )
    newState |= rtripExceeding;
  else if( avgRoundtrip >= 0.75 )
    newState |= rtripApproaching;

  if( avgBlockDuration > 1.05 )
    newState |= blockHigh;
  else if( avgBlockDuration < 0.95 )
    newState |= blockLow;

  if( (newState & rtrip) > (mState & rtrip) )
    switch( newState & rtrip )
    {
      case rtripTooLarge:
        bcierr << "Roundtrip time consistently exceeds nominal block duration (currently "
               << avgRoundtrip * 100
               << "%)"
               << endl;
        break;
      case rtripExceeding:
        bciwarn << "Roundtrip time exceeds nominal block duration (currently "
               << avgRoundtrip * 100
               << "%)"
               << endl;
        break;
      case rtripApproaching:
        bciwarn << "Roundtrip time approaches nominal block duration (currently "
               << avgRoundtrip * 100
               << "%)"
               << endl;
        break;
    }

  if( (newState & block) != (mState & block) )
    switch( newState & block )
    {
      case blockLow:
      case blockHigh:
        bciwarn << "Average block duration is off by more than 5%.\n"
                << "Nominal: " << ioSignal( BlockNom, 0 ) << "ms, Actual: " << mean()[0] << "ms";
    }

  mState = newState;
}

