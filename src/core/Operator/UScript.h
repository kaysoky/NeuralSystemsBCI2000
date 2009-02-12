/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef UScriptH
#define UScriptH
//---------------------------------------------------------------------------

#include <iostream>

class ParamList;
class StateList;
class SYSLOG;
class TfMain;

class SCRIPT
{
private:  // User declarations
        ParamList       *paramlist;
        StateList       *statelist;
        SYSLOG          *syslog;
        TfMain          *fMain;
        int             get_argument(int ptr, char *buf, const char *line, int maxlen);
        int             cur_line;
        char            filename[256];
public:  // User declarations
                SCRIPT::SCRIPT(ParamList *paramlist, StateList *statelist, SYSLOG *syslog, TfMain *fMain);
        int     SCRIPT::ExecuteScript(const char *filename);
        int     SCRIPT::ExecuteScript( std::istream& );
        int     SCRIPT::ExecuteCommand(const char *line);
};

#endif
