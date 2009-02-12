////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Declarations for stream symbols related to error handling.
//              To report an error, write e.g.
//               bcierr << "My error message" << endl;
//              For an informational message, write
//               bciout << "My info message" << endl;
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef BCI_ERROR_H
#define BCI_ERROR_H

#include <sstream>

// Context info added to output.
// This is overridden with the context information
// set via SetContext() if that is non-empty.
#ifdef _MSC_VER
# define CONTEXT_   __FUNCTION__
#else
# define CONTEXT_   __FUNC__
#endif // _MSC_VER

// Declaration of user symbols with stream syntax (must be macros to include debug info):
#define bcierr      bcierr__( CONTEXT_ )
#define bciout      bciout__( CONTEXT_ )
#define bcidbg      bcidbg__( CONTEXT_ )( 1 )
#define bcierr_     bcierr__( typeid( *this ).name() )
#define bciout_     bciout__( typeid( *this ).name() )

class EnvironmentBase;
class CoreModule;

namespace BCIError
{
  // Stream message handling actions for bcierr/bciout.
  void DebugMessage( const std::string& );
  void PlainMessage( const std::string& );
  void Warning( const std::string& );
  void ConfigurationError( const std::string& );
  void RuntimeError( const std::string& );
  void LogicError( const std::string& );

  class OutStream : public std::ostream
  {
   friend class ::EnvironmentBase;
   friend class ::CoreModule;

   public:
    OutStream()
      : std::ostream( 0 )
      { this->init( &mBuf ); }

    OutStream& operator()( const char* );
    OutStream& operator()(); // in case CONTEXT_ is defined empty
    // Specifying a debug level with a message will result in filtering out all
    // messages above the global debug level variable.
    OutStream& operator()( int debugLevel )
      { return Debug( debugLevel ); }
    OutStream& Debug( int debugLevel );

    int Flushes()
      { return mBuf.Flushes(); }
    void Clear()
      { clear(); }
    void clear()
      { std::ostream::clear(); mBuf.Clear(); }
    void Reset()
      { std::ostream::clear(); mBuf.Reset(); }

   private:
    static void SetContext( const std::string& s )
      { sContext = s; }
    static void SetDebugLevel( int i )
      { sDebugLevel = i; }

    typedef void ( *FlushHandler )( const std::string& );
    FlushHandler SetFlushHandler( FlushHandler f = NULL )
      { return mBuf.SetFlushHandler( f ); }

    class StringBuf : public std::stringbuf
    {
     public:
      StringBuf()
      : mpOnFlush( LogicError ),
        mNumFlushes( 0 ),
        std::stringbuf( std::ios_base::out )
      {}

      void SetContext( const std::string& s )
        { mContext = s; }
      FlushHandler SetFlushHandler( FlushHandler f = NULL );
      int Flushes()
        { return mNumFlushes; }
      void Clear()
        { clear(); }
      void clear()
        { SetFlushHandler( mpOnFlush ); mNumFlushes = 0; }
      void Reset()
        { str( "" ); mNumFlushes = 0; }

     private:
      FlushHandler mpOnFlush;
      int          mNumFlushes;
      std::string  mContext;

     protected:
      // This function gets called on ostream::flush().
      virtual int   sync();
    } mBuf;

    static std::string sContext;
    static int         sDebugLevel; // global debug level
  };

}

extern BCIError::OutStream bcierr__;
extern BCIError::OutStream bciout__;
extern BCIError::OutStream bcidbg__;

#endif // BCI_ERROR_H

