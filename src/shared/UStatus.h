//---------------------------------------------------------------------------
#ifndef UStatusH
#define UStatusH

#define ERRSTATUS_NOERR          0

#define LENGTH_STATUSLINE        512

class STATUS
{
private: 	// User declarations
        char    status[LENGTH_STATUSLINE];
        int     code;
public:		// User declarations
        int     ParseStatus(char *line, int length);
        char    *GetStatus();
        int     GetCode();
};

//---------------------------------------------------------------------------
#endif

