////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Output into a GDF data file.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "GDFOutputFormat.h"

#include "GDF.h"
#include "BCIError.h"

using namespace std;
using namespace GDF;

GDFOutputFormat::GDFOutputFormat()
{
}


GDFOutputFormat::~GDFOutputFormat()
{
}


void
GDFOutputFormat::Publish() const
{
  EDFOutputBase::Publish();

  BEGIN_PARAMETER_DEFINITIONS
    "Storage matrix EventCodes= 7 "
      "[Condition                                 GDF%20Event] "
      "TargetCode!=0                              0x0300 " // trial begin
      "TargetCode==0                              0x8300 " // trial end
      "TargetCode==1                              0x030c " // cue up
      "TargetCode==2                              0x0306 " // cue down
      "(ResultCode!=0)&&(TargetCode==ResultCode)  0x0381 " // hit
      "(ResultCode!=0)&&(TargetCode!=ResultCode)  0x0382 " // miss
      "Feedback!=0                                0x030d " // feedback onset
      "// GDF events entered into the event table when associated conditions become true",
  END_PARAMETER_DEFINITIONS
}


void
GDFOutputFormat::Preflight( const SignalProperties& inProperties,
                            const StateVector& inStatevector ) const
{
  EDFOutputBase::Preflight( inProperties, inStatevector );
  switch( inProperties.Type() )
  {
    case SignalType::int16:
    case SignalType::int32:
    case SignalType::float24:
    case SignalType::float32:
      // These are OK
      break;
    default:
      bcierr << inProperties.Type().Name()
             << " data type unsupported for GDF files"
             << endl;
  }
  GenericSignal preflightSignal( inProperties );
  for( int i = 0; i < Parameter( "EventCodes" )->NumRows(); ++i )
    Expression( Parameter( "EventCodes" )( i, "Expression" ) ).Evaluate( &preflightSignal );

  Parameter( "SubjectName" );
  Parameter( "SubjectYearOfBirth" );
  Parameter( "EquipmentID" );
  Parameter( "TechnicianID" );
  Parameter( "LabID" );
  Parameter( "EquipmentID" );
  Parameter( "LabID" );
  Parameter( "TechnicianID" );
  Parameter( "SampleBlockSize" );
  Parameter( "SamplingRate" );
}


void
GDFOutputFormat::Initialize( const SignalProperties& inProperties,
                             const StateVector& inStatevector )
{
  EDFOutputBase::Initialize( inProperties, inStatevector );

  mEvents.clear();
  mEventConditions.clear();
  mEventCodes.clear();
  mPreviousConditionValues.clear();

  GenericSignal initSignal( inProperties );
  for( int i = 0; i < Parameter( "EventCodes" )->NumRows(); ++i )
  {
    mEventConditions.push_back( Expression( Parameter( "EventCodes" )( i, "Condition" ) ) );
    mEventCodes.push_back( Parameter( "EventCodes" )( i, "GDF Event" ) );
    mPreviousConditionValues.push_back( mEventConditions.back().Evaluate( &initSignal ) );
  }
}


