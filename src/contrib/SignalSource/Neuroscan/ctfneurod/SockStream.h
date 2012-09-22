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
#ifndef SOCK_STREAM_H
#define SOCK_STREAM_H

#ifdef _WIN32
# include <winsock.h>
#else
# include <netinet/in.h>
# define SOCKET int
# ifdef __GNUC__
#  if __GNUC__ < 3
#   define IN_AVAIL_BROKEN
//  In the STL shipped with older gcc versions, streambuf::in_avail() does not call
//  streambuf::showmanyc() as it should.
//  Replacing in_avail() with showmanyc() should be logically correct for all streambufs
//  (though it might adversely affect performance).
#  endif
# endif  // __GNUC__
#endif // _WIN32

#include <string>
#include <iostream>
#include <set>

class streamsock
{
  public:
    enum
    {
      infiniteTimeout = -1,
      defaultTimeout = 5000, // ms
    };
    typedef union address
    {
      struct sockaddr sa;
      struct sockaddr_in sa_in;
    } address;

  private:
    streamsock( const streamsock& );            // Don't allow copies.
    streamsock& operator=( const streamsock& ); // Don't allow assignment.

  protected:
    streamsock();

  public:
    virtual     ~streamsock();
    void        open();
    void        open( const char* address );
    void        open( const char* ip, unsigned short port );
    void        close();
    bool        is_open() const; // A streamsock may be open but not connected.
    bool        connected();
    std::string ip() const;
    unsigned short port() const;
    // If there is data available, this function returns true.
    bool        can_read()
                { return wait_for_read( 0 ); }
    bool        wait_for_read( int timeout = defaultTimeout, bool return_on_accept = false );
    // As soon as the streamsock is ready for writing, this function returns true.
    bool        can_write()
                { return wait_for_write( 0 ); }
    bool        wait_for_write( int timeout = defaultTimeout, bool return_on_accept = false );

    size_t      read( char* buffer, size_t count );
    size_t      write( const char* buffer, size_t count );

    typedef std::set<streamsock*> set_of_instances;
    // These functions block until the given sockets are prepared to read or write.
    static bool wait_for_read( const set_of_instances&,
                               int timeout = defaultTimeout,
                               bool return_on_accept = false );
    static bool wait_for_write( const set_of_instances&,
                                int timeout = defaultTimeout,
                                bool return_on_accept = false );

    struct ip_compare
    { bool operator()( const std::string&, const std::string& ); };
    typedef std::set<std::string, ip_compare> set_of_addresses;
    // Return a list of local addresses. Addresses will be ordered by "externality":
    // local (127), auto (169), internal (10,192), external.
    static set_of_addresses local_addresses();


  private:
    virtual void do_open() = 0;
    virtual void do_accept() {}
    void         set_address( const char* address );
    void         set_address( const char* ip, unsigned short port );

  protected:
    virtual void set_socket_options();
    void         update_address();

  protected:
    SOCKET  m_handle;
    bool    m_listening;
    address m_address;

  // static members
  private:
    static int s_instance_count;
};

class tcpsocket : public streamsock
{
  public:
    tcpsocket() : m_tcpnodelay( false )
      {}
    void set_tcpnodelay( bool in_val )
      { m_tcpnodelay = in_val; set_socket_options(); }
    bool tcpnodelay() const
      { return m_tcpnodelay; }
    void set_handle( SOCKET );
  protected:
    virtual void set_socket_options();
  private:
    virtual void do_open() {}
    bool m_tcpnodelay;
};

class server_tcpsocket : public tcpsocket
{
  private:
    server_tcpsocket( const server_tcpsocket& ); // prevent copying
    server_tcpsocket& operator=( const server_tcpsocket& ); // prevent assignment
  public:
    server_tcpsocket()
      {}
    explicit server_tcpsocket( const char* address )
      { open( address ); }
    virtual ~server_tcpsocket()
      {}
    bool wait_for_accept( tcpsocket&, int timeout = defaultTimeout );
  private:
    virtual void do_open();
    virtual void do_accept();
};

class client_tcpsocket : public tcpsocket
{
  private:
    client_tcpsocket( const client_tcpsocket& ); // prevent copying
    client_tcpsocket& operator=( const client_tcpsocket& ); // prevent assignment
  public:
    client_tcpsocket()
      {}
    explicit client_tcpsocket( const char* address )
      { open( address ); }
    virtual ~client_tcpsocket()
      {}
  private:
    virtual void do_open();
};

class receiving_udpsocket : public streamsock
{
  private:
    receiving_udpsocket( const receiving_udpsocket& ); // prevent copying
    receiving_udpsocket& operator=( const receiving_udpsocket& ); // prevent assignment
  public:
    receiving_udpsocket()
      {}
    explicit receiving_udpsocket( const char* address )
      { open( address ); }
    virtual ~receiving_udpsocket()
      {}
  private:
    virtual void do_open();
};

class sending_udpsocket : public streamsock
{
  private:
    sending_udpsocket( const sending_udpsocket& ); // prevent copying
    sending_udpsocket& operator=( const sending_udpsocket& ); // prevent assignment
  public:
    sending_udpsocket()
      {}
    explicit sending_udpsocket( const char* address )
      { open( address ); }
    virtual ~sending_udpsocket()
      {}
  protected:
    virtual void set_socket_options();
  private:
    virtual void do_open();
};

class sockbuf : public std::streambuf
{
    enum
    {
      // Use 512 as a small buffer size to trigger bugs with messages that exceed it.
      // For efficient operation, use something like 64k.
      c_buf_size = 64 * 1024,
    };

  public:
    sockbuf();
    virtual ~sockbuf();
    void     set_timeout( int t )
             { m_timeout = t; }
    int      get_timeout() const
             { return m_timeout; }
    bool     is_open() const
             { return m_socket && m_socket->connected(); }
    sockbuf* open( streamsock& s )
             { m_socket = &s; return this; }
    sockbuf* close()
             { m_socket = NULL; return this; }

#ifdef IN_AVAIL_BROKEN
  public:
#else
  protected:
#endif // IN_AVAIL_BROKEN
    virtual std::streamsize showmanyc();           // Called from streambuf::in_avail().

  protected:
    virtual std::ios::int_type underflow();        // Called from read operations if empty.
    virtual std::ios::int_type overflow( int c );  // Called if write buffer is filled.
    virtual int sync();                            // Called from iostream::flush().

  protected:
    streamsock* m_socket;
    int         m_timeout;
};

class sockstream : public std::iostream
{
  public:
    sockstream();
    explicit sockstream( streamsock& );
    virtual ~sockstream()
             {}
    operator void*()
             { return std::iostream::operator void*(); }
    void     set_timeout( int t )
             { buf.set_timeout( t ); }
    int      get_timeout() const
             { return buf.get_timeout(); }
    bool     is_open() const
             { return buf.is_open(); }
    void     open( streamsock& );
    void     close();

  private:
    sockbuf buf;
};

#ifdef IN_AVAIL_BROKEN
# define in_avail showmanyc
#endif // IN_AVAIL_BROKEN

#endif // SOCK_STREAM_H
