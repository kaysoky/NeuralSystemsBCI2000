#include "DummySound.h"

#include <cstdio>

DummySound::DummySound() : playing(false) {printf("sound 0x%08lx constructed\n", (unsigned long)this);}
DummySound::~DummySound()                 {printf("sound 0x%08lx destructed\n",  (unsigned long)this);}
bool DummySound::Initialize(const double* data,
                            unsigned long samplesPerSecond,
                            unsigned long bitsPerSample,
                            unsigned long nSamples,
                            unsigned long nChannels,
                            unsigned long byteStrideSample2Sample,
                            unsigned long byteStrideChannel2Channel
                           )
{
	printf("sound 0x%08lx initialized\n", (unsigned long)this);
	return true;
}
void DummySound::Sleep(double msec) {}
bool DummySound::IsPlaying(void) {return playing;}
bool DummySound::Play(int nTimes)
{
	if(IsPlaying()) {printf("sound 0x%08lx is already playing\n", (unsigned long)this); return false;}
	Preplay();
	if(nTimes < 0) {playing = 1; printf("sound 0x%08lx looping\n", (unsigned long)this); return true;}
	for(int i = 0; i < nTimes; i++) printf("sound 0x%08lx played\n", (unsigned long)this);
	Postplay();
	return true;
}
bool DummySound::Stop(void)
{
	if(!IsPlaying()) return false;
	printf("sound 0x%08lx stopped\n", (unsigned long)this);
	playing = 0;
	Postplay();
	return true;
}
bool DummySound::SetSpeed(double val)  {printf("sound 0x%08lx set speed to %g\n",  (unsigned long)this, val); return true;}
bool DummySound::SetVolume(double val) {printf("sound 0x%08lx set volume to %g\n", (unsigned long)this, val); return true;}
bool DummySound::SetPan(double val)    {printf("sound 0x%08lx set pan to %g\n",    (unsigned long)this, val); return true;}
