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
        ULONG   sample_number;                  // samples in this run
        ULONG   sample_number_run[999];         // samples in a particular run
        ULONG   sample_number_total;            // samples in this dataset
        int     buffer_size;
        char    filename[256];
        bool    initialized, initializedtotal;
        int     channels;
        int     headerlength;
        int     statevectorlength;
        int     sample_freq;
        void    get_next_string(char *buf, int *start_idx, char *dest);
        void    InvalidateBuffer();
        void    CalculateSampleNumber();
public:		// User declarations
        BCI2000DATA::BCI2000DATA();
        BCI2000DATA::~BCI2000DATA();
        int     Initialize(char *filename, int);
        int     GetFirstRunNumber();
        int     GetLastRunNumber();
        int     SetRun(int);
        int     GetHeaderLength();
        int     GetStateVectorLength();
        int     GetNumChannels();
        int     GetSampleFrequency();
        int     ReadHeader();
        ULONG   GetNumSamples();
        bool    Initialized();
        PARAMLIST   *GetParamListPtr();
        STATELIST   *GetStateListPtr();
        STATEVECTOR *GetStateVectorPtr();
        short   ReadValue(int channel, ULONG sample);
        short   ReadValue(int channel, ULONG sample, int run);
        void    ReadStateVector(ULONG sample);
        void    ReadStateVector(ULONG sample, int run);
        // the following are functions that make the underlying file structure transparent to the programmer
        // they all look at the whole dataset as one contigous block, rather than different files
        ULONG   GetNumSamplesTotal();
        bool    InitializedTotal();
        int     InitializeTotal(char *new_filename, int buf_size);
        int     DetermineRunNumber(ULONG sample);
};

#endif
