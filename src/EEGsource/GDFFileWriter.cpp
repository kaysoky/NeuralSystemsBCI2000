////////////////////////////////////////////////////////////////////////////////
// $Id$
// File: GDFFileWriter.cpp
//
// Date: Feb 3, 2006
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A filter that stores data into a EDF or GDF data file.
//
// $Log$
// Revision 1.1  2006/02/18 12:11:00  mellinger
// Support for EDF and GDF data formats.
//
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "GDFFileWriter.h"

#include "GDF.h"
#include "UBCIError.h"

using namespace std;
using namespace GDF;

// File writer filters must have a position string greater than
// that of the DataIOFilter.
RegisterFilter( GDFFileWriter, 1 );


GDFFileWriter::GDFFileWriter()
{
}


GDFFileWriter::~GDFFileWriter()
{
}


void
GDFFileWriter::Publish() const
{
  EDFFileWriterBase::Publish();

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
GDFFileWriter::Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const
{
  EDFFileWriterBase::Preflight( Input, Output );
  switch( Input.Type() )
  {
    case SignalType::int16:
    case SignalType::int32:
    case SignalType::float24:
    case SignalType::float32:
      // These are OK
      break;
    default:
      bcierr << Input.Type().Name()
             << " data type unsupported for GDF files"
             << endl;
  }
  GenericSignal preflightSignal( Input );
  for( size_t i = 0; i < Parameter( "EventCodes" )->GetNumRows(); ++i )
    Expression( Parameter( "EventCodes" )( i, "Expression" ) ).Evaluate( &preflightSignal );
}


void
GDFFileWriter::Initialize2( const SignalProperties& Input,
                            const SignalProperties& Output )
{
  EDFFileWriterBase::Initialize2( Input, Output );

  mEvents.clear();
  mEventConditions.clear();
  mEventCodes.clear();
  mPreviousConditionValues.clear();

  GenericSignal initSignal( Input );
  for( size_t i = 0; i < Parameter( "EventCodes" )->GetNumRows(); ++i )
  {
    mEventConditions.push_back( Expression( Parameter( "EventCodes" )( i, "Condition" ) ) );
    mEventCodes.push_back( Parameter( "EventCodes" )( i, "GDF Event" ) );
    mPreviousConditionValues.push_back( mEventConditions.back().Evaluate( &initSignal ) );
  }
}


void
GDFFileWriter::StartRun()
{
  EDFFileWriterBase::StartRun();

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

  uint64::ValueType numEquipmentID = 0,
                         numLabID = 0,
                         numTechnicianID = 0;
  istringstream iss;
  iss.str( string( Parameter( "EquipmentID" ) ) );
  iss >> numEquipmentID;
  iss.str( string( Parameter( "LabID" ) ) );
  iss >> numLabID;
  iss.str( string( Parameter( "TechnicianID" ) ) );
  iss >> numTechnicianID;

  ostream& os = OutputStream();

  PutField< Str<8>       >( os, "GDF 1.25" );
  PutField< Str<80>      >( os, patient.str() );
  PutField< Str<80>      >( os, recording.str() );
  PutField< Str<16>      >( os, startdate.str() );
  PutField< Num<int64>   >( os, 256 * ( Channels().size() + 1 ) );
  PutField< Num<uint64>  >( os, numEquipmentID );
  PutField< Num<uint64>  >( os, numLabID );
  PutField< Num<uint64>  >( os, numTechnicianID );
  PutField< Str<20>      >( os );
  PutField< Num<int64>   >( os, -1 );
  PutField< Num<uint32>  >( os, Parameter( "SampleBlockSize" ) );
  PutField< Num<uint32>  >( os, Parameter( "SamplingRate" ) );
  PutField< Num<uint32>  >( os, Channels().size() );

  PutArray< Str<16>      >( os, Channels(), &ChannelInfo::Label );
  PutArray< Str<80>      >( os, Channels(), &ChannelInfo::TransducerType );
  PutArray< Str<8>       >( os, Channels(), &ChannelInfo::PhysicalDimension );
  PutArray< Num<float64> >( os, Channels(), &ChannelInfo::PhysicalMinimum );
  PutArray< Num<float64> >( os, Channels(), &ChannelInfo::PhysicalMaximum );
  PutArray< Num<int64>   >( os, Channels(), &ChannelInfo::DigitalMinimum );
  PutArray< Num<int64>   >( os, Channels(), &ChannelInfo::DigitalMaximum );
  PutArray< Str<80>      >( os, Channels(), &ChannelInfo::Filtering );
  PutArray< Num<uint32>  >( os, Channels(), &ChannelInfo::SamplesPerRecord );
  PutArray< Num<uint32>  >( os, Channels(), &ChannelInfo::DataType );
  PutArray< Str<32>      >( os, Channels() );
}


void
GDFFileWriter::StopRun()
{
  ostream& os = OutputStream();

  // Event Table
  if( !mEvents.empty() && os.seekp( 0, ios_base::end ) )
  {
    uint64::ValueType intSamplingRate = Parameter( "SamplingRate" );
    PutField< Num<uint8>  >( os, 1 );
    PutField< Num<uint8>  >( os, intSamplingRate & 0xff );
    PutField< Num<uint8>  >( os, intSamplingRate >> 8 & 0xff );
    PutField< Num<uint8>  >( os, intSamplingRate >> 16 & 0xff );
    PutField< Num<uint32> >( os, mEvents.size() );
    PutArray< Num<uint32> >( os, mEvents, &EventInfo::SamplePosition );
    PutArray< Num<uint16> >( os, mEvents, &EventInfo::Code );
  }
  // Number of Records
  const int NumRecFieldPos = 236;
  if( os.seekp( NumRecFieldPos ) )
    PutField< Num<int64> >( os, NumRecords() );

  EDFFileWriterBase::StopRun();
}


void
GDFFileWriter::Write( const GenericSignal& inSignal,
                      const STATEVECTOR&   inStatevector )
{
  EDFFileWriterBase::Write( inSignal, inStatevector );
  
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

