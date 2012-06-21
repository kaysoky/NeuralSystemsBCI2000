////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A std::iostream interface for TCP and UDP socket communication.
//              streamsock: A streamsock wrapper, tied to sockstream with
//                sockstream::open().
//                Also offers synchronization across multiple
//                sockets with wait_for_read() and wait_for_write().
//                Addresses/ports are specified as in "192.2.14.18:21"
//                or as in "dog.animals.org:8080".
//                Note that only one address is maintained which is local
//                or remote address, dependent on context and streamsock state.
//                It might be a good idea to change this.
//              server_tcpsocket, client_tcpsocket: TCP sockets.
//              sending_udpsocket, receiving_udpsocket: UDP sockets.
//              sockstream: A std::iostream interface to the data stream on a
//                streamsock. Will wait for flush or eof before sending data.
//                Send/receive is blocking; one can use rdbuf()->in_avail()
//                to check for data.
//              sockbuf: A helper class that does the actual send/receive
//                calls.
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
#ifdef __BORLANDC__
# include "PCHIncludes.h"
# pragma hdrstop
#endif // __BORLANDC__

#include "SockStream.h"

#ifdef _WIN32
# define socklen_t int
#else
# include <unistd.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <netdb.h>
# define INVALID_SOCKET   (SOCKET)( ~0 )
# define SOCKET_ERROR     ( -1 )
# define closesocket( s ) close( s )
# ifdef __GNUC__
#  if __GNUC__ < 3
#   define EMULATE_TRAITS_TYPE
#  endif
# endif  // __GNUC__
#endif   // _WIN32

#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>

#ifdef EMULATE_TRAITS_TYPE
# define traits_type char_traits
struct char_traits
{
  static int  eof()                        { return EOF; }
  static char not_eof( const int& i )      { return i == EOF ? 0 : i; }
  static int  to_int_type( const char& c ) { return *reinterpret_cast<const unsigned char*>( &c ); }
};
#endif // EMULATE_TRAITS_TYPE

namespace {

class Timeout
{
 public:
  Timeout( int inTimeout )
  : mpTimeout( inTimeout < 0 ? NULL : &mTimeout )
  {
    mTimeout.tv_sec = inTimeout / 1000;
    mTimeout.tv_usec = 1000 * ( inTimeout % 1000 );
  }
  operator ::timeval*()
  { return mpTimeout; }
 private:
  ::timeval mTimeout, *mpTimeout;
};

} // namespace

using namespace std;
////////////////////////////////////////////////////////////////////////////////
// streamsock definitions
////////////////////////////////////////////////////////////////////////////////
int streamsock::s_instance_count = 0;

streamsock::streamsock()
: m_handle( INVALID_SOCKET ),
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

streamsock::~streamsock()
{
  close();

  --s_instance_count;
#ifdef _WIN32
  if( s_instance_count < 1 )
    ::WSACleanup();
#endif // _WIN32
}

void
streamsock::open()
{
  set_address( NULL, 0 );
  do_open();
}

void
streamsock::open( const char* address )
{
  set_address( address );
  do_open();
}

void
streamsock::open( const char* ip, unsigned short port )
{
  set_address( ip, port );
  do_open();
}

void
streamsock::set_address( const char* address )
{
  const char port_separator = ':';
  istringstream address_stream( address );
  string hostname;
  getline( address_stream, hostname, port_separator );
  unsigned short port = 0;
  address_stream >> port;
  set_address( hostname.c_str(), port );
}

void
streamsock::set_address( const char* inIP, unsigned short inPort )
{
  char* buf = NULL;
  const char* ip = "";
  if( inIP != NULL )
    ip = inIP;
  else
  {
    const int buflen = 1024;
    char buf [ buflen ];
    if( SOCKET_ERROR != ::gethostname( buf, buflen ) )
      ip = buf;
  }
  ::memset( &m_address, 0, sizeof( m_address ) );
  m_address.sa_in.sin_family = AF_INET;
  m_address.sa_in.sin_port = htons( inPort );
  if( *ip == '*' ) // A "*" as IP address means "any local address" (for bind() ).
    m_address.sa_in.sin_addr.s_addr = INADDR_ANY;
  else if( INADDR_NONE == ( m_address.sa_in.sin_addr.s_addr = ::inet_addr( ip ) ) )
    m_address.sa_in.sin_addr.s_addr = ::inet_addr( "127.0.0.1" );
  delete[] buf;
}

