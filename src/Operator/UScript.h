//---------------------------------------------------------------------------

#ifndef UScriptH
#define UScriptH
//---------------------------------------------------------------------------

class PARAMLIST;
class STATELIST;
class SYSLOG;
class TfMain;

class SCRIPT
{
private:	// User declarations
        PARAMLIST       *paramlist;
        STATELIST       *statelist;
        SYSLOG          *syslog;
        TfMain          *fMain;
        int             get_argument(int ptr, char *buf, char *line, int maxlen);
        int             cur_line;
        char            filename[256];
public:		// User declarations
                SCRIPT::SCRIPT(PARAMLIST *paramlist, STATELIST *statelist, SYSLOG *syslog, TfMain *fMain);
        int     SCRIPT::ExecuteScript(char *filename);
        int     SCRIPT::ExecuteCommand(char *line);
};

#endif
