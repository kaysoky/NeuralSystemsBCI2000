/******************************************************************************
 * Program:   BCI2000                                                         *
 * Module:    BCI2000DATA.cpp                                                 *
 * Comment:   This unit provides support for the BCI2000 data format          *
 * Version:   0.02                                                            *
 * Author:    Gerwin Schalk                                                   *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.01 - 04/17/2001 - First try                                             *
 * V0.02 - 05/03/2001 - Added support for multiple runs                       *
 * V0.03 - 06/26/2003 - Added the Value() member to access calibrated         *
 *                      data, jm                                              *
 ******************************************************************************/
#include "PCHIncludes.h"
#pragma hdrstop

#include "UBCI2000Data.h"
#include "defines.h"

#include <fstream>

using namespace std;

// **************************************************************************
// Function:   BCI2000DATA
// Purpose:    The constructor for the BCI2000DATA object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
BCI2000DATA::BCI2000DATA()
: mpStatevector( NULL ),
  mpBuffer( NULL ),
  mfpReadValueBinary( NULL )
{
  ResetTotal();
}

// **************************************************************************
// Function:   Reset
// Purpose:    Resets file related data members to a defined state.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void BCI2000DATA::Reset()
{
  mInitialized = false;

  mParamlist.Clear();
  mStatelist.Clear();
  delete mpStatevector;
  mpStatevector = NULL;

  mFilename = "";
  mFile.close();
  mFile.clear();
  delete mpBuffer;
  mpBuffer = NULL;
  mBufferSize = 0;
  mBufferBegin = 0;
  mBufferEnd = 0;

  mFileFormatVersion = "n/a";
  mChannels = 0;
  mHeaderLength = 0;
  mStatevectorLength = 0;
  mSamplingRate = 1;
  mSignalType = SignalType::int16;
  mDataSize = 2;
  mfpReadValueBinary = IsBigEndianMachine() ? ReadValueInt16_BigEndian : ReadValueInt16_LittleEndian;
  mSampleNumber = 0;
}

// **************************************************************************
// Function:   Reset
// Purpose:    Resets session related data members to a defined state.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void BCI2000DATA::ResetTotal()
{
  Reset();
  mInitializedTotal = false;
  mSampleNumberTotal = 0;
  mSampleNumberRun.clear();
}

