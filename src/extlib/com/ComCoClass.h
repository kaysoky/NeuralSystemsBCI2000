////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A template for a COM co-class. To declare a co-class "MyClass"
//   immediately inheriting IInterface, specify inheritance and template para-
//   meters as follows:
//
//   class MyClass : public CoClass<IInterface, &IID_Interface, MyClass, &CLSID_MyClass>
//   {
//     ...
//   };
//
//   When MyClass inherits from IDispatch, you need to provide its library's
//   TypeLib as a "TYPELIB" resource in the COM DLL.
//   In order to allow instantiation from outside the DLL, put a
//
//   ComRegisterCoClass( MyClass );
//
//   at the beginning of MyClass.cpp, and add a "REGISTRY" type resource to your
//   COM DLL. Use MyCoClass.rgs as a template for the content of that resource.
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
#ifndef COM_CO_CLASS_H
#define COM_CO_CLASS_H

#include "ComClassFactory.h"
#include "ComModule.h"

#define ComRegisterCoClass(x) static const CLSID* x##Registration_ = x::Register();

namespace com {

template<class T, const IID* U, class V, const CLSID* W>
class CoClass : public T
{
 public:
  CoClass()
  : mRefCount( 0 ),
    mpTypeInfo( NULL )
  {
    sFactory.IncObjectCount();
    Module::GetTypeLib()->GetTypeInfoOfGuid( *U, mpTypeInfo.Assignee() );
  }
  virtual ~CoClass()
  {
    sFactory.DecObjectCount();
  }
  // IUnknown
  STDMETHOD( QueryInterface )( const IID& inIID, void** outInterface )
  {
    *outInterface = NULL;
    if( inIID == *U )
      *outInterface = static_cast<T*>( this );
    else if( inIID == IID_IUnknown )
      *outInterface = static_cast<IUnknown*>( this );
    else if( mpTypeInfo && inIID == IID_IDispatch )
      *outInterface = static_cast<IDispatch*>( this );
    if( *outInterface )
      this->AddRef();
    return *outInterface ? S_OK : E_NOINTERFACE;
  }
  virtual ULONG STDMETHODCALLTYPE AddRef()
  {
    return ++mRefCount;
  }
  virtual ULONG STDMETHODCALLTYPE Release()
  {
    if( --mRefCount == 0 )
    {
      delete this;
      return 0;
    }
    return mRefCount;
  }
  // IDispatch
  STDMETHOD( GetTypeInfoCount )( UINT* pOut )
  {
    *pOut = 1;
    return S_OK;
  }
  STDMETHOD( GetTypeInfo )( UINT inIdx, LCID, ITypeInfo** pOut )
  {
    if( inIdx != 0 )
      return E_INVALIDARG;
    *pOut = mpTypeInfo;
    (*pOut)->AddRef();
    return S_OK;
  }
  STDMETHOD( GetIDsOfNames )( const IID&, LPOLESTR* inpNames, UINT inCount, LCID, DISPID* outpIDs )
  {
    return mpTypeInfo->GetIDsOfNames( inpNames, inCount, outpIDs );
  }
  STDMETHOD( Invoke )( DISPID inID, const IID&, LCID, WORD inFlags, DISPPARAMS* inpParams, VARIANT* outpResult, EXCEPINFO* outpExcepInfo, UINT* outpArgErr )
  {
    return mpTypeInfo->Invoke( static_cast<T*>( this ), inID, inFlags, inpParams, outpResult, outpExcepInfo, outpArgErr );
  }

 public:
  static const CLSID* Register()
  {
    return sFactory.ClassID();
  }

 private:
  int mRefCount;
  Ptr<ITypeInfo> mpTypeInfo;
  static ClassFactory<V,W> sFactory;
};

template<class T, const IID* U, class V, const CLSID* W>
ClassFactory<V, W> CoClass<T, U, V, W>::sFactory;

} // namespace

#endif // COM_CO_CLASS_H
