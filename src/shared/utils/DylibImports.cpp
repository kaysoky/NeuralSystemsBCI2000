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
#include "PCHIncludes.h"
#pragma hdrstop

#include "DylibImports.h"
#include "WildcardMatch.h"
#include "FileUtils.h"
#include "BCIException.h"
#include "BCIAssert.h"
#include <vector>

#if _WIN32
# include <Windows.h>
# include <WinNT.h>
#else
# include <dlfcn.h>
#endif

using namespace std;
using namespace bci;
using namespace Dylib;

typedef vector< pair<string, void*> > Exports;

static size_t sArchBits = 8 * sizeof( void* );

static void*
LoadDylib( const string& inName )
{
  void* result = 0;
#if _WIN32
  result = ::LoadLibraryA( inName.c_str() );
  if( !result )
  {
    ostringstream oss;
    oss << inName << sArchBits;
    result = ::LoadLibraryA( oss.str().c_str() );
    if( !result )
    {
      oss.str( inName );
      oss << "_" << sArchBits;
      result = ::LoadLibraryA( oss.str().c_str() );
    }
  }
  if( !result && sArchBits == 32 )
    result = ::LoadLibraryA( ( inName + "_x86" ).c_str() );
  if( !result && sArchBits == 64 )
    result = ::LoadLibraryA( ( inName + "_x64" ).c_str() );
#else
  string name = inName,
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
    outExports.push_back( make_pair<string, void*>( name, ::GetProcAddress( HMODULE( inHandle ), name ) ) );
  }
#elif __APPLE__
#else
#endif // _WIN32
}

// Loader class
Loader::Loader( const std::string& lib )
: mHandle( 0 ), mLibrary( lib ), mState( none )
{
  mHandle = LoadDylib( lib );
  if( !mHandle && mError.empty() )
    mError = "Library \"" + lib + "\" could not be found";
  mState = ( mHandle ? found : notFound );
}

Loader::~Loader()
{
  UnloadDylib( mHandle );
}

bool
Loader::Resolve( const Import* inImports, int inCount )
{
  if( mState != found )
    return false;

  mState = resolvedNone;
  mError.clear();
  Exports exports;
  GetDylibExports( mHandle, exports );
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
      for( size_t j = 0; ( countMatches || !addr ) && j < exports.size(); ++j )
        if( WildcardMatch( patterns[i], exports[j].first, true ) )
        {
          ++matches;
          addr = exports[j].second;
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
    bciassert( *p->pointer == 0 );
    *p->pointer = addr;
    ++p;
  }
  if( mState == resolvedSome && mError.empty() )
    mState = resolvedAll;
  return mError.empty();
}

// StartupLoader class
StartupLoader::StartupLoader( const char* inLib, const Import* inImports, const char* inMsg, const char* inUrl, ThrowFunc inF )
: Loader( inLib )
{
  if( !Loader::Error().empty() )
  {
    string exe = FileUtils::ExtractBase( FileUtils::ExecutablePath() ),
           msg = inMsg ? inMsg : "",
           url = inUrl ? inUrl : "";
    if( msg.empty() )
    {
      msg = "Library \"" + Loader::Library() + "\" is not available, but is necessary for "
          + exe + " to run.";
      if( WildcardMatch( "*source*", exe, false ) || WildcardMatch( "*adc*", exe, false ) )
        msg += " You may need to install the driver software that came with your amplifier.";
    }
    BuildMessage( msg, url );
  }
  else if( !Loader::Resolve( inImports, 0 ) )
  {
    string msg = "Could not load functions from dynamic library \"" + Loader::Library()
               + "\" due to an error: " + Loader::Error();
    if( Loader::State() == resolvedSome )
      msg += "An update to that library/driver may be necessary.";
    BuildMessage( msg, "http://www.google.com/search?q=site%3Abci2000.org+" + Loader::Library() );
  }
  for( const Import* p = inImports; p->name; ++p )
    if( !*p->pointer )
      *p->pointer = reinterpret_cast<void*>( inF );
}

void
StartupLoader::BuildMessage( const string& inMsg, const string& inUrl )
{
  mMessage = inMsg;
  if( !inUrl.empty() )
    mMessage += "More information may be available at " + inUrl + "\n";
}

void
StartupLoader::ThrowError() const
{
  throw bciexception_( mMessage );
}
