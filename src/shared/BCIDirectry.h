//-------------------------------------------------
//	BCIDtry Class
//	BCI Directory Management Functions
//-------------------------------------------------

#ifndef BCIDIRECTRY_H
#define BCIDIRECTRY_H

#include <vcl.h>
#include <string>

//-------------------------------------------------

class TFEForm : public TForm
{
private:
        TButton  *OKButton;
        TEdit    *BadPath;
        void __fastcall OKButtonClick(TObject*);
public:
        __fastcall TFEForm( const char *);
};


//--------------------------------------------------
class BCIDtry
{
 private:
    static const char  DirSeparator = '\\';
    static const char  DriveSeparator = ':';
    static std::string BaseDir;

    std::string        SubjDir,
                       SubjName,
                       SubjSession,
                       SubjPath;

    static std::string GetCwd();
    int         ChangeForceDir( const std::string& );

 public:
    void        SetDir( const char * );
    void        SetName( const char* );
    void        SetSession( const char * );
    int         ProcPath( void );
    const char* ProcSubDir( void );
    void        FileError( const char* );
    void        FileError( TApplication *, const char *s ) { FileError( s ); }
    int         GetLargestRun( const char * );
} ;

#endif // BCIDIRECTRY_H
