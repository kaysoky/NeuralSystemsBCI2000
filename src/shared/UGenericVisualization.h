////////////////////////////////////////////////////////////////////////////////
//
// File:    UGenericVisualization.h
//
// Authors: Gerwin Schalk, Juergen Mellinger
//
// Changes: Apr 15, 2003, juergen.mellinger@uni-tuebingen.de:
//          Reworked graph display double buffering scheme.
//          Untangled window painting from content changes.
//          Introduced clipping to reduce the amount of time spent blitting
//          graphics data.
//
//          May 27, 2003, jm:
//          Separated VISUAL and VISCFGLIST into a file belonging to
//          the operator module.
//
//          May 28, 2004, jm:
//          Introduced VisMemo, VisSignal, and VisCfg objects to allow for
//          centralization of message processing in the MessageHandler class.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef UGenericVisualizationH
#define UGenericVisualizationH

#include <sstream>
#include "defines.h"
#include "UGenericSignal.h"

class VisBase
{
  public:
    VisBase()
    : mSourceID( 0 ) {}
    VisBase( int inSourceID )
    : mSourceID( inSourceID ) {}
    virtual ~VisBase() {}

    class std::istream& ReadBinary( class std::istream& );
    class std::ostream& WriteBinary( class std::ostream& ) const;
    virtual void ReadBinarySelf( class std::istream& ) = 0;
    virtual void WriteBinarySelf( class std::ostream& ) const = 0;
    
    int GetSourceID() const { return mSourceID; }

  private:
    int mSourceID;
};

class VisCfg : public VisBase
{
  enum { invalidID = -1 };
  public:
    VisCfg()
    : mCfgID( invalidID ) {}
    VisCfg( int inSourceID, int inCfgID, const std::string& inCfgValue )
    : VisBase( inSourceID ), mCfgID( inCfgID ), mCfgValue( inCfgValue ) {}
    virtual void ReadBinarySelf( class std::istream& );
    virtual void WriteBinarySelf( class std::ostream& ) const;

    int GetCfgID() const { return mCfgID; }
    const std::string& GetCfgValue() const { return mCfgValue; }

  private:
    int mCfgID;
    std::string mCfgValue;
};

class VisMemo : public VisBase
{
  public:
    VisMemo() {}
    VisMemo( int inSourceID, const std::string& inMemo )
    : VisBase( inSourceID ), mMemo( inMemo ) {}
    virtual void ReadBinarySelf( class std::istream& );
    virtual void WriteBinarySelf( class std::ostream& ) const;

    const std::string& GetMemoText() const { return mMemo; }

  private:
    std::string mMemo;
};

class VisSignal : public VisBase
{
  public:
    VisSignal() {}
    VisSignal( int inSourceID, const GenericSignal& inSignal )
    : VisBase( inSourceID ), mSignal( inSignal ) {}
    VisSignal( const GenericSignal& inSignal )
    : VisBase( 0 ), mSignal( inSignal ) {}
    virtual void ReadBinarySelf( class std::istream& );
    virtual void WriteBinarySelf( class std::ostream& ) const;

    const GenericSignal& GetSignal() const { return mSignal; }
    operator const GenericSignal&() const { return mSignal; }

  private:
    GenericSignal mSignal;
};

class GenericVisualization : public std::ostream
{
  enum { invalidID = -1 };
  public:
    GenericVisualization()
    : std::ostream( 0 ), mSourceID( invalidID ), mBuf( this )
    { this->init( &mBuf ); }

    GenericVisualization( int sourceID )
    : std::ostream( 0 ), mSourceID( sourceID ), mBuf( this )
    { this->init( &mBuf ); }

    GenericVisualization( int sourceID, int /*visType*/ )
    : std::ostream( 0 ), mSourceID( sourceID ), mBuf( this )
    { this->init( &mBuf ); }

    ~GenericVisualization() {}

    // Setters and Getters.
    void SetSourceID( int sourceID ) { mSourceID = sourceID; }
    int  GetSourceID() const         { return mSourceID; }

    template<typename T> bool Send( CFGID::CFGID cfgID, const T& cfgValue );
    template<>           bool Send( CFGID::CFGID cfgID, const std::string& );
    bool Send( const std::string& memoString );
    bool Send( const GenericSignal* signal );
    bool Send( const GenericSignal& signal ) { return Send( &signal ); }

    bool Send2Operator( const GenericSignal* signal )
             { return Send( signal ); }
    bool SendMemo2Operator( const char* memoString )
             { return Send( memoString ); }
    bool SendCfg2Operator( int sourceID, int cfgID, const char* cfgString );
    bool SendCfg2Operator( int sourceID, int cfgID, int cfgValue );

  private:
    int  mSourceID;
    class VisStringbuf : public std::stringbuf
    {
      public:
        VisStringbuf( GenericVisualization* parent )
        : std::stringbuf( std::ios_base::out ),
          mpParent( parent )
        {}
      private:
        virtual int sync();
        GenericVisualization* mpParent;
    } mBuf;
};

template<>
bool
GenericVisualization::Send( CFGID::CFGID inCfgID, const std::string& inCfgString );

template<typename T>
bool
GenericVisualization::Send( CFGID::CFGID cfgID, const T& cfgValue )
{
  std::ostringstream oss;
  oss << cfgValue;
  return Send<std::string>( cfgID, oss.str() );
}

#endif // UGenericVisualizationH

