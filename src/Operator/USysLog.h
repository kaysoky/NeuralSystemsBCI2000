//---------------------------------------------------------------------------

#ifndef USysLogH
#define USysLogH
//---------------------------------------------------------------------------


class SYSLOG
{
private:	// User declarations
        TForm   *form;
        TMemo   *log;
public:		// User declarations
        SYSLOG::SYSLOG();
        SYSLOG::~SYSLOG();
        void    SYSLOG::ShowSysLog();
        void    SYSLOG::AddSysLogEntry(char *text);
};
#endif