bool
streamsock::ip_compare::operator()( const string& inAddr1, const string& inAddr2 )
{
  unsigned long addr1 = htonl( ::inet_addr( inAddr1.c_str() ) ),
                addr2 = htonl( ::inet_addr( inAddr2.c_str() ) );

  const unsigned int priority[] = { 127, 169, 10, 192, 0 };
  const unsigned int* p_begin = priority,
                    * p_end = priority + sizeof( priority ) / sizeof( *priority ) - 1;

  const unsigned int* p1 = find( p_begin, p_end, addr1 >> 24 ),
                    * p2 = find( p_begin, p_end, addr2 >> 24 );

  return ( p1 != p2 ) ? ( p1 < p2 ) : ( addr1 < addr2 );
}

streamsock::set_of_addresses
streamsock::local_addresses()
{
  set_of_addresses addresses;
  addresses.insert( "127.0.0.1" );

  const int buflen = 1024;
  char buf[ buflen ];
  if( SOCKET_ERROR != ::gethostname( buf, buflen ) )
  {
    ::hostent* host = ::gethostbyname( buf );
    if( host && host->h_addr_list )
      for( size_t i = 0; host->h_addr_list[ i ] != NULL; ++i )
      {
        in_addr* addr = reinterpret_cast<in_addr*>( host->h_addr_list[ i ] );
        string addr_string = ::inet_ntoa( *addr );
        if( addr_string != "" )
          addresses.insert( addr_string );
      }
  }
  return addresses;
}

void
streamsock::update_address()
{
  socklen_t addr_size = sizeof( m_address );
  if( SOCKET_ERROR == ::getsockname( m_handle, &m_address.sa, &addr_size ) )
    close();
}

string
streamsock::ip() const
{
  return ::inet_ntoa( m_address.sa_in.sin_addr );
}

unsigned short
streamsock::port() const
{
  return ntohs( m_address.sa_in.sin_port );
}

bool
streamsock::is_open() const
{
  if( m_handle == INVALID_SOCKET )
    return false;
  int err = 0;
  socklen_t err_size = sizeof( err );
  if( ::getsockopt( m_handle, SOL_SOCKET, SO_ERROR, ( char* )&err, &err_size ) || err )
    return false;
  return true;
}

bool
streamsock::connected()
{
  bool connected = !m_listening;
  if( connected && is_open() && can_read() )
  {
    // Check for a connection reset by the peer.
    char c;
    int result = ::recv( m_handle, &c, sizeof( c ), MSG_PEEK );
    connected = ( result > 0 && result != SOCKET_ERROR );
  }
  return connected && is_open();
}

void
streamsock::close()
{
  ::closesocket( m_handle );
  m_handle = INVALID_SOCKET;
  m_listening = false;
}

bool
streamsock::wait_for_read( int timeout, bool return_on_accept )
{
  set_of_instances sockets;
  sockets.insert( this );
  return wait_for_read( sockets, timeout, return_on_accept );
}

bool
streamsock::wait_for_write( int timeout, bool return_on_accept )
{
  set_of_instances sockets;
  sockets.insert( this );
  return wait_for_write( sockets, timeout, return_on_accept );
}

bool
streamsock::wait_for_read( const streamsock::set_of_instances& inSockets,
                           int   inTimeout,
                           bool  return_on_accept )
{
  Timeout timeout( inTimeout );
  int max_fd = -1;
  ::fd_set readfds;
  FD_ZERO( &readfds );
  for( set_of_instances::const_iterator i = inSockets.begin(); i != inSockets.end(); ++i )
    if( ( *i )->m_handle != INVALID_SOCKET )
    {
      max_fd = max( max_fd, int( ( *i )->m_handle ) );
      FD_SET( ( *i )->m_handle, &readfds );
    }
  if( max_fd < 0 )
  {
    if( inTimeout >= 0 )
    {
#ifdef  _WIN32  // Achieve similar behavior for empty sets/invalid sockets across platforms.
      ::Sleep( inTimeout );
#else
      ::select( 0, NULL, NULL, NULL, timeout );
#endif
    }
    return false;
  }
  int result = ::select( max_fd + 1, &readfds, NULL, NULL, timeout );
  if( result > 0 )
  {
    for( set_of_instances::const_iterator i = inSockets.begin(); i != inSockets.end(); ++i )
      if( ( *i )->m_listening && FD_ISSET( ( *i )->m_handle, &readfds ) )
      {
        ( *i )->do_accept();
        if( !return_on_accept )
          --result;
      }
    if( result < 1 )
      result = wait_for_read( inSockets, inTimeout );
  }
  return result > 0;
}

