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

#ifndef BACK_COMPAT
  #define BACK_COMPAT 1
#endif

#if BACK_COMPAT
#include "Environment.h"
#include "BCIException.h"
#define BCIDIR_OBSOLETE_(x) \
  throw bciexception( \
    "This function no longer available. Please use the CurrentRunFile() function to obtain the file name of the current run." \
  ); \
  return (x);

class BCIDirectory : private Environment
{
  enum { none = -1 };

  // These functions are now in FileUtils, and provided here for backward
  // compatibility only.
  static std::string InstallationDirectory()
                     { return FileUtils::InstallationDirectory(); }
  static std::string AbsolutePath( const std::string& inPath )
                     { return FileUtils::AbsolutePath( inPath ); }
  static std::string GetCWD()
                     { return FileUtils::WorkingDirectory(); }

  // Dummy/forwarding implementation of the BCIDirectory interface.
  BCIDirectory&    SetDataDirectory( const std::string& )
                     { return *this; }
  BCIDirectory&    SetSubjectName( const std::string& )
                     { return *this; }
  BCIDirectory&    SetFileExtension( const std::string& s )
                     { mFileExtension = s; return *this; }
  BCIDirectory&    SetSessionNumber( int i )
                     { return *this; }
  BCIDirectory&    SetRunNumber( int i )
                     { return *this; }
  // Read accessors
  const std::string& DataDirectory() const
                     { BCIDIR_OBSOLETE_( mFileExtension ); }
  const std::string& SubjectName() const
                     { BCIDIR_OBSOLETE_( mFileExtension ); }
  const std::string& FileExtension() const
                     { return mFileExtension; }
  int                SessionNumber() const
                     { BCIDIR_OBSOLETE_( 0 ); }
  int                RunNumber() const
                     { BCIDIR_OBSOLETE_( 0 ); }

  std::string        FilePath() const
                     {
                        string file = Environment::CurrentRun();
                        return FileUtils::ExtractDirectory( file ) + FileUtils::ExtractBase( file );
                     }
  std::string        DirectoryPath() const
                     { return FileUtils::ExtractDirectory( FilePath() ); }
  const BCIDirectory& CreatePath() const
                     { FilePath(); return *this; }

 private:
  std::string        mFileExtension;
};
#else // BACK_COMPAT
  #error BCIDirectory is obsolete. Please use EnvironmentBase::CurrentRun()/CurrentSession() instead.
#endif // BACK_COMPAT

#endif // BCI_DIRECTORY_H
