////////////////////////////////////////////////////////////////////////////////
//
// File: tcpstream.cpp
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date: Oct 28, 2003
//
// Description: A std::iostream interface for TCP socket communication.
//              tcpsocket: A socket wrapper, tied to tcpstream with
//                tcpstream::open().
//                Also offers synchronization across multiple
//                sockets with wait_for_read() and wait_for_write().
//                Addresses/ports are specified as in "192.2.14.18:21"
//                or as in "dog.animals.org:8080".
//              tcpstream: A std::iostream interface to the data stream on a
//                socket. Will wait for flush or eof before sending data.
//                Send/receive is blocking; one can use rdbuf()->in_avail()
//                to check for data.
//              tcpbuf: A helper class that does the actual send/receive
//                calls.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "TCPStream.h"

#include <string>
#include <sstream>

using namespace std;
////////////////////////////////////////////////////////////////////////////////
// tcpsocket definitions
////////////////////////////////////////////////////////////////////////////////
int tcpsocket::s_instance_count = 0;

tcpsocket::tcpsocket()
: m_handle( NULL ),
  m_listening( false )
{
#ifdef _WIN32
  if( s_instance_count < 1 )
  {
    ::WSADATA ignored;
    ::WSAStartup( 2, &ignored );
  }
#endif // _WIN32
  ++s_instance_count;
}

tcpsocket::~tcpsocket()
{
  close();

  --s_instance_count;
#ifdef _WIN32
  if( s_instance_count < 1 )
    ::WSACleanup();
#endif // _WIN32
}

void
tcpsocket::open()
{
  set_address( NULL, 0 );
  do_open();
}

void
tcpsocket::open( const char* address )
{
  set_address( address );
  do_open();
}

void
tcpsocket::open( const char* ip, u_short port )
{
  set_address( ip, port );
  do_open();
}

void
tcpsocket::set_address( const char* address )
{
  const char port_separator = ':';
  istringstream address_stream( address );
  string hostname;
  getline( address_stream, hostname, port_separator );
  u_short port = 0;
  address_stream >> port;
  set_address( hostname.c_str(), port );
}

void
tcpsocket::set_address( const char* ip, u_short port )
{
  ::memset( &m_address, 0, sizeof( m_address ) );
  m_address.sin_family = AF_INET;
  m_address.sin_port = ::htons( port );
  if( !ip || *ip == '\0' || *ip == '*' )
    m_address.sin_addr.s_addr = INADDR_ANY;
  if( INADDR_NONE == ( m_address.sin_addr.s_addr = ::inet_addr( ip ) ) )
  {
    ::hostent* host = ::gethostbyname( ip );
    if( host && host->h_addr_list )
      m_address.sin_addr = *reinterpret_cast<in_addr*>( host->h_addr_list[ 0 ] );
  }
}

void
tcpsocket::update_address()
{
  int addr_size = sizeof( m_address );
  if( SOCKET_ERROR == ::getsockname( m_handle, reinterpret_cast<sockaddr*>( &m_address ), &addr_size ) )
    close();
}

string
tcpsocket::ip() const
{
  return ::inet_ntoa( m_address.sin_addr );
}

u_short
tcpsocket::port() const
{
  return ::ntohs( m_address.sin_port );
}

bool
tcpsocket::is_open() const
{
  if( m_handle == NULL )
    return false;
  int err = 0,
      err_size = sizeof( err );
  if( ::getsockopt( m_handle, SOL_SOCKET, SO_ERROR, ( char* )&err, &err_size ) || err )
    return false;
  return true;
}

void
tcpsocket::close()
{
  ::closesocket( m_handle );
  m_handle = NULL;
  m_listening = false;
}

bool
tcpsocket::wait_for_read( int timeout, bool return_on_accept )
{
  set_of_instances sockets;
  sockets.insert( this );
  return wait_for_read( sockets, timeout, return_on_accept );
}

bool
tcpsocket::wait_for_write( int timeout, bool return_on_accept )
{
  set_of_instances sockets;
  sockets.insert( this );
  return wait_for_write( sockets, timeout, return_on_accept );
}

bool
tcpsocket::wait_for_read( const tcpsocket::set_of_instances& inSockets,
                          int inTimeout,
                          bool return_on_accept )
{
  const int msecs_per_sec = 1000;
  ::timeval  timeout = { inTimeout / msecs_per_sec, inTimeout % msecs_per_sec },
           * timeoutPtr = &timeout;
  if( inTimeout < 0 )
    timeoutPtr = NULL;
  if( inSockets.size() < 1 )
    return false;

  ::fd_set readfds;
  FD_ZERO( &readfds );
  for( set_of_instances::iterator i = inSockets.begin(); i != inSockets.end(); ++i )
    FD_SET( ( *i )->m_handle, &readfds );

  int result = ::select( 1, &readfds, NULL, NULL, timeoutPtr );
  if( result > 0 )
  {
    for( set_of_instances::iterator i = inSockets.begin(); i != inSockets.end(); ++i )
      if( ( *i )->m_listening && FD_ISSET( ( *i )->m_handle, &readfds ) )
      {
        ( *i )->accept();
        if( !return_on_accept )
          --result;
      }
    if( result < 1 )
      result = wait_for_read( inSockets, inTimeout );
  }
  return result > 0;
}

