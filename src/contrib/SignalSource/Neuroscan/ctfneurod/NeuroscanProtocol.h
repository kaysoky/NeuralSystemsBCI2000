/////////////////////////////////////////////////////////////////////////////////////////////
//
//  File:        NeuroscanProtocol.h
//
//  Author:      juergen.mellinger@uni-tuebingen.de
//
//  Date:        Jul 27, 2004
//
//  Description: A std::iostream based implementation of the Neuroscan Acquire protocol
//               intended to be more platform independent than the original headers are.
//               Remaining requirements for the target system are 4-Byte-ints and IEEE floats.
//
///////////////////////////////////////////////////////////////////////////////////////////////
#ifndef NEUROSCAN_H
#define NEUROSCAN_H

#ifndef DBG_LEVEL
# define DBG_LEVEL 1
#endif

#include <iostream>
#include <cstring>
#include <cfloat>
#include <climits>

#if UINT_MAX != 0xFFFFFFFF
# error This code requires 4-byte ints.
#endif

#if USHRT_MAX != 0xFFFF
# error This code requires 2-byte shorts.
#endif

#if UCHAR_MAX != 0xFF
# error This code requires 1-byte chars.
#endif

#if FLT_RADIX != 2 || FLT_MANT_DIG != 24 || FLT_MAX_EXP != 128 || FLT_MIN_EXP != -125
# error This code requires IEEE floats.
#endif

enum
{
  GeneralControlCode = 1,
    RequestForVersion = 1,
    ClosingUp,
    ServerDisconnected,
  ServerControlCode = 2,
    StartAcquisition = 1,
    StopAcquisition,
    StartImpedance,
    ChangeSetup,
    DCCorrection,
  ClientControlCode = 3,
    RequestEDFHeader = 1,
    RequestAstFile,
    RequestStartData,
    RequestStopData,
    RequestBasicInfo,

  SetupFile = 1,
    NeuroscanASTFormat = 1,

  DataType_InfoBlock = 1,
    InfoType_Version = 1,
    InfoType_EdfHeader,
    InfoType_BasicInfo,

  DataType_EegData = 2,
    DataTypeRaw16bit = 1,
    DataTypeRaw32bit,
};


namespace BigEndian
{
  template<typename T>
  static void put( std::ostream& os, T t )
  {
    for( int i = sizeof( T ) - 1; i >= 0; --i )
      os.put( ( t >> ( i * 8 ) ) & 0xff );
  }
  template<>
  static void put( std::ostream& os, float f )
  {
    put( os, *reinterpret_cast<unsigned int*>( &f ) );
  }

  template<typename T>
  static void get( std::istream& is, T& t )
  {
    t = 0;
    for( size_t i = 0; i < sizeof( T ); ++i )
    {
      char c;
      is.get( c );
      t <<= 8;
      t |= c;
    }
  }
  template<>
  static void get( std::istream& is, float& f )
  {
    get( is, reinterpret_cast<unsigned int&>( f ) );
  }
};

namespace LittleEndian
{
  template<typename T>
  static void put( std::ostream& os, T t )
  {
    for( size_t i = 0; i < sizeof( T ); ++i )
    {
      os.put( t & 0xff );
      t >>= 8;
    }
  }
  template<>
  static void put( std::ostream& os, float f )
  {
    put( os, *reinterpret_cast<unsigned int*>( &f ) );
  }

  template<typename T>
  static void get( std::istream& is, T& t )
  {
    t = 0;
    for( size_t i = 0; i < sizeof( T ); ++i )
    {
      char c;
      is.get( c );
      t |= T( c ) << ( i * 8 );
    }
  }
  template<>
  static void get( std::istream& is, float& f )
  {
    get( is, reinterpret_cast<unsigned int&>( f ) );
  }
};

