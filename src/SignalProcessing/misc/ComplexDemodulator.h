////////////////////////////////////////////////////////////////////////////////
// $Id$
// File:        ComplexDemodulator.h
// Date:        Jan 2, 2006
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that computes the squared amplitudes for a small number
//              of bands.
//              Its operation is roughly equivalent to a short-term fourier
//              transform followed by demodulation for selected frequency bins.
// $Log$
// Revision 1.1  2006/01/13 15:04:46  mellinger
// Initial version.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef ComplexDemodulatorH
#define ComplexDemodulatorH

#include <vector>
#include <complex>

#include "UGenericFilter.h"

class ComplexDemodulator : public GenericFilter
{
 public:
   ComplexDemodulator();
   ~ComplexDemodulator();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize2( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal*, GenericSignal* );

 private:
   std::vector<std::vector<std::complex<double> > >  mCoefficients;
   std::vector<std::vector<double> >                 mSignalBuffer;
};
#endif // ComplexDemodulatorH
