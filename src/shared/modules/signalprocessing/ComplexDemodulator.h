////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that computes the squared amplitudes for a small number
//              of bands.
//              Its operation is roughly equivalent to a short-term fourier
//              transform followed by demodulation for selected frequency bins.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef COMPLEX_DEMODULATOR_H
#define COMPLEX_DEMODULATOR_H

#include <vector>
#include <complex>

#include "GenericFilter.h"

class ComplexDemodulator : public GenericFilter
{
 public:
   ComplexDemodulator();
   ~ComplexDemodulator();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal&, GenericSignal& );

 private:
   std::vector<std::vector<std::complex<double> > >  mCoefficients;
   std::vector<std::vector<double> >                 mSignalBuffer;
};
#endif // COMPLEX_DEMODULATOR_H