bool
streamsock::wait_for_write( const streamsock::set_of_instances& inSockets,
                            int inTimeout,
                            bool return_on_accept )
{
  Timeout timeout( inTimeout );
  int max_fd = -1;
  ::fd_set writefds,
           readfds;
  FD_ZERO( &writefds );
  FD_ZERO( &readfds );
  for( set_of_instances::const_iterator i = inSockets.begin(); i != inSockets.end(); ++i )
    if( ( *i )->m_handle != INVALID_SOCKET )
    {
      max_fd = max( max_fd, int( ( *i )->m_handle ) );
      FD_SET( ( *i )->m_handle, &writefds );
      if( ( *i )->m_listening )
        FD_SET( ( *i )->m_handle, &readfds );
    }
  if( max_fd < 0 )
  {
    if( inTimeout >= 0 )
    {
#ifdef  _WIN32  // Achieve similar behavior for empty sets/invalid sockets across platforms.
      ::Sleep( inTimeout );
#else
      ::select( 0, NULL, NULL, NULL, timeout );
#endif
    }
    return false;
  }
  int result = ::select( max_fd + 1, &readfds, &writefds, NULL, timeout );
  if( result > 0 )
  {
    for( set_of_instances::const_iterator i = inSockets.begin(); i != inSockets.end(); ++i )
      if( ( *i )->m_listening && FD_ISSET( ( *i )->m_handle, &readfds ) )
      {
        ( *i )->do_accept();
        if( !return_on_accept )
          --result;
      }
    if( result < 1 )
      result = wait_for_write( inSockets, inTimeout );
  }
  return result > 0;
}

size_t
streamsock::read( char* buffer, size_t count )
{
  int result = ::recv( m_handle, buffer, static_cast<int>( count ), 0 );
  if( result == SOCKET_ERROR || result < 1 && count > 0 ) // Connection has been reset or closed.
  {
    result = 0;
    close();
  }
  return result;
}

size_t
streamsock::write( const char* buffer, size_t count )
{
  int result = ::send( m_handle, buffer, static_cast<int>( count ), 0 );
  if( result == SOCKET_ERROR || result < 1 && count > 0 )
  {
    result = 0;
    close();
  }
  return result;
}

void
streamsock::set_socket_options()
{
  if( m_handle != INVALID_SOCKET )
  {
    struct linger val = { 1, 1 }; // close after one second
    ::setsockopt( m_handle, SOL_SOCKET, SO_LINGER, reinterpret_cast<const char*>( &val ), sizeof( val ) );
  }
}

void
tcpsocket::set_socket_options()
{
  streamsock::set_socket_options();
  if( m_handle != INVALID_SOCKET )
  {
    int val = m_tcpnodelay;
    ::setsockopt( m_handle, IPPROTO_TCP, TCP_NODELAY,
                        reinterpret_cast<const char*>( &val ), sizeof( val ) );
  }
}

void
tcpsocket::set_handle( SOCKET inHandle )
{
  close();
  m_handle = inHandle;
  set_socket_options();
  update_address();
}

void
server_tcpsocket::do_open()
{
  close();
  m_handle = ::socket( PF_INET, SOCK_STREAM, 0 );
  bool success = ( m_handle != INVALID_SOCKET );
  if( success )
  {
    int val = 1;
    success = SOCKET_ERROR != ::setsockopt( m_handle, SOL_SOCKET, SO_REUSEADDR,
                                                          reinterpret_cast<const char*>( &val ), sizeof( val ) );
  }
  if( success )
    success = SOCKET_ERROR != ::bind( m_handle, &m_address.sa, sizeof( m_address ) );
  if( success )
    success = SOCKET_ERROR != ::listen( m_handle, 1 );
  if( success )
  {
    m_listening = true;
    set_socket_options();
    update_address();
  }
  else
  {
    close();
  }
}

void
server_tcpsocket::do_accept()
{
  if( m_listening )
  {
    SOCKET new_handle = ::accept( m_handle, NULL, NULL );
    if( new_handle == INVALID_SOCKET )
      return;
    set_handle( new_handle );
  }
}

bool
server_tcpsocket::wait_for_accept( tcpsocket& outNew, int inTimeout )
{
  bool success = ( m_handle != INVALID_SOCKET && m_listening );
  if( success )
  {
    Timeout timeout( inTimeout );
    int max_fd = static_cast<int>( m_handle );
    ::fd_set readfds;
    FD_ZERO( &readfds );
    FD_SET( m_handle, &readfds );
    int result = ::select( max_fd + 1, &readfds, NULL, NULL, timeout );
    success = ( result > 0 && FD_ISSET( m_handle, &readfds ) );
  }
  SOCKET new_handle = INVALID_SOCKET;
  if( success )
  {
    new_handle = ::accept( m_handle, NULL, NULL );
    success = ( new_handle != INVALID_SOCKET );
  }
  if( success )
    outNew.set_handle( new_handle );
  return success;
}

void
client_tcpsocket::do_open()
{
  close();
  if( INVALID_SOCKET == ( m_handle = ::socket( PF_INET, SOCK_STREAM, 0 ) ) )
    return;

  if( SOCKET_ERROR == ::connect( m_handle, &m_address.sa, sizeof( m_address ) ) )
  {
    close();
    return;
  }
  set_socket_options();
}