bool
tcpsocket::wait_for_write( const tcpsocket::set_of_instances& inSockets,
                           int inTimeout,
                           bool return_on_accept )
{
  const int msecs_per_sec = 1000;
  ::timeval  timeout = { inTimeout / msecs_per_sec, inTimeout % msecs_per_sec },
           * timeoutPtr = &timeout;
  if( inTimeout == 0 )
    timeoutPtr = NULL;
  if( inSockets.size() < 1 )
    return false;

  ::fd_set writefds,
           readfds;
  FD_ZERO( &writefds );
  FD_ZERO( &readfds );
  for( set_of_instances::iterator i = inSockets.begin(); i != inSockets.end(); ++i )
  {
    FD_SET( ( *i )->m_handle, &writefds );
    if( ( *i )->m_listening )
      FD_SET( ( *i )->m_handle, &readfds );
  }
  int result = ::select( 2, &readfds, &writefds, NULL, timeoutPtr );
  if( result > 0 )
  {
    for( set_of_instances::iterator i = inSockets.begin(); i != inSockets.end(); ++i )
      if( ( *i )->m_listening && FD_ISSET( ( *i )->m_handle, &readfds ) )
      {
        ( *i )->accept();
        if( !return_on_accept )
          --result;
      }
    if( result < 1 )
      result = wait_for_write( inSockets, inTimeout );
  }
  return result > 0;
}

size_t
tcpsocket::read( char* buffer, size_t count )
{
  int result = ::recv( m_handle, buffer, count, 0 );
  if( result == SOCKET_ERROR || result < 1 && count > 0 ) // Connection has been reset or closed.
  {
    result = 0;
    close();
  }
  return result;
}

size_t
tcpsocket::write( char* buffer, size_t count )
{
  int result = ::send( m_handle, buffer, count, 0 );
  if( result == SOCKET_ERROR || result < 1 && count > 0 )
  {
    result = 0;
    close();
  }
  return result;
}

void
tcpsocket::accept()
{
  if( m_listening )
  {
    unsigned int new_handle = ::accept( m_handle, NULL, NULL );
    if( new_handle == INVALID_SOCKET )
      return;
    ::closesocket( m_handle );
    m_handle = new_handle;
    m_listening = false;
    update_address();
  }
}

void
server_tcpsocket::do_open()
{
  close();
  if( SOCKET_ERROR == ( m_handle = ::socket( PF_INET, SOCK_STREAM, 0 ) ) )
  {
    m_handle = NULL;
    return;
  }
  if( ( SOCKET_ERROR == ::bind( m_handle, reinterpret_cast<sockaddr*>( &m_address ), sizeof( m_address ) ) )
      ||
      ( SOCKET_ERROR == ::listen( m_handle, 1 ) ) )
  {
    close();
    return;
  }
  m_listening = true;
  update_address();
}

void
client_tcpsocket::do_open()
{
  close();
  if( SOCKET_ERROR == ( m_handle = ::socket( PF_INET, SOCK_STREAM, 0 ) ) )
  {
    m_handle = NULL;
    return;
  }
  if( SOCKET_ERROR == ::connect( m_handle, reinterpret_cast<sockaddr*>( &m_address ), sizeof( m_address ) ) )
    close();
}

////////////////////////////////////////////////////////////////////////////////
// tcpbuf definitions
////////////////////////////////////////////////////////////////////////////////
tcpbuf::tcpbuf()
: m_socket( NULL ),
  m_timeout( tcpsocket::infiniteTimeout )
{
}

tcpbuf::~tcpbuf()
{
  delete[] eback();
  delete[] pbase();
}

int
tcpbuf::showmanyc()
{
  // Are there any data available in the streambuffer?
  int result = egptr() - gptr();
  // Are there data waiting in the socket buffer?
  if( result < 1 && m_socket->can_read() && underflow() != traits_type::eof() )
    result = egptr() - gptr();
  return result;
}

int
tcpbuf::underflow()
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
  // Quite likely, this is due to a situation where all data is transmitted
  // but underflow() is called from the stream via snextc() to examine whether
  // there is an eof pending. Making sure that the last transferred byte is
  // either a terminating character or read with get(), not with read(), will
  // probably fix the situation.
  // The reason for this problem is fundamental because there is no "maybe eof"
  // alternative to returning eof().
  if( m_socket->wait_for_read( m_timeout ) )
  {
    int remaining_buf_size = buf_size;
    while( remaining_buf_size > 0 && m_socket->can_read() )
    {
      setg( eback(), gptr(), egptr() + m_socket->read( egptr(), remaining_buf_size ) );
      remaining_buf_size = buf_size - ( egptr() - eback() );
    }
    if( gptr() != egptr() )
      result = traits_type::to_int_type( *gptr() );
  }
  return result;
}

int
tcpbuf::overflow( int c )
{
  if( !pbase() )
  {
    char* buf = new char[ buf_size ];
    if( !buf )
      return traits_type::eof();
    setp( buf, buf + buf_size );
  }


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
tcpbuf::sync()
{
  char* write_ptr = pbase();
  while( m_socket->wait_for_write( m_timeout ) && write_ptr < pptr() )
    write_ptr += m_socket->write( write_ptr, pptr() - write_ptr );
  if( !m_socket->is_open() )
    return traits_type::eof();
  setp( pbase(), epptr() );
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// tcpstream definitions
////////////////////////////////////////////////////////////////////////////////
tcpstream::tcpstream()
: iostream( 0 ),
  buf()
{
  init( &buf );
}

tcpstream::tcpstream( tcpsocket& s )
: iostream( 0 ),
  buf()
{
  init( &buf );
  open( s );
}

void
tcpstream::open( tcpsocket& s )
{
  if( !buf.open( s ) )
    setstate( ios::failbit );
}

void
tcpstream::close()
{
  if( !buf.close() )
    setstate( ios::failbit );
}