class NscPacketHeader
{
  public:
    NscPacketHeader()
    : mId( 0 ),
      mCode( 0 ),
      mValue( 0 ),
      mDataSize( 0 )
    {
    };
    NscPacketHeader( int id, int code, int value, int dataSize )
    : mId( id ),
      mCode( code ),
      mValue( value ),
      mDataSize( dataSize )
    {
    };
    bool operator<( const NscPacketHeader& n ) const
    {
      return ::memcmp( this, &n, sizeof( *this ) ) < 0;
    };
    std::istream& ReadBinary( std::istream& is )
    {
      BigEndian::get( is, mId );
      BigEndian::get( is, mCode );
      BigEndian::get( is, mValue );
      BigEndian::get( is, mDataSize );
#if( DBG_LEVEL > 0 )
      std::cout << "-> ";
      WriteToStream( std::cout );
      std::cout << std::endl;
#endif
      return is;
    }
    std::ostream& WriteBinary( std::ostream& os ) const
    {
      BigEndian::put( os, mId );
      BigEndian::put( os, mCode );
      BigEndian::put( os, mValue );
      BigEndian::put( os, mDataSize );
#if( DBG_LEVEL > 0 )
# if( DBG_LEVEL < 2 )
      if( mId != 'DATA' || mCode != DataType_EegData )
# endif
#endif
      {
        std::cout << "<- ";
        WriteToStream( std::cout );
        std::cout << std::endl;
      }
      return os;
    }
    std::istream& ReadFromStream( std::istream& is )
    {
      BigEndian::get( is, mId );
      char delim;
      return is >> delim >> mCode >> delim >> mValue >> delim >> mDataSize;
    }

#define CONSIDER(x) case x: match = true; os << #x " : ";
    std::ostream& WriteToStream( std::ostream& os ) const
    {
      BigEndian::put( os, mId );
      os << " : ";
      bool match = false;
      switch( mId )
      {
        case 'CTRL':
	  switch( mCode )
	  {
	    CONSIDER( GeneralControlCode )
	    switch( mValue )
	    {
	      CONSIDER( RequestForVersion ) break;
	      CONSIDER( ClosingUp ) break;
	      CONSIDER( ServerDisconnected ) break;
	    }
	    break;
	    CONSIDER( ServerControlCode )
	    switch( mValue )
	    {
	      CONSIDER( StartAcquisition ) break;
	      CONSIDER( StopAcquisition ) break;
	      CONSIDER( StartImpedance ) break;
	      CONSIDER( ChangeSetup ) break;
	      CONSIDER( DCCorrection ) break;
	    }
	    break;
	    CONSIDER( ClientControlCode )
	    switch( mValue )
	    {
	      CONSIDER( RequestEDFHeader ) break;
	      CONSIDER( RequestAstFile ) break;
	      CONSIDER( RequestStartData ) break;
	      CONSIDER( RequestStopData ) break;
	      CONSIDER( RequestBasicInfo ) break;
	    }
	    break;
	  }
	  break;
 
      case 'FILE':
	switch( mCode )
        {
          CONSIDER( SetupFile )
          switch( mValue )
          {
            CONSIDER( NeuroscanASTFormat ) break;
          }
          break;
	}
	break;

      case 'DATA':
	switch( mCode )
	{
          CONSIDER( DataType_InfoBlock )
          switch( mValue )
          {
            CONSIDER( InfoType_Version ) break;
            CONSIDER( InfoType_EdfHeader ) break;
            CONSIDER( InfoType_BasicInfo ) break;
          }
          break;
          CONSIDER( DataType_EegData )
          switch( mValue )
          {
            CONSIDER( DataTypeRaw16bit ) break;
            CONSIDER( DataTypeRaw32bit ) break;
          }
          break;
        }
	break;
      }
      if( !match )
	os << mCode << ':' << mValue << ':';
      return os << mDataSize << " data bytes";
    }
#undef CONSIDER
    
  private:
    int    mId;
    short  mCode,
           mValue;
    int    mDataSize;
};

inline
std::ostream&
operator<<( std::ostream& os, const NscPacketHeader& h )
{
  return h.WriteToStream( os );
}

class NscBasicInfo
{
  static const int cDataSize = 7 * 4;
  public:
    NscBasicInfo()
    : mSizeField( cDataSize ),
      mEEGChannels( 0 ),
      mEventChannels( 0 ),
      mSamplesInBlock( 0 ),
      mSamplingRate( 0 ),
      mDataDepth( 0 ),
      mResolution( 1.0 )
    {
    }
    NscBasicInfo( int EEGChannels, int eventChannels, int samplesInBlock,
                  int samplingRate, int dataDepth, float resolution )
    : mSizeField( cDataSize ),
      mEEGChannels( EEGChannels ),
      mEventChannels( eventChannels ),
      mSamplesInBlock( samplesInBlock ),
      mSamplingRate( samplingRate ),
      mDataDepth( dataDepth ),
      mResolution( resolution )
    {
    }

    int   EEGChannels() const    { return mEEGChannels; }
    int   EventChannels() const  { return mEventChannels; }
    int   SamplesInBlock() const { return mSamplesInBlock; }
    int   SamplingRate() const   { return mSamplingRate; }
    int   DataDepth() const      { return mDataDepth; }
    float Resolution() const     { return mResolution; }

    std::ostream& WriteBinary( std::ostream& os )
    {
      LittleEndian::put( os, mSizeField );
      LittleEndian::put( os, mEEGChannels );
      LittleEndian::put( os, mEventChannels );
      LittleEndian::put( os, mSamplesInBlock );
      LittleEndian::put( os, mSamplingRate );
      LittleEndian::put( os, mDataDepth );
      LittleEndian::put( os, mResolution );
      return os;
    }
    std::istream& ReadBinary( std::istream& is )
    {
      LittleEndian::get( is, mSizeField );
      LittleEndian::get( is, mEEGChannels );
      LittleEndian::get( is, mEventChannels );
      LittleEndian::get( is, mSamplesInBlock );
      LittleEndian::get( is, mSamplingRate );
      LittleEndian::get( is, mDataDepth );
      LittleEndian::get( is, mResolution );
      return is;
    }
    std::ostream& WriteToStream( std::ostream& os ) const
    {
      return os << mEEGChannels    << " Signal channels, \t"
                << mEventChannels  << " Event channels, \t"
                << mSamplesInBlock << " Samples per block, \t"
                << mSamplingRate   << " Samples per second, \t"
                << mDataDepth      << " Bytes per sample, \t"
                << mResolution     << " Physical units per LSB\n";
    }
  private:
    int   mSizeField,      // Size of structure, used for version control
          mEEGChannels,    // Number of EEG channels
          mEventChannels,  // Number of event channels
          mSamplesInBlock, // Samples in block
          mSamplingRate,   // Sampling rate (in Hz)
          mDataDepth;      // 2 for "short", 4 for "int" type of data
    float mResolution;     // Resolution for LSB
};

inline
std::ostream&
operator<<( std::ostream& os, const NscBasicInfo& h )
{
  return h.WriteToStream( os );
}

#endif // NEUROSCAN_H
