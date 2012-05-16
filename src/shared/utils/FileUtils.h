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

  std::string ExecutablePath();
  const std::string& InstallationDirectory();

  // This returns the current working directory.
  std::string GetCWD();
  // This sets the current working directory.
  bool ChDir( const std::string& );

  // This transforms relative paths into absolute paths, using the
  // current directory as a reference.
  std::string AbsolutePath( const std::string& );

  // This returns a path free of relative elements, and without symlinks.
  std::string CanonicalPath( const std::string& );

  // This returns the parent of the specified object (file or directory).
  std::string ParentDirectory( const std::string& );

  // These return the directory portion, file name, base name portion, and extension of a path.
  std::string ExtractDirectory( const std::string& );
  std::string ExtractFile( const std::string& );
  std::string ExtractBase( const std::string& );
  std::string ExtractExtension( const std::string& );

  bool IsFile( const std::string& );
  bool IsDirectory( const std::string& );
  bool IsAbsolutePath( const std::string& );

  bool MkDir( const std::string& );

} // namespace FileUtils

#endif // FILE_UTILS_H

