////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class hierarchy descending from VisBase that represents 
//   various types of BCI2000 visualization messages, and a GenericVisualization
//   class that represents a channel for sending visualization messages.
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
#ifndef GENERIC_VISUALIZATION_H
#define GENERIC_VISUALIZATION_H

#include <iostream>
#include <string>
#include <sstream>
#include "CfgID.h"
#include "VisID.h"
#include "GenericSignal.h"
#include "SignalProperties.h"
#include "BitmapImage.h"

class OSMutex;

#define BACK_COMPAT 1
#ifdef TODO
# error Remove BACK_COMPAT condition when VisID/SourceID duplicity has been resolved
#endif // TODO

class VisBase
{
  public:
    enum { InvalidID = 0xff };

    VisBase() {}
    VisBase( const ::VisID& inVisID )
    : mVisID( inVisID ) {}
    virtual ~VisBase() {}

#ifdef BACK_COMPAT
    const ::VisID& SourceID() const { return mVisID; }
#endif // BACK_COMPAT
    const ::VisID& VisID() const { return mVisID; }

    std::istream& ReadBinary( std::istream& );
    std::ostream& WriteBinary( std::ostream& ) const;

  private:
    virtual void ReadBinarySelf( std::istream& ) = 0;
    virtual void WriteBinarySelf( std::ostream& ) const = 0;

  private:
    ::VisID mVisID;
};

class VisCfg : public VisBase
{
  public:
    VisCfg()
    : mCfgID( InvalidID ) {}
    VisCfg( const ::VisID& inVisID, int inCfgID, const std::string& inCfgValue )
    : VisBase( inVisID ), mCfgID( inCfgID ), mCfgValue( inCfgValue ) {}

    int CfgID() const { return mCfgID; }
    const std::string& CfgValue() const { return mCfgValue; }

  private:
    virtual void ReadBinarySelf( std::istream& );
    virtual void WriteBinarySelf( std::ostream& ) const;

  private:
    int mCfgID;
    std::string mCfgValue;
};

class VisMemo : public VisBase
{
  public:
    VisMemo() {}
    VisMemo( const ::VisID& inVisID, const std::string& inMemo )
    : VisBase( inVisID ), mMemo( inMemo ) {}

    const std::string& MemoText() const { return mMemo; }

  private:
    virtual void ReadBinarySelf( std::istream& );
    virtual void WriteBinarySelf( std::ostream& ) const;

  private:
    std::string mMemo;
};

class VisSignal : public VisBase
{
  public:
    VisSignal() {}
    VisSignal( const ::VisID& inVisID, const GenericSignal& inSignal )
    : VisBase( inVisID ), mSignal( inSignal ) {}
    VisSignal( const GenericSignal& inSignal )
    : mSignal( inSignal ) {}

    const GenericSignal& Signal() const   { return mSignal; }
    operator const GenericSignal&() const { return mSignal; }

  private:
    virtual void ReadBinarySelf( std::istream& );
    virtual void WriteBinarySelf( std::ostream& ) const;

  private:
    GenericSignal mSignal;
};

class VisSignalProperties : public VisBase
{
  public:
    VisSignalProperties() {}
    VisSignalProperties( const ::VisID& inVisID, const ::SignalProperties& inSignalProperties )
    : VisBase( inVisID ), mSignalProperties( inSignalProperties ) {}
    VisSignalProperties( const ::SignalProperties& inSignalProperties )
    : mSignalProperties( inSignalProperties ) {}

    const ::SignalProperties& SignalProperties() const { return mSignalProperties; }
    operator const ::SignalProperties&() const         { return mSignalProperties; }

  private:
    virtual void ReadBinarySelf( std::istream& );
    virtual void WriteBinarySelf( std::ostream& ) const;

  private:
    ::SignalProperties mSignalProperties;
};

class VisBitmap : public VisBase
{
  public:
    VisBitmap() {}
    VisBitmap( const ::VisID& inVisID, const ::BitmapImage& inBitmap )
    : VisBase( inVisID ), mBitmap( inBitmap ) {}
    VisBitmap( const ::BitmapImage& inBitmap )
    : mBitmap( inBitmap ) {}

    const ::BitmapImage& BitmapImage() const { return mBitmap; }
    operator const ::BitmapImage&() const    { return mBitmap; }

  private:
    virtual void ReadBinarySelf( std::istream& );
    virtual void WriteBinarySelf( std::ostream& ) const;

  private:
    ::BitmapImage mBitmap;
};

class GenericVisualization : public std::ostream
{
  public:
    GenericVisualization()
    : std::ostream( 0 ), mBuf( this )
    { this->init( &mBuf ); }

    explicit GenericVisualization( int inVisID )
    : std::ostream( 0 ), mBuf( this )
    {
      this->init( &mBuf );
      std::ostringstream oss;
      oss << inVisID;
      mVisID = oss.str();
    }

    explicit GenericVisualization( const ::VisID& inVisID )
    : std::ostream( 0 ), mVisID( inVisID ), mBuf( this )
    { this->init( &mBuf ); }

    GenericVisualization( const GenericVisualization& v )
    : std::ostream( 0 ), mVisID( v.mVisID ), mBuf( this )
    {
      this->init( &mBuf );
      mBuf.str( v.mBuf.str() );
    }

    GenericVisualization& operator=( const GenericVisualization& v )
    {
      if( this != &v )
      {
        mVisID = v.mVisID;
        mBuf.str( v.mBuf.str() );
      }
      return *this;
    }

    ~GenericVisualization() { mBuf.str( "" ); }

    // Setters and Getters.
#ifdef BACK_COMPAT
    GenericVisualization& SetSourceID( const ::VisID& inVisID )
                          { mVisID = inVisID; return *this; }
    const ::VisID&        SourceID() const
                          { return mVisID; }
#endif // BACK_COMPAT
    GenericVisualization& SetVisID( const ::VisID& inVisID )
                          { mVisID = inVisID; return *this; }
    const ::VisID&        VisID() const
                          { return mVisID; }

    template<typename T> GenericVisualization& Send( CfgID cfgID, const T& cfgValue );
    GenericVisualization& Send( const std::string& memo );
    GenericVisualization& Send( const GenericSignal& );
    GenericVisualization& Send( const SignalProperties& );
    GenericVisualization& Send( const BitmapImage& );

    static void SetOutputStream( std::ostream* pStream, const OSMutex* pLock = NULL )
                { spOutputStream = pStream; spOutputLock = pLock; }

  private:
    GenericVisualization& SendCfgString( CfgID, const std::string& );
    template<typename T> GenericVisualization& SendObject( const T& );

    ::VisID  mVisID;
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

    static std::ostream* spOutputStream;
    static const OSMutex* spOutputLock;
};

class BitmapVisualization : public GenericVisualization
{
  public:
    BitmapVisualization() {}
    
    explicit BitmapVisualization( int inVisID )
    : GenericVisualization( inVisID ) {}

    explicit BitmapVisualization( const ::VisID& inVisID )
    : GenericVisualization( inVisID ) {}

    BitmapVisualization( const BitmapVisualization& v )
    : GenericVisualization( v ) {}

    void SendReferenceFrame( const BitmapImage& );
    void SendDifferenceFrame( const BitmapImage& );

  private:
    BitmapImage mImageBuffer;
};

template<typename T>
GenericVisualization&
GenericVisualization::Send( CfgID cfgID, const T& cfgValue )
{
  std::ostringstream oss;
  oss << cfgValue;
  return SendCfgString( cfgID, oss.str() );
}

#endif // GENERIC_VISUALIZATION_H

