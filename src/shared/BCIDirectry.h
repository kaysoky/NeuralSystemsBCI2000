///////////////////////////////////////////////////
//  $Id$
//  BCIDirectory Class
//  BCI Directory Management Functions
//  $Log$
//  Revision 1.12  2006/07/04 16:08:06  mellinger
//  Removed platform dependencies.
//
//  Revision 1.11  2006/02/18 12:02:12  mellinger
//  Introduced support for arbitrary file extensions.
//
//  Revision 1.10  2005/12/20 11:42:41  mellinger
//  Added CVS id and log to comment.
//
///////////////////////////////////////////////////
#ifndef BCIDirectryH
#define BCIDirectryH

#define OLD_BCIDTRY

#ifdef OLD_BCIDTRY
# include <stdlib.h>
#endif

#include <string>

class BCIDirectory
{
  enum { none = -1 };

 public:
  static const std::string& InstallationDirectory();

  BCIDirectory();
  // Write accessors return an instance reference to allow for "named parameter"
  // constructs as in
  //   BCIDirectory().SubjectDirectory( "c:\\" ).SubjectName( "test" );
  BCIDirectory&      SubjectDirectory( const char* s )
                     { mSubjectDirectory = s; return UpdateRunNumber(); }
  BCIDirectory&      SubjectName( const char* s )
                     { mSubjectName = s; return UpdateRunNumber(); }
  BCIDirectory&      FileExtension( const std::string& s )
                     { mFileExtension = s; return UpdateRunNumber(); }
  BCIDirectory&      SessionNumber( int i )
                     { mSessionNumber = i; return UpdateRunNumber(); }
  BCIDirectory&      RunNumber( int i )
                     { mDesiredRunNumber = i; return UpdateRunNumber(); }
 public:
  // Read accessors
  const std::string& SubjectDirectory() const
                     { return mSubjectDirectory; }
  const std::string& SubjectName() const
                     { return mSubjectName; }
  const std::string& FileExtension() const
                     { return mFileExtension; }
  int                SessionNumber() const
                     { return mSessionNumber; }
  int                RunNumber() const
                     { return mActualRunNumber; }
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
  static int         GetLargestRun( const std::string& path,
                                    const std::string& extension );
  static int         ExtractRunNumber( const std::string& fileName );
  static int         ChangeForceDir( const std::string& );
  static std::string GetCWD();
  static int         MkDir( const std::string& );
  static bool        IsAbsolutePath( const std::string& );

  BCIDirectory&      UpdateRunNumber();
  std::string        ConstructFileName() const;

#ifdef _WIN32
  static const char  DirSeparator = '\\';
  static const char  DriveSeparator = ':';
#else
  static const char  DirSeparator = '/';
  static const char  DriveSeparator = '/';
#endif

  std::string        mSubjectDirectory,
                     mSubjectName,
                     mFileExtension;
  int                mSessionNumber,
                     mDesiredRunNumber,
                     mActualRunNumber;
  static std::string sInstallationDirectory;
#ifdef OLD_BCIDTRY
  std::string        mSubjectPath;
#endif // OLD_BCIDTRY
};

#ifdef OLD_BCIDTRY
typedef BCIDirectory BCIDtry; // Compatibility alias name.
#endif // OLD_BCIDTRY

#endif // BCIDirectryH
