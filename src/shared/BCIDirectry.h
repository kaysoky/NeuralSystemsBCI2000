//-------------------------------------------------
//	BCIDtry Class
//	BCI Directory Management Functions
//-------------------------------------------------

#ifndef BCIDirectryH
#define BCIDirectryH

#include <vcl.h>
#include <string>
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

    int         ChangeForceDir( const std::string& );

 public:
    static std::string GetCwd();
    
    void        SetDir( const char * );
    void        SetName( const char* );
    void        SetSession( const char * );
    int         ProcPath( void );
    const char* ProcSubDir( void );
    int         GetLargestRun( const char * );
} ;

#endif // BCIDirectryH
