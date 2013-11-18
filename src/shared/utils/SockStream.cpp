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
#include "SockStream.h"

#ifdef _WIN32
# define socklen_t int
# undef errno
# define errno WSAGetLastError()
# define EINTR WSAEINTR
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
#endif   // _WIN32

#ifndef IPPROTO_TCP
# define IPPROTO_TCP 0
#endif

#ifndef IPPROTO_UDP
# define IPPROTO_UDP 0
#endif

#if defined(NDEBUG)
# define SOCKCALL(x) (x);
#else
# define SOCKCALL(x) assert(!(x<0));
#endif

#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>

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

int GlobalInit()
{
  static struct Init
  {
    Init() : error( 0 )
    {
#if _WIN32
      ::WSADATA ignored;
      if( ::WSAStartup( 2, &ignored ) )
        error = -1;
#endif
    }
    ~Init()
    {
#if _WIN32
      ::WSACleanup();
#endif
    }
    int error;
  } init;
  return init.error;
}

} // namespace

using namespace std;
////////////////////////////////////////////////////////////////////////////////
// streamsock definitions
////////////////////////////////////////////////////////////////////////////////
static int init = GlobalInit();

streamsock::streamsock()
: m_handle( INVALID_SOCKET ),
  m_listening( false ),
  m_max_msg_size( 0 ),
  m_ready_to_read( false ),
  m_ready_to_write( false ),
  m_conn_reset( false )
{
  init();
  SOCKCALL( GlobalInit() );
}

streamsock::~streamsock()
{
  close();
}

void
streamsock::open( const std::string& address )
{
  if( set_address( address ) )
  {
    init();
    do_open();
  }
}

void
streamsock::open( const std::string& ip, unsigned short port )
{
  if( set_address( ip, port ) )
  {
    init();
    do_open();
  }
}

void
streamsock::init()
{
  m_handle = INVALID_SOCKET;
  m_listening = false;
  m_max_msg_size = 0;
  m_ready_to_read = false;
  m_ready_to_write = false;
  m_conn_reset = false;
}

bool
streamsock::set_address( const std::string& address )
{
  string hostname;
  istringstream iss( address );
  if( getline( iss, hostname, ':' ) )
  {
    unsigned short port = 0;
    iss >> port; // Not providing a port is ok, and means "any".
    return set_address( hostname, port );
  }
  return false;
}

bool
streamsock::set_address( const std::string& inIP, unsigned short inPort )
{
  string ip = inIP;
  if( ip == "localhost" )
    ip = "127.0.0.1"; // Avoid INADDR_LOOPBACK due to ambiguity

  bool result = true;
  ::memset( &m_address, 0, sizeof( m_address ) );
  m_address.sin_family = AF_INET;
  m_address.sin_port = htons( inPort );
  in_addr& addr = m_address.sin_addr;
  if( ip == "*" ) // A "*" as IP address means "any local address" (for bind() ).
    addr.s_addr = htonl( INADDR_ANY );
  else
  {
    addr.s_addr = ::inet_addr( ip.c_str() );
    if( addr.s_addr == htonl( INADDR_NONE ) )
    {
      ::hostent* host = ::gethostbyname( ip.c_str() );
      result = ( host && host->h_addrtype == AF_INET && host->h_addr_list && *host->h_addr_list );
      if( result )
        addr.s_addr = *reinterpret_cast<unsigned long*>( *host->h_addr_list );
    }
  }
  return result;
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
  if( SOCKET_ERROR == ::getsockname( m_handle, (sockaddr*)&m_address, &addr_size ) )
    close();
}

string
streamsock::ip() const
{
  if( m_handle == INVALID_SOCKET )
    return "<N/A>";
  return ::inet_ntoa( m_address.sin_addr );
}

int
streamsock::port() const
{
  if( m_handle == INVALID_SOCKET )
    return -1;
  return ntohs( m_address.sin_port );
}

string
streamsock::address() const
{
  ostringstream oss;
  oss << ip();
  if( port() >= 0 )
    oss << ':' << port(); 
  return oss.str();
}

bool
streamsock::is_open() const
{
  if( m_handle == INVALID_SOCKET )
    return false;
  if( m_conn_reset )
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
  return !m_listening && is_open();
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
  streamsock* p = this;
  return m_ready_to_read || select( &p, 1, 0, 0 ,timeout, return_on_accept );
}

