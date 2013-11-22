////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: See the associated header file for details.
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
#include "DylibImports.h"
#include "WildcardMatch.h"
#include "FileUtils.h"
#include "Exception.h"
#include "Debugging.h"
#include <vector>

#if _WIN32
# include <Windows.h>
# include <WinNT.h>
#else
# include <dlfcn.h>
#endif

using namespace std;
using namespace Dylib;

static size_t sArchBits = 8 * sizeof( void* );

#if _WIN32
#define OS_WOW6432 30
#define IsOS_ORDINAL 437
static BOOL (WINAPI *IsOS_)( DWORD ) = 0;
static HMODULE LoadLibrary_( const char* s )
{
  if( !IsOS_ )
  {
    HMODULE lib = ::LoadLibraryA( "shlwapi" );
    if( lib )
    {
      const char* name = reinterpret_cast<const char*>( IsOS_ORDINAL );
      *reinterpret_cast<void**>( &IsOS_ ) = ::GetProcAddress( lib, name );
    }
  }
  HMODULE h = 0;
  try
  {
    h = ::LoadLibraryA( s );
  }
  catch( ... )
  {
  }
  return h;
}
#undef LoadLibrary
#define LoadLibraryA *dontuse*
#define LoadLibraryW *dontuse*
#endif // _WIN32

static void*
LoadDylib( const string& inName )
{
  void* result = 0;
#if _WIN32
  result = LoadLibrary_( inName.c_str() );
  if( !result )
  {
    ostringstream oss;
    oss << inName << sArchBits;
    result = LoadLibrary_( oss.str().c_str() );
    if( !result )
    {
      oss.str( inName );
      oss << "_" << sArchBits;
      result = LoadLibrary_( oss.str().c_str() );
    }
  }
  if( !result && sArchBits == 32 )
    result = LoadLibrary_( ( inName + "_x86" ).c_str() );
  if( !result && sArchBits == 64 )
    result = LoadLibrary_( ( inName + "_x64" ).c_str() );
#else
  string name = inName;
# if __APPLE__
  name += ".dylib";
# else
  name += ".so";
# endif
  result = ::dlopen( name.c_str(), RTLD_LAZY | RTLD_LOCAL );
#endif // _WIN32
  return result;
}

static void
UnloadDylib( void* inHandle )
{
#if _WIN32
  ::FreeLibrary( HMODULE( inHandle ) );
#else
  ::dlclose( inHandle );
#endif
}

static void
GetDylibExports( void* inHandle, Exports& outExports )
{
  outExports.clear();
#if _WIN32
  char* p = static_cast<char*>( inHandle );
  PIMAGE_DOS_HEADER d = PIMAGE_DOS_HEADER( p );
  if( d->e_magic != IMAGE_DOS_SIGNATURE )
    return;
  PIMAGE_NT_HEADERS n = PIMAGE_NT_HEADERS( p + d->e_lfanew );
  if( n->Signature != IMAGE_NT_SIGNATURE )
    return;
  if( n->OptionalHeader.NumberOfRvaAndSizes <= 0 )
    return;
  PIMAGE_DATA_DIRECTORY pEntry = n->OptionalHeader.DataDirectory + IMAGE_DIRECTORY_ENTRY_EXPORT;
  PIMAGE_EXPORT_DIRECTORY pExports = PIMAGE_EXPORT_DIRECTORY( p + pEntry->VirtualAddress );
  PDWORD names = PDWORD( p + pExports->AddressOfNames );
  for( size_t i = 0; i < pExports->NumberOfNames; ++i )
  {
    const char* name = p + names[i];
    Export export_ = { name, (void*)::GetProcAddress( HMODULE( inHandle ), name ) };
    outExports.push_back( export_ );
  }
#elif __APPLE__
#else
#endif // _WIN32
}

// Library class
Library::Library( const std::string& lib )
: mHandle( 0 ), mState( none )
{
  mNames.push_front( lib );
  Init();
}

Library::Library( const std::string& lib, const Dylib::Names& names )
: mHandle( 0 ), mState( none ), mNames( names )
{
  mNames.push_front( lib );
  Init();
}

Library::~Library()
{
  UnloadDylib( mHandle );
}

void
Library::Init()
{
  for( Names::const_iterator i = mNames.begin(); !mHandle && i != mNames.end(); ++i )
    mHandle = LoadDylib( *i );
  if( mHandle )
    GetDylibExports( mHandle, mExports );
  else if( mError.empty() )
    mError = "Library " + Name() + " could not be found";
  mState = ( mHandle ? found : notFound );
}

string
Library::Name() const
{
  string name = "<n/a>";
  if( !mNames.empty() )
  {
    name = "\"" + *mNames.begin() + "\"";
    for( Dylib::Names::const_iterator i = ++mNames.begin(); i != mNames.end(); ++i )
      name += " alias \"" + *i + "\"";
  }
  return name;
}

