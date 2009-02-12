//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A std::ostream based accessor interface to add events
//   to a globally maintained event queue.
//   Basing the accessor on std::ostream allows for convenient
//   conversion of numbers (e.g., state values) into event descriptor
//   strings.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#ifndef BCI_EVENT_H
#define BCI_EVENT_H

#include "EventQueue.h"
#include <sstream>

#define bcievent BCIEvent()()

class DataIOFilter;

class BCIEvent : public std::ostream
{
 friend class ::DataIOFilter;

 class StringBuf;
 friend class StringBuf;

 public:
  BCIEvent()
    : std::ostream( 0 )
    { this->init( &mBuf ); }
  ~BCIEvent()
    { flush(); }
  std::ostream& operator()( const char* inDescriptor = "" )
    { return *this << inDescriptor; }

 private:
  static void SetEventQueue( EventQueue& inQueue )
    { spQueue = &inQueue; }

  static EventQueue* spQueue;

  class StringBuf : public std::stringbuf
  {
   public:
    StringBuf()
    : std::stringbuf( std::ios_base::out )
    {}
   protected:
    // This function gets called on ostream::flush().
    virtual int sync();
  } mBuf;
};

#endif // BCI_EVENT_H