bool
streamsock::wait_for_write( int timeout, bool return_on_accept )
{
  streamsock* p = this;
  return m_ready_to_write || select( 0, 0, &p, 1, timeout, return_on_accept );
}

bool
streamsock::select( streamsock** in_readers, size_t in_nreaders,
                    streamsock** in_writers, size_t in_nwriters,
                           int   in_timeout,
                           bool  return_on_accept )
{
  int max_fd = -1;
  ::fd_set writefds,
           readfds;
  FD_ZERO( &writefds );
  FD_ZERO( &readfds );
  for( size_t i = 0; i < in_nreaders; ++i )
  {
    const streamsock* s = in_readers[i];
    if( s->m_handle != INVALID_SOCKET )
    {
      max_fd = max( max_fd, int( s->m_handle ) );
      FD_SET( s->m_handle, &readfds );
    }
  }
  for( size_t i = 0; i < in_nwriters; ++i )
  {
    const streamsock* s = in_writers[i];
    if( s->m_handle != INVALID_SOCKET )
    {
      max_fd = max( max_fd, int( s->m_handle ) );
      FD_SET( s->m_handle, &writefds );
      if( s->m_listening )
        FD_SET( s->m_handle, &readfds );
    }
  }
  int result = 0;
  bool keep_trying = true;
  if( max_fd < 0 )
    streamsock::sleep( in_timeout );
  else
  {
    Timeout t( in_timeout );
    while( keep_trying )
    {
      result = ::select( max_fd + 1, &readfds, &writefds, NULL, t );
      keep_trying = ( check_result( result ) == retry );
    }
  }
  if( result > 0 )
  {
    streamsock** sets[] = { in_readers, in_writers };
    size_t counts[] = { in_nreaders, in_nwriters };
    for( size_t i = 0; i < sizeof( sets ) / sizeof( *sets ); ++i )
    {
      for( streamsock** p = sets[i]; p < sets[i] + counts[i]; ++p )
      {
        streamsock* s = *p;
        if( FD_ISSET( s->m_handle, &readfds ) )
        {
          if( s->m_listening )
          {
            s->do_accept();
            if( !return_on_accept )
              --result;
          }
          else
            s->m_ready_to_read = true;
        }
        if( FD_ISSET( s->m_handle, &writefds ) )
          s->m_ready_to_write = true;
      }
    }
    if( result < 1 )
      result = streamsock::select( in_readers, in_nreaders, in_writers, in_nwriters, in_timeout );
  }
  return result > 0;
}

void
streamsock::sleep( int in_timeout )
{
  if( in_timeout < 0 )
    return;

#if  _WIN32  // Achieve similar behavior for empty sets/invalid sockets across platforms.
      ::Sleep( in_timeout );
#else
      ::select( 0, NULL, NULL, NULL, Timeout( in_timeout ) );
#endif
}

int
streamsock::check_result( int result )
{
  if( result >= 0 )
    return ok;

  switch( errno )
  {
#if _WIN32
    case WSAEINPROGRESS:
#endif
    case EINTR:
      return retry;
  }
  return fatal;
}

bool
streamsock::wait_for_read( streamsock::set_of_instances& sockets,
                           int   timeout,
                           bool  return_on_accept )
{
  streamsock** s = sockets.empty() ? 0 : &sockets[0];
  return select( s, sockets.size(), 0, 0, timeout, return_on_accept );
}

bool
streamsock::wait_for_write( streamsock::set_of_instances& sockets,
                           int   timeout,
                           bool  return_on_accept )
{
  streamsock** s = sockets.empty() ? 0 : &sockets[0];
  return select( 0, 0, s, sockets.size(), timeout, return_on_accept );
}

size_t
streamsock::read( char* buffer, size_t count )
{
  assert( m_max_msg_size == 0 || count >= m_max_msg_size );
  assert( buffer != 0 || count == 0 );
  bool keep_trying = true;
  int result = 0;
  while( keep_trying )
  {
    result = ::recv( m_handle, buffer, static_cast<int>( count ), 0 );
    int kind = check_result( result );
    if( kind == ok && result == 0 && count > 0 )
      kind = fatal;
    switch( kind )
    {
      case ok:
        keep_trying = false;
        break;
      case retry:
        keep_trying = true;
        break;
      default:
        keep_trying = false;
        m_conn_reset = true;
        result = 0;
        close();
    }
  }
  m_ready_to_read = false;
  return result;
}

