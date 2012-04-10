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
#include <Shlwapi.h>

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
      spLocation = new wchar_t[::wcslen( pPath ) + 1];
      ::wcscpy( spLocation, pPath );
      ::PathRemoveFileSpecW( spLocation );
      ::PathAddBackslashW( spLocation );
    }
  }
  return spLocation;
}

Ptr<ITypeLib>
Module::GetTypeLib()
{
  if( !spTypeLib )
    ::LoadRegTypeLib( *spLibID, 1, 0, 0, spTypeLib.Assignee() );
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
Module::DllRegisterServer()
{
  Ptr<ITypeLib> pTypeLib;
  HRESULT result = ::LoadTypeLibEx( GetFileName(), REGKIND_NONE, pTypeLib.Assignee() );
  if( S_OK != result )
    return result;
  result = ::RegisterTypeLib( pTypeLib, const_cast<wchar_t*>( GetFileName() ), NULL );
  if( S_OK != result )
    return result;
  result = RunRegScripts( Registrar::Create );
  return result;
}

HRESULT
Module::DllUnregisterServer()
{
  Ptr<ITypeLib> pTypeLib;
  HRESULT result = ::LoadTypeLibEx( GetFileName(), REGKIND_NONE, pTypeLib.Assignee() );
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
  result = ::UnRegisterTypeLib( guid, majorVerNum, minorVerNum, lcid, syskind );

  HRESULT regScriptResult = RunRegScripts( Registrar::Remove );

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
    if( !::_wcsicmp( inpCmdLine, L"system" ) )
    {
      system = true;
      user = false;
    }
    else if( !::_wcsicmp( inpCmdLine, L"user" ) )
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
    else
      return E_FAIL;
  }
  if( inInstall )
  {
    if( system || any )
    {
      if( ERROR_SUCCESS != RedirectHKCR( System ) )
        return E_FAIL;
      resultSystem = Module::DllRegisterServer();
      if( FAILED( resultSystem ) )
        Module::DllUnregisterServer();
    }
    if( user || any && FAILED( resultSystem ) )
    {
      if( ERROR_SUCCESS != RedirectHKCR( User ) )
        return E_FAIL;
      resultUser = Module::DllRegisterServer();
      if( FAILED( resultUser ) )
        Module::DllUnregisterServer();
    }
  }
  else // Uninstall
  {
    if( system || any )
    {
      if( ERROR_SUCCESS != RedirectHKCR( System ) )
        return E_FAIL;
      resultSystem = Module::DllUnregisterServer();
    }
    if( user || any )
    {
      if( ERROR_SUCCESS != RedirectHKCR( User ) )
        return E_FAIL;
      resultUser = Module::DllUnregisterServer();
    }
  }
  RedirectHKCR( None );

  HRESULT result = resultSystem;
  if( user && FAILED( resultUser ) || any && FAILED( resultSystem ) )
    result = resultUser;
  return result;
}

LONG
Module::RedirectHKCR( RedirectionType inType )
{
  LONG result = ERROR_SUCCESS;
  HKEY key = NULL;
  switch( inType )
  {
    case None:
      key = NULL;
      break;
    case System:
      key = HKEY_LOCAL_MACHINE;
      break;
    case User:
      key = HKEY_CURRENT_USER;
      break;
    default:
      return -1;
  }
  if( key )
    result = ::RegCreateKeyExA( key, "Software\\Classes", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key, NULL );

  if( ERROR_SUCCESS != result )
    return result;

  HINSTANCE module = ::LoadLibraryA( "Advapi32.dll" );
  if( !module )
    return ::GetLastError();

  typedef LONG (WINAPI *funcType)( HKEY, HKEY );
  funcType RegOverridePredefKey = reinterpret_cast<funcType>( ::GetProcAddress( module, "RegOverridePredefKey" ) );
  if( !RegOverridePredefKey )
    return ::GetLastError();
  result = RegOverridePredefKey( HKEY_CLASSES_ROOT, key );

  if( key )
    ::RegCloseKey( key );
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
  RegScriptInfo info = { inAction, ERROR_SUCCESS };
#ifdef __GNUC__ // Bug in EnumResourceNamesW declaration in Winbase.h
#define ENUMRESNAMEPROCW ENUMRESNAMEPROC
#endif // __GNUC__
  ::EnumResourceNamesW( sHInstance, L"REGISTRY", reinterpret_cast<ENUMRESNAMEPROCW>( &Module::RunRegScript ), reinterpret_cast<LONG_PTR>( &info ) );
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
  Registrar registrar;
  if( !registrar.Parse( script, environment ) )
    return FALSE;
  LONG status = registrar.Execute( pInfo->action );
  if( pInfo->result == S_OK )
    pInfo->result = ( status == ERROR_SUCCESS ) ? S_OK : TYPE_E_REGISTRYACCESS;
  return ( pInfo->action & Registrar::Remove ) ? TRUE : ( pInfo->result == S_OK );
}

Module::Cleanup::~Cleanup()
{
  delete[] spFileName;
  delete[] spLocation;
}

