//-------------------------------------------------
//  BCIDirectry.cpp
//  BCI Dircetory Management Functions
//-------------------------------------------------
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIDirectry.h"

#include "UBCIError.h"
#include <dir.h>

#pragma package(smart_init)

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


