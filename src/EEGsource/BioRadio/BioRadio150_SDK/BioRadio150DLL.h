/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef _BIORADIO_150_DLL_H_
#define _BIORADIO_150_DLL_H_

extern "C" __declspec(dllexport) DWORD _stdcall CreateBioRadio();
extern "C" __declspec(dllexport) int _stdcall DestroyBioRadio(DWORD);

extern "C" __declspec(dllexport) int _stdcall StartAcq(DWORD, char, char *);
extern "C" __declspec(dllexport) int _stdcall StopAcq(DWORD);

extern "C" __declspec(dllexport) int _stdcall LoadConfig(DWORD, char *);
extern "C" __declspec(dllexport) int _stdcall PingConfig(DWORD, char);
extern "C" __declspec(dllexport) int _stdcall ProgramConfig(DWORD, char, const char *);

extern "C" __declspec(dllexport) int _stdcall ReadScaled(DWORD, double *, int, int *);
extern "C" __declspec(dllexport) int _stdcall TransferBuffer(DWORD);

extern "C" __declspec(dllexport) double _stdcall SetBadDataValue(DWORD, double);

extern "C" __declspec(dllexport) long _stdcall GetNumChannels(DWORD);
extern "C" __declspec(dllexport) char _stdcall GetEnabledChannels(DWORD);
extern "C" __declspec(dllexport) int _stdcall GetFreqHoppingMode(DWORD);
extern "C" __declspec(dllexport) int _stdcall GetFreqHoppingModeIndicator();
extern "C" __declspec(dllexport) int _stdcall SetFreqHoppingMode(DWORD, bool);
extern "C" __declspec(dllexport) long _stdcall GetSampleRate(DWORD);
extern "C" __declspec(dllexport) long _stdcall GetBitResolution(DWORD);

extern "C" __declspec(dllexport) DWORD _stdcall GetGoodPackets(DWORD);
extern "C" __declspec(dllexport) DWORD _stdcall GetBadPackets(DWORD);
extern "C" __declspec(dllexport) DWORD _stdcall GetDroppedPackets(DWORD);
extern "C" __declspec(dllexport) int _stdcall GetUpRSSI(DWORD);
extern "C" __declspec(dllexport) int _stdcall GetDownRSSI(DWORD);
extern "C" __declspec(dllexport) int _stdcall GetLinkBufferSize(DWORD);
extern "C" __declspec(dllexport) int _stdcall GetBitErrCount(DWORD);
extern "C" __declspec(dllexport) int _stdcall GetBitErrRate(DWORD);

extern "C" __declspec(dllexport) int _stdcall GetRFChannel(DWORD);
extern "C" __declspec(dllexport) int _stdcall SetRFChannel(DWORD, int);
extern "C" __declspec(dllexport) int _stdcall GetUsableRFChannelList(int *, int);

#endif //_BIORADIO_150_DLL_H_


