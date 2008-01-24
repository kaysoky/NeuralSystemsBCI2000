////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A std::iostream interface for blocking serial communication.
//              serialstream: A std::iostream interface to the data stream on a
//                serial line. Will wait for flush or eof before sending data.
//                Send/receive is blocking; one can use rdbuf()->in_avail()
//                to check for data.
//              serialbuf: A helper class that does the actual send/receive
//                calls.
//              NOTE: Configuration of the serial interface is independent of
//                stream communication, and not provided here.
//                Configuration must take place before opening the interface
//                from the serialstream class.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifdef __BORLANDC__
# include "PCHIncludes.h"
# pragma hdrstop
#endif // __BORLANDC__

#include "SerialStream.h"
#include "Windows.h"

using namespace std;
////////////////////////////////////////////////////////////////////////////////
// serialbuf definitions
////////////////////////////////////////////////////////////////////////////////
serialbuf::serialbuf()
: m_handle( INVALID_HANDLE_VALUE ),
  m_timeout( serialbuf::defaultTimeout )
{
}

serialbuf::~serialbuf()
{
  close();
  delete[] eback();
  delete[] pbase();
}

void
serialbuf::set_timeout( int t )
{
  m_timeout = t;
  if( m_timeout == serialbuf::infiniteTimeout )
    m_timeout = MAXDWORD;
  COMMTIMEOUTS c =
  {
    MAXDWORD, MAXDWORD, m_timeout,
    MAXDWORD, m_timeout
  };
  ::SetCommTimeouts( m_handle, &c );
}

serialbuf*
serialbuf::open( const char* device )
{
  if( m_handle != INVALID_HANDLE_VALUE )
    ::CloseHandle( m_handle );
  m_handle = ::CreateFile( device, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );
  set_timeout( m_timeout );
  return this;
}

bool
serialbuf::is_open() const
{
  return m_handle != INVALID_HANDLE_VALUE;
}

serialbuf*
serialbuf::close()
{
  if( m_handle != INVALID_HANDLE_VALUE )
    ::CloseHandle( m_handle );
  m_handle = INVALID_HANDLE_VALUE;
  return this;
}

int
serialbuf::showmanyc()
{
  // Are there any data available in the streambuffer?
  int result = egptr() - gptr();
  if( result < 1 )
  {
    // Are there data waiting in the serial device buffer?
    COMMTIMEOUTS dont_block =
    {
      MAXDWORD, 0, 0,
      0, 0
    };
    ::SetCommTimeouts( m_handle, &dont_block );
    if( underflow() != traits_type::eof() )
      result = egptr() - gptr();
    set_timeout( m_timeout );
  }
  return result;
}

int
serialbuf::underflow()
{
  if( sync() == traits_type::eof() )
    return traits_type::eof();

  if( !eback() )
  {
    char* buf = new char[ buf_size ];
    if( !buf )
      return traits_type::eof();
    setg( buf, buf, buf );
  }

  int result = traits_type::eof();
  setg( eback(), eback(), eback() );
  // If your program blocks here, changing the timeout value will not help.
  // Quite likely, this is due to a situation where all transmitted data has been read
  // but underflow() is called from the stream via snextc() to examine whether
  // there is an eof pending. Making sure that the last transferred byte is
  // either a terminating character or reading it with get(), not with read(),
  // will probably fix the situation.
  // The reason for this problem is fundamental because there is no "maybe eof"
  // alternative to returning eof().
  DWORD actuallyRead = 0;
  if( ::ReadFile( m_handle, egptr(), buf_size, &actuallyRead, NULL ) )
    setg( eback(), gptr(), egptr() + actuallyRead );
  if( gptr() != egptr() )
    result = traits_type::to_int_type( *gptr() );
  return result;
}

int
serialbuf::overflow( int c )
{
  if( sync() == traits_type::eof() )
    return traits_type::eof();
  if( c != traits_type::eof() )
  {
    *pptr() = c;
    pbump( 1 );
  }
  return traits_type::not_eof( c );
}

int
serialbuf::sync()
{
  if( m_handle == INVALID_HANDLE_VALUE )
    return traits_type::eof();

  char* write_ptr = pbase();
  if( !write_ptr )
  {
    char* buf = new char[ buf_size ];
    if( !buf )
      return traits_type::eof();
    setp( buf, buf + buf_size );
    write_ptr = pbase();
  }
  DWORD actuallyWritten = 0;
  while( ::WriteFile( m_handle, write_ptr, pptr() - write_ptr, &actuallyWritten, NULL )
    && actuallyWritten > 0 )
    write_ptr += actuallyWritten;
  setp( pbase(), epptr() );
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// serialstream definitions
////////////////////////////////////////////////////////////////////////////////
serialstream::serialstream()
: iostream( 0 ),
  buf()
{
  init( &buf );
}

serialstream::serialstream( const char* device )
: iostream( 0 ),
  buf()
{
  init( &buf );
  open( device );
}

void
serialstream::open( const char* device )
{
  if( !buf.open( device ) )
    setstate( ios::failbit );
}

void
serialstream::close()
{
  if( !buf.close() )
    setstate( ios::failbit );
}

