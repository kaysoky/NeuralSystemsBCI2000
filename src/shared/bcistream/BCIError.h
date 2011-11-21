////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Declarations for stream symbols related to error handling.
//              To report an error, write e.g.
//               bcierr << "My error message" << endl;
//              For an informational message, write
//               bciout << "My info message" << endl;
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
#ifndef BCI_ERROR_H
#define BCI_ERROR_H

#include <sstream>
#include "ClassName.h"
#include "Lockable.h"

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
#define bcierr_     bcierr__( bci::ClassName( typeid( *this ) ) )
#define bciout_     bciout__( bci::ClassName( typeid( *this ) ) )
#define bcierr__    TemporaryLock( bcierr___ )()
#define bciout__    TemporaryLock( bciout___ )()
#define bcidbg__    TemporaryLock( bcidbg___ )()

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

  void SetOperatorStream( std::ostream*, const OSMutex* = NULL );

  class OutStream : public Lockable, public std::ostream
  {
   friend class ::EnvironmentBase;
   friend class ::CoreModule;

   typedef void ( *FlushHandler )( const std::string& );

   public:
    OutStream( FlushHandler f = NULL )
      : std::ostream( 0 )
      {
        this->init( &mBuf );
        mBuf.SetFlushHandler( f );
      }

    OutStream& operator()( const std::string& );
    OutStream& operator()(); // in case CONTEXT_ is defined empty
    // Specifying a debug level with a message will result in filtering out all
    // messages above the global debug level variable.
    OutStream& operator()( int debugLevel )
      { return Debug( debugLevel ); }
    OutStream& Debug( int debugLevel );

    int Flushes()
      { return mBuf.Flushes(); }
    void Clear()
      { std::ostream::clear(); mBuf.Clear(); }
    void Reset()
      { std::ostream::clear(); mBuf.Reset(); }

   private:
    static void SetContext( const std::string& s )
      { sContext = s; }
    static void SetDebugLevel( int i )
      { sDebugLevel = i; }

    FlushHandler SetFlushHandler( FlushHandler f = NULL )
      { return mBuf.SetFlushHandler( f ); }

    class StringBuf : public std::stringbuf
    {
     public:
      StringBuf()
      : std::stringbuf( std::ios_base::out ),
        mpOnFlush( LogicError ),
        mNumFlushes( 0 )
      {}

      void SetContext( const std::string& s )
        { mContext = s; }
      FlushHandler SetFlushHandler( FlushHandler f = NULL );
      int Flushes()
        { return mNumFlushes; }
      void Clear()
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

extern BCIError::OutStream bcierr___;
extern BCIError::OutStream bciout___;
extern BCIError::OutStream bcidbg___;

#endif // BCI_ERROR_H

