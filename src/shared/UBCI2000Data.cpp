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

//---------------------------------------------------------------------------
#include "PCHIncludes.h"
#pragma hdrstop

#include "UBCI2000Data.h"

#include "UBCIError.h"
#include <stdio.h>
#include <fstream>

using namespace std;
//---------------------------------------------------------------------------

#pragma package(smart_init)


// **************************************************************************
// Function:   BCI2000DATA
// Purpose:    The constructor for the BCI2000DATA object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
BCI2000DATA::BCI2000DATA()
: mpBuf_mem( NULL ),
  mpStatevector( NULL )
{
  InitializeInstance();
}

// **************************************************************************
// Function:   InitializeInstance
// Purpose:    Initializes the BCI2000DATA instance to a defined state.
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void BCI2000DATA::InitializeInstance()
{
  mBuf_mem_start = 0;
  mSample_number = 0;
  //! mSample_number_run.clear();
  mSample_number_total = 0;
  mBuffer_size = 0;
  mFilename = "";
  mInitialized = false;
  mInitializedtotal = false;
  mChannels = 0;
  mHeaderlength = 0;
  mStatevectorlength = 0;
  mSample_freq = 0;
}

// **************************************************************************
// Function:   ~BCI2000DATA
// Purpose:    The destructor for the BCI2000DATA object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
BCI2000DATA::~BCI2000DATA()
{
  if( mpBuf_mem )
    ::free( mpBuf_mem );
  delete mpStatevector;
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
BCI2000DATA::Initialize( const char* inNew_filename, int inBuf_size )
{
  InitializeInstance();

  mBuffer_size = inBuf_size;
  mFilename = inNew_filename;
  mpBuf_mem = static_cast<__int16*>( ::realloc( mpBuf_mem, static_cast<size_t>( inBuf_size ) ) );

  InvalidateBuffer();

  mParamlist.ClearParamList();
  mStatelist.ClearStateList();

  int ret = ReadHeader();
  if( ret != BCI2000ERR_NOERR )
    return ret;

  mInitialized = true;
  CalculateSampleNumber();

  const float defaultOffset = 0.0;
  mSourceOffsets.clear();
  const PARAM* sourceChOffset = mParamlist.GetParamPtr( "SourceChOffset" );
  if( sourceChOffset != NULL )
    for( size_t i = 0; i < sourceChOffset->GetNumValues(); ++i )
      mSourceOffsets.push_back( ::atof( sourceChOffset->GetValue( i ) ) );
  mSourceOffsets.resize( mChannels, defaultOffset );

  const float defaultGain = 0.033;
  mSourceGains.clear();
  const PARAM* mSourceChGain = mParamlist.GetParamPtr( "SourceChGain" );
  if( mSourceChGain != NULL )
    for( size_t i = 0; i < mSourceChGain->GetNumValues(); ++i )
      mSourceGains.push_back( ::atof( mSourceChGain->GetValue( i ) ) );
  mSourceGains.resize( mChannels, defaultGain );

  return BCI2000ERR_NOERR;
}


// **************************************************************************
// Function:   CalculateSampleNumber
// Purpose:    Calculates the number of samples in the file
//             Assumes that Initialize() has been called before
//             if there is an unforeseen problem, number of samples is set to 0
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void
BCI2000DATA::CalculateSampleNumber()
{
  mSample_number = 0;
  if( !mInitialized )
    return;

  FILE* fp = ::fopen( mFilename.c_str(), "rb" );
  if( !fp )
    return;

  ::fseek( fp, 0L, SEEK_END );
  size_t data_size = ::ftell( fp ) - GetHeaderLength();
  mSample_number = data_size / ( sizeof( __int16 ) * GetNumChannels() + static_cast<size_t>( GetStateVectorLength() ) );
  ::fclose( fp );
}


// **************************************************************************
// Function:   GetNumSamples
// Purpose:    Returns the number of samples in this run
// Parameters: N/A
// Returns:    number of samples in this run, or
//             0, if not initialized
// **************************************************************************
unsigned long
BCI2000DATA::GetNumSamples()
{
  return mSample_number;
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
  mInitializedtotal = false;

  // initialize everything to this file first
  int ret = Initialize( new_filename, buf_size );
  if( ret != BCI2000ERR_NOERR )
    return ret;

  // find out about the first and the last run
  int firstrun = GetFirstRunNumber(),
      lastrun = GetLastRunNumber();

  // now, count the samples in each run
  mSample_number_run.resize( 1, 0 ); // Indices are one-based.
  for( int cur_run = firstrun; cur_run <= lastrun; ++cur_run )
  {
    SetRun( cur_run );
    mSample_number_run.push_back( GetNumSamples() );
  }

  // finally, initialize everything to this file again
  Initialize( new_filename, buf_size );
  mInitializedtotal = true;

  return BCI2000ERR_NOERR;
}


// **************************************************************************
// Function:   GetNumSamplesTotal
// Purpose:    Returns the number of samples in this dataset
// Parameters: N/A
// Returns:    number of samples in this dataset, or
//             0, if there was an error (e.g., not initialized)
// **************************************************************************
unsigned long
BCI2000DATA::GetNumSamplesTotal() const
{
  if( !mInitializedtotal )
    return 0;

  return mSample_number_total;
}


// **************************************************************************
// Function:   Initialized()
// Purpose:    determines, whether the object has been correctly initialized
// Parameters: N/A
// Returns:    true or false
// **************************************************************************
bool
BCI2000DATA::Initialized() const
{
  return mInitialized;
}


// **************************************************************************
// Function:   Initialized()
// Purpose:    determines, whether the object has been correctly initialized for the whole dataset
// Parameters: N/A
// Returns:    true or false
// **************************************************************************
bool
BCI2000DATA::InitializedTotal() const
{
  return mInitializedtotal;
}


// **************************************************************************
// Function:   GetParamListPtr
// Purpose:    Returns a pointer to the list of parameters
// Parameters: N/A
// Returns:    pointer to the list of parameters
//             NULL, if not initialized
// **************************************************************************
const PARAMLIST*
BCI2000DATA::GetParamListPtr() const
{
  if( !mInitialized )
    return NULL;

  return &mParamlist;
}


// **************************************************************************
// Function:   GetStateVectorPtr
// Purpose:    Returns a pointer to the list of states
// Parameters: N/A
// Returns:    pointer to the statevector
//             NULL, if not initialized
// **************************************************************************
const STATEVECTOR*
BCI2000DATA::GetStateVectorPtr() const
{
  if( !mInitialized )
    return NULL;

  return mpStatevector;
}


// **************************************************************************
// Function:   GetStateListPtr
// Purpose:    Returns a pointer to the list of states
// Parameters: N/A
// Returns:    pointer to the list of states
//             NULL, if not initialized
// **************************************************************************
const STATELIST*
BCI2000DATA::GetStateListPtr() const
{
  if( !mInitialized )
    return NULL;

  return &mStatelist;
}


// **************************************************************************
// Function:   GetHeaderLength
// Purpose:    Returns the length of the BCI2000 header in bytes
// Parameters: N/A
// Returns:    length of the header, or
//             0, if not initialized
// **************************************************************************
int
BCI2000DATA::GetHeaderLength() const
{
  if( !mInitialized )
    return 0;

  return mHeaderlength;
}


// **************************************************************************
// Function:   GetStateVectorLength
// Purpose:    Returns the length of the BCI2000 state vector in bytes
// Parameters: N/A
// Returns:    length of the state vector, or
//             0, if not initialized
// **************************************************************************
int
BCI2000DATA::GetStateVectorLength() const
{
  if( !mInitialized )
    return 0;

  return mStatevectorlength;
}


// **************************************************************************
// Function:   GetNumChannels
// Purpose:    Returns the number of channels in the BCI2000 data file
// Parameters: N/A
// Returns:    number of channels in the file, or
//             0, if not initialized
// **************************************************************************
int
BCI2000DATA::GetNumChannels() const
{
  if( !mInitialized )
    return 0;

  return mChannels;
}



// **************************************************************************
// Function:   GetSampleFrequency
// Purpose:    Returns the sampling frequency of the data
// Parameters: N/A
// Returns:    sampling frequency, or
//             0, if not initialized
// **************************************************************************
int
BCI2000DATA::GetSampleFrequency() const
{
  if( !mInitialized )
    return 0;

  return mSample_freq;
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
 res=Initialize(cur_filename, mBuffer_size);
 if (res != BCI2000ERR_NOERR)
    return(0);

 return(1);
}


// **************************************************************************
// Function:   ReadHeader
// Purpose:    This method reads the header of a BCI2000 data file
// Parameters: N/A
// Returns:    BCI2000ERR_NOERR           ... no error
//             BCI2000ERR_FILENOTFOUND    ... file not found
//             BCI2000ERR_MALFORMEDHEADER ... not the correct file type ?
// **************************************************************************
int
BCI2000DATA::ReadHeader()
{
  mStatelist.ClearStateList();
  mParamlist.clear();
  mHeaderlength = 0;
  mChannels = 0;
  mSample_freq = 0;
  mStatevectorlength = 0;
  delete mpStatevector;
  mpStatevector = NULL;

  ifstream file( mFilename.c_str(), ios::in | ios::binary );
  if( !file.is_open() )
    return BCI2000ERR_FILENOTFOUND;

  // read the first line and do consistency checks
  string element;
  file >> element >> mHeaderlength;
  if( element != "HeaderLen=" )
    return BCI2000ERR_MALFORMEDHEADER;

  file >> element >> mChannels;
  if( element != "SourceCh=" )
    return BCI2000ERR_MALFORMEDHEADER;

  file >> element >> mStatevectorlength;
  if( element != "StatevectorLen=" )
    return BCI2000ERR_MALFORMEDHEADER;

  // now go through the header and read all parameters and states
  string line;
  getline( file >> ws, line, '\n' );
  if( line.find( "[ State Vector Definition ]" ) != 0 )
    return BCI2000ERR_MALFORMEDHEADER;
  while( getline( file, line, '\n' )
         && line.find( "[ Parameter Definition ]" ) == line.npos )
    mStatelist.AddState2List( line.c_str() );
  while( getline( file, line, '\n' )
         && line != "" && line != "\r" )
    mParamlist.AddParameter2List( line.c_str() );

  // build statevector using specified positions
  mpStatevector = new STATEVECTOR( &mStatelist, true );
  PARAM* param = mParamlist.GetParamPtr( "SamplingRate" );
  if( param == NULL )
    return BCI2000ERR_MALFORMEDHEADER;
  mSample_freq = ::atoi( param->GetValue() );

  return file ? BCI2000ERR_NOERR : BCI2000ERR_MALFORMEDHEADER;
}

// **************************************************************************
// Function:   InvalidateBuffer
// Purpose:    invalidates the buffer that is currently in memory, so that
//             the next operation will then retrieve the values directly from disc
//             (this is done in case the disc file is being changed and not the buffer)
// **************************************************************************
//---------------------------------------------------------------------------
void
BCI2000DATA::InvalidateBuffer()
{
  mBuf_mem_start = - 2 * mBuffer_size;
}

// **************************************************************************
// Function:   DetermineRunNumber
// Purpose:    returns the run number for a particular sample number in the whole dataset
//             assumes that InitializeTotal() has been called before
// Parameters: sample - sample number
// Returns:    run number, or 0 on error
// **************************************************************************
int
BCI2000DATA::DetermineRunNumber(ULONG sample)
{
int     firstrun, lastrun, cur_run, runnr;
ULONG   samplesleft;

 if (!InitializedTotal()) return(0);

 firstrun=GetFirstRunNumber();
 lastrun=GetLastRunNumber();

 samplesleft=sample;
 runnr=0;
 // go through all runs and determine the run number for this sample
 for (cur_run=firstrun; cur_run<=lastrun; cur_run++)
  {
  samplesleft-=mSample_number_run[cur_run];
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
float
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
short
BCI2000DATA::ReadValue( int channel, unsigned long sample )
{
FILE   *fp;
long   file_ptr;
short  retval;
char   *cur_bufptr;

 file_ptr=GetHeaderLength()+(ULONG)channel*2+sample*(2*(ULONG)GetNumChannels()+GetStateVectorLength());
 if ((file_ptr < mBuf_mem_start) || (file_ptr >= (mBuf_mem_start+mBuffer_size-2))) // the -2 is because it could be that the first byte of the sample is in the buffer, but the second isn't; thus, "just to make sure", reload buffer a little sooner
    {
    fp=fopen(mFilename.c_str(), "rb");
    if (fp)
       {
       fseek(fp, (long)file_ptr, SEEK_SET);
       memset(mpBuf_mem, 0, mBuffer_size);                 // clear buffer memory first (in case that if we are at the end of the file, we don't get garbage
       fread(mpBuf_mem, 1, mBuffer_size, fp);
       fclose(fp);
       }
    mBuf_mem_start=file_ptr;
    }

 cur_bufptr=(char *)&mpBuf_mem[0];
 memcpy((void *)&retval, (const void *)&cur_bufptr[file_ptr-mBuf_mem_start], 2);
 return(retval);
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
short
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
FILE   *fp;
long   file_ptr;
char   *cur_bufptr;

 file_ptr=GetHeaderLength()+(ULONG)GetNumChannels()*2+inSample*(2*(ULONG)GetNumChannels()+GetStateVectorLength());
 if ((file_ptr < mBuf_mem_start) || (file_ptr+GetStateVectorLength() >= mBuf_mem_start+mBuffer_size))
    {
    fp=fopen(mFilename.c_str(), "rb");
    if (fp)
       {
       fseek(fp, (long)file_ptr, SEEK_SET);
       memset(mpBuf_mem, 0, mBuffer_size);                 // clear buffer memory first (in case that if we are at the end of the file, we don't get garbage
       fread(mpBuf_mem, 1, mBuffer_size, fp);
       fclose(fp);
       }
    mBuf_mem_start=file_ptr;
    }

 cur_bufptr=(char *)&mpBuf_mem[0];
 memcpy(mpStatevector->GetStateVectorPtr(), (const void *)&cur_bufptr[file_ptr-mBuf_mem_start], GetStateVectorLength());
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


#if 0
// **************************************************************************
// Function:   get_next_string
// Purpose:    gets the next delimited item in a string
// Parameters: buf ... source text buffer
//             start_idx ... the index into the string to start searching
//             dest ... buffer for the output string
// Returns:    N/A
// **************************************************************************
void BCI2000DATA::get_next_string(const char *buf, int *start_idx, char *dest) const
{
int     idx;

idx=*start_idx;
while ((buf[idx] != ',') && (buf[idx] != ' ') && (buf[idx] != 0x0D) && (buf[idx] != 0x0A) && (buf[idx] != 0x00) && (idx-*start_idx < 255))
 idx++;

strncpy(dest, (const char *)&buf[*start_idx], idx-*start_idx);
dest[idx-*start_idx]=0x00;

while (((buf[idx] == ',') || (buf[idx] == ' ')) && (idx-*start_idx < 2048))
 idx++;

*start_idx=idx;
}
#endif


