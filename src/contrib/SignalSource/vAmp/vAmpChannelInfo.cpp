//---------------------------------------------------------------------------


#pragma hdrstop

#include "vAmpChannelInfo.h"

///////////////////////////////////////////////////////////
// class CChannelInfo
///////////////////////////////////////////////////////////
CChannelInfo::CChannelInfo()
: nType(DEVICE_CHAN_TYPE_UNKNOWN)
, nPhysIdx(0)
, szLabel("")
, dResolution(1.0)
, bUsrUnit(FALSE)
, szUnitName("")
, fGradient(1.0f)
, fOffset(0.0f)
{
}

const CChannelInfo &
	CChannelInfo::operator = (const CChannelInfo &c2)
{
	nPhysIdx = c2.nPhysIdx;
	szLabel = c2.szLabel;
	bUsrUnit = c2.bUsrUnit;
	szUnitName = c2.szUnitName;
	fGradient = c2.fGradient;
	fOffset = c2.fOffset;
	nType = c2.nType;
	return *this;
}

const bool CChannelInfo::operator == (const CChannelInfo &c2)
{
	return (szLabel == c2.szLabel && 
		nPhysIdx == c2.nPhysIdx) ? TRUE : FALSE;
}
//---------------------------------------------------------------------------

#pragma package(smart_init)
