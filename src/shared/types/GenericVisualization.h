////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: Classes that represent BCI2000 visualization messages.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef GENERIC_VISUALIZATION_H
#define GENERIC_VISUALIZATION_H

#include <iostream>
#include <string>
#include <sstream>
#include "defines.h"
#include "GenericSignal.h"
#include "SignalProperties.h"
#include "BitmapImage.h"

class VisBase
{
  public:
    enum { InvalidID = 0xff };

    VisBase() {}
    VisBase( const std::string& inSourceID )
    : mSourceID( inSourceID ) {}
    virtual ~VisBase() {}

    const std::string& SourceID() const { return mSourceID; }

    std::istream& ReadBinary( std::istream& );
    std::ostream& WriteBinary( std::ostream& ) const;
    virtual void ReadBinarySelf( std::istream& ) = 0;
    virtual void WriteBinarySelf( std::ostream& ) const = 0;

  private:
    std::string mSourceID;
};

class VisCfg : public VisBase
{
  public:
    VisCfg()
    : mCfgID( InvalidID ) {}
    VisCfg( const std::string& inSourceID, int inCfgID, const std::string& inCfgValue )
    : VisBase( inSourceID ), mCfgID( inCfgID ), mCfgValue( inCfgValue ) {}
    virtual void ReadBinarySelf( std::istream& );
    virtual void WriteBinarySelf( std::ostream& ) const;

    int CfgID() const { return mCfgID; }
    const std::string& CfgValue() const { return mCfgValue; }

  private:
    int mCfgID;
    std::string mCfgValue;
};

class VisMemo : public VisBase
{
  public:
    VisMemo() {}
    VisMemo( const std::string& inSourceID, const std::string& inMemo )
    : VisBase( inSourceID ), mMemo( inMemo ) {}
    virtual void ReadBinarySelf( std::istream& );
    virtual void WriteBinarySelf( std::ostream& ) const;

    const std::string& MemoText() const { return mMemo; }

  private:
    std::string mMemo;
};

class VisSignal : public VisBase
{
  public:
    VisSignal() {}
    VisSignal( const std::string& inSourceID, const GenericSignal& inSignal )
    : VisBase( inSourceID ), mSignal( inSignal ) {}
    VisSignal( const GenericSignal& inSignal )
    : mSignal( inSignal ) {}
    virtual void ReadBinarySelf( std::istream& );
    virtual void WriteBinarySelf( std::ostream& ) const;

    const GenericSignal& Signal() const   { return mSignal; }
    operator const GenericSignal&() const { return mSignal; }

  private:
    GenericSignal mSignal;
};

class VisSignalProperties : public VisBase
{
  public:
    VisSignalProperties() {}
    VisSignalProperties( const std::string& inSourceID, const ::SignalProperties& inSignalProperties )
    : VisBase( inSourceID ), mSignalProperties( inSignalProperties ) {}
    VisSignalProperties( const ::SignalProperties& inSignalProperties )
    : mSignalProperties( inSignalProperties ) {}
    virtual void ReadBinarySelf( std::istream& );
    virtual void WriteBinarySelf( std::ostream& ) const;

    const ::SignalProperties& SignalProperties() const { return mSignalProperties; }
    operator const ::SignalProperties&() const         { return mSignalProperties; }

  private:
    ::SignalProperties mSignalProperties;
};

class VisBitmap : public VisBase
{
  public:
    VisBitmap() {}
    VisBitmap( const std::string& inSourceID, const ::BitmapImage& inBitmap )
    : VisBase( inSourceID ), mBitmap( inBitmap ) {}
    VisBitmap( const ::BitmapImage& inBitmap )
    : mBitmap( inBitmap ) {}
    virtual void ReadBinarySelf( std::istream& );
    virtual void WriteBinarySelf( std::ostream& ) const;

    const ::BitmapImage& BitmapImage() const { return mBitmap; }
    operator const ::BitmapImage&() const    { return mBitmap; }

  private:
    ::BitmapImage mBitmap;
};

class GenericVisualization : public std::ostream
{
  public:
    GenericVisualization()
    : std::ostream( 0 ), mBuf( this )
    { this->init( &mBuf ); }

    GenericVisualization( int inSourceID )
    : std::ostream( 0 ), mBuf( this )
    {
      this->init( &mBuf );
      std::ostringstream oss;
      oss << inSourceID;
      mSourceID = oss.str();
    }

    GenericVisualization( const std::string& inSourceID )
    : std::ostream( 0 ), mSourceID( inSourceID ), mBuf( this )
    { this->init( &mBuf ); }

    GenericVisualization( const GenericVisualization& v )
    : std::ostream( 0 ), mSourceID( v.mSourceID ), mBuf( this )
    {
      this->init( &mBuf );
      mBuf.str( v.mBuf.str() );
    }

    GenericVisualization& operator=( const GenericVisualization& v )
    {
      if( this != &v )
      {
        mSourceID = v.mSourceID;
        mBuf.str( v.mBuf.str() );
      }
      return *this;
    }

    ~GenericVisualization() { mBuf.str( "" ); }

    // Setters and Getters.
    GenericVisualization& SetSourceID( const std::string& inSourceID )
                          { mSourceID = inSourceID; return *this; }
    const std::string&    SourceID() const
                          { return mSourceID; }

    template<typename T> GenericVisualization& Send( CfgID::CfgID cfgID, const T& cfgValue );
    GenericVisualization& Send( const std::string& memo );
    GenericVisualization& Send( const GenericSignal& );
    GenericVisualization& Send( const SignalProperties& );
    GenericVisualization& Send( const BitmapImage& );

  private:
    GenericVisualization& SendCfgString( CfgID::CfgID, const std::string& );

    std::string  mSourceID;
    class VisStringbuf : public std::stringbuf
    {
      public:
        VisStringbuf( GenericVisualization* inParent )
        : std::stringbuf( std::ios_base::out ),
          mpParent( inParent )
        {}
      private:
        virtual int sync();
        GenericVisualization* mpParent;
    } mBuf;
};

template<typename T>
GenericVisualization&
GenericVisualization::Send( CfgID::CfgID cfgID, const T& cfgValue )
{
  std::ostringstream oss;
  oss << cfgValue;
  return SendCfgString( cfgID, oss.str() );
}

#endif // GENERIC_VISUALIZATION_H