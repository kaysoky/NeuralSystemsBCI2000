////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates BCI2000 directory and file naming
//   conventions.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#ifndef BCI_DIRECTORY_H
#define BCI_DIRECTORY_H

#include "FileUtils.h"

class BCIDirectory
{
  enum { none = -1 };

 public:
#if 1 // These functions are now in FileUtils, and provided here for backward
      // compatibility only.
  static std::string InstallationDirectory()
                     { return FileUtils::InstallationDirectory(); }
  static std::string AbsolutePath( const std::string& inPath )
                     { return FileUtils::AbsolutePath( inPath ); }
  static std::string GetCWD()
                     { return FileUtils::WorkingDirectory(); }
#endif

  // Class interface: Creating output directory, and file names for recorded files.
  BCIDirectory();
  // Write accessors return an instance reference
  // -- this allows for "named parameter" constructs as in
  //   BCIDirectory().SetDataDirectory( "c:\\" ).SetSubjectName( "test" );
  BCIDirectory&      SetDataDirectory( const std::string& s )
                     { mDataDirectory = s; return UpdateRunNumber(); }
  BCIDirectory&      SetSubjectName( const std::string& s )
                     { mSubjectName = s; return UpdateRunNumber(); }
  BCIDirectory&      SetFileExtension( const std::string& s )
                     { mFileExtension = s; return UpdateRunNumber(); }
  BCIDirectory&      SetSessionNumber( int i )
                     { mSessionNumber = i; return UpdateRunNumber(); }
  BCIDirectory&      SetRunNumber( int i )
                     { mDesiredRunNumber = i; return UpdateRunNumber(); }
 public:
  // Read accessors
  const std::string& DataDirectory() const
                     { return mDataDirectory; }
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

 private:
  static int         GetLargestRun( const std::string& path,
                                    const std::string& extension );
  static int         ExtractRunNumber( const std::string& fileName );
  static bool        ChangeForceDir( const std::string& );

  BCIDirectory&      UpdateRunNumber();
  std::string        ConstructFileName() const;

  std::string        mDataDirectory,
                     mSubjectName,
                     mFileExtension;
  int                mSessionNumber,
                     mDesiredRunNumber,
                     mActualRunNumber;
};

#endif // BCI_DIRECTORY_H
