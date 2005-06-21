#define MAXORDER  512
#define MAXCHANS   32
#define MAXDATA   512

class FIR
{
private:
     	float hc[MAXORDER];
        float coffs[MAXCHANS][MAXORDER];
        int order;
        int chans;
public:
        FIR( void );
        ~FIR(void );
        void fircof( double, double, double );
        void setFIR( int, int, float * );
        void convolve( int, int, float *, float * );
        float rms( int, float * );
        float mean( int, float * );
        float max( int, float * );
} ;