void
receiving_udpsocket::do_open()
{
  close();
  if( INVALID_SOCKET == ( m_handle = ::socket( PF_INET, SOCK_DGRAM, 0 ) ) )
    return;

  if( SOCKET_ERROR == ::bind( m_handle, &m_address.sa, sizeof( m_address ) ) )
  {
    close();
    return;
  }
  set_socket_options();
}

void
sending_udpsocket::set_socket_options()
{
  streamsock::set_socket_options();
  if( m_handle != INVALID_SOCKET )
  {
    int val = !::strcmp( ::inet_ntoa( m_address.sa_in.sin_addr ), "255.255.255.255" ); // Broadcast address
    ::setsockopt( m_handle, SOL_SOCKET, SO_BROADCAST,
                        reinterpret_cast<const char*>( &val ), sizeof( val ) );
  }
}

void
sending_udpsocket::do_open()
{
  close();
  if( INVALID_SOCKET == ( m_handle = ::socket( PF_INET, SOCK_DGRAM, 0 ) ) )
      return;

  address bind_addr = m_address;
  bind_addr.sa_in.sin_addr.s_addr = INADDR_ANY;
  bind_addr.sa_in.sin_port = 0;
  if( ( SOCKET_ERROR == ::bind( m_handle, &bind_addr.sa, sizeof( bind_addr ) ) )
      ||
      ( SOCKET_ERROR == ::connect( m_handle, &m_address.sa, sizeof( m_address ) ) ) )
  {
    close();
    return;
  }
  set_socket_options();
}

////////////////////////////////////////////////////////////////////////////////
// sockbuf definitions
////////////////////////////////////////////////////////////////////////////////
sockbuf::sockbuf()
: m_socket( NULL ),
  m_timeout( streamsock::infiniteTimeout )
{
}

sockbuf::~sockbuf()
{
  delete[] eback();
  delete[] pbase();
}

streamsize
sockbuf::showmanyc()
{
  // Are there any data available in the streambuffer?
  streamsize result = egptr() - gptr();
  // Are there data waiting in the streamsock buffer?
  if( result < 1 && m_socket && m_socket->can_read() && underflow() != traits_type::eof() )
    result = egptr() - gptr();
  return result;
}

ios::int_type
sockbuf::underflow()
{
  if( sync() == traits_type::eof() )
    return traits_type::eof();

  if( !eback() )
  {
    char* buf = new char[ c_buf_size ];
    if( !buf )
      return traits_type::eof();
    setg( buf, buf, buf );
  }

  ios::int_type result = traits_type::eof();
  setg( eback(), eback(), eback() );
  // If your program blocks here, changing the timeout value will not help.
  // Quite likely, this is due to a situation where all transmitted data has been read
  // but underflow() is called from the stream via snextc() to examine whether
  // there is an eof pending.
  // There are two workarounds:
  // 1) Make sure that the last transferred byte of a message is a terminating character,
  // 2) read the last byte of a message with get() rather than read().
  // The reason for this problem is fundamental because there is no "maybe eof"
  // alternative to returning eof().
  if( m_socket->wait_for_read( m_timeout ) )
  {
    size_t remaining_buf_size = c_buf_size;
    while( remaining_buf_size > 0 && m_socket->can_read() )
    {
      setg( eback(), gptr(), egptr() + m_socket->read( egptr(), remaining_buf_size ) );
      remaining_buf_size = c_buf_size - ( egptr() - eback() );
    }
    if( gptr() != egptr() )
      result = traits_type::to_int_type( *gptr() );
  }
  return result;
}

ios::int_type
sockbuf::overflow( int c )
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
sockbuf::sync()
{
  if( !m_socket )
    return traits_type::eof();

  char* write_ptr = pbase();
  if( !write_ptr )
  {
    char* buf = new char[ c_buf_size ];
    if( !buf )
      return traits_type::eof();
    setp( buf, buf + c_buf_size );
    write_ptr = pbase();
  }
  while( m_socket->wait_for_write( m_timeout ) && write_ptr < pptr() )
    write_ptr += m_socket->write( write_ptr, pptr() - write_ptr );
  if( !m_socket->is_open() )
    return traits_type::eof();
  setp( pbase(), epptr() );
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// sockstream definitions
////////////////////////////////////////////////////////////////////////////////
sockstream::sockstream()
: iostream( 0 ),
  buf()
{
  init( &buf );
}

sockstream::sockstream( streamsock& s )
: iostream( 0 ),
  buf()
{
  init( &buf );
  open( s );
}

void
sockstream::open( streamsock& s )
{
  if( !buf.open( s ) )
    setstate( ios::failbit );
}

void
sockstream::close()
{
  if( !buf.close() )
    setstate( ios::failbit );
}