void
GDFOutputFormat::StartRun( ostream& os )
{
  EDFOutputBase::StartRun( os );

  time_t now = ::time( NULL );
  struct tm* time = ::localtime( &now );
  ostringstream startdate;
  startdate << setw( 4 ) << setfill( '0' ) << time->tm_year + 1900
            << setw( 2 ) << setfill( '0' ) << time->tm_mon + 1
            << setw( 2 ) << setfill( '0' ) << time->tm_mday
            << setw( 2 ) << setfill( '0' ) << time->tm_hour
            << setw( 2 ) << setfill( '0' ) << time->tm_min
            << setw( 2 ) << setfill( '0' ) << time->tm_sec
            << "00";

  ostringstream patient;
  patient << GDF::EncodedString( Parameter( "SubjectName" ) )
          << " X ";
  switch( int( Parameter( "SubjectSex" ) ) )
  {
    case GDF::unspecified:
      patient << "X";
      break;
    case GDF::male:
      patient << "M";
      break;
    case GDF::female:
      patient << "F";
      break;
  }
  patient << " XX-XXX-" << GDF::EncodedString( Parameter( "SubjectYearOfBirth" ) );

  ostringstream recording;
  recording << GDF::DateTimeToString( now )
            << ' '
            << GDF::EncodedString( Parameter( "LabID" ) )
            << ' '
            << GDF::EncodedString( Parameter( "TechnicianID" ) )
            << ' '
            << GDF::EncodedString( Parameter( "EquipmentID" ) );

  GDF::uint64::ValueType numEquipmentID = 0,
                         numLabID = 0,
                         numTechnicianID = 0;
  istringstream iss;
  iss.str( Parameter( "EquipmentID" ) );
  iss >> numEquipmentID;
  iss.str( Parameter( "LabID" ) );
  iss >> numLabID;
  iss.str( Parameter( "TechnicianID" ) );
  iss >> numTechnicianID;

  PutField< Str<8>            >( os, "GDF 1.25" );
  PutField< Str<80>           >( os, patient.str() );
  PutField< Str<80>           >( os, recording.str() );
  PutField< Str<16>           >( os, startdate.str() );
  PutField< Num<GDF::int64>   >( os, 256 * ( Channels().size() + 1 ) );
  PutField< Num<GDF::uint64>  >( os, numEquipmentID );
  PutField< Num<GDF::uint64>  >( os, numLabID );
  PutField< Num<GDF::uint64>  >( os, numTechnicianID );
  PutField< Str<20>           >( os );
  PutField< Num<GDF::int64>   >( os, -1 );
  PutField< Num<GDF::uint32>  >( os, Parameter( "SampleBlockSize" ) );
  PutField< Num<GDF::uint32>  >( os, Parameter( "SamplingRate" ) );
  PutField< Num<GDF::uint32>  >( os, Channels().size() );

  PutArray< Str<16>           >( os, Channels(), &ChannelInfo::Label );
  PutArray< Str<80>           >( os, Channels(), &ChannelInfo::TransducerType );
  PutArray< Str<8>            >( os, Channels(), &ChannelInfo::PhysicalDimension );
  PutArray< Num<GDF::float64> >( os, Channels(), &ChannelInfo::PhysicalMinimum );
  PutArray< Num<GDF::float64> >( os, Channels(), &ChannelInfo::PhysicalMaximum );
  PutArray< Num<GDF::int64>   >( os, Channels(), &ChannelInfo::DigitalMinimum );
  PutArray< Num<GDF::int64>   >( os, Channels(), &ChannelInfo::DigitalMaximum );
  PutArray< Str<80>           >( os, Channels(), &ChannelInfo::Filtering );
  PutArray< Num<GDF::uint32>  >( os, Channels(), &ChannelInfo::SamplesPerRecord );
  PutArray< Num<GDF::uint32>  >( os, Channels(), &ChannelInfo::DataType );
  PutArray< Str<32>           >( os, Channels() );
}


void
GDFOutputFormat::StopRun( ostream& os )
{
  // Event Table
  if( !mEvents.empty() && os.seekp( 0, ios_base::end ) )
  {
    GDF::uint64::ValueType intSamplingRate = Parameter( "SamplingRate" );
    PutField< Num<GDF::uint8>  >( os, 1 );
    PutField< Num<GDF::uint8>  >( os, intSamplingRate & 0xff );
    PutField< Num<GDF::uint8>  >( os, intSamplingRate >> 8 & 0xff );
    PutField< Num<GDF::uint8>  >( os, intSamplingRate >> 16 & 0xff );
    PutField< Num<GDF::uint32> >( os, mEvents.size() );
    PutArray< Num<GDF::uint32> >( os, mEvents, &EventInfo::SamplePosition );
    PutArray< Num<GDF::uint16> >( os, mEvents, &EventInfo::Code );
  }
  // Number of Records
  const int NumRecFieldPos = 236;
  if( os.seekp( NumRecFieldPos ) )
    PutField< Num<GDF::int64> >( os, NumRecords() );

  EDFOutputBase::StopRun( os );
}


void
GDFOutputFormat::Write( ostream& os,
                        const GenericSignal& inSignal,
                        const StateVector&   inStatevector )
{
  EDFOutputBase::Write( os, inSignal, inStatevector );

  for( size_t i = 0; i < mEventConditions.size(); ++i )
  {
    bool curValue = mEventConditions[ i ].Evaluate( &inSignal );
    if( curValue && curValue != mPreviousConditionValues[ i ] )
    {
      EventInfo event;
      event.SamplePosition = NumRecords() * Parameter( "SampleBlockSize" );
      event.Code = mEventCodes[ i ];
      mEvents.push_back( event );
    }
    mPreviousConditionValues[ i ] = curValue;
  }
}

