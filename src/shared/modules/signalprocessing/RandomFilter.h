////////////////////////////////////////////////////////////////////////////////
//  $Id$
//  Author:      juergen.mellinger@uni-tuebingen.de
//  Description: A filter that returns zero-mean white noise multiplied by the
//               input signal's value.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef RANDOM_FILTER_H
#define RANDOM_FILTER_H

#include "GenericFilter.h"
#include "RandomGenerator.h"

class RandomFilter : public GenericFilter
{
 public:
   RandomFilter();
   ~RandomFilter();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal&, GenericSignal& );

 private:
   RandomGenerator mRandomGenerator;
   float GetRandomUniform();
   float ( RandomFilter::*mpGetRandom )();
   enum
   {
     none,
     uniform,
   } RandomType;
};
#endif // RANDOM_FILTER_H
