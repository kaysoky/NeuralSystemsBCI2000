/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#define HEADER_SIZE             6
#define DISPLAY_CHANNELS        16
#define DISPLAY_SAMPLES         768

#define DISPLAYDELTA_SAMPLE     DISPLAY_SAMPLES/2
#define DISPLAYDELTA_CHANNELS   4

#define MAX_CHANNELS            128

#define BUFFER_SIZE             100000                  // multiply of 4

#define STATE_IRI               0
#define STATE_GETTARGET         1
#define STATE_HIT               18
#define STATE_MISS              19
#define STATE_ITI               20
#define STATE_BASELINE          24

#define REFTYPE_NORMAL          1
#define REFTYPE_LGLAP           2
#define REFTYPE_CAR             3

#define SETSTATETYPE_BOOLEANOR  1
#define SETSTATETYPE_BOOLEANAND 2
#define SETSTATETYPE_JUSTSET    3

typedef struct {
        AnsiString      name;
        int             chanreftype;
        int             displayposition;
        } CHANNELPROPERTY;

typedef struct {
        __int16         rawstate;
        __int16         state;
        bool            artifact;
        } STATEDESC;

typedef struct
{
        int     headerlength;
        int     statevectorlength;
        int     sample_freq;
        int     channels;
} EEGFILEINFO;



