////////////////////////////////////////////////////////////////////////////////
//
// File: tcpstream.h
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
//                Note that only one address is maintained which is local
//                or remote address, dependent on context and socket state.
//                It might be a good idea to change this.
//              tcpstream: A std::iostream interface to the data stream on a
//                socket. Will wait for flush or eof before sending data.
//                Send/receive is blocking; one can use rdbuf()->in_avail()
//                to check for data.
//              tcpbuf: A helper class that does the actual send/receive
//                calls.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef TCPSTREAMH
#define TCPSTREAMH

#ifdef _WIN32
# include <winsock.h>
#else
# include <socket.h>
#endif

#include <iostream>
#include <set>

class tcpsocket
{
  public:
    enum
    {
      infiniteTimeout = -1,
      defaultTimeout = 5000, // ms
    };

  private:
    tcpsocket( const tcpsocket& );            // Don't allow copies.
    tcpsocket& operator=( const tcpsocket& ); // Don't allow assignments.

  protected:
    tcpsocket();

  public:
    virtual ~tcpsocket();
    void open();
    void open( const char* address );
    void open( const char* ip, u_short port );
    void close();
    bool is_open() const; // A socket may be open but not connected.
    bool connected();
    std::string ip() const;
    u_short port() const;
    // If there is data available, this function returns true.
    bool can_read() { return wait_for_read( 0 ); }
    bool wait_for_read( int timeout = defaultTimeout, bool return_on_accept = false );
    // As soon as the socket is ready for writing, this function returns true.
    bool can_write() { return wait_for_write( 0 ); }
    bool wait_for_write( int timeout = defaultTimeout, bool return_on_accept = false );

    size_t read( char* buffer, size_t count );
    size_t write( char* buffer, size_t count );

    typedef std::set<tcpsocket*> set_of_instances;
    // These functions block until the given sockets are prepared to read or write.
    static bool wait_for_read( const set_of_instances&,
                               int timeout = defaultTimeout,
                               bool return_on_accept = false );
    static bool wait_for_write( const set_of_instances&,
                                int timeout = defaultTimeout,
                                bool return_on_accept = false );

  private:
    virtual void do_open() = 0;
    void accept();
    void set_address( const char* address );
    void set_address( const char* ip, u_short port );

  protected:
    void update_address();

  protected:
    bool         m_listening;
    unsigned int m_handle;
    sockaddr_in  m_address;

  // static members
  private:
    static int s_instance_count;
};

class server_tcpsocket : public tcpsocket
{
  private:
    server_tcpsocket( const server_tcpsocket& ); // prevent copying
    server_tcpsocket& operator=( const server_tcpsocket& ); // prevent assignments
  public:
    server_tcpsocket()                               {}
    explicit server_tcpsocket( const char* address ) { open( address ); }
    virtual ~server_tcpsocket()                      {}
  private:
    virtual void do_open();
};

class client_tcpsocket : public tcpsocket
{
  private:
    client_tcpsocket( const client_tcpsocket& ); // prevent copying
    client_tcpsocket& operator=( const client_tcpsocket& ); // prevent assignments
  public:
    client_tcpsocket()                               {}
    explicit client_tcpsocket( const char* address ) { open( address ); }
    virtual ~client_tcpsocket()                      {}
  private:
    virtual void do_open();
};

class tcpbuf : public std::streambuf
{
    enum
    {
      // 512 is a small buffer size intended to trigger bugs with messages that exceed it.
      // For efficient operation, this should be something like 64k.
      // buf_size = 512,
      buf_size = 64 * 1024,
    };

  public:
    tcpbuf();
    virtual ~tcpbuf();
    void set_timeout( int t )       { m_timeout = t; }
    int  get_timeout() const        { return m_timeout; }
    bool is_open() const            { return m_socket && m_socket->connected(); }
    tcpbuf* open( tcpsocket& s )    { m_socket = &s; return this; }
    tcpbuf* close()                 { m_socket = NULL; return this; }

  protected:
    virtual int showmanyc();        // Called from streambuf::in_avail().
    virtual int underflow();        // Called from read operations if empty.
    virtual int overflow( int c );  // Called if write buffer is filled.
    virtual int sync();             // Called from iostream::flush().

  protected:
    tcpsocket* m_socket;
    int        m_timeout;
};

class tcpstream : public std::iostream
{
  public:
    tcpstream();
    explicit tcpstream( tcpsocket& );
    virtual ~tcpstream()            {}
    bool is_open() const            { return buf.is_open(); }
    void open( tcpsocket& );
    void close();

  private:
    tcpbuf buf;
};

#endif // TCPSTREAMH
