/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef USysLogH
#define USysLogH

#include <ComCtrls.hpp>
#include <SyncObjs.hpp>

class SYSLOG
{
  public:
    enum LogEntryMode
    {
      logEntryNormal = 0,
      logEntryWarning,
      logEntryError,
      numLogEntryModes
    };
    SYSLOG();
    ~SYSLOG();
    void ShowSysLog();
    void HideSysLog();
    bool Visible() const;
    bool Close( bool force = false );
    void AddSysLogEntry( const char* text, LogEntryMode = logEntryNormal );

  private:
    TForm*            mpForm;
    TRichEdit*        mpLog;
    TCriticalSection* mpCritsec;
    bool              mDontClose;
};
#endif // USysLogH

