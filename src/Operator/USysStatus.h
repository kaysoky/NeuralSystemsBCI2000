/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef USysStatusH
#define USysStatusH

#include <dstring.h>

typedef enum
{
  Operator = 0,
  EEGSource,
  SigProc,
  App,
  numMessageOrigins
} MessageOrigin;


struct SYSSTATUS
{
  SYSSTATUS();
  void ResetSysStatus();

  static const numModules = 4;
  enum State
  {
    Idle = 0,
    Publishing,
    Information,
    Initialization,
    Resting,
    RestingParamsModified,
    RunningInitiated,
    Running,
    SuspendInitiated,
    Suspended,
    SuspendedParamsModified,
    Fatal,
    numStates
  } SystemState;

  AnsiString Status[ numModules ],
             Address[ numModules ];
  bool       EOS[ numModules ];
  bool       INI[ numModules ];
  bool       suspendConfirmed[ numModules ];
  bool       runningConfirmed[ numModules ];
  long       NumMessagesRecv[ numModules ];
  long       NumStatesRecv[ numModules ];
  long       NumStateVecsRecv[ numModules ];
  long       NumParametersRecv[ numModules ];
  long       NumDataRecv[ numModules ];
  long       NumMessagesSent[ numModules ];
  long       NumStatesSent[ numModules ];
  long       NumStateVecsSent[ numModules ];
  long       NumParametersSent[ numModules ];
};

#endif // USysStatusH



