//---------------------------------------------------------------------------

#ifndef USysCommandH
#define USysCommandH
//---------------------------------------------------------------------------

#define LENGTH_SYSCMD          256

#define ERRSYSCMD_NOERR        0

class SYSCMD
{
private: 	// User declarations
        char    buffer[LENGTH_SYSCMD];
public:		// User declarations
        SYSCMD::SYSCMD();
        SYSCMD::~SYSCMD();
        int     get_argument(int ptr, char *buf, char *line, int maxlen);
        int     ParseSysCmd(char *line, int length);
        char    *SYSCMD::GetSysCmd();
};

#endif
