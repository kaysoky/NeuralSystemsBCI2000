//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: File-system related utility functions.
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
///////////////////////////////////////////////////////////////////////
#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <vector>

#ifdef RemoveDirectory
# undef RemoveDirectory
#endif // RemoveDirectory

namespace FileUtils
{
#ifdef _WIN32
  static const char  DirSeparator = '\\';
  static const char  DriveSeparator = ':';
  static const char  PathSeparator = ';';
  inline const char* SeparatorSet() { return "\\/"; }
#else
  static const char  DirSeparator = '/';
  static const char  DriveSeparator = '/';
  static const char  PathSeparator = ':';
  inline const char* SeparatorSet() { return "/"; }
#endif

  std::string EnsureSeparator( const std::string& );
  std::string ExecutablePath();
  std::string ApplicationTitle();
  const std::string& InstallationDirectoryS();
  inline std::string InstallationDirectory() { return InstallationDirectoryS() + DirSeparator; }

  // This returns the current working directory.
  std::string WorkingDirectoryS();
  inline std::string WorkingDirectory() { return WorkingDirectoryS() + DirSeparator; }
  // This sets the current working directory.
  bool ChangeDirectory( const std::string& );

  // This transforms relative paths into absolute paths, using the
  // current directory as a reference.
  std::string AbsolutePath( const std::string& );

  // This returns a path free of relative elements, and without symlinks.
  std::string CanonicalPath( const std::string& );

  // This returns the parent of the specified object (file or directory).
  std::string ParentDirectoryS( const std::string& );
  inline std::string ParentDirectory( const std::string& s ) { return ParentDirectoryS( s ) + DirSeparator; }

  // A list of entries in the specified directory.
  typedef std::vector<std::string> List;
  bool ListDirectory( const std::string&, List& );

  // These return the directory portion, file name, base name portion, and extension of a path.
  std::string ExtractDirectoryS( const std::string& );
  inline std::string ExtractDirectory( const std::string& s ) { return ExtractDirectoryS( s ) + DirSeparator; }
  std::string ExtractFile( const std::string& );
  std::string ExtractBase( const std::string& );
  std::string ExtractExtension( const std::string& );

  bool IsFile( const std::string& );
  bool IsDirectory( const std::string& );
  bool IsSymbolicLink( const std::string& );
  bool IsAbsolutePath( const std::string& );

  bool Rename( const std::string&, const std::string& );
  bool MakeDirectory( const std::string& );
  bool RemoveDirectory( const std::string&, bool force = false );
  bool RemoveFile( const std::string& );

} // namespace FileUtils

#endif // FILE_UTILS_H

