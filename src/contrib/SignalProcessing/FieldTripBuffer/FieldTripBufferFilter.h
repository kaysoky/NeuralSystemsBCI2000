////////////////////////////////////////////////////////////////////////////////
// $Id: FieldTripBufferFilter.h,v 1.4 2008/06/20 08:34:38 jurmel Exp $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that maintains a FieldTrip Realtime buffer, writes
//   its input data into that buffer, and optionally gets its output data from
//   buffered events. BCI2000 state variables are mapped to buffer events.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef FIELDTRIP_BUFFER_FILTER_H
#define FIELDTRIP_BUFFER_FILTER_H

#include "GenericFilter.h"

#include "buffer.h"
#include <pthread.h>
#include <set>
#include <map>
#include <string>

class FieldTripBufferFilter : public GenericFilter
{
 public:
          FieldTripBufferFilter();
  virtual ~FieldTripBufferFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Process( const GenericSignal& Input, GenericSignal& Output );

 private:
  typedef std::set<std::string> StateSet;
  typedef std::map<std::string, int> StateValueMap;
  void ConfigureStateSets( StateSet& ioToBufferSet, StateSet& ioFromBufferSet ) const;
  void ProcessEvent( const event_t& );
  void PutStateToBuffer( const std::string& );
  template<typename T> void EventToSignal( const event_t&, GenericSignal& ) const;
  template<typename T> message_t* SendRequest( int, T*, void* = NULL ) const;
  message_t* SendRequest( int type ) const
             { return SendRequest<messagedef_t>( type, NULL ); }
  void DeleteMessage( message_t* ) const;
  void ParseHostAddress( const std::string&, host_t& ) const;

  std::string    mFTBufferAddress;

  GenericSignal  mSignalBuffer;
  std::string    mFTOutputEventType;
  StateSet       mFTStatesToBuffer,
                 mFTStatesFromBuffer;
  StateValueMap  mPreviousStateValues;

  unsigned long  mBlockCount;
  host_t         mHostAddress;
  float*         mpDataBuffer;
  UINT8_T*       mpEventBuffer;
  datadef_t      mDatadef;
  headerdef_t    mHeaderdef;

  pthread_attr_t mThreadAttr;
  pthread_t      mServerThread;
};

#endif // FIELDTRIP_BUFFER_FILTER_H


