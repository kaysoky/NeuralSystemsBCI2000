//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A wrapper for event objects.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#ifndef OS_EVENT_H
#define OS_EVENT_H

class OSEvent
{
 public:
  OSEvent();
  virtual ~OSEvent();

  bool Set() const;
  bool Reset() const;
  bool Wait() const;

 private:
  HANDLE mHandle;
};

#endif // OS_EVENT_H
