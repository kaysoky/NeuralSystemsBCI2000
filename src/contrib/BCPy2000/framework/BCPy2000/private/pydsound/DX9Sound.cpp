#ifndef __DX9SOUND_CPP__
#define __DX9SOUND_CPP__

#include <cstdio> // TODO: remove
#include <cmath>
#include <windows.h>

#include "dsound.h"

#include "DX9Sound.h"
#define checkdserr(a)  ((err==a) && printf("%s\n", #a))

int InitializeDirectSound(int channels=2, int bitsPerSample=16, int samplesPerSecond=44100, HWND hWnd=NULL);
int DeinitializeDirectSound(void);

unsigned long g_objCount = 0;
LPDIRECTSOUND g_pDS = NULL;
LPDIRECTSOUNDBUFFER g_pDSB1 = NULL;

#ifndef DYNAMIC_DSOUND
#define DYNAMIC_DSOUND 1
#endif /* DYNAMIC_DSOUND */
#if DYNAMIC_DSOUND
HINSTANCE g_dsoundDLL = NULL;
typedef HRESULT (*dscreateproto)(const GUID*, IDirectSound**, IUnknown*);
dscreateproto g_directSoundCreate = NULL;
#else /* DYNAMIC_DSOUND is 0 */
#define g_directSoundCreate DirectSoundCreate
#endif /* DYNAMIC_DSOUND */

#ifdef MATLAB_MEX_FILE
#define makeMemoryPersistent(p) mexMakeMemoryPersistent(p)
#else /* MATLAB_MEX_FILE not defined */
#define makeMemoryPersistent(p)
#endif /* MATLAB_MEX_FILE */

int InitializeDirectSound(int channels, int bitsPerSample, int samplesPerSecond, HWND hWnd)
{
	if(g_pDSB1) return 0; // for now this is the sign that it's already inited

	if(!hWnd) {
		hWnd = GetActiveWindow();  // originally we had  hWnd = FindWindow(0, "MATLAB Command Window");
		if(!hWnd) hWnd = GetDesktopWindow(); // GetActiveWindow works from Matlab but fails from IPython
		if(!hWnd) return -1;
	}
	if(!g_pDS) {
#if DYNAMIC_DSOUND
		if(!g_dsoundDLL) g_dsoundDLL = LoadLibrary("dsound");
		if(!g_dsoundDLL) return -10;
		if(!g_directSoundCreate) g_directSoundCreate = (dscreateproto) GetProcAddress(g_dsoundDLL, "DirectSoundCreate");
		if(!g_directSoundCreate) return -11;
#endif /* DYNAMIC_DSOUND */
		if(FAILED(g_directSoundCreate(NULL, &g_pDS, NULL)) || !g_pDS) return -2;
		makeMemoryPersistent(g_pDS);
	}
	if(hWnd && FAILED(g_pDS->SetCooperativeLevel(hWnd, DSSCL_EXCLUSIVE))) return -3;
	
	// create the primary buffer
	DSBUFFERDESC bd; 
	ZeroMemory(&bd, sizeof(bd));
	bd.dwSize = sizeof(bd);
	bd.dwBufferBytes = 0; // must be 0 for primary buffers
	bd.lpwfxFormat = NULL; // must be NULL for primary buffers
	bd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_PRIMARYBUFFER | DSBCAPS_GLOBALFOCUS; 
	bd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_PRIMARYBUFFER | DSBCAPS_STICKYFOCUS; 
	// NB: CreateSoundBuffer seems to return DSERR_INVALIDPARAM when either DSBCAPS_CTRLFREQUENCY or (as in Ryota's original) DSBCAPS_GLOBALFOCUS are added
	//     Mexfile version may be silently failing all the time. Certainly most things seem to work even if it does fail.
	if(!g_pDSB1) {
		HRESULT err = g_pDS->CreateSoundBuffer(&bd, &g_pDSB1, NULL);
		if(err != DS_OK) printf("CreateSoundBuffer returned %ld\n", (long)err);
		checkdserr(DSERR_ALLOCATED);
		checkdserr(DSERR_BADFORMAT);
		checkdserr(DSERR_BUFFERTOOSMALL);
		checkdserr(DSERR_CONTROLUNAVAIL);
		checkdserr(DSERR_DS8_REQUIRED);
		checkdserr(DSERR_INVALIDCALL);
		checkdserr(DSERR_INVALIDPARAM);
		checkdserr(DSERR_NOAGGREGATION);
		checkdserr(DSERR_OUTOFMEMORY);
		checkdserr(DSERR_UNINITIALIZED);
		checkdserr(DSERR_UNSUPPORTED);
		if(!g_pDSB1) return -4;
		makeMemoryPersistent(g_pDSB1);
	}
	WAVEFORMATEX wf; 
	ZeroMemory(&wf, sizeof(wf)); 
	wf.wFormatTag = WAVE_FORMAT_PCM; 
	wf.nChannels = channels;
	wf.wBitsPerSample = bitsPerSample;
	wf.nSamplesPerSec = samplesPerSecond;
	wf.nBlockAlign = wf.nChannels * DX9Sound::BytesPerSample(bitsPerSample);
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
	
	if(FAILED(g_pDSB1->SetFormat(&wf))) return -5;
	
	//printf("initialized DirectSound\n");
	return 0;
}
int DeinitializeDirectSound(void)
{
	if (g_pDSB1) g_pDSB1->Stop();
	if (g_pDSB1) g_pDSB1->Release();
	g_pDSB1 = NULL;
	if (g_pDS) g_pDS->Release();
	g_pDS = NULL;
	//printf("deinitialized DirectSound\n");
	return 0;
}

