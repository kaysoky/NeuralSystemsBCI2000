//---------------------------------------------------------------------------

#ifndef UBCI2000DataH
#define UBCI2000DataH
//---------------------------------------------------------------------------

#include "UParameter.h"
#include "UState.h"

#define BCI2000ERR_NOERR                0
#define BCI2000ERR_FILENOTFOUND         1
#define BCI2000ERR_MALFORMEDHEADER      2
#define BCI2000ERR_NOBUFMEM             3
#define BCI2000ERR_CHSINCONSISTENT      4

class BCI2000DATA
{
private: 	// User declarations
        PARAMLIST       paramlist;
        STATELIST       statelist;
        STATEVECTOR     *statevector;
        __int16         *buf_mem;
        long    buf_mem_start;
        unsigned long   sample_number;                  // samples in this run
        unsigned long   sample_number_run[999];         // samples in a particular run
        unsigned long   sample_number_total;            // samples in this dataset
        int     buffer_size;
        char    filename[256];
        bool    initialized, initializedtotal;
        int     channels;
        int     headerlength;
        int     statevectorlength;
        int     sample_freq;
        void    get_next_string(const char *buf, int *start_idx, char *dest) const;
        void    InvalidateBuffer();
        void    CalculateSampleNumber();
public:		// User declarations
        BCI2000DATA::BCI2000DATA();
        BCI2000DATA::~BCI2000DATA();
        int     Initialize(const char *filename, int);
        int     GetFirstRunNumber() const;
        int     GetLastRunNumber() const;
        int     SetRun(int);
        int     GetHeaderLength() const;
        int     GetStateVectorLength() const;
        int     GetNumChannels() const;
        int     GetSampleFrequency() const;
        int     ReadHeader();
        unsigned long   GetNumSamples();
        bool    Initialized() const;
        const PARAMLIST   *GetParamListPtr() const;
        const STATELIST   *GetStateListPtr() const;
        const STATEVECTOR *GetStateVectorPtr() const;
        short   ReadValue(int channel, unsigned long sample);
        short   ReadValue(int channel, unsigned long sample, int run);
        void    ReadStateVector(unsigned long sample);
        void    ReadStateVector(unsigned long sample, int run);
        // the following are functions that make the underlying file structure transparent to the programmer
        // they all look at the whole dataset as one contigous block, rather than different files
        unsigned long   GetNumSamplesTotal() const;
        bool    InitializedTotal() const;
        int     InitializeTotal(const char *new_filename, int buf_size);
        int     DetermineRunNumber(unsigned long sample);
};

#endif
