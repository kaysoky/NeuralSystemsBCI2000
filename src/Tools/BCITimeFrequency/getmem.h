/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//*************************************************************************
// MEM is the Maximum Entropy Method for Autoregressive
//     spectral analysis adapted from Press et. al.
//     Numerical Recipes in C (chapter 13 )
//
//*************************************************************************

#define MAXCOF   128
#define MAXDATA 32768
#define NLINES  8196
#define MAXBINS 2048

class MEM
{
 private:
      int trend;            // detrend data ?
      int parms;            // newness of parameters- 0 = new
      int hz;               // sample rate
      int sp_lth;           // length of data vector
      int model_order;      // model order
      int f_width;          // bandwidth
      int n_bins;           // number of bins
      int n_bands;          // number of spectral bands
      float cof[MAXCOF];    // array of ar coefficients
      float pm;             // power of driving variance
      float data[MAXDATA];  // data vector
      float start;          // start of spectrum in % Nyquist rate
      float stop;           // end of spectrum in % Nyquist rate
      float delta;          // resolution in % Nyquist rate
      float wpr[MAXBINS];   // cosine lookup table
      float wpi[MAXBINS];   // sine lookup table
      float pwr[MAXBINS];   // power spectrum
      int memcof();
      int init_elv();
      int evlmem();
      void dtrnd();
 public:
      float bandwidth;
      float bstart;
      MEM( void );
      void setStart( float );
      void setStop( float );
      void setDelta( float );
      void setHz( int );
      void setModelOrder( int );
      void setBandWidth( float );
      void setTrend( int );
      int setData( int, float * );     // pass raw data to MEM
      int get_mem( void );             // do computations
      int get_pwr( float * );          // fill array with data- returns # bins
} ;



