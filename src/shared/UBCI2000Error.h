//---------------------------------------------------------------------------

#ifndef UBCI2000ErrorH
#define UBCI2000ErrorH
//---------------------------------------------------------------------------

#define ERR_MAXLENGTH   1024

class BCI2000ERROR
{
private: 	// User declarations
        char    error[ERR_MAXLENGTH];
        int     code;
public:		// User declarations
        void    SetErrorMsg(char *);
        char    *GetErrorMsg();
        int     GetErrorCode();
        void    CopyError(BCI2000ERROR *new_error);        
};
#endif
 