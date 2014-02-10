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
#endif // _WIN32

#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <list>

class streamsock
{
  public:
    enum
    {
      infiniteTimeout = -1,
      defaultTimeout = 5000, // ms
    };

  private:
    streamsock( const streamsock& );            // Don't allow copies.
    streamsock& operator=( const streamsock& ); // Don't allow assignment.

  protected:
    streamsock();

  public:
    virtual     ~streamsock();
    void        open( const std::string& address );
    void        open( const std::string& ip, unsigned short port );
    void        close();
    bool        is_open() const; // A streamsock may be open but not connected.
    enum { disconnected = 0, locally = 1, local = 1, remote = 2 };
    int connected();
    std::string ip() const;
    int port() const;
    std::string address() const;
    // If there is data available, this function returns true.
    bool        can_read()
                { return wait_for_read( 0 ); }
    bool        wait_for_read( int timeout = defaultTimeout, bool return_on_accept = false );
    // As soon as the streamsock is ready for writing, this function returns true.
    bool        can_write()
                { return wait_for_write( 0 ); }
    bool        wait_for_write( int timeout = defaultTimeout, bool return_on_accept = false );

    size_t      max_msg_size() const { return m_max_msg_size; }
    size_t      read( char* buffer, size_t count );
    size_t      write( const char* buffer, size_t count );

    enum { ok, retry, fatal };
    static int check_result( int );

    struct set_of_instances : std::vector<streamsock*>
    {
      iterator find( streamsock* s ) { for( iterator i = begin(); i != end(); ++i ) if( *i == s ) return i; return end(); }
      void insert( streamsock* s ) { if( find(s) == end() ) push_back( s ); }
      void erase( streamsock* s ) { for( iterator i = find(s); i != end(); i = find(s) ) std::vector<streamsock*>::erase(i); }
    };
    // These functions block until the given sockets are prepared to read or write.
    static bool wait_for_read( set_of_instances&,
                               int timeout = defaultTimeout,
                               bool return_on_accept = false );
    static bool wait_for_write( set_of_instances&,
                                int timeout = defaultTimeout,
                                bool return_on_accept = false );
    static void sleep( int );

    struct ip_compare
    { bool operator()( const std::string&, const std::string& ) const; };
    typedef std::set<std::string, ip_compare> set_of_addresses;
    // Return a list of local addresses. Addresses will be ordered by "externality":
    // local (127), auto (169), internal (10,192), external.
    static set_of_addresses local_addresses();


  private:
    virtual void do_open() = 0;
    virtual void do_accept() {}
    void init();
    bool set_address( const std::string& address );
    bool set_address( const std::string& ip, unsigned short port );

    static bool select( streamsock**, size_t,
                        streamsock**, size_t,
                        int timeout,
                        bool return_on_accept = false );

  protected:
    virtual void set_socket_options();
    void         update_address();

  protected:
    SOCKET      m_handle;
    bool        m_listening;
    sockaddr_in m_address;
    size_t      m_max_msg_size;
    int         m_address_type;
    bool        m_conn_reset, 
                m_ready_to_read, m_ready_to_write;
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
  public:
    server_tcpsocket()
      {}
    explicit server_tcpsocket( const std::string& address )
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
  public:
    client_tcpsocket()
      {}
    explicit client_tcpsocket( const std::string& address )
      { open( address ); }
    virtual ~client_tcpsocket()
      {}
  private:
    virtual void do_open();
};

class receiving_udpsocket : public streamsock
{
  public:
    receiving_udpsocket()
      {}
    explicit receiving_udpsocket( const std::string& address )
      { open( address ); }
    virtual ~receiving_udpsocket()
      {}
  private:
    virtual void do_open();
};

class sending_udpsocket : public streamsock
{
  public:
    sending_udpsocket()
      {}
    explicit sending_udpsocket( const std::string& address )
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
  public:
    sockbuf();
    virtual ~sockbuf();
    void     set_timeout( int t )
             { m_timeout = t; }
    int      get_timeout() const
             { return m_timeout; }
    void     set_allocation_limit( size_t limit )
             { m_allocation_limit = limit; }
    size_t   get_allocation_limit() const
             { return m_allocation_limit; }
    bool     is_open()
             { return socket_connected(); }
    sockbuf* open( streamsock& s )
             { m_socket = &s; return this; }
    sockbuf* close()
             { m_socket = NULL; return this; }

