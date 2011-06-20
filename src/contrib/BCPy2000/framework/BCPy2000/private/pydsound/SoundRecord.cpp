#include "SoundRecord.h"
#include "lpt.h"

// #include <cstdio>

SoundRecord::SoundRecord()
{
	if(!CheckLPT()) InitLPT();
	InitPreplayLPT(-1, 2);
	InitPostplayLPT(-1, 2);
	postplayPending = false;
}
SoundRecord::~SoundRecord()
{
	//printf("virtual-base-class destructor called on 0x%08lx\n", (unsigned long)this);
	RemoveFromGlobalList(this);
}
void SoundRecord::InitPreplayLPT(int val, int port)
{
	lpt_preplay_val  = val;
	switch(port) {
		case  0: break; // no change
		case  1: lpt_preplay_port = 0x3BC; break;
		case  2: lpt_preplay_port = 0x378; break;
		case  3: lpt_preplay_port = 0x278; break;
		default: lpt_preplay_port = port;  break;
	}
}
void SoundRecord::InitPostplayLPT(int val, int port)
{
	lpt_postplay_val  = val;
	switch(port) {
		case  0: break; // no change
		case  1: lpt_postplay_port = 0x3BC; break;
		case  2: lpt_postplay_port = 0x378; break;
		case  3: lpt_postplay_port = 0x278; break;
		default: lpt_postplay_port = port;  break;
	}
}

void SoundRecord::Preplay(void)
{
	postplayPending = true;
	if(lpt_preplay_port == -1 || lpt_preplay_val == -1) return;
	//printf("setting LPT state to %d on port 0x%X\n", lpt_preplay_val, lpt_preplay_port);
	LPTOut(lpt_preplay_val, lpt_preplay_port);
}
void SoundRecord::Postplay(void)
{
	postplayPending = false;
	if(lpt_postplay_port == -1 || lpt_postplay_val == -1) return;
	//printf("setting LPT state to %d on port 0x%X\n", lpt_postplay_val, lpt_postplay_port);
	LPTOut(lpt_postplay_val, lpt_postplay_port);
}
bool SoundRecord::Check(double msecToSleepFirst)
{
	if(msecToSleepFirst) Sleep(msecToSleepFirst);
	bool isplaying = IsPlaying();
	if(postplayPending && !isplaying) Postplay();
	return isplaying;
}

/////////////////////////////////////////////////////////////////////////

unsigned long SoundRecord::BytesPerSample(unsigned long bitsPerSample)
{
	unsigned long bytesPerSample = (bitsPerSample+7)/8;
	if(bytesPerSample == 3) bytesPerSample= 4;
	return bytesPerSample;
}

unsigned long SoundRecord::BufferSize(unsigned long bitsPerSample,
                                      unsigned long nSamples,
                                      unsigned long nChannels)
{
	return nSamples * nChannels * BytesPerSample(bitsPerSample);
}

void* SoundRecord::Doubles2Ints(const double* rawdoubles_in,
                                void*         rawbytes_out,
                                unsigned long bitsPerSample,
                                unsigned long nSamples,
                                unsigned long nChannels,
                                unsigned long byteStrideSample2Sample,
                                unsigned long byteStrideChannel2Channel)
{
	unsigned long bytesPerSample = BytesPerSample(bitsPerSample);
	if(bytesPerSample == 3) bytesPerSample= 4;
	
	double maxval = 1.0;
	for(unsigned long z = 0; z < bitsPerSample-1; z++) maxval *= 2.0;
	maxval -= 1.0;
	// TODO: this seems reasonable, but in the mex file it caused massive clipping until amplitude was reduced to 0.6 or so: check with oscilloscope whether this is still the case
	
	if(byteStrideSample2Sample == 0) {
		if(byteStrideChannel2Channel == sizeof(double)) byteStrideSample2Sample = sizeof(double) * nChannels;
		else byteStrideSample2Sample = sizeof(double);
	}
	if(byteStrideChannel2Channel == 0) {
		if(byteStrideSample2Sample == sizeof(double) * nChannels) byteStrideChannel2Channel = sizeof(double);
		else byteStrideChannel2Channel = byteStrideSample2Sample * nSamples;
	}
	
	unsigned long iSample, iChannel;
	const char* frame = (const char*)rawdoubles_in; 
	const char* in = frame;
	char* out = (char *)rawbytes_out;
	
	//printf("in = 0x%08lx\n", (unsigned long)in);
	//printf("out = 0x%08lx\n", (unsigned long)out);
	//printf("bitsPerSample = %lu\n", (unsigned long)bitsPerSample);
	//printf("bytesPerSample = %lu\n", (unsigned long)bytesPerSample);
	//printf("nSamples = %lu\n", (unsigned long)nSamples);
	//printf("nChannels = %lu\n", (unsigned long)nChannels);
	//printf("byteStrideSample2Sample = %lu\n", (unsigned long)byteStrideSample2Sample);
	//printf("byteStrideChannel2Channel = %lu\n", (unsigned long)byteStrideChannel2Channel);
	
	for(iSample=0; iSample<nSamples; iSample++, frame+=byteStrideSample2Sample) {
		for(iChannel=0, in=frame; iChannel<nChannels; iChannel++, in+=byteStrideChannel2Channel, out+=bytesPerSample) {
			double x = *(double*)in;
			x = ((x < -1.0) ? -1.0 : (x > +1.0) ? +1.0 : x);
			long val = (long)(0.5 + x * maxval);
			switch(bytesPerSample) {
				case 1: *(char*)out  = (char)val;  break;
				case 2: *(short*)out = (short)val; break;
				case 4: *(long*)out  = (long)val;  break;
			}
		}
	}
	//printf("in = 0x%08lx\n", (unsigned long)in);
	//printf("out = 0x%08lx\n", (unsigned long)out);
	return rawbytes_out;
}

/////////////////////////////////////////////////////////////////////////

#include <list>

std::list<SoundRecord *> gSoundRecords;

bool SoundRecord::IsListed(SoundRecord* p)
{
	std::list<SoundRecord*>::iterator it;
	for( it = gSoundRecords.begin(); it != gSoundRecords.end(); ++it ) {
		if(*it == p) return true;
	}
	return false;
}
void SoundRecord::AddToGlobalList(SoundRecord*p)
{
	if(!IsListed(p)) gSoundRecords.push_back(p);
}

void SoundRecord::RemoveFromGlobalList(SoundRecord*p)
{
	gSoundRecords.remove(p);
}

void SoundRecord::DeleteAllInGlobalList(void)
{
	while(!gSoundRecords.empty()) {
		SoundRecord *p = gSoundRecords.back();
		gSoundRecords.pop_back();
		delete p;
	}
}

/////////////////////////////////////////////////////////////////////////

