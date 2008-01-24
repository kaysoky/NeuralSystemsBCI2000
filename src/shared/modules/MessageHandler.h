////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Sending and dispatching of BCI2000 messages.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <iostream>
#include "SockStream.h"

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
    // A specialization that accounts for a sockstream's flushing needs.
    template<typename content_type>
    static std::ostream& PutMessage( sockstream& s, const content_type& c )
    { PutMessage( static_cast<std::ostream&>( s ), c ); s.flush(); return s; }

  protected:
    // Callback interface for inheritants to hook in. The return value indicates
    // whether the content was read (and removed) from the stream.
    // Because template functions cannot be virtual we need to explicitly
    // enumerate all of them.
    // Note that the name of a handler should be Handle<ClassName> to
    // allow the CONSIDER macro to work.
    virtual bool HandleProtocolVersion(     std::istream& ) { return false; }
    virtual bool HandleStatus(              std::istream& ) { return false; }
    virtual bool HandleParam(               std::istream& ) { return false; }
    virtual bool HandleState(               std::istream& ) { return false; }
    virtual bool HandleVisSignal(           std::istream& ) { return false; }
    virtual bool HandleVisMemo(             std::istream& ) { return false; }
    virtual bool HandleVisCfg(              std::istream& ) { return false; }
    virtual bool HandleStateVector(         std::istream& ) { return false; }
    virtual bool HandleSysCommand(          std::istream& ) { return false; }
    virtual bool HandleVisSignalProperties( std::istream& ) { return false; }
    virtual bool HandleVisBitmap(           std::istream& ) { return false; }

  private:
    template<typename content_type> struct Header;
};

template<> struct MessageHandler::Header<class ProtocolVersion>
{ enum { descSupp = 0x0000 }; };
template<> struct MessageHandler::Header<class Status>
{ enum { descSupp = 0x0100 }; };
template<> struct MessageHandler::Header<class Param>
{ enum { descSupp = 0x0200 }; };
template<> struct MessageHandler::Header<class State>
{ enum { descSupp = 0x0300 }; };
template<> struct MessageHandler::Header<class VisSignal>
{ enum { descSupp = 0x0401 }; };
template<> struct MessageHandler::Header<class VisMemo>
{ enum { descSupp = 0x0402 }; };
template<> struct MessageHandler::Header<class VisSignalProperties>
{ enum { descSupp = 0x0403 }; };
template<> struct MessageHandler::Header<class VisBitmap>
{ enum { descSupp = 0x0404 }; };
template<> struct MessageHandler::Header<class VisCfg>
{ enum { descSupp = 0x04ff }; };
template<> struct MessageHandler::Header<class StateVector>
{ enum { descSupp = 0x0500 }; };
template<> struct MessageHandler::Header<class SysCommand>
{ enum { descSupp = 0x0600 }; };

#endif // MESSAGE_HANDLER_H

