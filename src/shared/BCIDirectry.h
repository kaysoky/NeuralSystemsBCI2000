///////////////////////////////////////////////////
//  BCIDirectory Class
//  BCI Directory Management Functions
///////////////////////////////////////////////////
#ifndef BCIDirectryH
#define BCIDirectryH

#define OLD_BCIDTRY

#include <string>

class BCIDirectory
{
  enum { none = -1 };

 public:
  static const std::string& InstallationDirectory();

  BCIDirectory()
  : mSessionNumber( none ), mRunNumber( none ) {}
  BCIDirectory&      SubjectDirectory( const char* s )
                     { mSubjectDirectory = s; return *this; }
  BCIDirectory&      SubjectName( const char* s )
                     { mSubjectName = s; return *this; }
  BCIDirectory&      SessionNumber( int i )
                     { mSessionNumber = i; return *this; }
  BCIDirectory&      RunNumber( int i )
                     { mRunNumber = i; return *this; }
  // This returns the full path to the current file, but without the .dat extension.
  std::string        FilePath() const;
  std::string        DirectoryPath() const;
  // This creates all directories contained in the file path if they don't exist.
  const BCIDirectory& CreatePath() const;

#ifdef OLD_BCIDTRY
  // Legacy functions maintained for compatibility:
  void               SetDir( const char* inDir )
                     { SubjectDirectory( inDir ); }
  void               SetName( const char* inName )
                     { SubjectName( inName ); }
  void               SetSession( const char* inSession )
                     { SessionNumber( ::atoi( inSession ) ); }
  int                ProcPath();
  const char*        ProcSubDir();
#endif // OLD_BCIDTRY

 private:
  static int         GetLargestRun( const std::string& path );
  static int         ExtractRunNumber( const std::string& fileName );
  static int         ChangeForceDir( const std::string& );
  std::string        ConstructFileName() const;

  static const char  DirSeparator = '\\';
  static const char  DriveSeparator = ':';

  std::string        mSubjectDirectory,
                     mSubjectName;
  int                mSessionNumber,
                     mRunNumber;
#ifdef OLD_BCIDTRY
  std::string        mSubjectPath;
#endif // OLD_BCIDTRY
};

#ifdef OLD_BCIDTRY
typedef BCIDirectory BCIDtry; // Compatibility alias name.
#endif // OLD_BCIDTRY

#endif // BCIDirectryH