DX9Sound::DX9Sound()
: pB(NULL), mSamplesPerSecond(0)
{
	InitializeDirectSound();
	g_objCount++;
	//printf("sound 0x%08lx constructed\n", (unsigned long)this);
}

DX9Sound::~DX9Sound()
{
	if(pB) pB->Stop();
	if(pB) pB->Release();
	pB = NULL;
	//printf("sound 0x%08lx destructed\n",  (unsigned long)this);
	if(--g_objCount == 0) DeinitializeDirectSound();
}

bool DX9Sound::Initialize(const double* data,
                          unsigned long samplesPerSecond,
                          unsigned long bitsPerSample,
                          unsigned long nSamples,
                          unsigned long nChannels,
                          unsigned long byteStrideSample2Sample,
                          unsigned long byteStrideChannel2Channel)
{
	unsigned long bufferSize = BufferSize(bitsPerSample, nSamples, nChannels);
	unsigned long bytesPerSample = BytesPerSample(bitsPerSample);
	mSamplesPerSecond = samplesPerSecond;
	
	DSBUFFERDESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCDEFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STICKYFOCUS;
	bd.dwBufferBytes = bufferSize;
	bd.dwSize = sizeof(bd);
	
	tWAVEFORMATEX format;
	format.wFormatTag = 1;
	format.nChannels = nChannels;
	format.nSamplesPerSec = samplesPerSecond;
	format.nAvgBytesPerSec = samplesPerSecond * bytesPerSample * nChannels;
	format.nBlockAlign = bytesPerSample * nChannels;
	format.wBitsPerSample = bitsPerSample;
	format.cbSize = 0;
	
	bd.lpwfxFormat = &format;
	
	g_pDS->CreateSoundBuffer(&bd, &pB, NULL);
	if (!pB) return false;
	makeMemoryPersistent(pB);
	
	void *p1, *p2;
	unsigned long s1,  s2;
	if (FAILED(pB->Lock(0, bufferSize, &p1, &s1, &p2, &s2, 0))) return false;
	
	// memcpy(p1, rawdata, s1); if(s2) memcpy(p2, rawdata, s2);
	Doubles2Ints(data, (char*)p1, bitsPerSample, nSamples, nChannels, byteStrideSample2Sample, byteStrideChannel2Channel);
	if(s2) memcpy(p2, p1, s2);
	pB->Unlock(p1, s1, p2, s2);
	
	//printf("sound 0x%08lx initialized\n", (unsigned long)this);
	return true;
}

void DX9Sound::Sleep(double msec)
{
	::Sleep((int)(0.5 + msec));
}

bool DX9Sound::IsPlaying(void)
{
	DWORD status;
	if(!pB) return false;
	pB->GetStatus(&status);
	return (status & DSBSTATUS_PLAYING);
}

bool DX9Sound::Play(int nTimes)
{
	if(IsPlaying()) {
		//printf("sound 0x%08lx is already playing\n", (unsigned long)this);
		return false;
	}
	if (!pB) return false;
	DWORD flags = 0;
	if(nTimes == -1) flags |= DSBPLAY_LOOPING;
	else if(nTimes != 1) return false; // NB: So far there seems to be no way of specifying a finite multiple number of times to play.
	pB->SetCurrentPosition(0);
	Preplay(); // NB: Preplay timing will be tight. Postplay timing will be tight for infinite looped play (triggered by Stop), but otherwise will rely on periodic polling using the superclass Check method (is there another way?)
	pB->Play(0, 0, flags);
	return true;
}

bool DX9Sound::Stop(void)
{
	if(!IsPlaying()) return false;
	pB->Stop();
	Postplay();
	return true;
}

bool DX9Sound::SetSpeed(double val)
{
	if(!pB) return false;
	if(!(val > 0.0)) return false;
	int d = (int)(0.5 + val * mSamplesPerSecond);
	if(d < DSBFREQUENCY_MIN || d > DSBFREQUENCY_MAX) return false;
	pB->SetFrequency(d);
	return true;
}

bool DX9Sound::SetVolume(double val)
{
	LONG setting;
	if(!pB) return false;
	if(val < 0.0 || val > 1.0) return false;
	if(val == 0.0) setting = DSBVOLUME_MIN;
	else {
		val = 20.0 * log(val) / log(10.0);
		setting = DSBVOLUME_MAX + (LONG)(0.5 + 100.0 * val);
		if(setting < DSBVOLUME_MIN) setting = DSBVOLUME_MIN;
	}
	pB->SetVolume(setting);
	return true;
}

bool DX9Sound::SetPan(double val)
{
	LONG setting;
	if(!pB) return false;
	if(val < -1.0 || val > +1.0) return false;
	if(val < 0.0) {
		double multiply_right = val + 1.0; // in range [0, 1): its log is < 0
		if(multiply_right == 0.0) setting = DSBPAN_LEFT;
		else {
			val = 20.0 * log10(multiply_right) / log(10.0); // dB < 0
			setting = (LONG)(0.5 + 100.0 * val); // a negative value, which is correct for indicating pan to the left
			if(setting < DSBPAN_LEFT) setting = DSBPAN_LEFT;
		}
	}
	else {
		double multiply_left = 1.0 - val; // in range [0, 1]: its log is <=0
		if(multiply_left == 0.0) setting = DSBPAN_RIGHT;
		else {
			val = -20.0 * log10(multiply_left) / log(10.0); // dB >= 0
			setting = (LONG)(0.5 + 100.0 * val); // a positive value, which is correct for indicating pan to the right
			if(setting > DSBPAN_RIGHT) setting = DSBPAN_RIGHT;
		}
	}
	pB->SetPan(setting);
	return true;
}
#endif /* __DX9SOUND_CPP__ */
