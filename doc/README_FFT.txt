Enabling FFT support in BCI2000

The BCI2000 FFT Filter makes use of the free FFTW3 library by Mateo Frigo and Steven G. Johnson.
This is the fastest and most reliable FFT implementation available today, and also used internally by Matlab.

For legal reasons, the FFTW3 library cannot part of the BCI2000 distribution.
However, it is perfectly legal to _use_ it with BCI2000 provided that you obtain your own copy of FFTW3.
To obtain a copy, and use it with BCI2000, please perform the following steps:

1) Go to the FFTW website at http://www.fftw.org/.

2) Locate and download the 3.01 version of the pre-compiled FFTW DLL -- as of Aug 2006, it is located at
   ftp://ftp.fftw.org/pub/fftw/fftw-3.0.1-w32-pl1.zip.
   The archive should contain a file called FFTW3.DLL.
   
3) In order to make the FFTW library available to BCI2000, put FFTW3.DLL into your BCI2000 installation's prog directory where all BCI2000 executables are found.

