//-------------------------------------------------
//	BCIDirectry.cpp
//	BCI Dircetory Management Functions
//-------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "BCIDirectry.h"

//--------------------------------------------------


int BCIDtry::CheckPath( char *path )
{
        return( chdir( path ) );
}

int BCIDtry::MakePath( char *path )
{
        return( mkdir( path ) );
}

/*
int BCIDtry::CheckSubDir( char *subdir )
{
}

int BCIDtry::MakeSubDir( char *subdir )
{
}
*/

void BCIDtry::SetDir( char *dir )
{
        strcpy( SubjDir, dir );
}

void BCIDtry::SetName( char *name )
{
        strcpy( SubjName, name );
}

void BCIDtry::SetSession( char *session )
{
        strcpy( SubjSession, session );
}

int BCIDtry::ProcPath( void )
{
     int retval= 0;

     if( CheckPath( SubjDir ) < 0 )
     {
        // in the future check with operator

        retval= MakePath( SubjDir );
     }
     return( retval );
}

char *BCIDtry::ProcSubDir( void )
{
        int lth;

        char slash[2];
        char zero[2];

        slash[0]= 0x5c;
        slash[1]= 0x00;
        zero[0]= 0x00;
        zero[1]= 0x00;

        strcpy( SubjPath, SubjDir );
        lth= strlen( SubjPath );

        if( SubjPath[lth-1] != slash[0]  )
                strcat( SubjPath, slash );

        strcat( SubjPath, SubjName );
        strcat( SubjPath, SubjSession );

        if( CheckPath( SubjPath ) < 0 )
        {
                MakePath( SubjPath );
        }
        return( SubjPath );
}


void BCIDtry::FileError( TApplication *appl, char *badpath )
{
        char line[120];
        strcpy(line,badpath);

        TFEForm *FEForm = new TFEForm( appl, badpath );
        FEForm->ShowModal();
        delete FEForm;
      //  MessageBox(line, "Error Opening File", MB_OK);

}

int BCIDtry::GetLargestRun( char *path )
{
struct ffblk ffblk;
int done;
int lth;
int max;
int current;
int i;
int lastr;
char str[16];

 chdir( path );

 done= findfirst("*.dat", &ffblk, FA_ARCH);

 max= 0;
 while ( !done )
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


__fastcall TFEForm::TFEForm( TApplication *appl, char *path ): TForm(appl,0)
{
        Height= 150;
        Width= 250;
        // Position= poScreenCenter;
        Top= 20;
        Left= 20;

        Caption= "Error Opening File";

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

      //   statevector->SetStateValue("Running",CurrentRunning);
}

void __fastcall TFEForm::OKButtonClick(TObject *Sender )
{
        Close();
}


