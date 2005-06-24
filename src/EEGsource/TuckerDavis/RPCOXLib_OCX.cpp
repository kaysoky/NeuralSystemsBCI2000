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
// File generated on 3/18/2005 11:28:45 AM from Type Library described below.

// ************************************************************************  //
// Type Lib: c:\TDT\ActiveX\RPcoX.ocx (1)
// LIBID: {D323A622-1D13-11D4-8858-444553540000}
// LCID: 0
// Helpfile: c:\TDT\ActiveX\RPcoX.hlp
// HelpString: RPcoX ActiveX Control module
// DepndLst: 
//   (1) v2.0 stdole, (C:\WINDOWS\System32\stdole2.tlb)
// ************************************************************************ //

#include <vcl.h>
#pragma hdrstop

#include <olectrls.hpp>
#if defined(USING_ATL)
#include <atl\atlvcl.h>
#endif

#include "RPCOXLib_OCX.h"

#if !defined(__PRAGMA_PACKAGE_SMART_INIT)
#define      __PRAGMA_PACKAGE_SMART_INIT
#pragma package(smart_init)
#endif

namespace Rpcoxlib_tlb
{



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TRPcoX which
// allows "RPcoX Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
int   TRPcoX::EventDispIDs[1] = {
    0x00000001};

TControlData TRPcoX::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0xD323A625, 0x1D13, 0x11D4,{ 0x88, 0x58, 0x44,0x45, 0x53, 0x54,0x00, 0x00} }, // CoClass
  {0xD323A624, 0x1D13, 0x11D4,{ 0x88, 0x58, 0x44,0x45, 0x53, 0x54,0x00, 0x00} }, // Events

  // Count of Events and array of their DISPIDs
  1, &EventDispIDs,

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

GUID     TRPcoX::DEF_CTL_INTF = {0xD323A623, 0x1D13, 0x11D4,{ 0x88, 0x58, 0x44,0x45, 0x53, 0x54,0x00, 0x00} };
TNoParam TRPcoX::OptParam;

static inline void ValidCtrCheck(TRPcoX *)
{
   delete new TRPcoX((TComponent*)(0));
};

void __fastcall TRPcoX::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TRPcoX::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DRPcoXDisp __fastcall TRPcoX::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

BSTR __fastcall TRPcoX::GetError(void)
{
  return GetDefaultInterface()->GetError();
}

long __fastcall TRPcoX::Connect(long Interface, long DevNum)
{
  return GetDefaultInterface()->Connect(Interface, DevNum);
}

long __fastcall TRPcoX::SetTagVal(BSTR Name, float Val)
{
  return GetDefaultInterface()->SetTagVal(Name, Val);
}

long __fastcall TRPcoX::LoadCOF(BSTR FileName)
{
  return GetDefaultInterface()->LoadCOF(FileName);
}

long __fastcall TRPcoX::Run(void)
{
  return GetDefaultInterface()->Run();
}

long __fastcall TRPcoX::Halt(void)
{
  return GetDefaultInterface()->Halt();
}

long __fastcall TRPcoX::SoftTrg(long Trg_Bitn)
{
  return GetDefaultInterface()->SoftTrg(Trg_Bitn);
}

float __fastcall TRPcoX::GetTagVal(BSTR Name)
{
  return GetDefaultInterface()->GetTagVal(Name);
}

long __fastcall TRPcoX::ReadTag(BSTR Name, float* pBuf, long nOS, long nWords)
{
  return GetDefaultInterface()->ReadTag(Name, pBuf, nOS, nWords);
}

long __fastcall TRPcoX::WriteTag(BSTR Name, float* pBuf, long nOS, long nWords)
{
  return GetDefaultInterface()->WriteTag(Name, pBuf, nOS, nWords);
}

long __fastcall TRPcoX::SendParTable(BSTR Name, float IndexID)
{
  return GetDefaultInterface()->SendParTable(Name, IndexID);
}

long __fastcall TRPcoX::SendSrcFile(BSTR Name, long SeekOS, long nWords)
{
  return GetDefaultInterface()->SendSrcFile(Name, SeekOS, nWords);
}

long __fastcall TRPcoX::GetNames(BSTR NameList, long MaxNames, long ObjType)
{
  return GetDefaultInterface()->GetNames(NameList, MaxNames, ObjType);
}

VARIANT __fastcall TRPcoX::ReadTagV(BSTR Name, long nOS, long nWords)
{
  return GetDefaultInterface()->ReadTagV(Name, nOS, nWords);
}

long __fastcall TRPcoX::WriteTagV(BSTR Name, long nOS, VARIANT Buf)
{
  return GetDefaultInterface()->WriteTagV(Name, nOS, Buf);
}

long __fastcall TRPcoX::GetTagSize(BSTR Name)
{
  return GetDefaultInterface()->GetTagSize(Name);
}

long __fastcall TRPcoX::GetTagType(BSTR Name)
{
  return GetDefaultInterface()->GetTagType(Name);
}

