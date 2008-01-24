////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A pair of filters that send/receive states and signals over a
//         UDP connection.
//
//         Data transmission is done via UDP socket connections.
//         Messages consist in a name and a value, separated by white space
//         and terminated with a single newline '\n' character.
//
//         Names may identify
//         -- BCI2000 states by name, and are then followed
//            by an integer value in decimal ASCII representation;
//         -- Signal elements in the form Signal(<channel>,<element>), and are
//            then followed by a float value in decimal ASCII representation.
//
//         Examples:
//           Running 0
//           ResultCode 2
//           Signal(1,2) 1e-8
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef CONNECTOR_FILTERS_H
#define CONNECTOR_FILTERS_H

#include "GenericFilter.h"

#include "SockStream.h"
#include <string>

class ConnectorInput : public GenericFilter
{
 public:
          ConnectorInput();
  virtual ~ConnectorInput();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual bool AllowsVisualization() const { return false; }

 private:
  std::string         mConnectorInputAddress;
  receiving_udpsocket mSocket;
  sockstream          mConnection;
  std::string         mInputFilter;
  bool                mAllowAny;
};

class ConnectorOutput : public GenericFilter
{
 public:
          ConnectorOutput();
  virtual ~ConnectorOutput();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual bool AllowsVisualization() const { return false; }

 private:
  std::string       mConnectorOutputAddress;
  sending_udpsocket mSocket;
  sockstream        mConnection;
};
#endif // CONNECTOR_FILTERS_H


