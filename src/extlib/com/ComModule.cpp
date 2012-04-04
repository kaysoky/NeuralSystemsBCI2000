////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that provides implementations for the interface
//   functions of a COM DLL.
//   From DllMain(), call Com::Module::Init(), specifying the CLSID associated
//   with the MIDL-defined library contained in your module. There is only one
//   library supported per module.
//   The Dll* functions are supposed to be called from the respective exported
//   functions of a COM DLL.
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
#include "ComModule.h"
#include "ComClassFactory.h"
#include "ComRegistrar.h"
#include "ComStrings.h"

using namespace com;

HINSTANCE Module::sHInstance = NULL;
wchar_t* Module::spFileName = NULL;
wchar_t* Module::spLocation = NULL;
const CLSID* Module::spLibID = NULL;
Ptr<ITypeLib> Module::spTypeLib = NULL;

void
Module::Init( HINSTANCE inHInstance, const CLSID* inCLSID )
{
  sHInstance = inHInstance;
  spLibID = inCLSID;
}

HINSTANCE
Module::GetHInstance()
{
  return sHInstance;
}

const wchar_t*
Module::GetFileName()
{
  if( spFileName == NULL )
  {
    DWORD size = 1024; // Must be large enough to hold path in WinXP.
    DWORD result = 0;
    do {
      delete[] spFileName;
      spFileName = new wchar_t[size];
      result = ::GetModuleFileNameW( sHInstance, spFileName, size );
      size *= 2;
    } while( result != 0 && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER );
    if( result == 0 )
    {
      delete[] spFileName;
      spFileName = NULL;
    }
  }
  return spFileName;
}

const wchar_t*
Module::GetLocation()
{
  if( spLocation == NULL )
  {
    const wchar_t* pPath = Module::GetFileName();
    if( pPath != 0 )
    {
      DWORD size = ::wcslen( pPath );
      wchar_t pDrive[_MAX_DRIVE];
      wchar_t* pDir = new wchar_t[size];
      errno_t err = ::_wsplitpath_s( pPath, pDrive, _MAX_DRIVE, pDir, size, NULL, 0, NULL, 0 );
      if( err == 0 )
      {
        spLocation = new wchar_t[size];
        ::wcscpy( spLocation, pDrive );
        ::wcscat( spLocation, pDir );
      }
      delete[] pDir;
    }
  }
  return spLocation;
}

Ptr<ITypeLib>
Module::GetTypeLib()
{
  if( !spTypeLib )
    ::LoadRegTypeLib( *spLibID, 1, 0, 0, &spTypeLib );
  return spTypeLib;
}

HRESULT
Module::DllCanUnloadNow()
{
  return ClassFactoryBase::DllCanUnloadNow();
}

HRESULT
Module::DllGetClassObject( REFCLSID inCSLID, REFIID inIID, LPVOID* outObject )
{
  return ClassFactoryBase::DllGetClassObject( inCSLID, inIID, outObject );
}

HRESULT
Module::DllRegisterServer( bool inForUser )
{
  Ptr<ITypeLib> pTypeLib;
  HRESULT result = ::LoadTypeLibEx( GetFileName(), REGKIND_NONE, &pTypeLib );
  if( S_OK != result )
    return result;
  if( inForUser )
    result = ::RegisterTypeLibForUser( pTypeLib, const_cast<wchar_t*>( GetFileName() ), NULL );
  else
    result = ::RegisterTypeLib( pTypeLib, GetFileName(), NULL );
  if( S_OK != result )
    return result;

  int action = Registrar::Create;
  if( inForUser )
    action |= Registrar::ForUser;
  result = RunRegScripts( action );

  return result;
}

