//-------------------------------------------------
//	BCIDirectry.cpp
//	BCI Dircetory Management Functions
//-------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "BCIDirectry.h"
#include <stdlib.h>
#include <dir.h>
#include <errno.h>

//--------------------------------------------------

using namespace std;

string BCIDtry::BaseDir = BCIDtry::GetCwd();

string BCIDtry::GetCwd()
{
    const size_t    pathLenMax = 2048;
    const char*     cwd = ::getcwd( NULL, pathLenMax );
    string          cwdString = "";
    if( cwd != NULL )
    {
        cwdString = cwd;
        ::free( ( void* )cwd );
    }

    if( cwdString.length() < 1 || cwdString[ cwdString.length() - 1 ] != DirSeparator )
        cwdString += DirSeparator;

    return cwdString;
}

int BCIDtry::ChangeForceDir( const string& path )
{
    string fullPath = path;
    if( fullPath.length() < 2 || fullPath[ 1 ] != DriveSeparator )
        fullPath = BaseDir + fullPath;
    if( fullPath.length() < 1 || fullPath[ fullPath.length() - 1 ] != DirSeparator )
        fullPath += DirSeparator;
    int err = ::chdir( fullPath.c_str() );
    if( err )
    {
        if( errno == EACCES )
            return err;
        else
        {
            size_t p = fullPath.rfind( DirSeparator, fullPath.length() - 2 );
            if( p == std::string::npos )
                return err;
            err = ChangeForceDir( fullPath.substr( 0, p ) );
            if( err )
                return err;
            err = ::mkdir( fullPath.c_str() );
            if( err )
                return err;
            err = ::chdir( fullPath.c_str() );
        }
    }
    return err;
}

void BCIDtry::SetDir( const char *dir )
{
        SubjDir = dir;
}

void BCIDtry::SetName( const char *name )
{
        SubjName = name;
}

void BCIDtry::SetSession( const char *session )
{
        SubjSession = session;
}

int BCIDtry::ProcPath( void )
{
    return ChangeForceDir( SubjDir );
}

const char *BCIDtry::ProcSubDir( void )
{
    SubjPath = SubjDir;
    if( SubjPath.length() < 2 || SubjPath[ 1 ] != DriveSeparator )
        SubjPath = BaseDir + SubjPath;
    if( SubjPath.length() > 0 && SubjPath[ SubjPath.length() - 1 ] != DirSeparator )
        SubjPath += DirSeparator;
    SubjPath += SubjName;
    SubjPath += SubjSession;
    ChangeForceDir( SubjPath );
    return SubjPath.c_str();
}

void BCIDtry::FileError( const char *badpath )
{
    TFEForm *FEForm = new TFEForm( badpath );
    FEForm->ShowModal();
    delete FEForm;
}

int BCIDtry::GetLargestRun( const char *path )
{
        struct ffblk ffblk;
        int done;
        int res;
        int lth;
        int max;
        int current;
        int i;
        int lastr;
        char str[16];

        if( chdir( path ) )
            return 0;

        done= findfirst("*.dat", &ffblk, FA_ARCH);

        max= 0;

        while( !done )
        {
                lth= strlen( ffblk.ff_name );

                for(i=0;i<lth;i++)     // find last "r"
                        if( ffblk.ff_name[i] == 0x52 ) lastr= i;
                str[ 0 ] = ffblk.ff_name[lastr+1];
                str[ 1 ] = ffblk.ff_name[lastr+2];
                str[ 2 ] = 0;

                current= atoi( str );
                if( current > max ) max= current;

                done= findnext( &ffblk );
        }
        return( max );
}

__fastcall TFEForm::TFEForm( const char *path )
// As we delete the form object ourselves, we don't want it to have
// an owner at all.
: TForm( ( TComponent* )NULL, 1 )
{
        Height= 150;
        Width= 250;
        // Position= poScreenCenter;
        Top= 20;
        Left= 20;
        AutoScroll = false;

        Caption= Application->Title + ": File I/O Error";

        OKButton= new TButton(this);
        OKButton->Parent= this;
        OKButton->Top= 75;
        OKButton->Left= 75;
        OKButton->OnClick= OKButtonClick;
        OKButton->Caption= "OK";

        BadPath= new TEdit( this );
        BadPath->Parent= this;
        BadPath->Text= path;
        BadPath->Left= 10;
        BadPath->Top= 50;
        BadPath->Width= 220;
        BadPath->Height= 30;
        BadPath->ReadOnly = true;
}

void __fastcall TFEForm::OKButtonClick(TObject *Sender )
{
        Close();
}