bool
Library::Resolve( const Import* inImports, int inCount )
{
  if( mState != found )
    return false;

  mState = resolvedNone;
  mError.clear();
  const Import* p = inImports;
  while( p->name && ( inCount < 1 || p < inImports + inCount ) )
  {
    bool countMatches = false;
    vector<string> patterns;
    patterns.push_back( p->name );
    if( p->options & Import::cMangled )
    {
      patterns.push_back( "_" + patterns.back() );
      patterns.push_back( *patterns.begin() + "@[0-9]*" );
      patterns.push_back( "_" + patterns.back() );
    }
    if( p->options & Import::cppMangled )
    {
      static const string nonletter = "[^a-zA-Z]";
      patterns.back() = "*" + nonletter + *patterns.begin() + nonletter + "*";
      countMatches = true;
    }
    int matches = 0;
    void* addr = 0;
    for( size_t i = 0; ( countMatches || !addr ) && i < patterns.size(); ++i )
      for( size_t j = 0; ( countMatches || !addr ) && j < mExports.size(); ++j )
        if( WildcardMatch( patterns[i], mExports[j].name, true ) )
        {
          ++matches;
          addr = mExports[j].address;
        }
    if( matches > 1 && ( p->options & Import::cppMangled ) )
    {
      mError += "C++ function ";
      mError += p->name;
      mError += " is overloaded, cannot decide which to use\n";
      addr = 0;
    }
    else if( !addr )
    {
      mError += "Function ";
      mError += p->name;
      mError += " not found in library\n";
    }
    if( addr )
      mState = resolvedSome;
    Assert( *p->pointer == 0 );
    *p->pointer = addr;
    ++p;
  }
  if( mState == resolvedSome && mError.empty() )
    mState = resolvedAll;
  return mError.empty();
}

// StartupLoader class
StartupLoader::StartupLoader( const char* inLib, const char* inAliases, const Import* inImports, const char* inMsg, const char* inUrl, ThrowFunc inF )
: Library( inLib, ParseAliases( inAliases ) )
{
  if( !Library::Error().empty() )
  {
    string exe = FileUtils::ExtractBase( FileUtils::ExecutablePath() ),
           msg = inMsg ? inMsg : "",
           url = inUrl ? inUrl : "";
    if( msg.empty() )
    {
      ostringstream oss;
      oss << sArchBits << "-bit Library "
          << Library::Name()
          << " is not available, but is necessary for "
          << exe
          << " to run.";
      bool isWow = false;
#if _WIN32
      isWow = ( IsOS_ && IsOS_( OS_WOW6432 ) );
      if( isWow )
        oss << "\nNOTE: You are running a 32-bit executable on a 64-bit Windows installation. "
            << "This requires a 32-bit version of the "
            << Library::Name()
            << " DLL to be available. "
            << "32-bit system DLLs must reside in the SysWOW64 (yes, actually \"64\") "
            << "subdirectory of your Windows system directory.";
#endif // _WIN32
      if( WildcardMatch( "*source*", exe, false ) || WildcardMatch( "*adc*", exe, false ) )
      {
        oss << "\nYou may need to install ";
        if( isWow )
          oss << "the 32-bit version of ";
        oss << "the driver software that came with your amplifier.";
      }
      msg = oss.str();
    }
    BuildMessage( msg, url );
  }
  else if( !Library::Resolve( inImports, 0 ) )
  {
    string msg = "Could not load functions from dynamic library " + Library::Name()
               + " due to an error: " + Library::Error();
    if( Library::State() == resolvedSome )
      msg += "An update to that library/driver may be necessary.";
    BuildMessage( msg, "" );
  }
  for( const Import* p = inImports; p->name; ++p )
    if( !*p->pointer )
      *p->pointer = reinterpret_cast<void*>( inF );
}

Dylib::Names
StartupLoader::ParseAliases( const char* inAliases )
{
  Dylib::Names names;
  const char* p = inAliases;
  if( p ) do
  {
    string name;
    while( *p && *p != '|' )
      name += *p++;
    names.push_back( name );
  } while( *p++ );
  return names;
}

void
StartupLoader::BuildMessage( const string& inMsg, const string& inUrl )
{
  mMessage = inMsg;
  string url = inUrl;
  if( url.empty() && !Library::Names().empty() )
  {
    Dylib::Names::const_iterator i = Library::Names().begin();
    string names = *i++;
    for( ; i != Library::Names().end(); ++i )
      names += "%20OR%20" + *i;
    url = FileUtils::SearchURL( names );
  }
  if( !url.empty() )
    mMessage += " More information may be available at:\n" + url;
}

void
StartupLoader::ThrowError() const
{
  throw std_runtime_error( mMessage );
}