HRESULT
Module::DllUnregisterServer( bool inForUser )
{
  Ptr<ITypeLib> pTypeLib;
  HRESULT result = ::LoadTypeLibEx( GetFileName(), REGKIND_NONE, &pTypeLib );
  if( FAILED( result ) )
    return result;

  TLIBATTR* pLibAttr = NULL;
  result = pTypeLib->GetLibAttr( &pLibAttr );
  if( FAILED( result ) || pLibAttr == NULL )
    return result;
  GUID guid = pLibAttr->guid;
  LCID lcid = pLibAttr->lcid;
  SYSKIND syskind = pLibAttr->syskind;
  WORD majorVerNum = pLibAttr->wMajorVerNum,
       minorVerNum = pLibAttr->wMinorVerNum;
  pTypeLib->ReleaseTLibAttr( pLibAttr );

  if( inForUser )
    result = ::UnRegisterTypeLibForUser( guid, majorVerNum, minorVerNum, lcid, syskind );
  else
    result = ::UnRegisterTypeLib( guid, majorVerNum, minorVerNum, lcid, syskind );

  int action = Registrar::Remove;
  if( inForUser )
    action |= Registrar::ForUser;
  HRESULT regScriptResult = RunRegScripts( action );

  if( FAILED( result ) )
    return result;

  if( FAILED( regScriptResult ) )
    return regScriptResult;

  return S_OK;
}

HRESULT
Module::DllInstall( BOOL inInstall, LPCWSTR inpCmdLine )
{
  HRESULT resultSystem = S_OK,
          resultUser = S_OK;
  bool system = true,
       user = false,
       any = false;
  if( inpCmdLine != NULL )
  {
    if( !::_wcsicmp( inpCmdLine, L"user" ) )
    {
      user = true;
      system = false;
    }
    else if( !::_wcsicmp( inpCmdLine, L"any" ) )
    {
      any = true;
    }
    else if( !::_wcsicmp( inpCmdLine, L"both" ) )
    {
      user = true;
      system = true;
    }
  }
  if( inInstall )
  {
    if( system || any )
    {
      resultSystem = Module::DllRegisterServer( false );
      if( FAILED( resultSystem ) )
        Module::DllUnregisterServer( false );
    }
    if( user || any && FAILED( resultSystem ) )
    {
      resultUser = Module::DllRegisterServer( true );
      if( FAILED( resultUser ) )
        Module::DllUnregisterServer( true );
    }
  }
  else // Uninstall
  {
    if( system || any )
      resultSystem = Module::DllUnregisterServer( false );
    if( user || any && FAILED( resultSystem ) )
      resultUser = Module::DllUnregisterServer( true );
  }
  HRESULT result = resultSystem;
  if( user && FAILED( resultUser ) || any && FAILED( resultSystem ) )
    result = resultUser;
  return result;
}

struct RegScriptInfo
{
  int action;
  HRESULT result;
};

HRESULT
Module::RunRegScripts( int inAction )
{
  RegScriptInfo info = { inAction, E_FAIL };
  ::EnumResourceNamesW( sHInstance, L"REGISTRY", &Module::RunRegScript, reinterpret_cast<LONG_PTR>( &info ) );
  return info.result;
}

BOOL CALLBACK
Module::RunRegScript( HMODULE inHModule, LPCWSTR inType, LPWSTR inName, LONG_PTR inParam )
{
  RegScriptInfo* pInfo = reinterpret_cast<RegScriptInfo*>( inParam );
  HRSRC res = ::FindResourceW( inHModule, inName, inType );
  if( !res )
    return FALSE;
  HGLOBAL hRes = ::LoadResource( inHModule, res );
  if( !hRes )
    return FALSE;
  DWORD size = ::SizeofResource( inHModule, res );
  if( size == 0 )
    return FALSE;
  LPVOID pRes = ::LockResource( hRes );
  if( !pRes )
    return FALSE;
  std::string script( reinterpret_cast<const char*>( pRes ), size );
  Registrar::Environment environment;
  environment["MODULE"] = DualString( GetFileName() );
  Registrar parser;
  if( !parser.Parse( script, environment ) )
    return FALSE;
  LONG status = parser.Execute( pInfo->action );
  pInfo->result = ( status == ERROR_SUCCESS ) ? S_OK : TYPE_E_REGISTRYACCESS;
  return ( pInfo->result == S_OK );
}

Module::Cleanup::~Cleanup()
{
  delete[] spFileName;
  delete[] spLocation;
}

