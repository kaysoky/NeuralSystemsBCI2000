////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Sending and dispatching of BCI2000 messages.
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
#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <iostream>
#include "MessageQueue.h"
#include "SockStream.h"

class MessageHandler
{
  protected:
    MessageHandler() {}
    virtual ~MessageHandler() {}

  public:
    // Calling interface to the outside world.
    void HandleMessage( MessageQueue& );
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
    void CheckForError( std::istream&, const char* );
};

class ProtocolVersion;
template<> struct MessageHandler::Header<ProtocolVersion>
{ enum { descSupp = 0x0000 }; };

class Status;
template<> struct MessageHandler::Header<Status>
{ enum { descSupp = 0x0100 }; };

class Param;
template<> struct MessageHandler::Header<Param>
{ enum { descSupp = 0x0200 }; };

class State;
template<> struct MessageHandler::Header<State>
{ enum { descSupp = 0x0300 }; };

class VisSignal;
template<> struct MessageHandler::Header<VisSignal>
{ enum { descSupp = 0x0401 }; };

class VisMemo;
template<> struct MessageHandler::Header<VisMemo>
{ enum { descSupp = 0x0402 }; };

class VisSignalProperties;
template<> struct MessageHandler::Header<VisSignalProperties>
{ enum { descSupp = 0x0403 }; };

class VisBitmap;
template<> struct MessageHandler::Header<VisBitmap>
{ enum { descSupp = 0x0404 }; };

class VisCfg;
template<> struct MessageHandler::Header<VisCfg>
{ enum { descSupp = 0x04ff }; };

class StateVector;
template<> struct MessageHandler::Header<StateVector>
{ enum { descSupp = 0x0500 }; };

class SysCommand;
template<> struct MessageHandler::Header<SysCommand>
{ enum { descSupp = 0x0600 }; };

#endif // MESSAGE_HANDLER_H