    std::ostream& debug_print( std::ostream& );

  protected:
    virtual std::streamsize showmanyc();           // Called from streambuf::in_avail().
    virtual std::ios::int_type underflow();        // Called from read operations if empty.
    virtual std::streamsize xsgetn( char*, std::streamsize );
    virtual std::ios::int_type overflow( int c );  // Called if write buffer is filled.
    virtual std::streamsize xsputn( const char*, std::streamsize );
    virtual int sync();                            // Called from iostream::flush().
    virtual std::streampos seekoff( std::streamoff, std::ios_base::seekdir, std::ios_base::openmode );

  protected:
    virtual size_t send_bufsize() const;
    virtual size_t recv_bufsize() const;

    virtual void socket_sync( bool ) {}
    virtual void pbuf_lock( bool ) {}
    virtual void pbuf_sync( bool ) {}
    virtual void gbuf_lock( bool ) {}
    virtual void gbuf_sync( bool ) {}

    virtual int write_to_socket( int );
    virtual int read_from_socket( int );

  private:
    bool socket_connected();
    bool socket_can_read();
    bool socket_can_write();

    bool ensure_write_buffer();
    std::streamsize check_read_buffer();
    std::streamsize check_for_data( int );

    typedef void (sockbuf::*lockfun)(bool);
    template<lockfun F> class lock
    {
     public:
      lock( sockbuf* s ) : mp_s( s ) { (mp_s->*F)(true); }
      ~lock() { (mp_s->*F)(false); }
     private:
      sockbuf* mp_s;
    };
    template<lockfun> friend class lock;
    typedef lock<&sockbuf::socket_sync> sync_socket;
    typedef lock<&sockbuf::pbuf_sync> sync_pbuf;
    typedef lock<&sockbuf::pbuf_lock> lock_pbuf;
    typedef lock<&sockbuf::gbuf_sync> sync_gbuf;
    typedef lock<&sockbuf::gbuf_lock> lock_gbuf;

    struct buffer
    {
      buffer* next;
      char* data;
      size_t write_count, read_count;
      bool done;
      buffer()
        : m_size( 0 ), next( 0 ), data( 0 ), write_count( 0 ), read_count( 0 ), done( false ) {}
      ~buffer()
        { next = 0; allocate( 0 ); }
      size_t size() const
        { return m_size; }
      void allocate( size_t s )
        { delete[] data; data = 0; m_size = s; if( m_size ) data = new char[m_size]; }
     private:
      buffer& operator=( buffer& );
      size_t m_size;
    };
    struct buffers
    {
      buffers();
      ~buffers();
      buffer* advance_read();
      buffer* advance_write( size_t );
      buffer* read, *write;
      size_t bytes_used() const;
      size_t bytes_allocated() const;
      buffers& gc();
    };
    int send( buffer* );
    int recv( buffer* );

    streamsock* m_socket;
    int m_timeout;
    size_t m_allocation_limit;
    buffers m_pbufs, m_gbufs;
    std::streampos m_pbase_pos, m_eback_pos;
    int m_short_reads, m_short_writes;
};

class sockstream : public std::iostream
{
  private:
    sockstream( const sockstream& );
    sockstream& operator=( const sockstream& );

  public:
    explicit sockstream( sockbuf* = 0 );
    explicit sockstream( streamsock& );
    virtual ~sockstream()
             { delete owned_buf; }
    operator void*()
             { return std::iostream::operator void*(); }
    void     set_timeout( int t )
             { buf->set_timeout( t ); }
    int      get_timeout() const
             { return buf->get_timeout(); }
    bool     is_open() const
             { return buf->is_open(); }
    void     open( streamsock& );
    void     close();

  private:
    sockbuf* owned_buf, *buf;
};

#endif // SOCK_STREAM_H