// **************************************************************************
// Function:   ~BCI2000DATA
// Purpose:    The destructor for the BCI2000DATA object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
BCI2000DATA::~BCI2000DATA()
{
  delete mpStatevector;
  delete mpBuffer;
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This method initializes the object by reading in
//             the header and creating a list of all parameters and states
// Parameters: filename - name of the file of interest
//             buf_size - size of input buffer to use
// Returns:    BCI2000ERR_NOERR           ... no error
//             BCI2000ERR_FILENOTFOUND    ... file not found
//             BCI2000ERR_MALFORMEDHEADER ... not the correct file type ?
//             BCI2000ERR_NOBUFMEM        ... could not allocate buffer
// **************************************************************************
int
BCI2000DATA::Initialize( const char* inNewFilename, int inBufSize )
{
  Reset();

  mFile.open( inNewFilename, ios::in | ios::binary );
  if( !mFile.is_open() )
    return BCI2000ERR_FILENOTFOUND;

  mFilename = inNewFilename;
  int ret = ReadHeader();
  if( ret != BCI2000ERR_NOERR )
    return ret;
  CalculateSampleNumber();

  mBufferSize = inBufSize;
  mpBuffer = new char[ mBufferSize ];
  mBufferBegin = 0;
  mBufferEnd = 0;

  const float defaultOffset = 0.0;
  mSourceOffsets.clear();
  if( mParamlist.Exists( "SourceChOffset" ) )
  {
    const PARAM& SourceChOffset = mParamlist[ "SourceChOffset" ];
    for( size_t i = 0; i < SourceChOffset.GetNumValues(); ++i )
      mSourceOffsets.push_back( ::atof( SourceChOffset.GetValue( i ) ) );
  }
  mSourceOffsets.resize( mChannels, defaultOffset );

  const float defaultGain = 0.033;
  mSourceGains.clear();
  if( mParamlist.Exists( "SourceChGain" ) )
  {
    const PARAM& SourceChGain = mParamlist[ "SourceChGain" ];
    for( size_t i = 0; i < SourceChGain.GetNumValues(); ++i )
      mSourceGains.push_back( ::atof( SourceChGain.GetValue( i ) ) );
  }
  mSourceGains.resize( mChannels, defaultGain );

  mInitialized = true;
  return BCI2000ERR_NOERR;
}


// **************************************************************************
// Function:   CalculateSampleNumber
// Purpose:    Calculates the number of samples in the file
//             Assumes that ReadHeader() has been called before
//             if there is an unforeseen problem, number of samples is set to 0
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
BCI2000DATA::CalculateSampleNumber()
{
  mSampleNumber = 0;
  if( mFile.is_open() )
  {
    streampos curPos = mFile.tellg();
    mFile.seekg( 0, ios_base::end );
    long dataSize = static_cast<long>( mFile.tellg() ) - mHeaderLength;
    mFile.seekg( curPos, ios_base::beg );
    mSampleNumber = dataSize / ( mDataSize * mChannels + mStatevectorLength );
  }
}

// **************************************************************************
// Function:   InitializeTotal
// Purpose:    This method gathers initial information about
//             the sample numbers in each run and finally initializes
//             the object to the provided single file
//             while Initialize() does not assume filename structure,
//             all functions related to the whole dataset do assume the structure
//             NAMExx.dat (with xx being the run number)
// Parameters: filename - name of the file of interest
//             buf_size - size of input buffer to use
// Returns:    BCI2000ERR_NOERR           ... no error
//             BCI2000ERR_FILENOTFOUND    ... file not found
//             BCI2000ERR_MALFORMEDHEADER ... not the correct file type ?
//             BCI2000ERR_NOBUFMEM        ... could not allocate buffer
// **************************************************************************
int
BCI2000DATA::InitializeTotal( const char* new_filename, int buf_size )
{
  ResetTotal();

  // initialize everything to this file first
  int ret = Initialize( new_filename, buf_size );
  if( ret != BCI2000ERR_NOERR )
    return ret;

  // find out about the first and the last run
  int firstrun = GetFirstRunNumber(),
      lastrun = GetLastRunNumber();

  // now, count the samples in each run
  mSampleNumberRun.resize( 1, 0 ); // Indices are one-based.
  for( int cur_run = firstrun; cur_run <= lastrun; ++cur_run )
  {
    SetRun( cur_run );
    mSampleNumberRun.push_back( GetNumSamples() );
  }

  // finally, initialize everything to this file again
  Initialize( new_filename, buf_size );
  mInitializedTotal = true;

  return BCI2000ERR_NOERR;
}

// **************************************************************************
// Function:   GetFirstRunNumber
// Purpose:    Determines the first run in the data
// Parameters: N/A
// Returns:    first run in that data set, or
//             0, if there was a problem
//             (e.g., not yet initialized, file names don't follow the
//             nameRxx.dat, etc.)
// **************************************************************************
int
BCI2000DATA::GetFirstRunNumber() const
{
FILE *fp;
char cur_run[1024], cur_filename[1024], prefix[1024];
int  pos, idx, cur_runnr, runnr, firstrun;

 if (!mInitialized) return(0);

 // if the last 4 characters in the filename are not ".dat", then the file name
 // does not follow the BCI2000 filename conventions
 idx=strlen(mFilename.c_str())-4;
 if (idx < 0) return(0);
 pos=stricmp(mFilename.c_str()+idx, ".DAT");
 if (pos != 0) return(0);

 // get the position of the first character of the run number
 idx-=2;
 if (idx < 0) return(0);
 strncpy(cur_run, mFilename.c_str()+idx, 2);
 cur_run[2]=0;
 cur_runnr=atoi(cur_run);
 strncpy(prefix, mFilename.c_str(), idx);
 prefix[idx]=0;

 // go through all runs and figure out, which run is the first
 // i.e., which file is the first one to exist
 runnr=0;
 firstrun=0;
 while (runnr <= cur_runnr)
  {
  sprintf(cur_filename, "%s%02d.dat", prefix, runnr);
  fp=fopen(cur_filename, "rb");
  if (fp)
     {
     fclose(fp);
     firstrun=runnr;
     break;
     }
  runnr++;
  }

 return(firstrun);
}

// **************************************************************************
// Function:   GetLastRunNumber
// Purpose:    Determines the last run in the data
// Parameters: N/A
// Returns:    last run in that data set, or
//             0, if there was a problem
//             (e.g., not yet initialized, file names don't follow the
//             nameRxx.dat, etc.)
// **************************************************************************
int
BCI2000DATA::GetLastRunNumber() const
{
FILE *fp;
char cur_run[1024], cur_filename[1024], prefix[1024];
int  pos, idx, cur_runnr, runnr, lastrun;

 if (!mInitialized) return(0);

 // if the last 4 characters in the filename are not ".dat", then the file name
 // does not follow the BCI2000 filename conventions
 idx=strlen(mFilename.c_str())-4;
 if (idx < 0) return(0);
 pos=stricmp(mFilename.c_str()+idx, ".DAT");
 if (pos != 0) return(0);

 // get the position of the first character of the run number
 idx-=2;
 if (idx < 0) return(0);
 strncpy(cur_run, mFilename.c_str()+idx, 2);
 cur_run[2]=0;
 cur_runnr=atoi(cur_run);
 strncpy(prefix, mFilename.c_str(), idx);
 prefix[idx]=0;

 // go through all runs and figure out, which run is the last
 // i.e., which file is the last one to exist
 runnr=cur_runnr;
 while (true)
  {
  sprintf(cur_filename, "%s%02d.dat", prefix, runnr);
  fp=fopen(cur_filename, "rb");
  if (!fp)
     {
     lastrun=runnr-1;
     break;
     }
  else
     fclose(fp);
  runnr++;
  }

 return(lastrun);
}


// **************************************************************************
// Function:   SetRun
// Purpose:    Sets the run; assumes that Initialize() has been called before
// Parameters: N/A
// Returns:    1 ... OK
//             0 ... a problem (i.e., run not found, could not be initialized, etc.)
// **************************************************************************
int
BCI2000DATA::SetRun( int runnr )
{
FILE *fp;
char cur_filename[1024], prefix[1024];
int  pos, idx, cur_runnr, lastrun, res;

 if (!mInitialized) return(0);

 // if the last 4 characters in the filename are not ".dat", then the file name
 // does not follow the BCI2000 filename conventions
 idx=strlen(mFilename.c_str())-4;
 if (idx < 0) return(0);
 pos=stricmp(mFilename.c_str()+idx, ".DAT");
 if (pos != 0) return(0);

 // get the position of the first character of the run number
 idx-=2;
 if (idx < 0) return(0);
 strncpy(prefix, mFilename.c_str(), idx);
 prefix[idx]=0;

 // now, create the file name for the specified run
 sprintf(cur_filename, "%s%02d.dat", prefix, runnr);
 fp=fopen(cur_filename, "rb");

 // could not open file
 if (!fp) return(0);
 fclose(fp);

 // now, that that worked, initalize the object
 res=Initialize(cur_filename);
 if (res != BCI2000ERR_NOERR)
    return(0);

 return(1);
}


// **************************************************************************
// Function:   ReadHeader
// Purpose:    This method reads the header of a BCI2000 data file
// Parameters: N/A
// Returns:    BCI2000ERR_NOERR           ... no error
//             BCI2000ERR_MALFORMEDHEADER ... not the correct file type ?
// **************************************************************************
int
BCI2000DATA::ReadHeader()
{
  // read the first line and do consistency checks
  string line, element;
  if( !getline( mFile, line, '\n' ) )
    return BCI2000ERR_MALFORMEDHEADER;
  istringstream linestream( line );
  linestream >> element;
  if( element == "BCI2000V=" )
    linestream >> mFileFormatVersion >> element;
  else
    mFileFormatVersion = "1.0";
  if( element != "HeaderLen=" )
    return BCI2000ERR_MALFORMEDHEADER;

  linestream >> mHeaderLength >> element >> mChannels;
  if( element != "SourceCh=" )
    return BCI2000ERR_MALFORMEDHEADER;

  linestream >> element >> mStatevectorLength;
  if( element != "StatevectorLen=" )
    return BCI2000ERR_MALFORMEDHEADER;

  mSignalType = SignalType::int16;
  if( linestream >> element )
  {
    if( element != "DataFormat=" )
      return BCI2000ERR_MALFORMEDHEADER;
    if( !( linestream >> mSignalType ) )
      return BCI2000ERR_MALFORMEDHEADER;
  }
  mDataSize = mSignalType.Size();
  switch( mSignalType )
  {
    case SignalType::int16:
      mfpReadValueBinary 
	    = IsBigEndianMachine() ? ReadValueInt16_BigEndian : ReadValueInt16_LittleEndian;
      break;
    case SignalType::int32:
      mfpReadValueBinary 
	    = IsBigEndianMachine() ? ReadValueInt32_BigEndian : ReadValueInt32_LittleEndian;
      break;
    case SignalType::float32:
      mfpReadValueBinary
		= IsBigEndianMachine() ? ReadValueFloat32_BigEndian : ReadValueFloat32_LittleEndian;
      break;
  }

  // now go through the header and read all parameters and states
  getline( mFile >> ws, line, '\n' );
  if( line.find( "[ State Vector Definition ]" ) != 0 )
    return BCI2000ERR_MALFORMEDHEADER;
  while( getline( mFile, line, '\n' )
         && line.find( "[ Parameter Definition ]" ) == line.npos )
    mStatelist.Add( line.c_str() );
  while( getline( mFile, line, '\n' )
         && line != "" && line != "\r" )
    mParamlist.Add( line );

  // build statevector using specified positions
  mpStatevector = new STATEVECTOR( mStatelist, true );
  if( !mParamlist.Exists( "SamplingRate" ) )
    return BCI2000ERR_MALFORMEDHEADER;
  mSamplingRate = ::atoi( mParamlist[ "SamplingRate" ].GetValue() );

  return mFile ? BCI2000ERR_NOERR : BCI2000ERR_MALFORMEDHEADER;
}

// **************************************************************************
// Function:   DetermineRunNumber
// Purpose:    returns the run number for a particular sample number in the whole dataset
//             assumes that InitializeTotal() has been called before
// Parameters: sample - sample number
// Returns:    run number, or 0 on error
// **************************************************************************
int
BCI2000DATA::DetermineRunNumber(unsigned long sample)
{
int           firstrun, lastrun, cur_run, runnr;
unsigned long samplesleft;

 if (!InitializedTotal()) return(0);

 firstrun=GetFirstRunNumber();
 lastrun=GetLastRunNumber();

 samplesleft=sample;
 runnr=0;
 // go through all runs and determine the run number for this sample
 for (cur_run=firstrun; cur_run<=lastrun; cur_run++)
  {
  samplesleft-=mSampleNumberRun[cur_run];
  if (samplesleft < 0)
     {
     runnr=cur_run;
     break;
     }
  }

 return(runnr);
}


// **************************************************************************
// Function:   ReadValueTotal
// Purpose:    Returns the sample value in the dataset for a given sample and channel number
//             assumes that InitializeTotal() has been called before
// Parameters: channel - channel number
//             sample - sample number
// Returns:    value requested
// **************************************************************************
/*
short BCI2000DATA::ReadValueTotal(int channel, ULONG sample)
{
 if (GetCurrentRun() != DetermineRunNumber())
    SetRun(G

 return(retval);
} */

// **************************************************************************
// Function:   Value
// Purpose:    Returns the sample value in the .dat file for a given sample
//             and channel number that is, the sample in the current run,
//             in units of 1e-6 V, i.e. honouring the calibration parameters
//             present in the file.
// Parameters: channel - channel number
//             sample - sample number
// Returns:    value requested
// **************************************************************************
GenericSignal::value_type
BCI2000DATA::Value( int channel, unsigned long sample )
{
  return ( ReadValue( channel, sample ) - mSourceOffsets[ channel ] ) * mSourceGains[ channel ];
}

// **************************************************************************
// Function:   ReadValue
// Purpose:    Returns the sample value in the .raw file for a given sample and channel number
//             that is, the sample in the current run
// Parameters: channel - channel number
//             sample - sample number
// Returns:    value requested
// **************************************************************************
GenericSignal::value_type
BCI2000DATA::ReadValue( int inChannel, unsigned long inSample )
{
  return mfpReadValueBinary( BufferSample( inSample ) + mDataSize * inChannel );
}

// Determine whether the code runs on a big endian machine.
bool
BCI2000DATA::IsBigEndianMachine()
{
  uint8 t[] = { 0, 1 };
  return *reinterpret_cast<uint16*>( t ) == 1;
}

// Read a little endian value on little endian machines.
GenericSignal::value_type
BCI2000DATA::ReadValueInt16_LittleEndian( const char* p )
{
  return *reinterpret_cast<const sint16*>( p );
}

GenericSignal::value_type
BCI2000DATA::ReadValueInt32_LittleEndian( const char* p )
{
  return *reinterpret_cast<const sint32*>( p );
}

GenericSignal::value_type
BCI2000DATA::ReadValueFloat32_LittleEndian( const char* p )
{
  return *reinterpret_cast<const float32*>( p );
}

// Read a little endian value on big endian machines.
GenericSignal::value_type
BCI2000DATA::ReadValueInt16_BigEndian( const char* p )
{
  const uint8* bytes = reinterpret_cast<const uint8*>( p );
  uint8 value[] = { bytes[1], bytes[0] };
  return *reinterpret_cast<const sint16*>( value );
}

GenericSignal::value_type
BCI2000DATA::ReadValueInt32_BigEndian( const char* p )
{
  const uint8* bytes = reinterpret_cast<const uint8*>( p );
  uint8 value[] = { bytes[3], bytes[2], bytes[1], bytes[0] };
  return *reinterpret_cast<const sint32*>( value );
}

GenericSignal::value_type
BCI2000DATA::ReadValueFloat32_BigEndian( const char* p )
{
  const uint8* bytes = reinterpret_cast<const uint8*>( p );
  uint8 value[] = { bytes[3], bytes[2], bytes[1], bytes[0] };
  return *reinterpret_cast<const float32*>( value );
}
// **************************************************************************
// Function:   ReadValue
// Purpose:    Returns the value in the .raw file for a given sample, channel number, and run number
// Parameters: channel - channel number
//             sample - sample number
//             run - run number
// Returns:    value requested
//             it returns 0 on error
// **************************************************************************
GenericSignal::value_type
BCI2000DATA::ReadValue( int inChannel, unsigned long inSample, int inRun )
{
  if( SetRun( inRun ) == 0 )
    return 0;

  return ReadValue( inChannel, inSample );
}

// **************************************************************************
// Function:   ReadStateVector
// Purpose:    reads the statevector for a given sample
//             the results are in a statevector pointed to by "statevector"
// Parameters: sample - sample number
// Returns:    N/A
// **************************************************************************
void
BCI2000DATA::ReadStateVector( unsigned long inSample )
{
  ::memcpy( mpStatevector->Data(),
            BufferSample( inSample ) + mDataSize * GetNumChannels(),
            GetStateVectorLength() );
}

// **************************************************************************
// Function:   ReadStateVector
// Purpose:    reads the statevector for a given sample and run number
//             the results are in a statevector pointed to by "statevector"
// Parameters: sample - sample number
// Returns:    N/A
// **************************************************************************
void
BCI2000DATA::ReadStateVector( unsigned long inSample, int inRun )
{
  SetRun( inRun );
  ReadStateVector( inSample );
}

const char*
BCI2000DATA::BufferSample( unsigned long inSample )
{
  if( inSample >= GetNumSamples() )
    throw "BCI2000DATA::BufferSample: Sample position exceeds file size";
  long filepos = GetHeaderLength()
               + inSample * ( mDataSize * GetNumChannels() + GetStateVectorLength() );
  if( filepos < mBufferBegin || filepos + mDataSize * GetNumChannels() + GetStateVectorLength() >= mBufferEnd )
  {
    if( !mFile.seekg( filepos, ios_base::beg ) )
      throw "BCI2000DATA::BufferSample: Could not seek to sample position";

    mBufferBegin = filepos;
    mBufferEnd = mBufferBegin;
    while( mFile && ( mBufferEnd - mBufferBegin < GetStateVectorLength() ) )
    {
      mFile.read( mpBuffer + mBufferEnd - mBufferBegin,
        mBufferSize - ( mBufferEnd - mBufferBegin ) );
      mBufferEnd += mFile.gcount();
    }
    mFile.clear();
  }
  return mpBuffer + filepos - mBufferBegin;
}

