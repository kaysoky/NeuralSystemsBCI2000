//-------------------------------------------------
//	BCIDirectry Class
//	BCI Dircetory Management Functions
//-------------------------------------------------

#include <dir.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//--------------------------------------------------

#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ComCtrls.hpp>

class TFEForm : public TForm
{
private:
        TButton  *OKButton;
        TEdit *BadPath;
        void __fastcall OKButtonClick(TObject *Sender );
public:

        virtual __fastcall TFEForm( TApplication *, char *);
};


//--------------------------------------------------
class BCIDtry
{
private:
	char SubjDir[80];
	char SubjName[80];
	char SubjSession[80];
	char SubjPath[120];
public:
	int CheckPath( char * );
	int MakePath( char * );
	int CheckSubDir( char * );
	int MakeSubDir( char * );
        void SetDir( char * );
        void SetName( char* );
        void SetSession( char * );
        int ProcPath( void );
        char *ProcSubDir( void );
        void FileError( TApplication *, char * );
} ;
