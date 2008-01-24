/////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: sjuergen.mellinger@uni-tuebingen.de
// Description: A class for OS error codes and messages.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
/////////////////////////////////////////////////////////////////////////////
#ifndef OS_ERROR_H
#define OS_ERROR_H

#include <string>

class OSError
{
 public:
  OSError(); // This constructor obtains the last error code.
  OSError( long inErrorCode )
  : mCode( inErrorCode )
  {}
  // Properties
  long        Code() const
              { return mCode; }
  const char* Message() const;

 private:
  long mCode;
  static char* spMessageBuffer;
};

#endif // OS_ERROR_H
