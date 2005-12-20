////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File: MessageHandler.h
//
// Author: Juergen Mellinger
//
// Date:   Jul 24, 2003
//
// $Log$
// Revision 1.5  2005/12/20 11:42:41  mellinger
// Added CVS id and log to comment.
//
//
// Description: Facilities for centralized management of BCI2000 messages.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef MessageHandlerH
#define MessageHandlerH

#include <iostream>
#include "TCPStream.h"

class MessageHandler
{
  protected:
    MessageHandler() {}
    virtual ~MessageHandler() {}

  public:
    // Calling interface to the outside world.
    void HandleMessage( std::istream& );

    // Message composing functions.
    template<typename content_type>
    static std::ostream& PutMessage( std::ostream&, const content_type& );
    // A specialization that accounts for tcpstream's flushing needs.
    template<typename content_type>
    static std::ostream& PutMessage( tcpstream& s, const content_type& c )
    { PutMessage( static_cast<std::ostream&>( s ), c ); s.flush(); return s; }

  private:
    // Callback interface for inheritants to hook in. The return value indicates
    // whether the content was read (and removed) from the stream.
    // Because template functions cannot be virtual we need to explicitly
    // enumerate all of them.
    // Note that the name of a handler should be Handle<ClassName> to
    // allow the CONSIDER macro to work.
    virtual bool HandleSTATUS(      std::istream& ) { return false; }
    virtual bool HandlePARAM(       std::istream& ) { return false; }
    virtual bool HandleSTATE(       std::istream& ) { return false; }
    virtual bool HandleVisSignal(   std::istream& ) { return false; }
    virtual bool HandleVisMemo(     std::istream& ) { return false; }
    virtual bool HandleVisCfg(      std::istream& ) { return false; }
    virtual bool HandleSTATEVECTOR( std::istream& ) { return false; }
    virtual bool HandleSYSCMD(      std::istream& ) { return false; }

  private:
    template<typename content_type> struct Header;
    template<> struct Header<class STATUS>
    { enum { descSupp = 0x0100 }; };
    template<> struct Header<class PARAM>
    { enum { descSupp = 0x0200 }; };
    template<> struct Header<class STATE>
    { enum { descSupp = 0x0300 }; };
    template<> struct Header<class VisSignal>
    { enum { descSupp = 0x0401 }; };
    template<> struct Header<class VisMemo>
    { enum { descSupp = 0x0402 }; };
    template<> struct Header<class VisCfg>
    { enum { descSupp = 0x04ff }; };
    template<> struct Header<class STATEVECTOR>
    { enum { descSupp = 0x0500 }; };
    template<> struct Header<class SYSCMD>
    { enum { descSupp = 0x0600 }; };
};

#endif // MessageHandlerH

