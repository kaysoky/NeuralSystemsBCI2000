////////////////////////////////////////////////////////////////////////////////
//
// File:   ConnectorFilters.cpp
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date:   Jun 14, 2005
//
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
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef ConnectorFiltersH
#define ConnectorFiltersH

#include "UGenericFilter.h"

#include "TCPStream.h"
#include <string>

class ConnectorInput : public GenericFilter
{
 public:
          ConnectorInput();
  virtual ~ConnectorInput();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void StartRun();
  virtual void StopRun();
  virtual void Process( const GenericSignal*, GenericSignal* );

 private:
  std::string         mConnectorInputAddress;
  receiving_udpsocket mSocket;
  tcpstream           mConnection;
  std::string         mInputFilter;
  bool                mAllowAny;
};

class ConnectorOutput : public GenericFilter
{
 public:
          ConnectorOutput();
  virtual ~ConnectorOutput();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void StartRun();
  virtual void StopRun();
  virtual void Process( const GenericSignal*, GenericSignal* );

 private:
  std::string       mConnectorOutputAddress;
  sending_udpsocket mSocket;
  tcpstream         mConnection;
};
#endif // ConnectorFiltersH


