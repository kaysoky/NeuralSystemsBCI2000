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
#ifndef BCI_MESSAGE_CHANNEL_H
#define BCI_MESSAGE_CHANNEL_H

#include <iostream>
#include "Uncopyable.h"
#include "ProtocolVersion.h"

class Status;
class Param;
class State;
class VisSignalConst;
class VisSignal;
class VisMemo;
class VisSignalProperties;
class VisBitmap;
class VisCfg;
class StateVector;
class SysCommand;

namespace Tiny { class LockableObject; }

namespace bci
{

class MessageChannel : Uncopyable
{
  public:
    MessageChannel( std::iostream& );
    MessageChannel( std::istream&, std::ostream& );
    MessageChannel( std::istream& );
    MessageChannel( std::ostream& );
    virtual ~MessageChannel();

    // Methods
    void HandleMessage();
    template<typename T>
      bool Send( const T& );
    void ResetStatistics();

    // Properties
    std::ostream& Output()
      { return mrOutput; }
    std::istream& Input()
      { return mrInput; }
    void SetOutputLock( Tiny::LockableObject* p )
      { mpOutputLock = p; }
    void SetInputLock( Tiny::LockableObject* p )
      { mpInputLock = p; }

    const ProtocolVersion& Protocol() const
      { return mProtocol; }
    int MessagesSent() const
      { return mMessagesSent; }
    int MessagesReceived() const
      { return mMessagesReceived; }
    std::streamsize BytesSent() const
      { return mBytesSent; }
    std::streamsize BytesReceived() const
      { return mBytesReceived; }

  protected:
    // Callback interface for inheritants to hook in. The return value indicates
    // whether the content was read (and removed) from the stream.
    // Note that the name of a handler should be On<ClassName> to
    // allow the CONSIDER macro to work.
    virtual bool OnProtocolVersion( std::istream& );
    virtual bool OnStatus( std::istream& ) { return false; }
    virtual bool OnParam( std::istream& ) { return false; }
    virtual bool OnState( std::istream& ) { return false; }
    virtual bool OnVisSignal( std::istream& ) { return false; }
    virtual bool OnVisMemo( std::istream& ) { return false; }
    virtual bool OnVisCfg( std::istream& ) { return false; }
    virtual bool OnStateVector( std::istream& ) { return false; }
    virtual bool OnSysCommand( std::istream& ) { return false; }
    virtual bool OnVisSignalProperties( std::istream& ) { return false; }
    virtual bool OnVisBitmap( std::istream& ) { return false; }

    virtual bool OnSend( const ProtocolVersion& ) { return true; }
    virtual bool OnSend( const Status& ) { return true; }
    virtual bool OnSend( const Param& ) { return true; }
    virtual bool OnSend( const State& ) { return true; }
    virtual bool OnSend( const VisSignalConst& ) { return true; }
    virtual bool OnSend( const VisMemo& ) { return true; }
    virtual bool OnSend( const VisCfg& ) { return true; }
    virtual bool OnSend( const StateVector& ) { return true; }
    virtual bool OnSend( const SysCommand& ) { return true; }
    virtual bool OnSend( const VisSignalProperties& ) { return true; }
    virtual bool OnSend( const VisBitmap& ) { return true; }

    ProtocolVersion& Protocol()
      { return mProtocol; }

  private:
    void Init();

    template<class content_type> struct Header;
    std::ostream& mrOutput;
    std::istream& mrInput;
    Tiny::LockableObject* mpOutputLock, *mpInputLock;
    ProtocolVersion mProtocol;

    int mMessagesSent, mMessagesReceived;
    std::streamsize mBytesSent, mBytesReceived;

};

template<> struct MessageChannel::Header<ProtocolVersion>
{ enum { descSupp = 0x0000 }; };
template<> struct MessageChannel::Header<Status>
{ enum { descSupp = 0x0100 }; };
template<> struct MessageChannel::Header<Param>
{ enum { descSupp = 0x0200 }; };
template<> struct MessageChannel::Header<State>
{ enum { descSupp = 0x0300 }; };
template<> struct MessageChannel::Header<VisSignalConst>
{ enum { descSupp = 0x0401 }; };
template<> struct MessageChannel::Header<VisSignal>
{ enum { descSupp = 0x0401 }; };
template<> struct MessageChannel::Header<VisMemo>
{ enum { descSupp = 0x0402 }; };
template<> struct MessageChannel::Header<VisSignalProperties>
{ enum { descSupp = 0x0403 }; };
template<> struct MessageChannel::Header<VisBitmap>
{ enum { descSupp = 0x0404 }; };
template<> struct MessageChannel::Header<VisCfg>
{ enum { descSupp = 0x04ff }; };
template<> struct MessageChannel::Header<StateVector>
{ enum { descSupp = 0x0500 }; };
template<> struct MessageChannel::Header<SysCommand>
{ enum { descSupp = 0x0600 }; };

} // namespace bci


using bci::MessageChannel;

#endif // BCI_MESSAGE_CHANNEL_H

