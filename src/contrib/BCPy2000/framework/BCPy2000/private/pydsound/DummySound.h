#include "SoundRecord.h"

class DummySound
: public SoundRecord
{
	public:
		DummySound();
		~DummySound();

		bool Initialize(const double* data,
		                unsigned long samplesPerSecond,
		                unsigned long bitsPerSample,
		                unsigned long nSamples,
		                unsigned long nChannels,
		                unsigned long byteStrideSample2Sample,
		                unsigned long byteStrideChannel2Channel
		               );
		bool IsPlaying(void);
		bool Play(int nTimes=1);
		bool Stop(void);
		bool SetSpeed(double val=1.0);	
		bool SetVolume(double val=1.0);	
		bool SetPan(double val=0.0);	
		void Sleep(double msec);		

	private:
		bool playing;
};
