////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for module status messages.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef STATUS_H
#define STATUS_H

#include <iostream>

class Status
{
 public:
  typedef enum
  {
    unknown,
    initialized,
    running,
    suspended,
    debug,
    warning,
    error,
  } ContentType;

  typedef enum
  {
    firstPlainMessageCode = 200,
    plainMessage = 200,
    lastPlainMessageCode = 298,

    debugMessage = 299,

    firstInitializedMessage = 200,
    sourceInitialized = 200,
    sigprocInitialized = 201,
    appInitialized = 202,

    firstRunningMessage = 203,
    sourceRunning = 203,
    sigprocRunning = 205,
    appRunning = 207,

    firstSuspendedMessage = 204,
    sourceSuspended = 204,
    sigprocSuspended = 206,
    appSuspended = 208,

    firstWarningCode = 300,
    warningMessage = 301,
    lastWarningCode = 399,

    firstErrorCode = 400,
    configurationError = 408,
    runtimeError = 409,
    fail = 498,
    logicError = 499,
    lastErrorCode = 499,
  } MessageCode;

  Status();
  Status( const std::string& message, int code );

  const std::string& Message() const
                     { return mMessage; }
  int                Code() const
                     { return mCode; }
  ContentType        Content() const;

  std::istream&      ReadFromStream( std::istream& );
  std::ostream&      WriteToStream( std::ostream& ) const;
  std::istream&      ReadBinary( std::istream& );
  std::ostream&      WriteBinary( std::ostream& ) const;

  static const Status Fail;

 private:
  std::string mMessage;
  int         mCode;
};


inline
std::ostream& operator<<( std::ostream& os, const Status& s )
{ return s.WriteToStream( os ); }

inline
std::istream& operator>>( std::istream& is, Status& s )
{ return s.ReadFromStream( is ); }

#endif // STATUS_H

