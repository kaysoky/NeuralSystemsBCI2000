//---------------------------------------------------------------------------

#ifndef UScriptH
#define UScriptH
//---------------------------------------------------------------------------

#include "UParameter.h"
#include "UState.h"
#include "USysLog.h"

class SCRIPT
{
private:	// User declarations
        PARAMLIST       *paramlist;
        STATELIST       *statelist;
        SYSLOG          *syslog;
        TCustomWinSocket *socket;
        int             get_argument(int ptr, char *buf, char *line, int maxlen);
        int             cur_line;
        char            filename[256];
public:		// User declarations
        int     SCRIPT::Initialize(PARAMLIST *paramlist, STATELIST *statelist, SYSLOG *syslog, TCustomWinSocket *socket);
        int     SCRIPT::ExecuteScript(char *filename);
        int     SCRIPT::ExecuteCommand(char *line);
};

#endif