long __fastcall TRPcoX::SetSrcFileName(BSTR Name, BSTR FileName)
{
  return GetDefaultInterface()->SetSrcFileName(Name, FileName);
}

long __fastcall TRPcoX::GetNumOf(BSTR ObjTypeName)
{
  return GetDefaultInterface()->GetNumOf(ObjTypeName);
}

BSTR __fastcall TRPcoX::GetNameOf(BSTR ObjTypeName, long Index)
{
  return GetDefaultInterface()->GetNameOf(ObjTypeName, Index);
}

long __fastcall TRPcoX::ReadCOF(BSTR FileName)
{
  return GetDefaultInterface()->ReadCOF(FileName);
}

long __fastcall TRPcoX::ConnectRP2(BSTR IntName, long DevNum)
{
  return GetDefaultInterface()->ConnectRP2(IntName, DevNum);
}

long __fastcall TRPcoX::ConnectRL2(BSTR IntName, long DevNum)
{
  return GetDefaultInterface()->ConnectRL2(IntName, DevNum);
}

long __fastcall TRPcoX::ConnectRA16(BSTR IntName, long DevNum)
{
  return GetDefaultInterface()->ConnectRA16(IntName, DevNum);
}

VARIANT __fastcall TRPcoX::ReadTagVEX(BSTR Name, long nOS, long nWords, BSTR SrcType, BSTR DstType, 
                                      long nChans)
{
  return GetDefaultInterface()->ReadTagVEX(Name, nOS, nWords, SrcType, DstType, nChans);
}

long __fastcall TRPcoX::GetStatus(void)
{
  return GetDefaultInterface()->GetStatus();
}

long __fastcall TRPcoX::GetCycUse(void)
{
  return GetDefaultInterface()->GetCycUse();
}

long __fastcall TRPcoX::ClearCOF(void)
{
  return GetDefaultInterface()->ClearCOF();
}

long __fastcall TRPcoX::WriteTagVEX(BSTR Name, long nOS, BSTR DstType, VARIANT Buf)
{
  return GetDefaultInterface()->WriteTagVEX(Name, nOS, DstType, Buf);
}

long __fastcall TRPcoX::ZeroTag(BSTR Name)
{
  return GetDefaultInterface()->ZeroTag(Name);
}

float __fastcall TRPcoX::GetSFreq(void)
{
  return GetDefaultInterface()->GetSFreq();
}

long __fastcall TRPcoX::ConnectRV8(BSTR IntName, long DevNum)
{
  return GetDefaultInterface()->ConnectRV8(IntName, DevNum);
}

long __fastcall TRPcoX::GetDevCfg(long Addr, long Width32)
{
  return GetDefaultInterface()->GetDevCfg(Addr, Width32);
}

long __fastcall TRPcoX::SetDevCfg(long Addr, long Val, long Width32)
{
  return GetDefaultInterface()->SetDevCfg(Addr, Val, Width32);
}

long __fastcall TRPcoX::LoadCOFsf(BSTR FileName, float SampFreq)
{
  return GetDefaultInterface()->LoadCOFsf(FileName, SampFreq);
}

long __fastcall TRPcoX::DefStatus(long DefID)
{
  return GetDefaultInterface()->DefStatus(DefID);
}

VARIANT __fastcall TRPcoX::GetDefData(long DefID)
{
  return GetDefaultInterface()->GetDefData(DefID);
}

void __fastcall TRPcoX::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

long __fastcall TRPcoX::ConnectRM1(BSTR IntName, long DevNum)
{
  return GetDefaultInterface()->ConnectRM1(IntName, DevNum);
}

long __fastcall TRPcoX::ConnectRM2(BSTR IntName, long DevNum)
{
  return GetDefaultInterface()->ConnectRM2(IntName, DevNum);
}

long __fastcall TRPcoX::ConnectRX5(BSTR IntName, long DevNum)
{
  return GetDefaultInterface()->ConnectRX5(IntName, DevNum);
}

long __fastcall TRPcoX::ConnectRX6(BSTR IntName, long DevNum)
{
  return GetDefaultInterface()->ConnectRX6(IntName, DevNum);
}

long __fastcall TRPcoX::ConnectRX7(BSTR IntName, long DevNum)
{
  return GetDefaultInterface()->ConnectRX7(IntName, DevNum);
}


};     // namespace Rpcoxlib_tlb


// *********************************************************************//
// The Register function is invoked by the IDE when this module is 
// installed in a Package. It provides the list of Components (including
// OCXes) implemented by this module. The following implementation
// informs the IDE of the OCX proxy classes implemented here.
// *********************************************************************//
namespace Rpcoxlib_ocx
{

void __fastcall PACKAGE Register()
{
  // [1]
  TComponentClass cls_ocx[] = {
                              __classid(Rpcoxlib_tlb::TRPcoX)
                           };
  RegisterComponents("ActiveX", cls_ocx,
                     sizeof(cls_ocx)/sizeof(cls_ocx[0])-1);
}

};     // namespace Rpcoxlib_ocx
