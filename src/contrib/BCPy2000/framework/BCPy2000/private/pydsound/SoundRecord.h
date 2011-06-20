class SoundRecord;

class SoundRecord
{
	public:
		// static functions: management of the global list
		static  bool IsListed(SoundRecord*p);
		static  void AddToGlobalList(SoundRecord*p);
		static  void RemoveFromGlobalList(SoundRecord*p);
		static  void DeleteAllInGlobalList(void);

		// static functions: general-purpose tools
		static  unsigned long BytesPerSample(unsigned long bitsPerSample);
		static  unsigned long BufferSize(unsigned long bitsPerSample,
		                                 unsigned long nSamples,
		                                 unsigned long nChannels);
		static  void* Doubles2Ints(const double* rawdoubles_in,
		                           void*         rawbytes_out,
		                           unsigned long bitsPerSample,
		                           unsigned long nSamples,
		                           unsigned long nChannels,
		                           unsigned long byteStrideSample2Sample,
		                           unsigned long byteStrideChannel2Channel);
		
		// object methods
		SoundRecord();
		// ...virtual
		virtual ~SoundRecord();
		virtual void InitPreplayLPT( int val=-1, int port=0);
		virtual void InitPostplayLPT(int val=-1, int port=0);
		virtual void Preplay(void);
		virtual void Postplay(void);
		virtual bool Check(double msecToSleepFirst=0.0);
		
		// ...pure virtual
		virtual bool Initialize(const double* data,
		                        unsigned long samplesPerSecond,
		                        unsigned long bitsPerSample,
		                        unsigned long nSamples,
		                        unsigned long nChannels,
		                        unsigned long byteStrideSample2Sample,
		                        unsigned long byteStrideChannel2Channel) = 0;
		virtual bool IsPlaying(void) = 0;
		virtual bool Play(int nTimes=1) = 0;
		virtual bool Stop(void) = 0;
		virtual bool SetSpeed(double val=1.0) = 0;	
		virtual bool SetVolume(double val=1.0) = 0;	
		virtual bool SetPan(double val=0.0) = 0;	
		virtual void Sleep(double msec) = 0;
	
	
	private:
		int lpt_preplay_port;
		int lpt_preplay_val;
		int lpt_postplay_port;
		int lpt_postplay_val;
		bool postplayPending;
};
