////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: Functionality to resolve dynamically imported functions
//   at startup. Advantages over the approach of using import libraries are:
//   * Custom error message when library is not found -- avoids misleading
//     "reinstall application" suggestion in standard Windows message
//   * Cross-platform solution -- code compiles and links on any platform,
//     whether there is an import library available or not
//   * Architecture and compiler independence -- no need to maintain an
//     individual import library for each combination of architecture,
//     platform, and compiler
//   * Robust name matching -- avoids C name mangling issues on Windows,
//     able to deal with C++ mangled names if overloading is absent
//   * Automatic generation of interfacing code from the library's header file
//     (buildutils/dylib_imports.sh)
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
#ifndef DYLIB_IMPORTS_H
#define DYLIB_IMPORTS_H

#include <string>
#include <vector>
#include <list>

#if _WIN32
# define CDECL__ __cdecl
#else
# define CDECL__
#endif

#if DYNAMIC_IMPORTS
#define RegisterDylibWithAliases( name, aliases, imports, msg, url ) \
namespace Dylib { \
  void CDECL__ name##_ErrorStub_(); \
  StartupLoader name( #name, aliases, imports, msg, url, &name##_ErrorStub_ ); \
  void CDECL__ name##_ErrorStub_() \
  { name.ThrowError(); } \
  bool name##_Loaded() \
  { return name.State() == Library::resolvedAll; } \
}
#else // DYNAMIC_IMPORTS
#define RegisterDylibWithAliases( name, aliases, imports, msg, url ) \
namespace Dylib { \
  bool name##_Loaded() \
  { return true; } \
}
#endif // DYNAMIC_IMPORTS
#define RegisterDylib( name, imports, msg, url ) RegisterDylibWithAliases( name, 0, imports, msg, url )

namespace Dylib
{
  struct Import
  {
    const char* name;
    void** pointer;
    enum
    {
      exactMatch = 0,
      cMangled = 1,
      cppMangled = 2,
    };
    int options;
  };
  struct Export
  {
    const char* name;
    void* address;
  };
  typedef std::vector<Export> Exports;
  typedef std::list<std::string> Names;

  class Library
  {
   public:
    Library( const std::string& lib );
    Library( const std::string& lib, const Names& names );
    ~Library();
    std::string Name() const;
    const Names& Names() const
      { return mNames; }
    const Dylib::Exports& Exports() const
      { return mExports; }
    const std::string& Error() const
      { return mError; }
    enum { none, found, notFound, resolvedNone, resolvedSome, resolvedAll };
    int State() const
      { return mState; }
    bool Resolve( const Import*, int count = 1 );

   private:
    void Init();

    void* mHandle;
    int mState;
    Dylib::Exports mExports;
    Dylib::Names mNames;
    std::string mError;
  };

  class StartupLoader : public Library
  {
    // The ThrowFunc's address is assigned to function pointer that cannot be resolved, so
    // it may be called with any calling convention.
    // To avoid stack corruption, the ThrowFunc may not return.
    // Instead, it is supposed to throw an exception, or to call a non-return function such as
    // exit().
    // Typically, the ThrowFunc calls a StartupLoader's ThrowError() function.
    typedef void ( CDECL__ *ThrowFunc )();

   public:
    StartupLoader( const char* lib, const char* aliases, const Import*, const char* msg, const char* url, ThrowFunc );
    void ThrowError() const;

   private:
    static Dylib::Names ParseAliases( const char* );
    void BuildMessage( const std::string&, const std::string& );
    std::string mMessage;
  };

} // namespace

#endif // DYLIB_IMPORTS_H
