// ************************************************************************ //
// WARNING                                                                    
// -------                                                                    
// The types declared in this file were generated from data read from a       
// Type Library. If this type library is explicitly or indirectly (via        
// another type library referring to this type library) re-imported, or the   
// 'Refresh' command of the Type Library Editor activated while editing the   
// Type Library, the contents of this file will be regenerated and all        
// manual modifications will be lost.                                         
// ************************************************************************ //

// C++ TLBWRTR : $Revision$
// File generated on 1/23/2008 3:17:18 PM from Type Library described below.

// ************************************************************************  //
// Type Lib: c:\TDT\ActiveX\zBUSx.ocx (1)
// LIBID: {10055D3E-3938-4652-B6A2-6A6A4184D18D}
// LCID: 0
// Helpfile: c:\TDT\ActiveX\zBUSx.hlp
// HelpString: zBUSx ActiveX Control module
// DepndLst: 
//   (1) v2.0 stdole, (C:\WINDOWS\System32\stdole2.tlb)
// ************************************************************************ //

#include <vcl.h>
#pragma hdrstop

#include <olectrls.hpp>
#include <oleserver.hpp>
#if defined(USING_ATL)
#include <atl\atlvcl.h>
#endif

#include "ZBUSXLib_OCX.h"

#if !defined(__PRAGMA_PACKAGE_SMART_INIT)
#define      __PRAGMA_PACKAGE_SMART_INIT
#pragma package(smart_init)
#endif

namespace Zbusxlib_tlb
{



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TZBUSx which
// allows "ZBUSx Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TZBUSx::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x79734A6C, 0x8E6E, 0x4998,{ 0xB8, 0x34, 0x3E,0x4E, 0x48, 0x12,0x32, 0xB0} }, // CoClass
  {0x575833D5, 0x0B5E, 0x4759,{ 0xB3, 0x70, 0x40,0xFA, 0x23, 0xD4,0x09, 0xE5} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TZBUSx::DEF_CTL_INTF = {0x9F05A891, 0xD2B9, 0x41AF,{ 0x8C, 0x8E, 0x3F,0x42, 0x45, 0x26,0x14, 0x83} };
TNoParam TZBUSx::OptParam;

static inline void ValidCtrCheck(TZBUSx *)
{
   delete new TZBUSx((TComponent*)(0));
};

void __fastcall TZBUSx::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TZBUSx::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DZBUSxDisp __fastcall TZBUSx::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

long __fastcall TZBUSx::Connect(long Interface)
{
  return GetDefaultInterface()->Connect(Interface);
}

long __fastcall TZBUSx::GetDeviceAddr(long DevType, long DevNum)
{
  return GetDefaultInterface()->GetDeviceAddr(DevType, DevNum);
}

long __fastcall TZBUSx::GetDeviceVersion(long DevType, long DevNum)
{
  return GetDefaultInterface()->GetDeviceVersion(DevType, DevNum);
}

long __fastcall TZBUSx::HardwareReset(long RackNum)
{
  return GetDefaultInterface()->HardwareReset(RackNum);
}

long __fastcall TZBUSx::FlushIO(long RackNum)
{
  return GetDefaultInterface()->FlushIO(RackNum);
}

long __fastcall TZBUSx::zBusTrigA(long RackNum, long zTrgMode, long Delay)
{
  return GetDefaultInterface()->zBusTrigA(RackNum, zTrgMode, Delay);
}

long __fastcall TZBUSx::zBusTrigB(long RackNum, long zTrgMode, long Delay)
{
  return GetDefaultInterface()->zBusTrigB(RackNum, zTrgMode, Delay);
}

long __fastcall TZBUSx::zBusSync(long RackMask)
{
  return GetDefaultInterface()->zBusSync(RackMask);
}

long __fastcall TZBUSx::KillCode(long DevType, long DevNum, long MagicCode)
{
  return GetDefaultInterface()->KillCode(DevType, DevNum, MagicCode);
}

BSTR __fastcall TZBUSx::GetError(void)
{
  return GetDefaultInterface()->GetError();
}

BSTR __fastcall TZBUSx::GetDeviceAt(long RackNum, long PosNum, long* DevID, long* DevNum)
{
  return GetDefaultInterface()->GetDeviceAt(RackNum, PosNum, DevID, DevNum);
}

long __fastcall TZBUSx::ConnectZBUS(BSTR IntName)
{
  return GetDefaultInterface()->ConnectZBUS(IntName);
}

void __fastcall TZBUSx::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}


};     // namespace Zbusxlib_tlb


// *********************************************************************//
// The Register function is invoked by the IDE when this module is 
// installed in a Package. It provides the list of Components (including
// OCXes) implemented by this module. The following implementation
// informs the IDE of the OCX proxy classes implemented here.
// *********************************************************************//
namespace Zbusxlib_ocx
{

void __fastcall PACKAGE Register()
{
  // [1]
  TComponentClass cls_ocx[] = {
                              __classid(Zbusxlib_tlb::TZBUSx)
                           };
  RegisterComponents("ActiveX", cls_ocx,
                     sizeof(cls_ocx)/sizeof(cls_ocx[0])-1);
}

};     // namespace Zbusxlib_ocx
