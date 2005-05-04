////////////////////////////////////////////////////////////////////////////////
//
// File: UBCIError.h
//
// Description: Declarations for stream symbols related to error handling.
//              To report an error, write e.g.
//               bcierr << "My error message" << endl;
//              For an informational message, write
//               bciout << "My info message" << endl;
//
// Author: Juergen Mellinger
//
// Date:   Mar 27, 2003
//
// Changes: Apr 16, 2003: Replaced dummy implementations by objects that
//          actually hold messages.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef UBCIErrorH
#define UBCIErrorH

#include <sstream>

// Info added to error output.
#if( defined( _DEBUG ) && !defined( NDEBUG ) )
# define DEBUGINFO   /*__FILE__", "*/__FUNC__
#else
# define DEBUGINFO
#endif // _DEBUG && !NDEBUG

// Declaration of user symbols with stream syntax (must be macros to include debug info):
#define bcierr      __bcierr( DEBUGINFO )
#define bciout      __bciout( DEBUGINFO )
#define _bcierr     __bcierr( typeid( *this ).name() )
#define _bciout     __bciout( typeid( *this ).name() )

class EnvironmentBase;

namespace BCIError
{
  // Stream message handling actions for bcierr/bciout.
  void Warning( const std::string& );
  void ConfigurationError( const std::string& );
  void RuntimeError( const std::string& );
  void LogicError( const std::string& );

  class bci_ostream : public std::ostream
  {
   friend class EnvironmentBase;

   public:
    bci_ostream() : std::ostream( 0 )  { this->init( &m_buf ); }

    std::ostream& operator()( const char* );
    std::ostream& operator()()         { return *this; }
    int           flushes()            { return m_buf.flushes(); }
    void          clear()              { std::ostream::clear(); m_buf.clear(); }

   private:
    typedef void ( *flush_handler )( const std::string& );
    void SetFlushHandler( flush_handler f = NULL ) { m_buf.SetFlushHandler( f ); }

    class bci_stringbuf : public std::stringbuf
    {
     public:
      bci_stringbuf()
      : mp_on_flush( LogicError ),
        m_num_flushes( 0 ),
        std::stringbuf( std::ios_base::out )
      {}

      void SetFlushHandler( flush_handler f = NULL );
      int  flushes()      { return m_num_flushes; }
      void clear()        { SetFlushHandler( mp_on_flush ); m_num_flushes = 0; }

     private:
      flush_handler mp_on_flush;
      int           m_num_flushes;

     protected:
      // This function gets called on ostream::flush().
      virtual int   sync();
    } m_buf;
  };

}

extern BCIError::bci_ostream __bcierr;
extern BCIError::bci_ostream __bciout;

#endif // UBCIErrorH

