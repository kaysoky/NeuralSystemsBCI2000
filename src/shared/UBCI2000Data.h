#ifndef UBCI2000DataH
#define UBCI2000DataH

#include "UParameter.h"
#include "UState.h"

#include <vector>

#define BCI2000ERR_NOERR                0
#define BCI2000ERR_FILENOTFOUND         1
#define BCI2000ERR_MALFORMEDHEADER      2
#define BCI2000ERR_NOBUFMEM             3
#define BCI2000ERR_CHSINCONSISTENT      4

class BCI2000DATA
{
  private:
        PARAMLIST                  mParamlist;
        STATELIST                  mStatelist;
        STATEVECTOR*               mpStatevector;
        __int16*                   mpBuf_mem;
        long                       mBuf_mem_start;
        unsigned long              mSample_number;       // samples in this run
        std::vector<unsigned long> mSample_number_run;   // samples in a particular run
        unsigned long              mSample_number_total; // samples in this dataset
        int                        mBuffer_size;
        std::string                mFilename;
        bool                       mInitialized,
                                   mInitializedtotal;
        int                        mChannels,
                                   mHeaderlength,
                                   mStatevectorlength,
                                   mSample_freq;
        std::vector<float>         mSourceOffsets,
                                   mSourceGains;

        //void    get_next_string(const char *buf, int *start_idx, char *dest) const;
        void    InvalidateBuffer();
        void    CalculateSampleNumber();
        void    InitializeInstance();
public:		// User declarations
        enum
        {
          defaultBufSize = 50000,
        };
        BCI2000DATA();
        ~BCI2000DATA();
        int                Initialize( const char* filename, int bufferSize = defaultBufSize );
        int                GetFirstRunNumber() const;
        int                GetLastRunNumber() const;
        int                SetRun( int );
        int                GetHeaderLength() const;
        int                GetStateVectorLength() const;
        int                GetNumChannels() const;
        int                GetSampleFrequency() const;
        int                ReadHeader();
        unsigned long      GetNumSamples();
        bool               Initialized() const;
        const PARAMLIST*   GetParamListPtr() const;
        const STATELIST*   GetStateListPtr() const;
        const STATEVECTOR* GetStateVectorPtr() const;
        float              Value( int channel, unsigned long sample ) /* const */;
        short              ReadValue( int channel, unsigned long sample );
        short              ReadValue( int channel, unsigned long sample, int run );
        void               ReadStateVector( unsigned long sample );
        void               ReadStateVector( unsigned long sample, int run );
        // the following are functions that make the underlying file structure transparent to the programmer
        // they all look at the whole dataset as one contigous block, rather than different files
        unsigned long      GetNumSamplesTotal() const;
        bool               InitializedTotal() const;
        int                InitializeTotal( const char* new_filename, int buf_size = defaultBufSize );
        int                DetermineRunNumber( unsigned long sample );
};

#endif