size_t
streamsock::write( const char* buffer, size_t count )
{
  if( m_max_msg_size && m_max_msg_size < count )
    count = m_max_msg_size;
  bool keep_trying = true;
  int result = 0;
  while( keep_trying )
  {
    result = ::send( m_handle, buffer, static_cast<int>( count ), 0 );
    switch( check_result( result ) )
    {
      case ok:
        keep_trying = false;
        break;
      case retry:
        keep_trying = true;
        break;
      default:
        keep_trying = false;
        m_conn_reset = true;
        result = 0;
        close();
    }
  }
  m_ready_to_write = false;
  return result;
}

void
streamsock::set_socket_options()
{
  if( m_handle != INVALID_SOCKET )
  {
    int type = 0,
        len = sizeof( type );
    SOCKCALL( ::getsockopt( 
      m_handle, SOL_SOCKET, SO_TYPE,
      reinterpret_cast<char*>( &type ), &len )
    );
    switch( type )
    {
      case SOCK_DGRAM:
        m_max_msg_size = 64 * 1024;
        break;
      default:
        m_max_msg_size = 0;
    }
  }
}

void
tcpsocket::set_socket_options()
{
  streamsock::set_socket_options();
  if( m_handle != INVALID_SOCKET )
  {
    struct linger linger_ = { 0 };
    linger_.l_onoff = 1;
    linger_.l_linger = 1;
    SOCKCALL( ::setsockopt(
      m_handle, SOL_SOCKET, SO_LINGER, 
      reinterpret_cast<const char*>( &linger_ ), sizeof( linger_ ) )
    );
    int val = m_tcpnodelay;
    SOCKCALL( ::setsockopt(
      m_handle, IPPROTO_TCP, TCP_NODELAY,
      reinterpret_cast<const char*>( &val ), sizeof( val ) )
     );
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
  m_handle = ::socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
  bool success = ( m_handle != INVALID_SOCKET );
  if( success )
  {
    int val = 1;
    success = SOCKET_ERROR != ::setsockopt( m_handle, SOL_SOCKET, SO_REUSEADDR,
                                            reinterpret_cast<const char*>( &val ), sizeof( val ) );
  }
  if( success )
    success = SOCKET_ERROR != ::bind( m_handle, (const sockaddr*)&m_address, sizeof( m_address ) );
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
  if( INVALID_SOCKET == ( m_handle = ::socket( PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) )
    return;

  if( SOCKET_ERROR == ::connect( m_handle, (const sockaddr*)&m_address, sizeof( m_address ) ) )
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
  if( INVALID_SOCKET == ( m_handle = ::socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) )
    return;

  if( SOCKET_ERROR == ::bind( m_handle, (const sockaddr*)&m_address, sizeof( m_address ) ) )
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
    int val = !::strcmp( ::inet_ntoa( m_address.sin_addr ), "255.255.255.255" ); // Broadcast address
    SOCKCALL( ::setsockopt(
      m_handle, SOL_SOCKET, SO_BROADCAST,
      reinterpret_cast<const char*>( &val ), sizeof( val ) )
    );
  }
}

void
sending_udpsocket::do_open()
{
  close();
  if( INVALID_SOCKET == ( m_handle = ::socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) )
      return;

  sockaddr_in bind_addr = m_address;
  bind_addr.sin_addr.s_addr = htonl( INADDR_ANY );
  bind_addr.sin_port = 0;
  if( ( SOCKET_ERROR == ::bind( m_handle, (const sockaddr*)&bind_addr, sizeof( bind_addr ) ) )
      ||
      ( SOCKET_ERROR == ::connect( m_handle, (const sockaddr*)&m_address, sizeof( m_address ) ) ) )
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
: m_socket( 0 ),
  m_timeout( streamsock::infiniteTimeout ),
  m_short_reads( 0 ),
  m_short_writes( 0 ),
  m_pbase_pos( 0 ),
  m_eback_pos( 0 ),
  m_allocation_limit( 0 )
{
  // The allocation limit should exceed the
  // expected maximum size of a single
  // message by a safe amount.
  //
  // Under normal circumstances, the exact
  // value of the allocation limit does not
  // have any influence on memory usage.
  // Rather, it limits the amount of allocated
  // memory in worst-case scenarios, to avoid
  // worsening the situation by running into
  // out-of-physical-memory problems.
  m_allocation_limit = 16*1024*1024;
  sync_gbuf _(this);
  streambuf::setg( 0, 0, 0 );
  sync_pbuf __(this);
  streambuf::setp( 0, 0 );
}

sockbuf::~sockbuf()
{
}

bool
sockbuf::socket_connected()
{
  sync_socket _(this);
  return m_socket && m_socket->connected();
}

bool
sockbuf::socket_can_read()
{
  sync_socket _(this);
  return m_socket && m_socket->can_read();
}

bool
sockbuf::socket_can_write()
{
  sync_socket _(this);
  return m_socket && m_socket->can_write();
}

streamsize
sockbuf::showmanyc()
{
  read_from_socket( 0 );
  streamsize count = check_read_buffer();
  if( count < 1 )
    count = socket_can_read() ? 1 : 0;
  // In theory, we should return -1 to indicate failure if m_socket == 0,
  // but that might break some existing code that checks for (in_avail() != 0).
  return count;
}

ios::int_type
sockbuf::underflow()
{
  if( !check_for_data( m_timeout ) )
  {
    ++m_short_reads;
    return traits_type::eof();
  }
  sync_gbuf _(this);
  return traits_type::to_int_type( *gptr() );
}

ios::int_type
sockbuf::overflow( int c )
{
  ios::int_type fail = traits_type::eof();
  if( !is_open() )
    throw "Not connected";
  if( !ensure_write_buffer() )
    throw "Could not allocate";

  if( c != traits_type::eof() )
  {
    sync_pbuf _(this);
    *pptr() = c;
    pbump( 1 );
  }
  return traits_type::to_int_type( c );
}

streamsize
sockbuf::xsgetn( char* p, streamsize n )
{
  streamsize count = 0;
  while( count < n )
  {
    sync_gbuf _(this);
    streamsize avail = check_for_data( m_timeout );
    avail = min( n - count, avail );
    if( avail <= 0 )
    {
      ++m_short_reads;
      return count;
    }
    ::memcpy( p + count, gptr(), avail );
    gbump( avail );
    count += avail;
  }
  return count;
}

streamsize
sockbuf::xsputn( const char* p, streamsize n )
{
  streamsize count = 0;
  while( count < n )
  {
    overflow( traits_type::eof() );
    sync_pbuf _(this);
    size_t avail = min( n - count, epptr() - pptr() );
    if( !avail )
    {
      ++m_short_writes;
      return count;
    }
    ::memcpy( pptr(), p + count, avail );
    pbump( avail );
    count += avail;
  }
  return count;
}

int
sockbuf::sync()
{
  if( !is_open() )
    return traits_type::eof();
  if( write_to_socket( m_timeout ) >= 0 )
    return 0;
  return traits_type::eof();
}

streampos
sockbuf::seekoff( streamoff off, ios_base::seekdir way, ios_base::openmode which )
{
  const streampos fail = streamoff( -1 );
  if( way != ios_base::cur || off != 0 )
    return fail;
  if( which & ios_base::out )
  {
    if( which & ios_base::in )
      return fail;
    if( m_pbase_pos < 0 )
      return fail;
    sync_pbuf _(this);
    return m_pbase_pos + streamoff( pptr() - pbase() );
  }
  if( which & ios_base::in )
  {
    if( m_eback_pos < 0 )
      return fail;
    sync_gbuf _(this);
    return m_eback_pos + streamoff( gptr() - eback() );
  }
  return fail;
}

size_t
sockbuf::send_bufsize() const
{
  return 8 * 1024;
}

size_t
sockbuf::recv_bufsize() const
{
  size_t s = send_bufsize();
  if( m_socket && m_socket->max_msg_size() )
    s = max( s, m_socket->max_msg_size() );
  return s;
}

int
sockbuf::send( buffer* pBuf )
{
  int sent = 0;
  while( pBuf->read_count < pBuf->write_count && is_open() )
  {
    sync_pbuf _(this);
    size_t count = m_socket->write( pBuf->data + pBuf->read_count, pBuf->write_count - pBuf->read_count );
    sent += count;
    pBuf->read_count += count;
  }
  return sent;
}

int
sockbuf::recv( buffer* pBuf )
{
  // If your program blocks here, changing the timeout value will not help.
  // Quite likely, this is due to a situation where all transmitted data has been read
  // but underflow() is called from the stream via snextc() to examine whether
  // there is an eof pending.
  // There are two workarounds:
  // 1) Make sure that the last transferred byte of a message is a terminating character,
  // 2) read the last byte of a message with get() rather than read().
  // The reason for this problem is fundamental because there is no "maybe eof"
  // alternative to returning eof().
  int received = 0;
  size_t avail = pBuf->size() - pBuf->write_count;
  while( avail && avail >= m_socket->max_msg_size() && socket_can_read() )
  {
    sync_gbuf _(this);
    size_t count = m_socket->read( pBuf->data + pBuf->write_count, avail );
    received += count;
    pBuf->write_count += count;
    avail = pBuf->size() - pBuf->write_count;
  }
  if( avail == 0 || avail < m_socket->max_msg_size() )
    pBuf->done = true;
  return received;
}

streamsize
sockbuf::check_read_buffer()
{
  sync_gbuf _(this);
  if( gptr() == egptr() )
  {
    lock_gbuf _(this);
    buffer* r = m_gbufs.read;
    r->read_count = gptr() - eback();
    while( r->write_count == r->read_count && r != m_gbufs.write )
    {
      m_eback_pos += r->read_count;
      r = m_gbufs.advance_read();
    }
    streambuf::setg( r->data, r->data + r->read_count, r->data + r->write_count );
  }
  return egptr() - gptr();
}

streamsize
sockbuf::check_for_data( int timeout )
{
  read_from_socket( 0 );
  streamsize result = check_read_buffer();
  if( !result && read_from_socket( timeout ) > 0 )
  {
    result = check_read_buffer();
    while( !result && socket_connected() )
    {
      streamsock::sleep( 10 );
      result = check_read_buffer();
    }
  }
  return result;
}

bool
sockbuf::ensure_write_buffer()
{
  sync_pbuf _(this);
  m_pbufs.write->write_count = pptr() - pbase();
  if( epptr() == pptr() )
  {
    lock_pbuf _(this);
    size_t allocated = m_pbufs.bytes_allocated();
    if( allocated > m_allocation_limit )
      allocated = m_pbufs.gc().bytes_allocated();
    if( allocated < m_allocation_limit )
    {
      m_pbase_pos += m_pbufs.write->write_count;
      buffer& buf = *m_pbufs.advance_write( send_bufsize() );
      setp( buf.data, buf.data + buf.size() );
    }
  }
  return pptr() < epptr();
}

int
sockbuf::read_from_socket( int timeout )
{
  if( !socket_connected() )
    return -1;
  if( !socket_can_read() && !m_socket->wait_for_read( timeout ) )
    return 0;

  int received = 0;
  while( socket_can_read() )
  {
    if( m_gbufs.write->done )
    {
      lock_gbuf _(this);
      size_t allocated = m_gbufs.bytes_allocated();
      if( allocated > m_allocation_limit )
        allocated = m_gbufs.gc().bytes_allocated();
      if( allocated < m_allocation_limit )
        m_gbufs.advance_write( recv_bufsize() );
      else
        return received;
    }
    received += recv( m_gbufs.write );
  }
  return received;
}

int
sockbuf::write_to_socket( int timeout )
{
  if( !socket_connected() )
    return -1;
  if( socket_can_write() || m_socket->wait_for_write( timeout ) )
  {
    buffer* write, *read;
    {
      lock_pbuf _(this);
      write = m_pbufs.write;
      read = m_pbufs.read;
      write->write_count = pptr() - pbase();
    }
    while( read != write )
    {
      send( read );
      if( !is_open() )
        return -1;
      lock_pbuf _(this);
      read = m_pbufs.advance_read();
    };
    send( write );
  }
  lock_pbuf _(this);
  m_pbufs.write->write_count = pptr() - pbase();
  return m_pbufs.bytes_used();
}

sockbuf::buffers::buffers()
{
  write = new buffer;
  write->next = write;
  read = write;
}

sockbuf::buffers::~buffers()
{
  buffer* p = read;
  while( p->next != read )
  {
    buffer* q = p;
    p = p->next;
    delete q;
  }
  delete p;
}

sockbuf::buffer*
sockbuf::buffers::advance_read()
{
  return read = read->next;
}

sockbuf::buffer*
sockbuf::buffers::advance_write( size_t size )
{
  if( write->next == read && read->data )
  {
    write->next = new buffer;
    write->next->next = read;
  }
  buffer* p = write->next;
  p->write_count = 0;
  p->read_count = 0;
  p->done = false;
  if( !p->data )
    p->allocate( size );
  return write = p;
}

size_t
sockbuf::buffers::bytes_used() const
{
  size_t used = read->write_count - read->read_count;
  for( buffer* p = read; p != write; p = p->next )
    used += p->next->write_count - p->next->read_count;
  return used;
}

size_t
sockbuf::buffers::bytes_allocated() const
{
  size_t allocated = read->size();
  for( buffer* p = read; p->next != read; p = p->next )
    allocated += p->next->size();
  return allocated;
}

sockbuf::buffers&
sockbuf::buffers::gc()
{
  buffer* p = write;
  while( p->next != read )
  {
    buffer* q = p->next;
    p->next = p->next->next;
    delete q;
  }
  return *this;
}

ostream&
sockbuf::debug_print( ostream& os )
{
  os << "streambuf: short reads: " << m_short_reads
     << ", short writes: " << m_short_writes
     << endl;
  os << "socket: " << ( socket_connected() ? "connected" : "closed" )
     << ", can read: " << ( socket_can_read() ? "yes" : "no" )
     << ", can write: " << ( socket_can_write() ? "yes" : "no" )
     << endl;
  {
    lock_gbuf _(this);
    os << "gbufs bytes allocated: " << dec << m_gbufs.bytes_allocated() << endl;
    os << hex
       << "eback: " << (void*)eback()
       << ", gptr: " << (void*)gptr()
       << ", egptr: " << (void*)egptr()
       << ", gbufs.read: " << (void*)m_gbufs.read->data
       << ", gbufs.write: " << (void*)m_gbufs.write->data
       << endl;
    os << "write: ";
    buffer* p = m_gbufs.write;
    while( p->next != m_gbufs.write )
    {
      os << p << ":" << p->read_count << ":" << p->write_count << " -> ";
      p = p->next;
    }
    os << p << ":" << p->read_count << ":" << p->write_count
       << "-> write\nread: " << m_gbufs.read << endl;
  }
  {
    lock_pbuf _(this);
    os << "pbufs bytes allocated: " << dec << m_pbufs.bytes_allocated() << endl;
    os << hex
       << "pbase: " << (void*)pbase()
       << ", pptr: " << (void*)pptr()
       << ", epptr: " << (void*)epptr()
       << ", pbufs.read: " << (void*)m_pbufs.read->data
       << ", pbufs.write: " << (void*)m_pbufs.write->data
       << dec << "|" << m_pbufs.write->size() << "|" << m_pbufs.write->write_count
       << endl;
  }
  return os;
}

////////////////////////////////////////////////////////////////////////////////
// sockstream definitions
////////////////////////////////////////////////////////////////////////////////
sockstream::sockstream( sockbuf* s )
: iostream( 0 ),
  owned_buf( 0 ),
  buf( s )
{
  if( !buf )
  {
    owned_buf = new sockbuf;
    buf = owned_buf;
  }
  init( buf );
}

sockstream::sockstream( streamsock& s )
: iostream( 0 ),
  owned_buf( new sockbuf ),
  buf( owned_buf )
{
  init( buf );
  open( s );
}

void
sockstream::open( streamsock& s )
{
  if( !buf->open( s ) )
    setstate( ios::failbit );
}

void
sockstream::close()
{
  if( !buf->close() )
    setstate( ios::failbit );
}

